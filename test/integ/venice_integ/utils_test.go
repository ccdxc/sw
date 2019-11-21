// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
)

// createTenant creates a tenant using REST api
func (it *veniceIntegSuite) createTenant(tenantName string) (*cluster.Tenant, error) {
	// build tenant object
	ten := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: tenantName,
		},
	}
	ctx, err := it.loggedInCtx()
	if err != nil {
		return nil, err
	}
	// create it
	return it.restClient.ClusterV1().Tenant().Create(ctx, &ten)
}

// getTenant get tenant info. using REST api
func (it *veniceIntegSuite) getTenant(tenantName string) (*cluster.Tenant, error) {
	// build tenant meta
	meta := api.ObjectMeta{
		Name: tenantName,
	}
	ctx, err := it.loggedInCtx()
	if err != nil {
		return nil, err
	}
	// get tenant
	return it.restClient.ClusterV1().Tenant().Get(ctx, &meta)
}

// deleteTenant deletes a tenant using REST api
func (it *veniceIntegSuite) deleteTenant(tenantName string) (*cluster.Tenant, error) {
	ctx, err := it.loggedInCtx()
	if err != nil {
		return nil, err
	}

	// delete all alerts belonging to default alert policy
	alertPolicies, _ := it.restClient.MonitoringV1().Alert().List(ctx,
		&api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenantName}, FieldSelector: "status.reason.alert-policy-id=default-event-based-alerts"})
	for _, ap := range alertPolicies {
		it.restClient.MonitoringV1().Alert().Delete(ctx, &ap.ObjectMeta)
	}

	// delete default alert policy
	it.restClient.MonitoringV1().AlertPolicy().Delete(ctx, &api.ObjectMeta{Tenant: tenantName, Name: "default-event-based-alerts"})

	// delete it
	return it.restClient.ClusterV1().Tenant().Delete(ctx, &api.ObjectMeta{Name: tenantName})
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
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}
	ctx, err := it.loggedInCtx()
	if err != nil {
		return nil, err
	}
	// create it
	return it.apisrvClient.NetworkV1().Network().Create(ctx, &nw)
}

// deleteNetwork deletes a network using REST api
func (it *veniceIntegSuite) deleteNetwork(tenant, net string) (*network.Network, error) {
	// build meta object
	ometa := api.ObjectMeta{
		Name:      net,
		Namespace: "",
		Tenant:    tenant,
	}
	ctx, err := it.loggedInCtx()
	if err != nil {
		return nil, err
	}
	// delete it
	return it.apisrvClient.NetworkV1().Network().Delete(ctx, &ometa)
}

// getStatsPolicy gets a stats policy
func (it *veniceIntegSuite) getStatsPolicy(tenantName string) (*monitoring.StatsPolicy, error) {
	// build meta object
	ometa := api.ObjectMeta{
		Name:   tenantName,
		Tenant: tenantName,
	}

	ctx, err := it.loggedInCtx()
	if err != nil {
		return nil, err
	}

	return it.restClient.MonitoringV1().StatsPolicy().Get(ctx, &ometa)
}

// getFwlogPolicy gets a fwlog policy
func (it *veniceIntegSuite) getFwlogPolicy(tenantName string) (*monitoring.FwlogPolicy, error) {
	// build meta object
	ometa := api.ObjectMeta{
		Name:   tenantName,
		Tenant: tenantName,
	}

	ctx, err := it.loggedInCtx()
	if err != nil {
		return nil, err
	}

	return it.restClient.MonitoringV1().FwlogPolicy().Get(ctx, &ometa)
}

func (it *veniceIntegSuite) createUser(tenantName, username, password string) (*auth.User, error) {
	user := &auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tenantName,
			Name:   username,
		},
		Spec: auth.UserSpec{
			Password: password,
		},
	}
	ctx, err := it.loggedInCtx()
	if err != nil {
		return nil, err
	}

	return it.restClient.AuthV1().User().Create(ctx, user)
}
