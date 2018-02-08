// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net"
	"sync"

	"github.com/gogo/protobuf/proto"
	"github.com/willf/bitset"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// NetworkState is a wrapper for network object
type NetworkState struct {
	sync.Mutex                                // lock the network object
	network.Network                           // network object
	endpointDB      map[string]*EndpointState // endpoint database
	stateMgr        *Statemgr                 // pointer to network manager
}

// NetworkStateFromObj conerts from memdb object to network state
func NetworkStateFromObj(obj memdb.Object) (*NetworkState, error) {
	switch obj.(type) {
	case *NetworkState:
		nsobj := obj.(*NetworkState)
		return nsobj, nil
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

// allocIPv4Addr allocates new IP address
func (ns *NetworkState) allocIPv4Addr(reqAddr string) (string, error) {
	var allocatedAddr string

	// lock the network for bit allocation
	ns.Lock()
	defer ns.Unlock()

	// parse the subnet
	baseAddr, ipnet, err := net.ParseCIDR(ns.Spec.IPv4Subnet)
	if err != nil {
		log.Errorf("Error parsing subnet %v. Err: %v", ns.Spec.IPv4Subnet, err)
		return "", err
	}

	// read ipv4 allocation bitmap
	subnetMaskLen, maskLen := ipnet.Mask.Size()
	subnetSize := 1 << uint32(maskLen-subnetMaskLen)
	buf := bytes.NewBuffer(ns.Status.AllocatedIPv4Addrs)
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
			log.Errorf("Requested address %s is not in subnet %s", reqAddr, ns.Spec.IPv4Subnet)
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
	ns.Status.AllocatedIPv4Addrs = buf.Bytes()

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
	baseAddr, ipnet, err := net.ParseCIDR(ns.Spec.IPv4Subnet)
	if err != nil {
		log.Errorf("Error parsing subnet %v. Err: %v", ns.Spec.IPv4Subnet, err)
		return err
	}

	// verify the address is in subnet
	if !ipnet.Contains(reqIP) {
		log.Errorf("Requested address %s is not in subnet %s", reqAddr, ns.Spec.IPv4Subnet)
		return fmt.Errorf("requested address not in subnet")
	}

	// read ipv4 allocation bitmap
	subnetMaskLen, maskLen := ipnet.Mask.Size()
	subnetSize := 1 << uint32(maskLen-subnetMaskLen)
	buf := bytes.NewBuffer(ns.Status.AllocatedIPv4Addrs)
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
	ns.Status.AllocatedIPv4Addrs = buf.Bytes()

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

// CreateEndpoint creates an endpoint
func (ns *NetworkState) CreateEndpoint(epinfo *network.Endpoint) (*EndpointState, error) {
	// see if we already have this endpoint
	oldEps, ok := ns.FindEndpoint(epinfo.ObjectMeta.Name)
	if ok {
		// if we already have identical endpoint, we are done
		if proto.Equal(oldEps, epinfo) {
			return oldEps, nil
		}

		// FIXME: handle endpoint change
		log.Errorf("Endpoint {%+v} already exists {%+v} in network %v", epinfo, oldEps, ns.ObjectMeta.Name)
		return nil, fmt.Errorf("Endpoint already exists")
	}

	// allocate an IP address
	ipv4Addr, err := ns.allocIPv4Addr(epinfo.Status.IPv4Address)
	if err != nil {
		log.Errorf("Error allocating IP address from network {%+v}. Err: %v", ns, err)
		return nil, err
	}

	// allocate a mac address based on IP address
	macAddr := ipv4toMac([]byte{0x01, 0x01}, net.ParseIP(ipv4Addr))

	// convert address to CIDR
	_, ipNet, _ := net.ParseCIDR(ns.Spec.IPv4Subnet)
	subnetMaskLen, _ := ipNet.Mask.Size()
	ipv4Addr = fmt.Sprintf("%s/%d", ipv4Addr, subnetMaskLen)

	// populate allocated values
	epi := *epinfo
	epi.Status.IPv4Address = ipv4Addr
	epi.Status.IPv4Gateway = ns.Spec.IPv4Gateway

	// assign new mac address if we dont have one
	if epi.Status.MacAddress == "" {
		epi.Status.MacAddress = macAddr.String()
	}

	// create a new endpoint instance
	eps, err := NewEndpointState(epi, ns.stateMgr)
	if err != nil {
		log.Errorf("Error creating endpoint state from spec{%+v}, Err: %v", epinfo, err)
		return nil, err
	}

	// save the endpoint in the database
	ns.Lock()
	ns.endpointDB[eps.endpointKey()] = eps
	ns.Unlock()
	ns.stateMgr.memDB.AddObject(eps)

	// write the modified network state to api server
	err = ns.Write()
	if err != nil {
		log.Errorf("Error writing the network object. Err: %v", err)
		return nil, err
	}

	return eps, nil
}

// DeleteEndpoint deletes an endpoint
func (ns *NetworkState) DeleteEndpoint(epmeta *api.ObjectMeta) (*EndpointState, error) {
	// see if we have the endpoint
	eps, ok := ns.FindEndpoint(epmeta.Name)
	if !ok {
		log.Errorf("could not find the endpoint %+v", epmeta)
		return nil, ErrEndpointNotFound
	}

	// free the IPv4 address
	err := ns.freeIPv4Addr(eps.Status.IPv4Address)
	if err != nil {
		log.Errorf("Error freeing the endpoint address. Err: %v", err)
	}

	// delete the endpoint
	err = eps.Delete()
	if err != nil {
		log.Errorf("Error deleting the endpoint{%+v}. Err: %v", eps, err)
	}
	// remove it from the database
	ns.Lock()
	delete(ns.endpointDB, eps.endpointKey())
	ns.Unlock()
	ns.stateMgr.memDB.DeleteObject(eps)

	// write the modified network state to api server
	err = ns.Write()
	if err != nil {
		log.Errorf("Error writing the network object. Err: %v", err)
	}

	return eps, nil
}

// Write writes the object to api server
func (ns *NetworkState) Write() error {
	return ns.stateMgr.writer.WriteNetwork(&ns.Network)
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
func NewNetworkState(nw *network.Network, stateMgr *Statemgr) (*NetworkState, error) {
	// parse the subnet
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

	// create the network state
	ns := &NetworkState{
		Network:    *nw,
		endpointDB: make(map[string]*EndpointState),
		stateMgr:   stateMgr,
	}

	// mark gateway addr as used
	allocAddr, err := ns.allocIPv4Addr(nw.Spec.IPv4Gateway)
	if err != nil {
		log.Errorf("Error allocating gw address. Err: %v", err)
		return nil, err
	} else if allocAddr != nw.Spec.IPv4Gateway {
		log.Errorf("Error allocating gw address(req: %v, alloc: %v)", nw.Spec.IPv4Gateway, allocAddr)
		return nil, fmt.Errorf("Error allocating gw addr")
	}

	// save it to api server
	err = ns.Write()
	if err != nil {
		log.Errorf("Error writing the network state to api server. Err: %v", err)
		return nil, err
	}

	return ns, nil
}

// FindNetwork finds a network
func (sm *Statemgr) FindNetwork(tenant, name string) (*NetworkState, error) {
	// find the object
	obj, err := sm.FindObject("Network", tenant, name)
	if err != nil {
		return nil, err
	}

	return NetworkStateFromObj(obj)
}

// ListNetworks lists all networks
func (sm *Statemgr) ListNetworks() ([]*NetworkState, error) {
	objs := sm.memDB.ListObjects("Network")

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

// CreateNetwork creates local network state based on watch event
func (sm *Statemgr) CreateNetwork(nw *network.Network) error {
	// see if we already have it
	ens, err := sm.FindObject("Network", nw.ObjectMeta.Tenant, nw.ObjectMeta.Name)
	if err == nil {
		// FIXME: how do we handle an existing network object changing?
		log.Errorf("Can not change existing network {%+v}. New state: {%+v}", ens, nw)
		return fmt.Errorf("Can not change network after its created")
	}

	// create new network state
	ns, err := NewNetworkState(nw, sm)
	if err != nil {
		log.Errorf("Error creating new network state. Err: %v", err)
		return err
	}
	log.Infof("Created Network state {Meta: %+v, Spec: %+v}", ns.ObjectMeta, ns.Spec)

	// store it in local DB
	sm.memDB.AddObject(ns)

	return nil
}

// DeleteNetwork deletes a network
func (sm *Statemgr) DeleteNetwork(tenant, network string) error {
	// see if we already have it
	nso, err := sm.FindObject("Network", tenant, network)
	if err != nil {
		log.Errorf("Can not find the network %s|%s", tenant, network)
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
	return sm.memDB.DeleteObject(nso)
}
