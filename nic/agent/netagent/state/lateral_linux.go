// +build ignore

package state

import (
	"context"
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
	inbandIntf           = "bond0"
	oobIntf              = "oob_mnic0"
	refreshDuration      = time.Duration(time.Minute * 1)
	arpResolutionTimeout = time.Duration(time.Second * 3)
)

// CreateLateralNetAgentObjects creates the backing objects to ensure that telemetry works.
// It runs an arp client in the background to ensure that appropriate destEP is created. Owner is used to track
// the originator object of the request. Either FlowExportPolicy or MirrorSession
// TODO clean this up. Currently this is in NetAgent as there is no control plane which does this.
func (na *Nagent) CreateLateralNetAgentObjects(owner string, mgmtIP, destIP string, tunnelOp bool) error {
	log.Infof("Creating Lateral NetAgent objects. Owner: %v MgmtIP: %v DestIP: %v TunnelOp: %v", owner, mgmtIP, destIP, tunnelOp)
	var (
		ep                    *netproto.Endpoint
		err                   error
		objectName            = fmt.Sprintf("_internal-%s", destIP)
		tunnelCompositeKey    = fmt.Sprintf("tunnel|%s", objectName)
		collectorCompositeKey = fmt.Sprintf("collector|%s", objectName)
	)

	collectorKnown, tunnelKnown := na.reconcileLateralObjects(owner, objectName, mgmtIP, destIP, tunnelOp)
	if collectorKnown && tunnelKnown {
		log.Info("Collector and Tunnel already known. Nothing to do here.")
		log.Infof("Lateral object DB pre refcount increment: %v", na.LateralDB)

		// Add refcount here
		if tunnelOp {
			na.LateralDB[tunnelCompositeKey] = append(na.LateralDB[tunnelCompositeKey], owner)
		}

		na.LateralDB[collectorCompositeKey] = append(na.LateralDB[collectorCompositeKey], owner)

		// Dedup here to handle idempotent calls to lateral methods.
		na.dedupReferences(collectorCompositeKey)
		na.dedupReferences(tunnelCompositeKey)

		log.Infof("Lateral object DB post refcount increment: %v", na.LateralDB)
		return nil
	}

	log.Infof("One or more lateral object creation needed. CreateEP: %v, CreateTunnel: %v", !collectorKnown, !tunnelKnown)

	if !collectorKnown {
		ep, err = na.generateLateralEP(objectName, destIP, mgmtIP)
		if err != nil {
			return err
		}
	}

	log.Infof("Lateral object DB pre refcount increment: %v", na.LateralDB)

	// Add refcount here
	if tunnelOp {
		na.LateralDB[tunnelCompositeKey] = append(na.LateralDB[tunnelCompositeKey], owner)
	}

	na.LateralDB[collectorCompositeKey] = append(na.LateralDB[collectorCompositeKey], owner)

	// Dedup here to handle idempotent calls to lateral methods.
	na.dedupReferences(collectorCompositeKey)
	na.dedupReferences(tunnelCompositeKey)

	log.Infof("Lateral object DB post refcount increment: %v", na.LateralDB)

	if !collectorKnown && ep != nil {
		// Test for already existing collector EP
		if _, err := na.FindEndpoint(ep.ObjectMeta); err == nil {
			log.Infof("Lateral EP: %v already present. Skipping...", ep.Name)
		} else {
			log.Infof("Creating internal endpoint %v", ep.ObjectMeta)
			if err := na.CreateEndpoint(ep); err != nil {
				log.Errorf("Failed to create TEP EP. Err: %v", err)
				return err
			}
		}
	}

	if tunnelOp && !tunnelKnown {
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
				Dst:         destIP,
			},
		}

		if _, err := na.FindTunnel(tun.ObjectMeta); err == nil {
			log.Infof("Lateral Tunnel: %v already present. Skipping...", tun.Name)
		} else {
			log.Infof("Creating internal tunnel %v for mirror session %v", tun.ObjectMeta, owner)
			if err := na.CreateTunnel(tun); err != nil {
				log.Errorf("Failed to create tunnel. Err: %v", err)
				return err
			}
		}
	}
	return nil
}

// DeleteLateralNetAgentObjects deletes the backing objects to ensure that telemetry works.
// TODO clean this up. Currently this is in NetAgent as there is no control plane which does this.
func (na *Nagent) DeleteLateralNetAgentObjects(owner string, mgmtIP, destIP string, tunnelOp bool) error {
	log.Infof("Deleting Lateral NetAgent objects. Owner: %v MgmtIP: %v DestIP: %v TunnelOp: %v", owner, mgmtIP, destIP, tunnelOp)

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIP),
	}

	tunnelCompositeKey := fmt.Sprintf("tunnel|%s", lateralObjMeta.Name)
	collectorCompositeKey := fmt.Sprintf("collector|%s", lateralObjMeta.Name)

	log.Infof("Lateral object DB pre refcount decrement: %v", na.LateralDB)

	// Remove Dependency
	if tunnelOp {
		for idx, dep := range na.LateralDB[tunnelCompositeKey] {
			if dep == owner {
				na.LateralDB[tunnelCompositeKey] = append(na.LateralDB[tunnelCompositeKey][:idx], na.LateralDB[tunnelCompositeKey][idx+1:]...)
			}
		}

	}

	for idx, dep := range na.LateralDB[collectorCompositeKey] {
		if dep == owner {
			na.LateralDB[collectorCompositeKey] = append(na.LateralDB[collectorCompositeKey][:idx], na.LateralDB[collectorCompositeKey][idx+1:]...)
		}
	}

	log.Infof("Lateral object DB post refcount decrement: %v", na.LateralDB)

	// Check for pending dependencies
	if tunnelOp {
		if len(na.LateralDB[tunnelCompositeKey]) == 0 {
			// Find Tunnel
			if tun, err := na.FindTunnel(lateralObjMeta); err == nil {
				log.Infof("Deleting lateral tunnel %v", lateralObjMeta.Name)
				if err := na.DeleteTunnel(tun.Tenant, tun.Namespace, tun.Name); err != nil {
					log.Errorf("Failed to delete lateral tunnel. Owner: %v | Err: %v", owner, err)
					return fmt.Errorf("failed to delete lateral tunnel. Owner: %v | Err: %v", owner, err)
				}
			}
			delete(na.LateralDB, tunnelCompositeKey)
		} else {
			log.Infof("Disallowing deletion of lateral tunnels. Pending refs: %v", na.LateralDB[tunnelCompositeKey])
		}
	}

	// Check for pending dependencies
	if len(na.LateralDB[collectorCompositeKey]) == 0 {
		// Find EP and delete only if found
		if lateralEP, err := na.FindEndpoint(lateralObjMeta); err == nil {
			log.Infof("Deleting lateral endpoint %v", lateralObjMeta.Name)
			if err := na.DeleteEndpoint(lateralEP.Tenant, lateralEP.Namespace, lateralEP.Name); err != nil {
				log.Errorf("Failed to delete lateral ep. Owner: %v | Err: %v", owner, err)
				return fmt.Errorf("failed to delete lateral ep. Owner: %v | Err: %v", owner, err)
			}
			log.Infof("Cancelling ARP Refresh loop for destIP: %v", destIP)
			cancel, ok := na.ArpCache.DoneCache[destIP]
			if ok {
				cancel()
			}
			delete(na.LateralDB, collectorCompositeKey)
		}
		return nil
	}

	log.Infof("Disallowing deletion of lateral endpoints. Pending refs: %v", na.LateralDB[lateralObjMeta.Name])
	return nil
}

func (na *Nagent) startRefreshLoop(refreshCtx context.Context, IP net.IP) {
	log.Infof("Starting ARP refresh loop for %v", IP)

	defer func() {
		if err := recover(); err != nil {
			log.Errorf("Refresh control loop exited. Err: %v", err)
			log.Infof("Restarting refresh loop...")
			go na.startRefreshLoop(refreshCtx, IP)
		}
	}()

	ticker := time.NewTicker(refreshDuration)
	go func(refreshCtx context.Context, IP net.IP) {
		mac := na.resolveWithDeadline(refreshCtx, IP)
		log.Infof("Resolved MAC 1st Tick: %s", mac)
		for {
			select {
			case <-ticker.C:
				na.resolveWithDeadline(refreshCtx, IP)
			case <-refreshCtx.Done():
				log.Infof("Cancelling ARP refresh loop for %v ", IP)
				return
			}
		}
	}(refreshCtx, IP)
}

func (na *Nagent) resolveWithDeadline(ctx context.Context, IP net.IP) string {
	resolveCtx, cancel := context.WithCancel(ctx)
	defer cancel()
	arpChan := make(chan string, 1)

	go func(arpChan chan string, IP net.IP) {
		mac, err := na.ArpClient.Resolve(IP)
		if err != nil {
			log.Errorf("Failed to resolve MAC for %v", err)
		}
		// Populate the ARPCache.
		na.ArpCache.Cache.LoadOrStore(IP.String(), mac.String())
		arpChan <- mac.String()
	}(arpChan, IP)

	select {
	case <-time.After(arpResolutionTimeout):
		cancel()
		return resolveCtx.Err().Error()
	case mac := <-arpChan:
		return mac
	}
}

func (na *Nagent) reconcileLateralObjects(owner string, lateralObjName, mgmtIP, destIP string, createTunnel bool) (epKnown, tunnelKnown bool) {
	// Find EP
	log.Infof("Testing for known collector with Name: %v MgmtIP: %v DestIP: %v, TunnelOp: %v", lateralObjName, mgmtIP, destIP, createTunnel)
	if createTunnel == true {
		var knownTunnel *netproto.Tunnel
		var knownCollector *netproto.Endpoint
		for _, t := range na.ListTunnel() {
			if t.Spec.Dst == destIP {
				knownTunnel = t
			}
		}
		// Find EP and add refcount
		for _, ep := range na.ListEndpoint() {
			for _, address := range ep.Spec.IPv4Addresses {
				epIP, _, _ := net.ParseCIDR(address)
				if epIP.String() == destIP {
					knownCollector = ep
					break
				}
			}
		}

		if knownTunnel != nil {
			// Found a precreated tunnel. Add refcount
			tunnelKnown = true
		}

		if knownCollector != nil {
			// Found a precreated tunnel. Add refcount
			epKnown = true
		}
		return
	}
	var knownCollector *netproto.Endpoint

	// Find EP and add refcount
	for _, ep := range na.ListEndpoint() {
		for _, address := range ep.Spec.IPv4Addresses {
			epIP, _, _ := net.ParseCIDR(address)
			if epIP.String() == destIP {
				knownCollector = ep
				break
			}
		}
	}
	if knownCollector != nil {
		// Found a precreated ep. Add refcount
		epKnown = true
		tunnelKnown = true // Mark tunnel as known here so that it doesn't get created
	}
	return
}

// dedupReferences does inplace dedup of reference slices
func (na *Nagent) dedupReferences(compositeKey string) {
	log.Infof("Lateral DB: %v pre dedup: %v", compositeKey, na.LateralDB[compositeKey])
	if len(na.LateralDB[compositeKey]) < 2 {
		return
	}
	marker := 0
	for idx := 1; idx < len(na.LateralDB[compositeKey]); idx++ {
		if na.LateralDB[compositeKey][marker] == na.LateralDB[compositeKey][idx] {
			continue
		}
		marker++
		na.LateralDB[compositeKey][marker] = na.LateralDB[compositeKey][idx]
	}
	na.LateralDB[compositeKey] = na.LateralDB[compositeKey][:marker+1]
}

func (na *Nagent) generateLateralEP(objectName, destIP, mgmtIP string) (*netproto.Endpoint, error) {
	var dMAC string
	epIP := destIP
	mgmtLink := getMgmtLink(mgmtIP)
	if mgmtLink == nil {
		log.Errorf("Failed to get the mgmt interface")
		return nil, nil
	}
	log.Infof("Management Link: %v", mgmtLink.Attrs().Name)
	mgmtIntf, _ := net.InterfaceByName(mgmtLink.Attrs().Name)
	log.Infof("Management Inft: %v", mgmtIntf.Name)

	client, err := arp.Dial(mgmtIntf)
	if err != nil {
		log.Errorf("Failed to initiate an ARP client. Err: %v", err)
		return nil, err
	}
	na.ArpClient = client

	//// Cache the handler for outer done. This needs to be called during the object deletes
	refreshCtx, done := context.WithCancel(context.Background())
	na.ArpCache.DoneCache[destIP] = done

	log.Infof("Resolving for: %s", destIP)
	addrs, err := netlink.AddrList(mgmtLink, netlink.FAMILY_V4)
	mgmtSubnet := addrs[0].IPNet

	// Check if in the same subnet
	if !mgmtSubnet.Contains(net.ParseIP(destIP)) {
		routes, err := netlink.RouteGet(net.ParseIP(destIP))
		if err != nil || len(routes) == 0 {
			log.Errorf("No routes found for the dest IP %s. Err: %v", destIP, err)
			return nil, fmt.Errorf("no routes found for %s", destIP)
		}

		// Pick the first route. Dest IP in not in the local subnet. Use GW IP as the destIP for ARP'ing
		if routes[0].Gw == nil {
			log.Errorf("Default gateway not configured for destIP %s.", destIP)
			return nil, fmt.Errorf("default gateway not configured for destIP %s", destIP)
		}
		gwIP := routes[0].Gw.String()
		log.Infof("Dest IP %s not in management subnet %v. Using Gateway IP: %v  as destIP for ARPing...", destIP, mgmtSubnet, gwIP)
		// Update destIP to be GwIP
		destIP = gwIP
	}

	// Set the correct dIP
	dIP := net.ParseIP(destIP)

	log.Infof("Starting refresh loop for %s", dIP.String())

	go na.startRefreshLoop(refreshCtx, dIP)

	time.Sleep(time.Second * 3)

	for i := 0; i < 3; i++ {
		d, ok := na.ArpCache.Cache.Load(destIP)
		if !ok {
			continue
		}
		dMAC = d.(string)
	}

	// Bail out if the arp resolution fails.
	if len(dMAC) == 0 {
		log.Errorf("Failed to find resolve MAC for %s. Retrying...", destIP)
		time.Sleep(time.Second * 3)

		for i := 0; i < 10; i++ {
			d, ok := na.ArpCache.Cache.Load(destIP)
			if !ok {
				continue
			}
			dMAC = d.(string)
			time.Sleep(time.Second * 1)
		}

		if done != nil {
			done() // cancel arp
		}

		log.Errorf("failed to resolve MAC despite retries for %s", destIP)
		return nil, fmt.Errorf("failed to resolve mac address for %s", destIP)
	}

	ep := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      objectName,
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   types.InternalUntaggedNetwork,
			NodeUUID:      "REMOTE",
			IPv4Addresses: []string{fmt.Sprintf("%s/32", epIP)},
			MacAddress:    dMAC,
		},
	}

	return ep, nil
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
