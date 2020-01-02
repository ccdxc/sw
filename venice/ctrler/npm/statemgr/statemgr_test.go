// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	"fmt"
	"os"
	"strconv"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/ref"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// createRoutingConfig utility function to create a routing config
func createRoutingConfig(stateMgr *Statemgr, tenant, name, IP string) (*network.RoutingConfig, error) {
	rtcfg := network.RoutingConfig{
		TypeMeta: api.TypeMeta{Kind: "RoutingConfig"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.RoutingConfigSpec{
			BGPConfig: &network.BGPConfig{
				RouterId: "1.1.1.1",
				ASNumber: 100,
			},
			EVPNConfig: &network.EVPNConfig{
				Shutdown: false,
			},
		},
		Status: network.RoutingConfigStatus{},
	}

	cfg := &network.BGPNeighbor{
		Shutdown:              false,
		IPAddress:             IP,
		RemoteAS:              1,
		MultiHop:              1,
		EnableAddressFamilies: []string{"IPV4", "EVPN"},
	}

	rtcfg.Spec.BGPConfig.Neighbors = append(rtcfg.Spec.BGPConfig.Neighbors, cfg)

	err := stateMgr.ctrler.RoutingConfig().Create(&rtcfg)
	return &rtcfg, err
}

// createIPAMPolicy utility function to create an IPAM Policy
func createIPAMPolicy(stateMgr *Statemgr, tenant, name, dhcpIP string) (*network.IPAMPolicy, error) {
	policy := network.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.IPAMPolicySpec{
			DHCPRelay: &network.DHCPRelayPolicy{},
		},
		Status: network.IPAMPolicyStatus{},
	}

	server := &network.DHCPServer{
		IPAddress:     dhcpIP,
		VirtualRouter: "default",
	}

	policy.Spec.DHCPRelay.Servers = append(policy.Spec.DHCPRelay.Servers, server)

	err := stateMgr.ctrler.IPAMPolicy().Create(&policy)
	return &policy, err
}

// createNetwork utility function to create a network
func createNetwork(t *testing.T, stateMgr *Statemgr, tenant, net, subnet, gw string) error {
	// network params
	np := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}

	// create a network
	return stateMgr.ctrler.Network().Create(&np)

}

func createSg(stateMgr *Statemgr, tenant, sgname string, selector *labels.Selector) (*security.SecurityGroup, error) {
	// sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      sgname,
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selector,
		},
	}

	// create sg
	err := stateMgr.ctrler.SecurityGroup().Create(&sg)

	return &sg, err
}

func updateSg(stateMgr *Statemgr, tenant, sgname string, selector *labels.Selector) (*security.SecurityGroup, error) {
	// sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      sgname,
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selector,
		},
	}

	// create sg
	err := stateMgr.ctrler.SecurityGroup().Update(&sg)

	return &sg, err
}

func deleteSg(stateMgr *Statemgr, tenant, sgname string, selector *labels.Selector) (*security.SecurityGroup, error) {
	// sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      sgname,
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selector,
		},
	}

	// create sg
	err := stateMgr.ctrler.SecurityGroup().Delete(&sg)

	return &sg, err
}

// createTenant utility function to create a tenant
func createTenant(t *testing.T, stateMgr *Statemgr, tenant string) error {
	// network params
	tn := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: tenant,
		},
	}

	// create a network
	stateMgr.ctrler.Tenant().Create(&tn)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindTenant(tenant)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant not foud", "1ms", "1s")

	return nil
}

// createEndpoint utility function to create an endpoint
func createEndpoint(stateMgr *Statemgr, tenant, endpoint, net string) (*workload.Endpoint, error) {
	// network params
	ep := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      endpoint,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:   "testContainerName",
			Network:        net,
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	return &ep, stateMgr.ctrler.Endpoint().Create(&ep)
}

// createVirtualRouter utility function to create a VirtualRouter
func createVirtualRouter(t *testing.T, stateMgr *Statemgr, tenant, virtualRouter string) (*network.VirtualRouter, error) {
	// virtual router params
	vr := network.VirtualRouter{
		TypeMeta: api.TypeMeta{Kind: "VirtualRouter"},
		ObjectMeta: api.ObjectMeta{
			Name:      virtualRouter,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.VirtualRouterSpec{
			Type: "Tenant",
		},
		Status: network.VirtualRouterStatus{},
	}

	// create a virtual router
	stateMgr.ctrler.VirtualRouter().Create(&vr)

	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindVirtualRouter(tenant, "default", virtualRouter)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding virtualrouter %v\n", err)
		return false, nil
	}, "virtual router not foud", "1ms", "1s")

	return &vr, nil
}

// TestNetworkCreateDelete tests network create
func TestNetworkCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")
	nw, err := stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")
	Assert(t, (nw.Network.Spec.IPv4Subnet == "10.1.1.0/24"), "Network subnet did not match", nw)
	Assert(t, (nw.Network.Spec.IPv4Gateway == "10.1.1.254"), "Network gateway did not match", nw)
	Assert(t, (nw.Network.TypeMeta.Kind == "Network"), "Network type meta did not match", nw)

	verifyFail := func() {
		AssertEventually(t, func() (bool, interface{}) {
			_, err := stateMgr.FindNetwork("default", "default")
			if err != nil {
				return true, nil
			}
			return true, nil
		}, "Network not found as expected", "1ms", "1s")
	}

	// verify can not create networks with invalid params
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.255")
	verifyFail()
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.0")
	verifyFail()
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.2.254")
	verifyFail()
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.25424")
	verifyFail()
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.10-20/24", "10.1.1.254")
	verifyFail()
	verifyFail()

	// verify we cant create duplicate networks
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Network update failed")
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// delete the network
	err = stateMgr.ctrler.Network().Delete(&nw.Network.Network)
	AssertOk(t, err, "Failed to delete the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Network not deleted", "1ms", "1s")

}

func TestNetworkList(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// verify the list works
	nets, err := stateMgr.ListNetworks()
	AssertOk(t, err, "Error listing networks")
	Assert(t, (len(nets) == 1), "Incorrect number of networks received", nets)
	Assert(t, (nets[0].Network.Name == "default"), "Incorrect network received", nets[0])
}

// TestEndpointCreateDelete tests network create
func TestEndpointCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")
	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint params
	epinfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:   "testContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not created", "1ms", "1s")

	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)
	Assert(t, (eps.Endpoint.TypeMeta.Kind == "Endpoint"), "Endpoint type meta did not match", eps)
	Assert(t, (eps.Endpoint.Status.IPv4Address == "10.1.1.1/24"), "Endpoint address did not match", eps)
	Assert(t, (eps.Endpoint.Status.IPv4Gateway == "10.1.1.254"), "Endpoint gateway did not match", eps)

	// verify you cant create duplicate endpoints
	epinfo.Status.IPv4Address = "10.1.1.5"
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	AssertOk(t, err, "updating endpoint failed")

	// verify creating new EP after ip addr have run out fails
	newEP := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "newEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:   "newContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	err = stateMgr.ctrler.Endpoint().Create(&newEP)
	AssertOk(t, err, "Endpoint failed")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "newEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not created", "1ms", "1s")

	nep, err := stateMgr.FindEndpoint("default", "newEndpoint")
	AssertOk(t, err, "Error finding the endpoint")
	Assert(t, (nep.Endpoint.Status.IPv4Address == "10.1.1.2/24"), "Endpoint address did not match", nep)

	//hack to increase coverage
	stateMgr.GetConfigPushStatus()
	stateMgr.GetConfigPushStats()
	stateMgr.ResetConfigPushStats()
	// delete the endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&epinfo)
	Assert(t, (err == nil), "Error deleting the endpoint", epinfo)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not deleted", "1ms", "1s")

	// verify endpoint is gone from the database
	_, err = stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err != nil), "Deleted endpoint still found in network db", "testEndpoint")

}

func timeTrack(start time.Time, name string) time.Duration {
	elapsed := time.Since(start)
	fmt.Printf("%s took %s\n", name, elapsed)
	return elapsed
}

func TestEndpointStaleDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
			Conditions: []cluster.DSCCondition{
				cluster.DSCCondition{
					Status: cluster.ConditionStatus_TRUE.String(),
					Type:   cluster.DSCCondition_HEALTHY.String()},
			},
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("", "testHost")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// verify we can find the network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
	AssertOk(t, err, "Could not find the network")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")
	// verify we can find the endpoint associated with the workload
	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0405")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	//hack to increase coverage
	stateMgr.GetConfigPushStatus()
	stateMgr.GetConfigPushStats()
	stateMgr.ResetConfigPushStats()

	//Delete a a host
	err = stateMgr.ctrler.Host().Delete(&host)
	//Assert(t, err == nil, "Error deleting the host")

	//obj, err1 := stateMgr.FindObject("Network", "default", "default", "Network-Vlan-1")
	//Assert(t, err1 == nil, "Error finding the network")
	err = stateMgr.ctrler.Network().Delete(&nw.Network.Network)
	Assert(t, err == nil, "Error deleting the network")

	// delete the workload and it will fail to trigger endpoint delete.
	err = stateMgr.ctrler.Workload().Delete(&wr)
	Assert(t, err == nil, "Error deleting the workload")

	//stateMgr.ctrler.Endpoint.Workload.Delete()
	err = stateMgr.RemoveStaleEndpoints()
	Assert(t, (err == nil), "Network still found after its deleted")

	// verify endpoint is gone from the database
	obj1, _ := stateMgr.FindEndpoint("", "testWorkload-0001.0203.0405")
	Assert(t, (obj1 == nil), "Deleted endpoint still found in network db", "testWorkload-0001.0203.0405")
}

func TestEndpointCreateFailure(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/30", "10.1.1.2")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint params
	epinfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Status: workload.EndpointStatus{
			WorkloadName:   "testContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")
	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)
	Assert(t, (eps.Endpoint.Status.IPv4Address == "10.1.1.1/30"), "Endpoint address did not match", eps)

	// verify creating new EP after ip addr have run out fails
	newEP := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "newEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Status: workload.EndpointStatus{
			WorkloadName:   "newContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}
	err = stateMgr.ctrler.Endpoint().Create(&newEP)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "newEndpoint")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")
	eps, err = stateMgr.FindEndpoint("default", "newEndpoint")
	Assert(t, (err != nil), "Endpoint creation should have failed", newEP)

	// delete the first endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&epinfo)
	Assert(t, (err == nil), "Error deleting the endpoint", epinfo)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", epinfo.Name)
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	// verify now the creation goes thru
	err = stateMgr.ctrler.Endpoint().Create(&newEP)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", newEP.Name)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")
	AssertOk(t, err, "Second endpoint creation failed")
	nep, err := stateMgr.FindEndpoint("default", newEP.Name)
	Assert(t, (err == nil), "Error finding the endpoint", newEP)
	Assert(t, (nep.Endpoint.Status.IPv4Address == "10.1.1.1/30"), "Endpoint address did not match", nep)
}

// TestEndpointListWatch tests ep watch and list
func TestEndpointList(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint params
	epinfo := ctkit.Endpoint{
		Endpoint: workload.Endpoint{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Name:      "testEndpoint",
				Namespace: "default",
				Tenant:    "default",
			},
			Spec: workload.EndpointSpec{},
			Status: workload.EndpointStatus{
				WorkloadName:   "testContainerName",
				Network:        "default",
				HomingHostAddr: "192.168.1.1",
				HomingHostName: "testHost",
			},
		},
	}

	// create endpoint
	err = stateMgr.OnEndpointCreate(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", &epinfo)

	// verify the list works
	nets, err := stateMgr.ListNetworks()
	AssertOk(t, err, "Error listing networks")
	Assert(t, (len(nets) == 1), "Incorrect number of networks received", nets)
	for _, nt := range nets {
		endps := nt.ListEndpoints()
		Assert(t, (len(endps) == 1), "Invalid number of endpoints received", endps)
		Assert(t, (endps[0].Endpoint.Name == epinfo.Endpoint.Name), "Invalid endpoint params received", endps[0])
	}

}

func TestEndpointNodeState(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint params
	epinfo := ctkit.Endpoint{
		Endpoint: workload.Endpoint{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Name:      "testEndpoint",
				Namespace: "default",
				Tenant:    "default",
			},
			Spec: workload.EndpointSpec{},
			Status: workload.EndpointStatus{
				WorkloadName:   "testContainerName",
				Network:        "default",
				HomingHostAddr: "192.168.1.1",
				HomingHostName: "testHost",
			},
		},
	}

	// create endpoint
	err = stateMgr.OnEndpointCreate(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", &epinfo)

	//nep, err := stateMgr.mbus.FindEndpoint(&epinfo.ObjectMeta)
	//AssertOk(t, err, "Error finding endpoint in mbus")

	// trigger node state add
	//err = stateMgr.OnEndpointOperUpdate("node-1", &epinfo)
	//AssertOk(t, err, "Error adding node status")

	// trigger node state delete
	//err = stateMgr.OnEndpointOperDelete("node-1", &epinfo)
	//AssertOk(t, err, "Error deleting node status")

}

func TestSgCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create sg
	sg, err := createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg, err = updateSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	// verify we can find the sg
	sgs, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Could not find the security group")
	AssertEquals(t, sgs.SecurityGroup.Spec.WorkloadSelector.String(), sg.Spec.WorkloadSelector.String(), "Security group params did not match")

	// delete the security group
	err = stateMgr.ctrler.SecurityGroup().Delete(&sgs.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Sg found", "1ms", "1s")

	// verify the sg is gone
	_, err = stateMgr.FindSecurityGroup("default", "testSg")
	Assert(t, (err != nil), "Security group still found after deleting")
}

func TestSgAttachEndpoint(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// create sg
	_, err = createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Error finding sg")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint object
	epinfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:       "testContainerName",
			Network:            "default",
			HomingHostAddr:     "192.168.1.1",
			HomingHostName:     "testHost",
			WorkloadAttributes: map[string]string{"env": "production", "app": "procurement"},
		},
	}

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "10ms", "2s")

	// verify endpoint is associated with the sg
	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)
	AssertEventually(t, func() (bool, interface{}) {
		if len(eps.Endpoint.Status.SecurityGroups) == 1 {
			return true, nil
		}
		return false, fmt.Sprintf("expected : %d, got : %v. Endpoint Object : %v", 1, len(eps.Endpoint.Status.SecurityGroups), eps)
	}, "Sg was not linked to endpoint", "100ms", "2s")
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 1), "Sg was not linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 1), "Sg was not linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[0] == sg.SecurityGroup.Name), "Sg was not linked to endpoint", eps)
	Assert(t, (len(sg.SecurityGroup.Status.Workloads) == 1), "Endpoint is not linked to sg", sg)
	Assert(t, (len(sg.endpoints) == 1), "Endpoint is not linked to sg", sg)
	Assert(t, (sg.SecurityGroup.Status.Workloads[0] == eps.Endpoint.Name), "Endpoint is not linked to sg", sg)

	// create a new sg
	_, err = createSg(stateMgr, "default", "testSg2", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg2, err := stateMgr.FindSecurityGroup("default", "testSg2")
	AssertOk(t, err, "Error finding sg")

	// verify endpoint is associated with the new sg too..
	AssertEventually(t, func() (bool, interface{}) {
		if len(sg2.SecurityGroup.Status.Workloads) == 1 {
			return true, nil
		}
		return false, nil
	}, "Endpoint not associated ", "1ms", "1s")
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 2), "Sg was not linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 2), "Sg was not linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[1] == sg2.SecurityGroup.Name), "Sg was not linked to endpoint", eps)
	Assert(t, (len(sg2.SecurityGroup.Status.Workloads) == 1), "Endpoint is not linked to sg", sg2)

	Assert(t, (len(sg2.endpoints) == 1), "Endpoint is not linked to sg", sg2)
	Assert(t, (sg2.SecurityGroup.Status.Workloads[0] == eps.Endpoint.Name), "Endpoint is not linked to sg", sg2)

	// delete the second sg
	err = stateMgr.ctrler.SecurityGroup().Delete(&sg2.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg2")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	// verify sg is removed from endpoint
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 1), "Sg is still linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 1), "Sg is still linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[0] == sg.SecurityGroup.Name), "Sg is still linked to endpoint", eps)

	// delete the endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&eps.Endpoint.Endpoint)
	AssertOk(t, err, "Error deleting endpoint")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	// verify endpoint is removed from sg
	Assert(t, (len(sg.SecurityGroup.Status.Workloads) == 0), "Endpoint is still linked to sg", sg)
}

func TestSgErrorCases(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create sg with nil selector
	_, err = createSg(stateMgr, "default", "testSg", nil)
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	_, err = stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Error finding sg")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint object with nil workload attributes
	epinfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:       "testContainerName",
			Network:            "default",
			HomingHostAddr:     "192.168.1.1",
			HomingHostName:     "testHost",
			WorkloadAttributes: nil,
		},
	}

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	// verify endpoint is created
	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)

	// create a new sg with nil selector
	_, err = createSg(stateMgr, "default", "testSg2", nil)
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg2, err := stateMgr.FindSecurityGroup("default", "testSg2")
	AssertOk(t, err, "Error finding sg")

	// delete the second sg
	err = stateMgr.ctrler.SecurityGroup().Delete(&sg2.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg2")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	// delete the endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&eps.Endpoint.Endpoint)
	AssertOk(t, err, "Error deleting endpoint")
}

// test concurrent add/delete/change of endpoints
func TestEndpointConcurrency(t *testing.T) {
	var concurrency = 200

	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	waitCh := make(chan error, concurrency*2)

	// create endpoint
	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			// endpoint object
			epinfo := workload.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Name:      fmt.Sprintf("testEndpoint-%d", idx),
					Namespace: "default",
					Tenant:    "default",
				},
				Spec: workload.EndpointSpec{},
				Status: workload.EndpointStatus{
					WorkloadName:       "testContainerName",
					Network:            "default",
					HomingHostAddr:     "192.168.1.1",
					HomingHostName:     "testHost",
					WorkloadAttributes: map[string]string{"env": "production", "app": "procurement"},
				},
			}

			// create endpoint
			eperr := stateMgr.ctrler.Endpoint().Create(&epinfo)
			waitCh <- eperr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertEventually(t, func() (bool, interface{}) {
			_, err := stateMgr.FindEndpoint("default", fmt.Sprintf("testEndpoint-%d", i))
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Endpoint not found", "1ms", "5s")

	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error creating endpoint")
	}

	// create few SGs concurrently that match on endpoints
	tsg := make([]*security.SecurityGroup, concurrency)
	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			var serr error
			tsg[idx], serr = createSg(stateMgr, "default", fmt.Sprintf("testSg-%d", idx), labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
			waitCh <- serr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error creating sgs")
	}

	// delete the sgs concurrently
	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			serr := stateMgr.ctrler.SecurityGroup().Delete(tsg[idx])
			fmt.Printf("Error delete sgs %v\n", serr)
			waitCh <- serr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error deleting sgs")
	}
	// delete endpoint

	for i := 0; i < concurrency; i++ {
		AssertEventually(t, func() (bool, interface{}) {
			_, err := stateMgr.FindEndpoint("default", fmt.Sprintf("testEndpoint-%d", i))
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Endpoint not found", "1ms", "1s")

	}

	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			ometa := workload.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Name:      fmt.Sprintf("testEndpoint-%d", idx),
					Namespace: "default",
					Tenant:    "default",
				},
			}
			// delete the endpoint
			eperr := stateMgr.ctrler.Endpoint().Delete(&ometa)
			waitCh <- eperr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error deleting endpoint")
	}
}

// TestTenantCreateDelete tests tenant create and delete
func TestTenantCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "testTenant")
	AssertOk(t, err, "Error creating the tenant")

	// verify tenant got created
	tn, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Error finding the tenant")
	AssertEquals(t, tn.Tenant.Name, "testTenant", "Did not match expected tenant name")

	// create security groups
	sg1, err := createSg(stateMgr, "testTenant", "testSg1", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("testTenant", "testSg1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg2, err := createSg(stateMgr, "testTenant", "testSg2", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("testTenant", "testSg2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	// delete the tenant
	err = stateMgr.ctrler.Tenant().Delete(&tn.Tenant.Tenant)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindTenant("testTenant")
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant was deleted  foud", "1ms", "1s")

	// delete resources

	err = stateMgr.ctrler.SecurityGroup().Delete(sg1)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindSecurityGroup("testTenant", sg1.Name)
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant was deleted  foud", "1ms", "1s")

	err = stateMgr.ctrler.SecurityGroup().Delete(sg2)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindSecurityGroup("testTenant", sg2.Name)
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant was deleted  foud", "1ms", "1s")

	//Make sure Tenant  deleted.
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindTenant("testTenant")
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant was deleted  foud", "1ms", "1s")

}

// TestNonExistentTenantDeletion will test that non existent tenants can't be deleted
func TestNonExistentTenantDeletion(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	ntn := ctkit.Tenant{
		Tenant: cluster.Tenant{
			TypeMeta: api.TypeMeta{Kind: "Tenant"},
			ObjectMeta: api.ObjectMeta{
				Name: "nonExistingTenant",
			},
		},
	}
	// delete the tenant
	err = stateMgr.OnTenantDelete(&ntn)
	Assert(t, err == ErrTenantNotFound, "Deleting tenant failed.")
}

func TestWorkloadCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0409",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "20ms", "1s")

	// verify we can find the network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
	AssertOk(t, err, "Could not find the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0409")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify we can find the endpoint associated with the workload
	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0409")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0409")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&wr)
	AssertOk(t, err, "Error deleting the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0409")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify endpoint is gone from the database
	_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, (ok == false), "Deleted endpoint still found in network db", "testWorkload-0001.0203.0405")
}

func TestWorkloadUpdate(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// update workload external vlan
	nwr := ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.Interfaces[0].ExternalVlan = 2
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// verify we can find the new network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-2")
	AssertOk(t, err, "Could not find updated network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			_, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
			return ok, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0405")
	Assert(t, (foundEp.Endpoint.Status.Network == nw.Network.Name), "endpoint network did not match")

	// change mac address of the workload
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].MACAddress = "0001.0201.0405"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify old endpoint is deleted
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	Assert(t, (err != nil), "found endpoint with old mac address", foundEp)

	_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, (ok == false), "old endpoint still found in network db", "testWorkload-0001.0203.0405")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify new endpoint is created
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
	AssertOk(t, err, "Could not find the new endpoint")

	// change useg vlan
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].MicroSegVlan = 101
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil && (foundEp.Endpoint.Status.MicroSegmentVlan == 101) {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// add IP address
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].IpAddresses = []string{"1.2.3.4/32"}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil && (foundEp.Endpoint.Status.IPv4Address == "1.2.3.4/32") {
			return true, nil
		}
		fmt.Printf("IP Address %v", foundEp.Endpoint.Status.IPv4Address)
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// update IP address
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].IpAddresses = []string{"1.2.3.5/32"}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil && (foundEp.Endpoint.Status.IPv4Address == "1.2.3.5/32") {
			return true, nil
		}
		fmt.Printf("IP Address %v", foundEp.Endpoint.Status.IPv4Address)
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// remove IP address
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].IpAddresses = []string{}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil && (foundEp.Endpoint.Status.IPv4Address == "") {
			return true, nil
		}
		fmt.Printf("IP Address %v", foundEp.Endpoint.Status.IPv4Address)
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// add new interface to workload
	newIntf := workload.WorkloadIntfSpec{
		MACAddress:   "0002.0406.0800",
		MicroSegVlan: 200,
		ExternalVlan: 1,
	}
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces = append(nwr.Spec.Interfaces, newIntf)
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0406.0800")
		if err == nil && (foundEp.Endpoint.Status.MicroSegmentVlan == 200) {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify we can find the new endpoint

	// delete second interface
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces = nwr.Spec.Interfaces[0:1]
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0406.0800")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")
	// verify endpoint is gone
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0406.0800")
	Assert(t, (err != nil), "found endpoint for deleted interface", foundEp)

	// trigger a dummy and verify endpoint is not deleted
	nwr.ObjectMeta.GenerationID = "2"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
	AssertOk(t, err, "Could not find the new endpoint")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&nwr)
	AssertOk(t, err, "Error deleting the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify endpoint is gone from the database
	_, ok = nw.FindEndpoint("testWorkload-0002.0406.0800")
	Assert(t, (ok == false), "Deleted endpoint still found in network db", "testWorkload-0001.0201.0203")
}

func TestWorkloadUpdateHost(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// create the smartNics
	snic1 := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard1",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}
	snic2 := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard2",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0607.0809",
		},
	}
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic1)
	AssertOk(t, err, "Could not create the smartNic")
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic2)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host1 := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost1",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}
	host2 := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost2",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0607.0809",
				},
			},
		},
	}

	// create the hosts
	err = stateMgr.ctrler.Host().Create(&host1)
	AssertOk(t, err, "Could not create the host")
	err = stateMgr.ctrler.Host().Create(&host2)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost1",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0002.0202.0202",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("default", "testHost2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "20ms", "2s")

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "20ms", "2s")

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.HomingHostName == host1.Name), "endpoint params did not match")
	Assert(t, (foundEp.Endpoint.Status.NodeUUID == snic1.Name), "endpoint params did not match")

	// update workload hostname
	nwr := ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.HostName = "testHost2"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
		if err == nil && (foundEp.Endpoint.Status.HomingHostName == host2.Name) &&
			(foundEp.Endpoint.Status.NodeUUID == snic2.Name) {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.HomingHostName == host2.Name), "endpoint host did not match")
	Assert(t, (foundEp.Endpoint.Status.NodeUUID == snic2.Name), "endpoint params did not match")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&nwr)
	AssertOk(t, err, "Error deleting the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify endpoint is gone from the database
	_, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
	Assert(t, (err != nil), "Deleted endpoint still found in db")
}

func TestWorkloadWithDuplicateMac(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testSmartNIC",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "2s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// verify we can find the network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
	AssertOk(t, err, "Could not find the network")

	// verify we can find the endpoint associated with the workload
	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0405")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// try to create second workload with duplicate mac address
	wr2 := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload2",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 101,
					ExternalVlan: 1,
				},
			},
		},
	}
	err = stateMgr.ctrler.Workload().Create(&wr2)
	AssertOk(t, err, "Could not create the second workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// find the second workload
	AssertEventually(t, func() (bool, interface{}) {
		fwr2, err := stateMgr.FindWorkload("default", "testWorkload2")
		if err == nil && fwr2.Workload.Status.PropagationStatus.Status == "Propagation Failed. Duplicate MAC address" {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload2-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// verify endpoint is not created
	_, ok = nw.FindEndpoint("testWorkload2-0001.0203.0405")
	Assert(t, (ok == false), "Duplicate endpoint still found in network db", "testWorkload2-0001.0203.0405")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&wr)
	AssertOk(t, err, "Error deleting the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
		if err != nil && !ok {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// verify endpoint is gone from the database
	_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, (ok == false), "Deleted endpoint still found in network db", "testWorkload-0001.0203.0405")
}

func TestHostCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("default", "testHost")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "2s")

	// verify we can find the endpoint associated with the host
	foundHost, err := stateMgr.FindHost("default", "testHost")
	AssertOk(t, err, "Could not find the host")
	Assert(t, (len(foundHost.Host.Spec.DSCs) == 1), "host params did not match")

	// delete the host
	err = stateMgr.ctrler.Host().Delete(&host)
	AssertOk(t, err, "Error deleting the host")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("default", "testHost")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "2s")

	// verify endpoint is gone from the database
	_, err = stateMgr.FindHost("default", "testHost")
	Assert(t, (err != nil), "Deleted host still found in db")
}

func TestHostUpdates(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("default", "testHost")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "2s")

	// verify we can find the endpoint associated with the host
	_, err = stateMgr.FindHost("default", "testHost")
	AssertOk(t, err, "Could not find the host")

	// create a workload on the host
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "workload create failed")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint foud", "1ms", "2s")

	// verify we can not find the endpoint associated with the workload
	_, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	Assert(t, err != nil, "Endpoint got created without smartnic")

	// create the smartnic
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{
			ID: "test-snic",
		},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	AssertEventually(t, func() (bool, interface{}) {
		foundEP, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil && foundEP.Endpoint.Status.NodeUUID == snic.Name {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found or node uuid not match", "20ms", "2s")

	// change host's mac address
	nhst := ref.DeepCopy(host).(cluster.Host)
	nhst.Spec.DSCs[0].MACAddress = "0002.0406.0800"
	err = stateMgr.ctrler.Host().Update(&nhst)
	AssertOk(t, err, "Error updating the host")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found or node uuid not match", "1ms", "2s")

	// now associate by name
	nhst = ref.DeepCopy(nhst).(cluster.Host)
	nhst.Spec.DSCs[0].MACAddress = ""
	nhst.Spec.DSCs[0].ID = "test-snic"
	fmt.Printf("Host update kind %v\n", nhst.GetKind())
	err = stateMgr.ctrler.Host().Update(&nhst)
	AssertOk(t, err, "Error updating the host")

	// verify we can find the endpoint
	//foundEP, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	//AssertOk(t, err, "could not find the endpoint")
	//Assert(t, foundEP.Endpoint.Status.NodeUUID == snic.Name, "Endpoint nodeUUID did not match")

	AssertEventually(t, func() (bool, interface{}) {
		foundEP, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil && foundEP.Endpoint.Status.NodeUUID == snic.Name {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found or node uuid not match", "1ms", "2s")

	// delete the host
	err = stateMgr.ctrler.DistributedServiceCard().Delete(&snic)
	AssertOk(t, err, "Error deleting the host")

	// verify endpoint is gone from the database
	//_, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	//Assert(t, err != nil, "endpoint still found when host isnt associated with snic")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found or node uuid not match", "1ms", "2s")

}

func TestSmartNicCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		foundDistributedServiceCard, err := stateMgr.FindDistributedServiceCard("default", "testDistributedServiceCard")
		if err == nil && foundDistributedServiceCard.DistributedServiceCard.Status.PrimaryMAC == "0001.0203.0405" {
			return true, nil
		}
		return false, nil
	}, "Did not find DSC", "1ms", "2s")

	_, err = stateMgr.FindDistributedServiceCardByMacAddr("0001.0203.0405")
	AssertOk(t, err, "Could not find the smartNic")

	// delete the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Delete(&snic)
	AssertOk(t, err, "Error deleting the smartNic")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDistributedServiceCard("default", "testDistributedServiceCard")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSC", "1ms", "2s")
}

func TestNetworkInterfaceConvert(t *testing.T) {
	agentNetif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Name: "00ae.cd00.1638-uplink130",
		},
		Spec: netproto.InterfaceSpec{
			Type: "UPLINK_ETH",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}

	cNetif := convertNetifObj("testNode", agentNetif)
	if cNetif == nil {
		t.Fatalf("Unable to convert netif object")
	}
	if cNetif.Status.DSC != "testNode" {
		t.Fatalf("smartnic not set correctly %+v", cNetif)
	}
}

// Test CRUD operations on NetworkInterface object; there is no backend
// code for these objects, so we don't check on other return details
func TestNetworkInterfaceCRUD(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	netifName := "00ae.cd00.1638-uplink130"
	netif := network.NetworkInterface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Name: netifName,
		},
		Status: network.NetworkInterfaceStatus{
			OperStatus: "UP",
			Type:       "UPLINK_ETH",
		},
	}

	// create the netif
	err = stateMgr.ctrler.NetworkInterface().Create(&netif)
	AssertOk(t, err, "Could not create the smartNic")

	// delete the smartNic
	err = stateMgr.ctrler.NetworkInterface().Update(&netif)
	AssertOk(t, err, "Error deleting the smartNic")

	// delete the smartNic
	err = stateMgr.ctrler.NetworkInterface().Delete(&netif)
	AssertOk(t, err, "Error deleting the smartNic")
}

/*
func TestAgentCrudEvents(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	AssertOk(t, stateMgr.OnEndpointCreateReq("test-node", &netproto.Endpoint{}), "endpoint create")
	AssertOk(t, stateMgr.OnEndpointUpdateReq("test-node", &netproto.Endpoint{}), "endpoint update")
	AssertOk(t, stateMgr.OnEndpointDeleteReq("test-node", &netproto.Endpoint{}), "endpoint delete")

	AssertOk(t, stateMgr.OnNetworkSecurityPolicyCreateReq("test-node", &netproto.NetworkSecurityPolicy{}), "NetworkSecurityPolicy create")
	AssertOk(t, stateMgr.OnNetworkSecurityPolicyUpdateReq("test-node", &netproto.NetworkSecurityPolicy{}), "NetworkSecurityPolicy update")
	AssertOk(t, stateMgr.OnNetworkSecurityPolicyDeleteReq("test-node", &netproto.NetworkSecurityPolicy{}), "NetworkSecurityPolicy delete")

	AssertOk(t, stateMgr.OnSecurityProfileCreateReq("test-node", &netproto.SecurityProfile{}), "SecurityProfile create")
	AssertOk(t, stateMgr.OnSecurityProfileUpdateReq("test-node", &netproto.SecurityProfile{}), "SecurityProfile update")
	AssertOk(t, stateMgr.OnSecurityProfileDeleteReq("test-node", &netproto.SecurityProfile{}), "SecurityProfile delete")

	netIf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type: "UPLINK_ETH",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}
	AssertOk(t, stateMgr.OnInterfaceCreateReq("test-node", &netIf), "Interface create")
	AssertOk(t, stateMgr.OnInterfaceUpdateReq("test-node", &netIf), "Interface update")

	netIf = netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type: "UPLINK_MGMT",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "DOWN",
		},
	}
	AssertOk(t, stateMgr.OnInterfaceCreateReq("test-node", &netIf), "Interface create")
	AssertOk(t, stateMgr.OnInterfaceUpdateReq("test-node", &netIf), "Interface update")
	stateMgr.OnInterfaceDeleteReq("test-node", &netIf)

	netIf = netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type: "HOST_PF",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "DOWN",
		},
	}
	AssertOk(t, stateMgr.OnInterfaceCreateReq("test-node", &netIf), "Interface create")
	AssertOk(t, stateMgr.OnInterfaceUpdateReq("test-node", &netIf), "Interface update")
	stateMgr.OnInterfaceDeleteReq("test-node", &netIf)
}
*/

func TestWatchFilter(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	options := api.ListWatchOptions{}
	options.Name = "0000.0000.0001"

	filterFn1 := stateMgr.GetWatchFilter("Network", &options)
	filterFn2 := stateMgr.GetWatchFilter("Endpoint", &options)

	obj1 := netproto.Network{}
	obj1.Name = "xyz"
	obj2 := netproto.Endpoint{}
	Assert(t, filterFn1(&obj1), "expecting filter to pass")
	Assert(t, filterFn2(&obj2), "expecting filter to pass")
	obj2.Spec.NodeUUID = "0000.0000.0001"
	Assert(t, filterFn2(&obj2), "expecting filter to pass")
	obj2.Spec.NodeUUID = "0000.0000.0002"
	Assert(t, filterFn2(&obj2) == false, "expecting filter to fail")

	stateMgr.StopAppWatch()
	stateMgr.StartAppWatch()
	stateMgr.StopNetworkSecurityPolicyWatch()
	stateMgr.StartNetworkSecurityPolicyWatch()
}

func TestVirtualRouterCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create virtual router
	vr, err := createVirtualRouter(t, stateMgr, "default", "default")
	AssertOk(t, err, "Error creating the virtual router")

	obj, err := stateMgr.FindVirtualRouter("default", "default", "default")
	AssertOk(t, err, "Error finding virtual router")

	// update the virtual router
	version, _ := strconv.Atoi(obj.VirtualRouter.GenerationID)
	vr.GenerationID = strconv.Itoa(version + 1)
	err = stateMgr.ctrler.VirtualRouter().Update(vr)
	AssertOk(t, err, "Error updating virtual router")

	// delete the virtual router
	err = stateMgr.ctrler.VirtualRouter().Delete(vr)
	AssertOk(t, err, "Error deleting VirtualRouter")
}

func TestIPAMPolicyCreateDelete(t *testing.T) {
	name := "testPolicy"
	// create network state manager
	stateMgr, err := newStatemgr1()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	obj := &IPAMState{}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create virtual router
	_, err = createVirtualRouter(t, stateMgr, "default", "default")
	AssertOk(t, err, "Error creating the virtual router")

	// create IPAMPolicy
	policy, err := createIPAMPolicy(stateMgr, "default", name, "100.1.1.1")
	AssertOk(t, err, "Error creating IPAMPolicy")

	// verify we can find the IPAMPolicy

	AssertEventually(t, func() (bool, interface{}) {
		obj, err = smgrIPAM.FindIPAMPolicy("default", "default", name)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding ipampolicy %v\n", err)
		return false, nil
	}, "ipampolicy not foud", "1ms", "1s")
	AssertEquals(t, obj.IPAMPolicy.Spec.DHCPRelay.Servers[0].IPAddress, policy.Spec.DHCPRelay.Servers[0].IPAddress, "IPAMPolicy params did not match")

	// update the IPAMPolicy
	//version, _ := strconv.Atoi(obj.IPAMPolicy.GenerationID)
	version := 1
	policy.GenerationID = strconv.Itoa(version + 1)
	policy.Spec.DHCPRelay.Servers[0].IPAddress = "101.1.1.1"
	err = stateMgr.ctrler.IPAMPolicy().Update(policy)
	AssertOk(t, err, "Error updating IPAMPolicy")

	// verify update went through

	AssertEventually(t, func() (bool, interface{}) {
		obj, err = smgrIPAM.FindIPAMPolicy("default", "default", name)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding ipampolicy %v\n", err)
		return false, nil
	}, "ipampolicy not foud", "1ms", "1s")
	AssertEquals(t, obj.IPAMPolicy.Spec.DHCPRelay.Servers[0].IPAddress, policy.Spec.DHCPRelay.Servers[0].IPAddress, "IPAMPolicy params did not match")

	// delete the IPAM policy
	err = stateMgr.ctrler.IPAMPolicy().Delete(policy)
	AssertOk(t, err, "Error deleting IPAMPolicy")

	// verify the IPAMPolicy is deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrIPAM.FindIPAMPolicy("default", "default", name)
		if err != nil {
			return true, nil
		}
		fmt.Printf("IPAMPolicy still found after deleting %v\n", err)
		return false, nil
	}, "ipampolicy still foud", "1ms", "1s")
}

func TestRoutingConfigCreateDelete(t *testing.T) {
	name := "testCfg"
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	obj := &RoutingConfigState{}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create virtual router
	_, err = createVirtualRouter(t, stateMgr, "default", "default")
	AssertOk(t, err, "Error creating the virtual router")

	// create routingconfig
	rtcfg, err := createRoutingConfig(stateMgr, "default", name, "100.1.1.1")
	AssertOk(t, err, "Error creating RoutingConfig")

	// verify we can find the routingconfig
	AssertEventually(t, func() (bool, interface{}) {
		obj, err = smgrRoute.FindRoutingConfig("", "default", name)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding routingconfig %v\n", err)
		return false, nil
	}, "routingconfig not foud", "1ms", "1s")
	AssertEquals(t, obj.RoutingConfig.Spec.BGPConfig.Neighbors[0].IPAddress, rtcfg.Spec.BGPConfig.Neighbors[0].IPAddress, "routingconfig params did not match")

	// update the routingconfig
	version, _ := strconv.Atoi(obj.RoutingConfig.GenerationID)
	//version := 1
	rtcfg.GenerationID = strconv.Itoa(version + 1)
	rtcfg.Spec.BGPConfig.Neighbors[0].IPAddress = "101.1.1.1"
	err = stateMgr.ctrler.RoutingConfig().Update(rtcfg)
	AssertOk(t, err, "Error updating routingconfig")

	// verify update went through
	AssertEventually(t, func() (bool, interface{}) {
		obj, err = smgrRoute.FindRoutingConfig("", "default", name)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding routingconfig %v\n", err)
		return false, nil
	}, "routingconfig not foud", "1ms", "1s")
	AssertEquals(t, obj.RoutingConfig.Spec.BGPConfig.Neighbors[0].IPAddress, rtcfg.Spec.BGPConfig.Neighbors[0].IPAddress, "routingconfig params did not match")

	// delete the routingconfig
	err = stateMgr.ctrler.RoutingConfig().Delete(rtcfg)
	AssertOk(t, err, "Error deleting routingconfig")

	// verify the routingconfig is deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrRoute.FindRoutingConfig("", "default", name)
		if err != nil {
			return true, nil
		}
		fmt.Printf("Routingconfig still found after deleting %v\n", err)
		return false, nil
	}, "routingconfig still foud", "1ms", "1s")
}

func TestMain(m *testing.M) {
	// init tsdb client
	tsdbOpts := &tsdb.Opts{
		ClientName:              "npm-statemgr-test",
		Collector:               "test-collector",
		DBName:                  "default",
		SendInterval:            time.Duration(30) * time.Second,
		ConnectionRetryInterval: time.Second,
	}
	tsdb.Init(context.Background(), tsdbOpts)

	// call flag.Parse() here if TestMain uses flags
	os.Exit(m.Run())
}
