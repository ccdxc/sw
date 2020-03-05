// +build iris
// +build linux

package iris

import (
	"context"
	"fmt"
	"net"
	"sort"
	"sync"
	"time"

	"github.com/mdlayher/arp"
	"github.com/pkg/errors"
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	refreshDuration      = time.Duration(time.Minute * 15)
	arpResolutionTimeout = time.Duration(time.Second * 3)
)

var lateralDB = map[string][]string{}
var doneCache = map[string]context.CancelFunc{}
var arpCache sync.Map

// ArpClient is the global arp client for netagent
var ArpClient *arp.Client

// MgmtLink is the global management link for netagent
var MgmtLink netlink.Link

// GwCache is the global cache for ip to their gateway (if needed)
var GwCache = map[string]string{}

// CreateLateralNetAgentObjects creates lateral objects for telmetry objects and does refcounting. This is temporary code till HAL subsumes ARP'ing for dest IPs
func CreateLateralNetAgentObjects(infraAPI types.InfraAPI, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, vrfID uint64, owner string, mgmtIP, destIP string, tunnelOp bool) error {
	log.Infof("Creating Lateral NetAgent objects. Owner: %v MgmtIP: %v DestIP: %v TunnelOp: %v", owner, mgmtIP, destIP, tunnelOp)
	var (
		ep                    *netproto.Endpoint
		err                   error
		objectName            string
		tunnelName            = fmt.Sprintf("_internal-%s", destIP)
		tunnelCompositeKey    = fmt.Sprintf("tunnel|%s", tunnelName)
		collectorCompositeKey string
	)

	collectorKnown, tunnelKnown := reconcileLateralObjects(infraAPI, owner, destIP, tunnelOp)
	if collectorKnown {
		var knownCollector *netproto.Endpoint
		eDat, err := infraAPI.List("Endpoint")
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Err: %v", types.ErrObjNotFound))
		}
		for _, o := range eDat {
			var endpoint netproto.Endpoint
			err := endpoint.Unmarshal(o)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
				continue
			}
			for _, address := range endpoint.Spec.IPv4Addresses {
				if address == destIP {
					knownCollector = &endpoint
					break
				}
			}
		}

		objectName = fmt.Sprintf("_internal-%s", knownCollector.Spec.MacAddress)
		collectorCompositeKey = fmt.Sprintf("collector|%s", objectName)
	}
	if collectorKnown && tunnelKnown {
		log.Info("Collector and Tunnel already known. Nothing to do here.")
		log.Infof("Lateral object DB pre refcount increment: %v", lateralDB)

		// Add refcount here
		if tunnelOp {
			lateralDB[tunnelCompositeKey] = append(lateralDB[tunnelCompositeKey], owner)
		}

		lateralDB[collectorCompositeKey] = append(lateralDB[collectorCompositeKey], owner)
		lateralDB[destIP] = append(lateralDB[destIP], owner)

		// Dedup here to handle idempotent calls to lateral methods.
		dedupReferences(collectorCompositeKey)
		dedupReferences(tunnelCompositeKey)
		dedupReferences(destIP)

		log.Infof("Lateral object DB post refcount increment: %v", lateralDB)
		return nil
	}

	log.Infof("One or more lateral object creation needed. CreateEP: %v, CreateTunnel: %v", !collectorKnown, !tunnelKnown)

	if !collectorKnown {
		ep, err = generateLateralEP(destIP)
		if err != nil {
			return err
		}
		gwIP := GwCache[destIP]
		dmac, ok := arpCache.Load(gwIP)
		if ok {
			log.Infof("Set the composite key name with dmac: %s", dmac)
			objectName = fmt.Sprintf("_internal-%s", dmac)
			collectorCompositeKey = fmt.Sprintf("collector|%s", objectName)
		}
	}

	log.Infof("Lateral object DB pre refcount increment: %v", lateralDB)

	// Add refcount here -- using this to keep the same ip ref count
	if tunnelOp {
		lateralDB[tunnelCompositeKey] = append(lateralDB[tunnelCompositeKey], owner)
	}

	lateralDB[collectorCompositeKey] = append(lateralDB[collectorCompositeKey], owner)
	lateralDB[destIP] = append(lateralDB[destIP], owner)

	// Dedup here to handle idempotent calls to lateral methods.
	dedupReferences(collectorCompositeKey)
	dedupReferences(tunnelCompositeKey)
	dedupReferences(destIP)

	log.Infof("Lateral object DB post refcount increment: %v", lateralDB)

	if !collectorKnown && ep != nil {
		// Test for already existing collector EP
		// FindEndpoint with matching Mac if found update else create
		var knownEp *netproto.Endpoint
		eDat, err := infraAPI.List("Endpoint")
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Err: %v", types.ErrObjNotFound))
		}
		for _, o := range eDat {
			var endpoint netproto.Endpoint
			err := endpoint.Unmarshal(o)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
				continue
			}
			if endpoint.Spec.MacAddress == ep.Spec.MacAddress {
				knownEp = &endpoint
				break
			}
		}
		if knownEp != nil {
			log.Infof("Updating internal endpoint %v", ep.ObjectMeta)
			ep := &netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    knownEp.ObjectMeta.Tenant,
					Namespace: knownEp.ObjectMeta.Namespace,
					Name:      knownEp.ObjectMeta.Name,
				},
				Spec: netproto.EndpointSpec{
					NetworkName:   knownEp.Spec.NetworkName,
					NodeUUID:      knownEp.Spec.NodeUUID,
					IPv4Addresses: knownEp.Spec.IPv4Addresses,
					MacAddress:    knownEp.Spec.MacAddress,
				},
			}
			ep.Spec.IPv4Addresses = append(ep.Spec.IPv4Addresses, destIP)
			if err = updateEndpointHandler(infraAPI, epClient, intfClient, *ep, vrfID, types.UntaggedCollVLAN); err != nil {
				log.Error(errors.Wrapf(types.ErrCollectorEPUpdateFailure, "MirrorSession: %s |  Err: %v", owner, err))
				return errors.Wrapf(types.ErrCollectorEPUpdateFailure, "MirrorSession: %s |  Err: %v", owner, err)
			}
		} else {
			log.Infof("Creating internal endpoint %v", ep.ObjectMeta)
			err := createEndpointHandler(infraAPI, epClient, intfClient, *ep, vrfID, types.UntaggedCollVLAN)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrCollectorEPCreateFailure, "MirrorSession: %s |  Err: %v", owner, err))
				return errors.Wrapf(types.ErrCollectorEPCreateFailure, "MirrorSession: %s |  Err: %v", owner, err)
			}
		}
	}

	if tunnelOp && !tunnelKnown {
		tun := netproto.Tunnel{
			TypeMeta: api.TypeMeta{Kind: "Tunnel"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      tunnelName,
			},
			Spec: netproto.TunnelSpec{
				Type:        "GRE",
				AdminStatus: "UP",
				Src:         mgmtIP,
				Dst:         destIP,
			},
			Status: netproto.TunnelStatus{
				TunnelID: infraAPI.AllocateID(types.TunnelID, types.TunnelOffset),
			},
		}

		if _, ok := isCollectorTunnelKnown(infraAPI, tun); ok {
			log.Infof("Lateral Tunnel: %v already present. Skipping...", tun.Name)
		} else {
			log.Infof("Creating internal tunnel %v for mirror session %v", tun.ObjectMeta, owner)
			err := createTunnelHandler(infraAPI, intfClient, tun, vrfID)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrCollectorTunnelCreateFailure, "MirrorSession: %s |  Err: %v", owner, err))
				return errors.Wrapf(types.ErrCollectorTunnelCreateFailure, "MirrorSession: %s |  Err: %v", owner, err)
			}
		}
	}
	return nil
}

// DeleteLateralNetAgentObjects deletes lateral objects for telmetry objects and does refcounting. This is temporary code till HAL subsumes ARP'ing for dest IPs
func DeleteLateralNetAgentObjects(infraAPI types.InfraAPI, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, vrfID uint64, owner string, mgmtIP, destIP string, tunnelOp bool) error {
	log.Infof("Deleting Lateral NetAgent objects. Owner: %v MgmtIP: %v DestIP: %v TunnelOp: %v", owner, mgmtIP, destIP, tunnelOp)

	var lateralEP *netproto.Endpoint
	eDat, err := infraAPI.List("Endpoint")
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Err: %v", types.ErrObjNotFound))
	}
	for _, o := range eDat {
		var endpoint netproto.Endpoint
		err := endpoint.Unmarshal(o)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			continue
		}
		for _, address := range endpoint.Spec.IPv4Addresses {
			if address == destIP {
				lateralEP = &endpoint
				break
			}
		}
	}

	if lateralEP == nil {
		log.Errorf("Lateral EP not found for destIP: %s", destIP)
		//Todo check with abhi if we have to handle idemopotncy. for idempotency case to pass retun nil
		return nil
		//return fmt.Errorf("endpoint not found Owner: %v Obj: %v", owner, destIP)
	}

	objName := fmt.Sprintf("_internal-%s", lateralEP.Spec.MacAddress)
	tunnelName := fmt.Sprintf("_internal-%s", destIP)
	tunnelCompositeKey := fmt.Sprintf("tunnel|%s", tunnelName)
	collectorCompositeKey := fmt.Sprintf("collector|%s", objName)

	log.Infof("Lateral object DB pre refcount decrement: %v", lateralDB)

	// Remove Dependency
	counter := 0
	for _, dep := range lateralDB[tunnelCompositeKey] {
		if dep != owner {
			lateralDB[tunnelCompositeKey][counter] = dep
			counter++
		}
	}
	lateralDB[tunnelCompositeKey] = lateralDB[tunnelCompositeKey][:counter]

	counter = 0
	for _, dep := range lateralDB[collectorCompositeKey] {
		if dep != owner {
			lateralDB[collectorCompositeKey][counter] = dep
			counter++
		}
	}
	lateralDB[collectorCompositeKey] = lateralDB[collectorCompositeKey][:counter]

	counter = 0
	for _, dep := range lateralDB[destIP] {
		if dep != owner {
			lateralDB[destIP][counter] = dep
			counter++
		}
	}
	lateralDB[destIP] = lateralDB[destIP][:counter]

	log.Infof("Lateral object DB post refcount decrement: %v", lateralDB)

	// Check for pending dependencies
	if tunnelOp {
		if len(lateralDB[tunnelCompositeKey]) == 0 {
			// Find Tunnel
			t := netproto.Tunnel{
				TypeMeta: api.TypeMeta{Kind: "Tunnel"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      fmt.Sprintf("_internal-%s", destIP),
				},
			}
			if tun, ok := isCollectorTunnelKnown(infraAPI, t); ok {
				log.Infof("Deleting lateral tunnel %v", t.Name)
				err := deleteTunnelHandler(infraAPI, intfClient, *tun)
				if err != nil {
					log.Error(errors.Wrapf(types.ErrCollectorTunnelDeleteFailure, "MirrorSession: %s |  Err: %v", owner, err))
					return errors.Wrapf(types.ErrCollectorTunnelDeleteFailure, "MirrorSession: %s |  Err: %v", owner, err)
				}
			}
			delete(lateralDB, tunnelCompositeKey)
		} else {
			log.Infof("Disallowing deletion of lateral tunnels. Pending refs: %v", lateralDB[tunnelCompositeKey])
		}
	}

	gwIP, ok := GwCache[destIP]
	var internalEP bool
	if ok {
		internalEP = true
	}
	if internalEP && len(lateralDB[destIP]) == 0 {
		// Check for pending dependencies
		if len(lateralDB[collectorCompositeKey]) == 0 {
			log.Infof("Deleting lateral endpoint %v", lateralEP.ObjectMeta.Name)
			err := deleteEndpointHandler(infraAPI, epClient, intfClient, *lateralEP, vrfID, types.UntaggedCollVLAN)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrCollectorEPDeleteFailure, "MirrorSession: %s |  Err: %v", owner, err))
				return errors.Wrapf(types.ErrCollectorEPDeleteFailure, "MirrorSession: %s |  Err: %v", owner, err)
			}

			cancel, ok := doneCache[gwIP]
			if ok {
				log.Infof("Calling cancel for IP: %v", destIP)
				cancel()
			}
			delete(lateralDB, collectorCompositeKey)
		} else {
			for idx, dep := range lateralEP.Spec.IPv4Addresses {
				if dep == destIP {
					log.Infof("Updating endpoint: %s", lateralEP.GetKey())
					ep := &netproto.Endpoint{
						TypeMeta: api.TypeMeta{Kind: "Endpoint"},
						ObjectMeta: api.ObjectMeta{
							Tenant:    "default",
							Namespace: "default",
							Name:      lateralEP.Name,
						},
						Spec: netproto.EndpointSpec{
							NetworkName: lateralEP.Spec.NetworkName,
							NodeUUID:    lateralEP.Spec.NodeUUID,
							MacAddress:  lateralEP.Spec.MacAddress,
						},
					}
					ep.Spec.IPv4Addresses = append(ep.Spec.IPv4Addresses, lateralEP.Spec.IPv4Addresses[:idx]...)
					ep.Spec.IPv4Addresses = append(ep.Spec.IPv4Addresses, lateralEP.Spec.IPv4Addresses[idx+1:]...)
					if err = updateEndpointHandler(infraAPI, epClient, intfClient, *ep, vrfID, types.UntaggedCollVLAN); err != nil {
						log.Error(errors.Wrapf(types.ErrCollectorEPUpdateFailure, "MirrorSession: %s |  Err: %v", owner, err))
						return errors.Wrapf(types.ErrCollectorEPUpdateFailure, "MirrorSession: %s |  Err: %v", owner, err)
					}
					break
				}
			}
		}
		delete(lateralDB, destIP)
		return nil
	}

	// if not internal ep and created by venice
	if len(lateralDB[destIP]) == 0 {
		delete(lateralDB, destIP)
	}
	if len(lateralDB[collectorCompositeKey]) == 0 {
		delete(lateralDB, collectorCompositeKey)
	}
	log.Infof("Disallowing deletion of lateral endpoints. Pending refs: %v", lateralDB[lateralEP.Name])
	return nil
}

func startRefreshLoop(refreshCtx context.Context, IP net.IP) {
	log.Infof("Starting ARP refresh loop for %v", IP)

	ticker := time.NewTicker(refreshDuration)
	go func(refreshCtx context.Context, IP net.IP) {
		mac := resolveWithDeadline(refreshCtx, IP)
		log.Infof("Resolved MAC 1st Tick: %s", mac)
		for {
			select {
			case <-ticker.C:
				resolveWithDeadline(refreshCtx, IP)
			case <-refreshCtx.Done():
				log.Infof("Cancelling ARP refresh loop for %v ", IP)
				return
			}
		}
	}(refreshCtx, IP)
}

func resolveWithDeadline(ctx context.Context, IP net.IP) string {
	resolveCtx, cancel := context.WithCancel(ctx)
	defer cancel()
	arpChan := make(chan string, 1)

	go func(arpChan chan string, IP net.IP) {
		mac, err := ArpClient.Resolve(IP)
		if err != nil {
			log.Errorf("Failed to resolve MAC for %v", err)
		}
		// Populate the ARPCache.
		arpCache.LoadOrStore(IP.String(), mac.String())
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

func reconcileLateralObjects(infraAPI types.InfraAPI, owner string, destIP string, createTunnel bool) (epKnown, tunnelKnown bool) {
	// Find EP
	log.Infof("Testing for known collector with DestIP: %v, TunnelOp: %v", destIP, createTunnel)
	var (
		tDat [][]byte
		eDat [][]byte
		err  error
	)
	tDat, err = infraAPI.List("Tunnel")
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Err: %v", types.ErrObjNotFound))
	}

	eDat, err = infraAPI.List("Endpoint")
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Err: %v", types.ErrObjNotFound))
	}

	if createTunnel == true {
		var knownTunnel *netproto.Tunnel
		var knownCollector *netproto.Endpoint

		for _, o := range tDat {
			var tunnel netproto.Tunnel
			err := tunnel.Unmarshal(o)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
				continue
			}
			if tunnel.Spec.Dst == destIP {
				knownTunnel = &tunnel
				break
			}
		}

		for _, o := range eDat {
			var endpoint netproto.Endpoint
			err := endpoint.Unmarshal(o)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
				continue
			}
			for _, address := range endpoint.Spec.IPv4Addresses {
				if address == destIP {
					knownCollector = &endpoint
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
	for _, o := range eDat {
		var endpoint netproto.Endpoint
		err := endpoint.Unmarshal(o)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			continue
		}
		for _, address := range endpoint.Spec.IPv4Addresses {
			if address == destIP {
				knownCollector = &endpoint
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
func dedupReferences(compositeKey string) {
	log.Infof("Lateral DB: %v pre dedup: %v", compositeKey, lateralDB[compositeKey])
	sort.Strings(lateralDB[compositeKey])
	if len(lateralDB[compositeKey]) < 2 {
		return
	}
	marker := 0
	for idx := 1; idx < len(lateralDB[compositeKey]); idx++ {
		if lateralDB[compositeKey][marker] == lateralDB[compositeKey][idx] {
			continue
		}
		marker++
		lateralDB[compositeKey][marker] = lateralDB[compositeKey][idx]
	}
	lateralDB[compositeKey] = lateralDB[compositeKey][:marker+1]
}

func generateLateralEP(destIP string) (*netproto.Endpoint, error) {
	var dMAC string
	epIP := destIP

	log.Infof("Resolving for: %s", destIP)
	addrs, err := netlink.AddrList(MgmtLink, netlink.FAMILY_V4)
	if err != nil {
		log.Errorf("Failed to list management interface addresses. Err: %v", err)
		// Temporary hack in Release A. generateLateralEP is expected to return errors only on ARP resolution failures.
		// Hence masking just logging the err
		return nil, nil
	}
	mgmtSubnet := addrs[0].IPNet
	GwCache[destIP] = destIP

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
		GwCache[destIP] = gwIP
		destIP = gwIP
	}

	// Set the correct dIP
	dIP := net.ParseIP(destIP)

	//// Cache the handler for outer done. This needs to be called during the object deletes
	refreshCtx, done := context.WithCancel(context.Background())
	doneCache[destIP] = done

	log.Infof("Starting refresh loop for %s", dIP.String())

	// Check if the dIP is already resolved. This makes the refresh loop a singleton for a given IP
	if _, ok := arpCache.Load(destIP); !ok {
		go startRefreshLoop(refreshCtx, dIP)
	}

	time.Sleep(time.Second * 3)

	for i := 0; i < 3; i++ {
		d, ok := arpCache.Load(destIP)
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
			d, ok := arpCache.Load(destIP)
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

	objectName := fmt.Sprintf("_internal-%s", dMAC)

	ep := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      objectName,
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   types.InternalDefaultUntaggedNetwork,
			NodeUUID:      "REMOTE",
			IPv4Addresses: []string{epIP},
			MacAddress:    dMAC,
		},
	}

	return ep, nil
}

func isCollectorEPKnown(infraAPI types.InfraAPI, collectorEP netproto.Endpoint) (ep netproto.Endpoint, known bool) {
	dat, err := infraAPI.Read(collectorEP.Kind, collectorEP.GetKey())
	if err != nil {
		return
	}
	err = ep.Unmarshal(dat)
	known = true
	return
}

func isCollectorTunnelKnown(infraAPI types.InfraAPI, collectorTunnel netproto.Tunnel) (*netproto.Tunnel, bool) {
	var knownTunnel netproto.Tunnel
	dat, err := infraAPI.Read(collectorTunnel.Kind, collectorTunnel.GetKey())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Tunnel: %s | Err: %v", collectorTunnel.GetKey(), types.ErrObjNotFound))
		return nil, false
	}

	err = knownTunnel.Unmarshal(dat)
	if err != nil {
		return nil, false
	}

	return &knownTunnel, true
}
