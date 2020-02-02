package objects

import (
	"math/rand"

	"github.com/pensando/sw/api/generated/network"
	"github.com/willf/bitset"
)

// Network represents a Vlan with a subnet (called network in venice)
type Network struct {
	Name          string // subnet name
	vlan          uint32
	ipPrefix      string
	bitmask       *bitset.BitSet
	subnetSize    int
	VeniceNetwork *network.Network
}

// NetworkCollection is a list of subnets
type NetworkCollection struct {
	CollectionCommon
	err     error
	subnets []*Network
}

func (n *NetworkCollection) Subnets() []*Network {
	return n.subnets
}

func (n *NetworkCollection) AddSubnet(nw *Network) {
	n.subnets = append(n.subnets, nw)
}

// Any returns any one of the subnets in random
func (snc *NetworkCollection) Any(num int) *NetworkCollection {
	if snc.HasError() || len(snc.subnets) <= num {
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
