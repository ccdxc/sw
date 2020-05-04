package objects

import (
	"fmt"
	"math/rand"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
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

//NewNetworkCollection create a new collector
func NewNetworkCollectionFromNetworks(client objClient.ObjClient, nws []*network.Network) *NetworkCollection {

	nwc := NetworkCollection{CollectionCommon: CollectionCommon{Client: client}}
	for _, nw := range nws {
		nwc.subnets = append(nwc.subnets,
			&Network{VeniceNetwork: nw, Name: nw.Name})
	}

	return &nwc
}

func (n *NetworkCollection) Subnets() []*Network {
	return n.subnets
}

func (n *NetworkCollection) Count() int {
	return len(n.subnets)
}

func (n *NetworkCollection) AddSubnet(nw *Network) {
	n.subnets = append(n.subnets, nw)
}

func (n *NetworkCollection) GetTenant() string {
	return n.subnets[0].VeniceNetwork.Tenant
}

// Any returns any one of the subnets in random
func (snc *NetworkCollection) Any(num int) *NetworkCollection {
	if snc.HasError() || len(snc.subnets) <= num {
		return snc
	}

	newSnc := NetworkCollection{subnets: []*Network{}, CollectionCommon: snc.CollectionCommon}
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

// Pop returns popped element and removes from currnet collection
func (snc *NetworkCollection) Pop(num int) *NetworkCollection {
	if snc.HasError() || len(snc.subnets) <= num {
		return snc
	}

	newSnc := NetworkCollection{subnets: []*Network{}, CollectionCommon: snc.CollectionCommon}
	tmpArry := make([]*Network, len(snc.subnets))
	copy(tmpArry, snc.subnets)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newSnc.subnets = append(newSnc.subnets, sn)
	}

	snc.subnets = snc.subnets[num:]

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

func NewNetworkCollection(client objClient.ObjClient, testbed *testbed.TestBed) *NetworkCollection {
	return &NetworkCollection{
		CollectionCommon: CollectionCommon{Client: client, Testbed: testbed},
	}
}

//SetIngressSecurityPolicy sets ingress security policy to this
func (nwc *NetworkCollection) SetIngressSecurityPolicy(policies *NetworkSecurityPolicyCollection) error {

	for _, nw := range nwc.subnets {
		nw.VeniceNetwork.Spec.IngressSecurityPolicy = []string{}
		if policies != nil {
			for _, pol := range policies.Policies {
				nw.VeniceNetwork.Spec.IngressSecurityPolicy = append(nw.VeniceNetwork.Spec.IngressSecurityPolicy, pol.VenicePolicy.Name)
			}
		}
		err := nwc.Client.UpdateNetwork(nw.VeniceNetwork)
		if err != nil {
			return err
		}
	}
	return nil
}

//SetEgressSecurityPolicy sets egress security policy to this
func (nwc *NetworkCollection) SetEgressSecurityPolicy(policies *NetworkSecurityPolicyCollection) error {

	for _, nw := range nwc.subnets {
		nw.VeniceNetwork.Spec.EgressSecurityPolicy = []string{}
		if policies != nil {
			for _, pol := range policies.Policies {
				nw.VeniceNetwork.Spec.EgressSecurityPolicy = append(nw.VeniceNetwork.Spec.EgressSecurityPolicy, pol.VenicePolicy.Name)
			}
		}
		err := nwc.Client.UpdateNetwork(nw.VeniceNetwork)
		if err != nil {
			return err
		}
	}
	return nil
}

// SetIPAM sets IPAM on the networks
func (nwc *NetworkCollection) SetIPAMOnNetwork(network *Network, ipam string) error {

	for _, nw := range nwc.subnets {
		if nw.Name == network.Name {
			nw.VeniceNetwork.Spec.IPAMPolicy = ipam
			err := nwc.Client.UpdateNetwork(nw.VeniceNetwork)
			if err != nil {
				return err
			}
		}
	}
	return nil
}

func (nw *Network) UpdateIPv4Subnet(prefix string) error {
	if nw.VeniceNetwork == nil {
		return fmt.Errorf("Invalid network\n")
	}

	spec := &nw.VeniceNetwork.Spec
	spec.IPv4Subnet = prefix
	return nil
}

func (nw *Network) UpdateIPv4Gateway(gw string) error {
	if nw.VeniceNetwork == nil {
		return fmt.Errorf("Invalid network\n")
	}

	spec := &nw.VeniceNetwork.Spec
	spec.IPv4Gateway = gw
	return nil
}

func VpcNetworkCollection(tenant, vpc string, n int, client objClient.ObjClient) (*NetworkCollection, error) {
	nws, err := client.ListNetwork(tenant)
	if err != nil || n <= 0 {
		return nil, err
	}

	if len(nws) < n {
		return nil, fmt.Errorf("Not enough Networks on tenant %s", tenant)
	}

	var nws_vpc []*network.Network
	count := 0
	for _, nw := range nws {
		if count >= n {
			break
		}
		if nw.Spec.VirtualRouter == vpc {
			nws_vpc = append(nws_vpc, nw)
			count++
		}
	}

	if count == 0 {
		return nil, fmt.Errorf("No Networks on VPC %s", vpc)
	}

	nwc := NewNetworkCollectionFromNetworks(client, nws_vpc)
	return nwc, nil
}

// returns the subnet collection of default config
func DefaultNetworkCollection(client objClient.ObjClient) (*NetworkCollection, error) {
	ten, err := client.ListTenant()
	if err != nil {
		return nil, err
	}
	if len(ten) == 0 {
		return nil, fmt.Errorf("No tenants to get network collection")
	}

	vpcs, err := client.ListVPC(ten[0].Name)
	if err != nil {
		return nil, err
	}
	if len(vpcs) == 0 {
		return nil, fmt.Errorf("No VPCs on tenant %V to get network collection", ten[0].Name)
	}

	vpc := ""
	for _, v := range vpcs {
		// stop at the first tenant VPC
		if v.Spec.Type == "tenant" {
			vpc = v.Name
			break
		}
	}

	nws, err := client.ListNetwork(ten[0].Name)
	if err != nil {
		return nil, err
	}

	var nws_vpc []*network.Network
	for _, nw := range nws {
		if nw.Spec.VirtualRouter == vpc {
			nws_vpc = append(nws_vpc, nw)
		}
	}

	nwc := NewNetworkCollectionFromNetworks(client, nws_vpc)
	return nwc, nil
}
