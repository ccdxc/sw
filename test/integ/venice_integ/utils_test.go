// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/telemetry"
)

// createTenant creates a tenant using REST api
func (it *veniceIntegSuite) createTenant(tenantName string) (*network.Tenant, error) {
	// build network object
	ten := network.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: tenantName,
		},
	}

	// create it
	return it.restClient.TenantV1().Tenant().Create(context.Background(), &ten)
}

// deleteTenant deletes a tenant using REST api
func (it *veniceIntegSuite) deleteTenant(tenantName string) (*network.Tenant, error) {
	// build network object
	meta := api.ObjectMeta{
		Name: tenantName,
	}

	// delete it
	return it.restClient.TenantV1().Tenant().Delete(context.Background(), &meta)
}

// createNetwork creates a network using REST api
func (it *veniceIntegSuite) createNetwork(tenant, namespace, net, subnet, gw string) (*network.Network, error) {
	// build network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: namespace,
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
	// build meta object
	ometa := api.ObjectMeta{
		Name:      net,
		Namespace: "",
		Tenant:    tenant,
	}

	// delete it
	return it.restClient.NetworkV1().Network().Delete(context.Background(), &ometa)
}

// getStatsPolicy gets a stats policy
func (it *veniceIntegSuite) getStatsPolicy(tenantName string) (*telemetry.StatsPolicy, error) {
	// build meta object
	ometa := api.ObjectMeta{
		Name:   tenantName,
		Tenant: tenantName,
	}

	// TODO: use rest api
	return it.apisrvClient.StatsPolicyV1().StatsPolicy().Get(context.Background(), &ometa)
}

// getFwlogPolicy gets a fwlog policy
func (it *veniceIntegSuite) getFwlogPolicy(tenantName string) (*telemetry.FwlogPolicy, error) {
	// build meta object
	ometa := api.ObjectMeta{
		Name:   tenantName,
		Tenant: tenantName,
	}

	// TODO: use rest api
	return it.apisrvClient.FwlogPolicyV1().FwlogPolicy().Get(context.Background(), &ometa)
}
