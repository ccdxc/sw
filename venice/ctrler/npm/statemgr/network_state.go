// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net"
	"sync"

	"github.com/willf/bitset"

	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// NetworkState is a wrapper for network object
type NetworkState struct {
	sync.Mutex                           // lock the network object
	Network    *ctkit.Network            `json:"-"` // network object
	endpointDB map[string]*EndpointState // endpoint database
	stateMgr   *Statemgr                 // pointer to network manager
}

// NetworkStateFromObj conerts from memdb object to network state
func NetworkStateFromObj(obj runtime.Object) (*NetworkState, error) {
	switch obj.(type) {
	case *ctkit.Network:
		nobj := obj.(*ctkit.Network)
		switch nobj.HandlerCtx.(type) {
		case *NetworkState:
			nts := nobj.HandlerCtx.(*NetworkState)
			return nts, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// utility functions to convert IPv4 address to int and vice versa
func ipv42int(ip net.IP) uint32 {
	if len(ip) == 16 {
		return binary.BigEndian.Uint32(ip[12:16])
	}

	return binary.BigEndian.Uint32(ip)
}

func int2ipv4(nn uint32) net.IP {
	ip := make(net.IP, 4)
	binary.BigEndian.PutUint32(ip, nn)
	return ip
}

func ipv4toMac(macPrefix []byte, ip net.IP) net.HardwareAddr {
	if len(ip) == 16 {
		return net.HardwareAddr(append(macPrefix[0:2], ip[12:16]...))
	}
	return net.HardwareAddr(append(macPrefix[0:2], ip...))
}

func convertNetwork(nw *NetworkState) *netproto.Network {
	nt := netproto.Network{
		TypeMeta:   nw.Network.TypeMeta,
		ObjectMeta: nw.Network.ObjectMeta,
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  nw.Network.Spec.IPv4Subnet,
			IPv4Gateway: nw.Network.Spec.IPv4Gateway,
			IPv6Subnet:  nw.Network.Spec.IPv6Subnet,
			IPv6Gateway: nw.Network.Spec.IPv6Gateway,
			VlanID:      nw.Network.Spec.VlanID,
		},
		Status: netproto.NetworkStatus{
			AllocatedVlanID: nw.Network.Spec.VlanID,
		},
	}

	return &nt
}

// allocIPv4Addr allocates new IP address
func (ns *NetworkState) allocIPv4Addr(reqAddr string) (string, error) {
	var allocatedAddr string
	// lock the network for bit allocation
	ns.Lock()
	defer ns.Unlock()

	// parse the subnet
	baseAddr, ipnet, err := net.ParseCIDR(ns.Network.Spec.IPv4Subnet)
	if err != nil {
		log.Errorf("Error parsing subnet %v. Err: %v", ns.Network.Spec.IPv4Subnet, err)
		return "", err
	}

	// read ipv4 allocation bitmap
	subnetMaskLen, maskLen := ipnet.Mask.Size()
	subnetSize := 1 << uint32(maskLen-subnetMaskLen)
	buf := bytes.NewBuffer(ns.Network.Status.AllocatedIPv4Addrs)
	bs := bitset.New(uint(subnetSize))
	bs.ReadFrom(buf)

	// see if caller requested a specific addr
	if reqAddr != "" {
		reqIPAddr, _, err := net.ParseCIDR(reqAddr)
		if err != nil {
			reqIPAddr = net.ParseIP(reqAddr)
		}

		// verify requested address is in this subnet
		if !ipnet.Contains(reqIPAddr) {
			log.Errorf("Requested address %s is not in subnet %s", reqAddr, ns.Network.Spec.IPv4Subnet)
			return "", fmt.Errorf("requested address not in subnet")
		}

		// determine the bit in bitmask
		addrBit := ipv42int(reqIPAddr) - ipv42int(ipnet.IP)

		// check if address is already allocated
		if bs.Test(uint(addrBit)) {
			log.Errorf("Address %s is already allocated", reqAddr)
			return "", fmt.Errorf("Requested address not available")
		}

		// alloc the bit
		bs.Set(uint(addrBit))
		allocatedAddr = reqIPAddr.String()

		log.Infof("Allocating requested addr: %v, bit: %d", allocatedAddr, addrBit)
	} else {
		// allocate next available address
		addrBit, ok := bs.NextClear(0)
		if !ok || addrBit >= uint(subnetSize) {
			log.Errorf("Could not find a free bit in bitset")
			return "", fmt.Errorf("Could not find a free addr")
		}

		bs.Set(uint(addrBit))
		allocatedAddr = int2ipv4(uint32(uint(ipv42int(baseAddr)) + addrBit)).String()

		log.Infof("Allocating addr: %v, bit: %d", allocatedAddr, addrBit)
	}

	// save the bitset back
	bs.WriteTo(buf)
	ns.Network.Status.AllocatedIPv4Addrs = buf.Bytes()

	return allocatedAddr, nil
}

// freeIPv4Addr free the address
func (ns *NetworkState) freeIPv4Addr(reqAddr string) error {
	ns.Lock()
	defer ns.Unlock()

	log.Infof("Freeing IPv4 address: %v", reqAddr)

	reqIP, _, err := net.ParseCIDR(reqAddr)
	if err != nil {
		log.Errorf("Error parsing ip address: %v. Err: %v", reqAddr, err)
		return err
	}

	// parse the subnet
	baseAddr, ipnet, err := net.ParseCIDR(ns.Network.Spec.IPv4Subnet)
	if err != nil {
		log.Errorf("Error parsing subnet %v. Err: %v", ns.Network.Spec.IPv4Subnet, err)
		return err
	}

	// verify the address is in subnet
	if !ipnet.Contains(reqIP) {
		log.Errorf("Requested address %s is not in subnet %s", reqAddr, ns.Network.Spec.IPv4Subnet)
		return fmt.Errorf("requested address not in subnet")
	}

	// read ipv4 allocation bitmap
	subnetMaskLen, maskLen := ipnet.Mask.Size()
	subnetSize := 1 << uint32(maskLen-subnetMaskLen)
	buf := bytes.NewBuffer(ns.Network.Status.AllocatedIPv4Addrs)
	bs := bitset.New(uint(subnetSize))
	bs.ReadFrom(buf)

	// determine the bit in bitmask
	addrBit := ipv42int(reqIP) - ipv42int(baseAddr)

	// verify the address is still allocated
	if !bs.Test(uint(addrBit)) {
		log.Errorf("Address %s was not allocated", reqAddr)
		return fmt.Errorf("Requested address was not allocated")
	}

	// clear the bit
	bs.Clear(uint(addrBit))

	// save the bitset back
	bs.WriteTo(buf)
	ns.Network.Status.AllocatedIPv4Addrs = buf.Bytes()

	return nil
}

// FindEndpoint finds an endpoint in a network
func (ns *NetworkState) FindEndpoint(epName string) (*EndpointState, bool) {
	// lock the endpoint db
	ns.Lock()
	defer ns.Unlock()

	// find the endpoint in the DB
	eps, ok := ns.endpointDB[epName]
	return eps, ok
}

// ListEndpoints lists all endpoints on this network
func (ns *NetworkState) ListEndpoints() []*EndpointState {
	var eplist []*EndpointState

	// lock the endpoint db
	ns.Lock()
	defer ns.Unlock()

	// walk all endpoints
	for _, ep := range ns.endpointDB {
		eplist = append(eplist, ep)
	}

	return eplist
}

// Delete cleans up all state associated with the network
func (ns *NetworkState) Delete() error {
	// check if network still has endpoints
	if len(ns.endpointDB) != 0 {
		log.Errorf("Can not delete the network, still has endpoints {%+v}", ns)
		return fmt.Errorf("Network still has endpoints")
	}

	return nil
}

// NewNetworkState creates new network state object
func NewNetworkState(nw *ctkit.Network, stateMgr *Statemgr) (*NetworkState, error) {
	// parse the subnet
	if nw.Spec.IPv4Subnet != "" {
		_, ipnet, err := net.ParseCIDR(nw.Spec.IPv4Subnet)
		if err != nil {
			log.Errorf("Error parsing subnet %v. Err: %v", nw.Spec.IPv4Subnet, err)
			return nil, err
		}

		subnetMaskLen, maskLen := ipnet.Mask.Size()
		subnetSize := 1 << uint32(maskLen-subnetMaskLen)

		// build the ip allocation bitset
		bs := bitset.New(uint(subnetSize))
		bs.ClearAll()

		// set first and last bits as used
		bs.Set(0).Set(uint(subnetSize - 1))

		// write the bitset into network object
		buf := bytes.NewBuffer([]byte{})
		bs.WriteTo(buf)
		nw.Status.AllocatedIPv4Addrs = buf.Bytes()
	}

	// create the network state
	ns := &NetworkState{
		Network:    nw,
		endpointDB: make(map[string]*EndpointState),
		stateMgr:   stateMgr,
	}
	nw.HandlerCtx = ns

	// mark gateway addr as used
	if nw.Spec.IPv4Gateway != "" {
		log.Infof("Requested gw addr %s", nw.Spec.IPv4Gateway)
		allocAddr, err := ns.allocIPv4Addr(nw.Spec.IPv4Gateway)
		if err != nil {
			log.Errorf("Error allocating gw address. Err: %v", err)
			return nil, err
		} else if allocAddr != nw.Spec.IPv4Gateway {
			log.Errorf("Error allocating gw address(req: %v, alloc: %v)", nw.Spec.IPv4Gateway, allocAddr)
			return nil, fmt.Errorf("Error allocating gw addr")
		}
	}

	// save it to api server
	err := ns.Network.Write()
	if err != nil {
		log.Errorf("Error writing the network state to api server. Err: %v", err)
		return nil, err
	}

	return ns, nil
}

// FindNetwork finds a network
func (sm *Statemgr) FindNetwork(tenant, name string) (*NetworkState, error) {
	// find the object
	obj, err := sm.FindObject("Network", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return NetworkStateFromObj(obj)
}

// ListNetworks lists all networks
func (sm *Statemgr) ListNetworks() ([]*NetworkState, error) {
	objs := sm.ListObjects("Network")

	var networks []*NetworkState
	for _, obj := range objs {
		nso, err := NetworkStateFromObj(obj)
		if err != nil {
			return networks, err
		}

		networks = append(networks, nso)
	}

	return networks, nil
}

// OnNetworkCreate creates local network state based on watch event
func (sm *Statemgr) OnNetworkCreate(nw *ctkit.Network) error {
	// create new network state
	ns, err := NewNetworkState(nw, sm)
	if err != nil {
		log.Errorf("Error creating new network state. Err: %v", err)
		return err
	}
	log.Infof("Created Network state {Meta: %+v, Spec: %+v}", ns.Network.ObjectMeta, ns.Network.Spec)

	// store it in local DB
	sm.mbus.AddObject(convertNetwork(ns))

	return nil
}

// OnNetworkUpdate handles network update
func (sm *Statemgr) OnNetworkUpdate(nw *ctkit.Network) error {
	return nil
}

// OnNetworkDelete deletes a network
func (sm *Statemgr) OnNetworkDelete(nto *ctkit.Network) error {
	// see if we already have it
	nso, err := sm.FindObject("Network", nto.Tenant, "default", nto.Name)
	if err != nil {
		log.Errorf("Can not find the network %s|%s", nto.Tenant, nto.Name)
		return fmt.Errorf("Network not found")
	}

	// convert it to network state
	ns, err := NetworkStateFromObj(nso)
	if err != nil {
		return err
	}

	// cleanup network state
	err = ns.Delete()
	if err != nil {
		log.Errorf("Error deleting the network {%+v}. Err: %v", ns, err)
		return err
	}
	// delete it from the DB
	return sm.mbus.DeleteObject(convertNetwork(ns))
}
