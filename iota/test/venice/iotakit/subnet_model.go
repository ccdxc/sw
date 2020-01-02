// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"fmt"
	"math/rand"
	"net"
	"strings"

	"github.com/willf/bitset"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/log"
)

// Network represents a Vlan with a subnet (called network in venice)
type Network struct {
	Name          string // subnet name
	vlan          uint32
	ipPrefix      string
	bitmask       *bitset.BitSet
	subnetSize    int
	veniceNetwork *network.Network
	sm            *SysModel // pointer back to the model
}

// NetworkCollection is a list of subnets
type NetworkCollection struct {
	err     error
	subnets []*Network
}

func (sm *SysModel) createNetwork(nw *network.Network) error {
	// parse the subnet
	_, ipnet, err := net.ParseCIDR(nw.Spec.IPv4Subnet)
	if err != nil {
		log.Errorf("Error parsing subnet %v. Err: %v", nw.Spec.IPv4Subnet, err)
		return err
	}

	// read ipv4 allocation bitmap
	subnetMaskLen, maskLen := ipnet.Mask.Size()
	subnetSize := 1 << uint32(maskLen-subnetMaskLen)
	bs := bitset.New(uint(subnetSize))
	bs.Set(0)
	snet := Network{
		Name:       fmt.Sprintf("Network-Vlan-%d", nw.Spec.VlanID),
		vlan:       nw.Spec.VlanID,
		ipPrefix:   nw.Spec.IPv4Subnet,
		subnetSize: subnetSize,
		bitmask:    bs,
		sm:         sm,
	}
	sm.subnets = append(sm.subnets, &snet)

	return nil
}

// allocateIPAddr allocates an IP addr
func (sn *Network) allocateIPAddr() (string, error) {
	// allocate next available address
	addrBit, ok := sn.bitmask.NextClear(0)
	if !ok || addrBit >= uint(sn.subnetSize) {
		log.Errorf("Could not find a free bit in bitset for network %v", sn.Name)
		return "", fmt.Errorf("Could not find a free addr")
	}

	sn.bitmask.Set(uint(addrBit))
	ipBytes := strings.Split(strings.Split(sn.ipPrefix, "/")[0], ".")
	return fmt.Sprintf("%s.%s.%s.%d/%s", ipBytes[0], ipBytes[1], ipBytes[2], addrBit, strings.Split(sn.ipPrefix, "/")[1]), nil
}

// Networks returns a list of subnets
func (sm *SysModel) Networks() *NetworkCollection {
	snc := NetworkCollection{}
	for _, sn := range sm.subnets {
		snc.subnets = append(snc.subnets, sn)
	}

	return &snc
}

// Any returns any one of the subnets in random
func (snc *NetworkCollection) Any(num int) *NetworkCollection {
	if snc.err != nil || len(snc.subnets) <= num {
		return snc
	}

	newSnc := NetworkCollection{subnets: []*Network{}}
	tmpArry := make([]*Network, len(snc.subnets))
	copy(tmpArry, snc.subnets)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newSnc.subnets = append(newSnc.subnets, sn)
	}

	return &newSnc
}
