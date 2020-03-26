package objects

import (
	"math/rand"

	"github.com/pensando/sw/api/generated/network"
	"github.com/willf/bitset"
	"github.com/pensando/sw/venice/utils/log"
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

// Commit writes the VPC config to venice
func (nwc *NetworkCollection) Commit() error {
	if nwc.HasError() {
		return nwc.err
	}
	for _, nw := range nwc.subnets {
		err := nwc.Client.CreateNetwork(nw.VeniceNetwork)
		if err != nil {
			err = nwc.Client.UpdateNetwork(nw.VeniceNetwork)

			if err != nil {
				nwc.err = err
				log.Infof("Creating/updating network failed %v", err)
				return err
			}
		}

		log.Debugf("Created/updated network : %#v", nwc.Subnets())
	}

	return nil
}

// Delete deletes all networks/subnets in collection
func (nwc *NetworkCollection) Delete() error {
	if nwc.err != nil {
		return nwc.err
	}

	// walk all sessions and delete them
	for _, nw := range nwc.subnets {
		err := nwc.Client.DeleteNetwork(nw.VeniceNetwork)
		if err != nil {
			return err
		}
	}

	return nil
}