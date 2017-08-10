// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
)

// createNetwork creates a network using REST api
func (it *veniceIntegSuite) createNetwork(tenant, net, subnet, gw string) (*network.Network, error) {
	// build network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: "",
			Tenant:    tenant,
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}

	// create it
	return it.restClient.NetworkV1().Network().Create(context.Background(), &nw)
}

// deleteNetwork deletes a network using REST api
func (it *veniceIntegSuite) deleteNetwork(tenant, net string) (*network.Network, error) {
	// build network object
	ometa := api.ObjectMeta{
		Name:      net,
		Namespace: "",
		Tenant:    tenant,
	}

	// delete it
	return it.restClient.NetworkV1().Network().Delete(context.Background(), &ometa)
}
