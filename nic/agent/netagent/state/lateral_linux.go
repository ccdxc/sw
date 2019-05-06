package state

import (
	"fmt"
	"net"
	"time"

	"github.com/pensando/sw/nic/agent/netagent/state/types"

	"github.com/mdlayher/arp"
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	inbandIntf      = "bond0"
	oobIntf         = "oob_mnic0"
	refreshDuration = time.Duration(time.Second * 1)
	timeout         = time.Duration(time.Millisecond * 500)
)

// CreateLateralNetAgentObjects creates the backing objects to ensure that telemetry works.
// It runs an arp client in the background to ensure that appropriate destEP is created.
// TODO clean this up. Currently this is in NetAgent as there is no control plane which does this.
func (na *Nagent) CreateLateralNetAgentObjects(mgmtIP, destIP string, tunnelOp bool) error {
	var epIfType, epIf string
	var objectName = fmt.Sprintf("_internal-%s", destIP)

	var sMAC, dMAC string
	epIP := destIP
	mgmtLink := getMgmtLink(mgmtIP)
	if mgmtLink == nil {
		log.Errorf("Failed to get the mgmt interface")
		return nil
	}
	log.Infof("Management Link: %v", mgmtLink.Attrs().Name)
	mgmtIntf, _ := net.InterfaceByName(mgmtLink.Attrs().Name)
	log.Infof("Management Inft: %v", mgmtIntf.Name)

	client, err := arp.Dial(mgmtIntf)
	if err != nil {
		log.Errorf("Failed to initiate an ARP client. Err: %v", err)
		return err
	}
	na.ArpClient = client

	log.Infof("Resolving for: %s", destIP)
	routes, err := netlink.RouteGet(net.ParseIP(destIP))
	if err != nil || len(routes) == 0 {
		log.Errorf("No routes found for the dest IP %s. Err: %v", destIP, err)
		err = fmt.Errorf("no routes found for the dest IP %s. Err: %v", destIP, err)
	}

	// Pick the first route. Dest IP in not in the local subnet. Use GW IP as the destIP for ARP'ing
	if routes[0].Gw != nil {
		destIP = routes[0].Gw.String()
	}

	// Set the correct dIP
	dIP := net.ParseIP(destIP)

	log.Infof("Starting refresh loop for %s", dIP.String())

	//// Cache the handler for outer done. This needs to be called during the object deletes
	//_, done := context.WithCancel(context.Background())
	//na.ArpCache.DoneCache[dIP.String()] = done
	go na.startRefreshLoop(dIP)

	time.Sleep(time.Second * 5)

	sMAC = mgmtLink.Attrs().HardwareAddr.String()
	for i := 0; i < 3; i++ {
		d, ok := na.ArpCache.Cache.Load(destIP)
		if !ok {
			continue
		}
		dMAC = d.(string)
	}

	// Bail out if the arp resolution fails.
	if len(dMAC) == 0 {
		log.Errorf("Failed to find resolve MAC for %s", destIP)
		return fmt.Errorf("failed to find resolve MAC for %s", destIP)
	}

	if na.isCollectorKnown(objectName, mgmtIP, epIP, dMAC, sMAC, tunnelOp) {
		log.Infof("The collector %s is known.", epIP)
		return nil
	}

	nw := &netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      objectName,
		},
		Spec: netproto.NetworkSpec{
			VlanID:      types.UntaggedVLAN,           // Untagged
			IPv4Subnet:  fmt.Sprintf("%s/32", mgmtIP), // TODO Get the actual mgmt intf's subnet
			IPv4Gateway: destIP,
			RouterMAC:   sMAC,
		},
	}

	if mgmtLink.Attrs().Name == types.OOBManagementInterface {
		epIf = types.ManagementUplink
		epIfType = "uplink"
	}

	ep := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      objectName,
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   objectName,
			NodeUUID:      "REMOTE",
			IPv4Address:   fmt.Sprintf("%s/32", epIP),
			MacAddress:    dMAC,
			InterfaceType: epIfType,
			Interface:     epIf,
		},
	}

	tun := &netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      objectName,
		},
		Spec: netproto.TunnelSpec{
			Type:        "GRE",
			AdminStatus: "UP",
			Src:         mgmtIP,
			Dst:         epIP,
		},
	}
	log.Infof("Creating internal network")

	if err := na.CreateNetwork(nw); err != nil {
		log.Errorf("Failed to create an untagged network. Err: %v", err)
		return err
	}

	log.Infof("Creating internal endpoint")
	if _, err := na.CreateEndpoint(ep); err != nil {
		log.Errorf("Failed to create TEP EP. Err: %v", err)
		return err
	}

	if tunnelOp {
		log.Infof("Creating internal tunnel")
		if err := na.CreateTunnel(tun); err != nil {
			log.Errorf("Failed to create tunnel. Err: %v", err)
			return err
		}
	}

	return nil
}

// DeleteLateralNetAgentObjects deletes the backing objects to ensure that telemetry works.
// TODO clean this up. Currently this is in NetAgent as there is no control plane which does this.
func (na *Nagent) DeleteLateralNetAgentObjects(mgmtIP, destIP string, tunnelOp bool) error {
	var objectName = fmt.Sprintf("_internal-%s", destIP)
	log.Infof("Deleting objects: %v", objectName)

	if tunnelOp {
		err := na.DeleteTunnel("default", "default", objectName)
		if err != nil {
			log.Errorf("Failed to delete the tunnel %v. Err: %v", objectName, err)
			return fmt.Errorf("failed to delete the tunnel %v. Err: %v", objectName, err)
		}
	}

	for _, ep := range na.ListEndpoint() {
		if ep.Spec.IPv4Address == fmt.Sprintf("%s/32", destIP) {
			// Lateral objects and not created by Venice
			if ep.Spec.NetworkName == objectName {
				if err := na.DeleteEndpoint("default", "default", objectName); err != nil {
					log.Errorf("Failed to delete the endpoint %v. Err: %v", objectName, err)
				}
				if err := na.DeleteNetwork("default", "default", objectName); err != nil {
					log.Errorf("Failed to delete the network %v. Err: %v", objectName, err)
				}
			}

		}
	}

	return nil
}
func getMgmtLink(mgmtIP string) (mgmtLink netlink.Link) {
	links, err := netlink.LinkList()
	if err != nil {
		log.Errorf("Failed to list the available links. Err: %v", err)
		return
	}

	for _, l := range links {
		addrs, _ := netlink.AddrList(l, netlink.FAMILY_V4)
		for _, a := range addrs {
			if a.IP.String() == mgmtIP {
				mgmtLink = l
				return
			}
		}
	}
	return
}

func (na *Nagent) startRefreshLoop(IP net.IP) {
	log.Infof("Starting ARP refresh loop for %v", IP)

	defer func() {
		if err := recover(); err != nil {
			log.Errorf("Refresh control loop exited. Err: %v", err)
			log.Infof("Restarting refresh loop...")
			go na.startRefreshLoop(IP)
		}
	}()

	log.Info("Done func cached")
	ticker := time.NewTicker(refreshDuration)

	for {
		select {
		case <-ticker.C:
			mac, err := na.ArpClient.Resolve(IP)
			if err != nil {
				log.Errorf("Failed to resolve MAC for %v", err)
			}
			log.Infof("FOUND MAC: %v", mac)
			// Populate the ARPCache.
			na.ArpCache.Cache.LoadOrStore(IP.String(), mac.String())
		}
	}
}

func (na *Nagent) isCollectorKnown(netagentCreatedEPName, mgmtIP, destIP, destMAC, srcMAC string, createTunnel bool) (known bool) {
	// Find EP
	for _, ep := range na.ListEndpoint() {
		if ep.Spec.IPv4Address == fmt.Sprintf("%s/32", destIP) {
			known = true
			// Collector known -> This can either be venice created or a agent created
			switch ep.Name {
			case netagentCreatedEPName:
				return
			default:
				// EP known to venice. Fetch the corresponding l2seg ID and trigger a network create with the same ID and trigger a L2Seg Update
				nwMeta := api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      ep.Spec.NetworkName,
				}
				net, err := na.FindNetwork(nwMeta)
				if err != nil {
					log.Errorf("Failed to lookup known network. Err: %v", err)
					known = false
					return
				}
				net.Spec.IPv4Subnet = fmt.Sprintf("%s/32", mgmtIP)
				net.Spec.IPv4Gateway = destIP
				net.Spec.RouterMAC = srcMAC

				err = na.UpdateNetwork(net)
				if err != nil {
					known = false
					log.Errorf("Failed to update the network for a venice known EP. Err: %v", err)
					return
				}

				tun := &netproto.Tunnel{
					TypeMeta: api.TypeMeta{Kind: "Tunnel"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    "default",
						Namespace: "default",
						Name:      netagentCreatedEPName,
					},
					Spec: netproto.TunnelSpec{
						Type:        "GRE",
						AdminStatus: "UP",
						Src:         mgmtIP,
						Dst:         destIP,
					},
				}

				// Create tunnel
				if createTunnel {
					log.Infof("Creating internal tunnel")
					if err := na.CreateTunnel(tun); err != nil {
						log.Errorf("Failed to create tunnel. Err: %v", err)
						known = false
						return
					}
				}
				known = true
			}
			return
		}
	}
	return
}
