// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	"fmt"
	"os"
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

// createNetwork utility function to create a network
func createNetwork(stateMgr *Statemgr, tenant, net, subnet, gw string) error {
	// network params
	np := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.NetworkSpec{
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
func createTenant(stateMgr *Statemgr, tenant string) error {
	// network params
	tn := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: tenant,
		},
	}

	// create a network
	return stateMgr.ctrler.Tenant().Create(&tn)
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

// TestNetworkCreateDelete tests network create
func TestNetworkCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	// verify network got created
	nw, err := stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")
	Assert(t, (nw.Network.Spec.IPv4Subnet == "10.1.1.0/24"), "Network subnet did not match", nw)
	Assert(t, (nw.Network.Spec.IPv4Gateway == "10.1.1.254"), "Network gateway did not match", nw)
	Assert(t, (nw.Network.TypeMeta.Kind == "Network"), "Network type meta did not match", nw)

	// verify can not create networks with invalid params
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.255")
	Assert(t, (err != nil), "Network with invalid gateway got created", "10.1.1.255")
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.0")
	Assert(t, (err != nil), "Network with invalid gateway got created", "10.1.1.0")
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.2.254")
	Assert(t, (err != nil), "Network with invalid gateway got created", "10.1.2.254")
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.25424")
	Assert(t, (err != nil), "Network with invalid gateway got created", "10.1.1.254/24")
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.10-20/24", "10.1.1.254")
	Assert(t, (err != nil), "Network with invalid subnet got created", "10.1.1.10-20/24")

	// verify we cant create duplicate networks
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Network update failed")

	// delete the network
	err = stateMgr.ctrler.Network().Delete(&nw.Network.Network)
	AssertOk(t, err, "Failed to delete the network")

	// verify network is deleted from the db
	_, err = stateMgr.FindNetwork("default", "default")
	Assert(t, (err != nil), "Network still found after its deleted")

}

func TestNetworkList(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

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

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

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

	// verify endpoint is gone from the database
	_, err = stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err != nil), "Deleted endpoint still found in network db", "testEndpoint")

	// verify deleting non existing endpoint returns an error
	err = stateMgr.ctrler.Endpoint().Delete(&epinfo)
	Assert(t, (err != nil), "Deleting non existing endpoint succeeded", epinfo)

	// delete the second endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&newEP)
	Assert(t, (err == nil), "Error deleting the endpoint", newEP)

}

func TestEndpointStaleDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

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

	// verify we can find the network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
	AssertOk(t, err, "Could not find the network")

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
	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/30", "10.1.1.2")
	AssertOk(t, err, "Error creating the network")

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

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)
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
	Assert(t, (err != nil), "Endpoint creation should have failed", newEP)

	// delete the first endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&epinfo)
	Assert(t, (err == nil), "Error deleting the endpoint", epinfo)

	// verify now the creation goes thru
	err = stateMgr.ctrler.Endpoint().Create(&newEP)
	AssertOk(t, err, "Second endpoint creation failed")
	nep, err := stateMgr.FindEndpoint("default", "newEndpoint")
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
	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

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

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

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
	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

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
	// create sg
	sg, err := createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	// verify we can find the sg
	sgs, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Could not find the security group")
	AssertEquals(t, sgs.SecurityGroup.Spec.WorkloadSelector.String(), sg.Spec.WorkloadSelector.String(), "Security group params did not match")

	// delete the security group
	err = stateMgr.ctrler.SecurityGroup().Delete(&sgs.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

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
	// create sg
	_, err = createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")
	sg, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Error finding sg")

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

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

	// verify endpoint is associated with the sg
	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 1), "Sg was not linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 1), "Sg was not linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[0] == sg.SecurityGroup.Name), "Sg was not linked to endpoint", eps)
	Assert(t, (len(sg.SecurityGroup.Status.Workloads) == 1), "Endpoint is not linked to sg", sg)
	Assert(t, (len(sg.endpoints) == 1), "Endpoint is not linked to sg", sg)
	Assert(t, (sg.SecurityGroup.Status.Workloads[0] == eps.Endpoint.Name), "Endpoint is not linked to sg", sg)

	// create a new sg
	_, err = createSg(stateMgr, "default", "testSg2", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")
	sg2, err := stateMgr.FindSecurityGroup("default", "testSg2")
	AssertOk(t, err, "Error finding sg")

	// verify endpoint is associated with the new sg too..
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 2), "Sg was not linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 2), "Sg was not linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[1] == sg2.SecurityGroup.Name), "Sg was not linked to endpoint", eps)
	Assert(t, (len(sg2.SecurityGroup.Status.Workloads) == 1), "Endpoint is not linked to sg", sg2)
	Assert(t, (len(sg2.endpoints) == 1), "Endpoint is not linked to sg", sg2)
	Assert(t, (sg2.SecurityGroup.Status.Workloads[0] == eps.Endpoint.Name), "Endpoint is not linked to sg", sg2)

	// delete the second sg
	err = stateMgr.ctrler.SecurityGroup().Delete(&sg2.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	// verify sg is removed from endpoint
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 1), "Sg is still linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 1), "Sg is still linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[0] == sg.SecurityGroup.Name), "Sg is still linked to endpoint", eps)

	// delete the endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&eps.Endpoint.Endpoint)
	AssertOk(t, err, "Error deleting endpoint")

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
	// create sg with nil selector
	_, err = createSg(stateMgr, "default", "testSg", nil)
	AssertOk(t, err, "Error creating security group")
	_, err = stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Error finding sg")

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

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

	// verify endpoint is created
	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)

	// create a new sg with nil selector
	_, err = createSg(stateMgr, "default", "testSg2", nil)
	AssertOk(t, err, "Error creating security group")
	sg2, err := stateMgr.FindSecurityGroup("default", "testSg2")
	AssertOk(t, err, "Error finding sg")

	// delete the second sg
	err = stateMgr.ctrler.SecurityGroup().Delete(&sg2.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	// delete the endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&eps.Endpoint.Endpoint)
	AssertOk(t, err, "Error deleting endpoint")
}

// test concurrent add/delete/change of endpoints
func TestEndpointConcurrency(t *testing.T) {
	var concurrency = 100

	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

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
			waitCh <- serr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error deleting sgs")
	}
	// delete endpoint
	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			ometa := workload.Endpoint{
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
	err = createTenant(stateMgr, "testTenant")
	AssertOk(t, err, "Error creating the tenant")

	// verify tenant got created
	tn, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Error finding the tenant")
	AssertEquals(t, tn.Tenant.Name, "testTenant", "Did not match expected tenant name")

	// create networks
	err = createNetwork(stateMgr, "testTenant", "testNetwork1", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating network")
	err = createNetwork(stateMgr, "testTenant", "testNetwork2", "192.168.1.1/24", "192.168.1.254")
	AssertOk(t, err, "Error creating network")

	// create security groups
	sg1, err := createSg(stateMgr, "testTenant", "testSg1", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")
	sg2, err := createSg(stateMgr, "testTenant", "testSg2", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	// create endpoints
	_, err = createEndpoint(stateMgr, "testTenant", "testEndpoint1", "testNetwork1")
	AssertOk(t, err, "Error creating endpoint")
	_, err = createEndpoint(stateMgr, "testTenant", "testEndpoint2", "testNetwork2")
	AssertOk(t, err, "Error creating endpoint")

	// delete the tenant
	err = stateMgr.ctrler.Tenant().Delete(&tn.Tenant.Tenant)
	AssertOk(t, err, "Deleting tenant failed.")

	// verify tenant is deleted from the db
	_, err = stateMgr.FindTenant("testTenant")
	Assert(t, err != nil, "Tenant still found after its deleted")

	// delete resources
	nw1, _ := stateMgr.FindNetwork("testTenant", "testNetwork1")
	nw2, _ := stateMgr.FindNetwork("testTenant", "testNetwork2")

	ep1, _ := stateMgr.FindEndpoint("testTenant", "testEndpoint1")
	ep2, _ := stateMgr.FindEndpoint("testTenant", "testEndpoint2")

	ep1.Delete()
	ep2.Delete()

	err = stateMgr.OnEndpointDelete(ep1.Endpoint)
	AssertOk(t, err, "Could not delete endpoint")
	err = stateMgr.OnEndpointDelete(ep2.Endpoint)
	AssertOk(t, err, "Could not delete endpoint")

	err = stateMgr.ctrler.SecurityGroup().Delete(sg1)
	AssertOk(t, err, "Error deleting the security group.")
	err = stateMgr.ctrler.SecurityGroup().Delete(sg2)
	AssertOk(t, err, "Error deleting the security group.")

	err = stateMgr.ctrler.Network().Delete(&nw1.Network.Network)
	AssertOk(t, err, "Error deleting the network.")
	err = stateMgr.ctrler.Network().Delete(&nw2.Network.Network)
	AssertOk(t, err, "Error deleting the network.")
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

	// verify we can find the network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
	AssertOk(t, err, "Could not find the network")

	// verify we can find the endpoint associated with the workload
	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0405")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&wr)
	AssertOk(t, err, "Error deleting the workload")

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

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// update workload external vlan
	nwr := ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.Interfaces[0].ExternalVlan = 2
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	// verify we can find the new network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-2")
	AssertOk(t, err, "Could not find updated network")

	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0405")
	Assert(t, (foundEp.Endpoint.Status.Network == nw.Network.Name), "endpoint network did not match")

	// change mac address of the workload
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].MACAddress = "0001.0201.0203"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	// verify old endpoint is deleted
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	Assert(t, (err != nil), "found endpoint with old mac address", foundEp)
	_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, (ok == false), "old endpoint still found in network db", "testWorkload-0001.0203.0405")

	// verify new endpoint is created
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0203")
	AssertOk(t, err, "Could not find the new endpoint")

	// change useg vlan
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].MicroSegVlan = 101
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	// verify new endpoint has new useg vlan
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0203")
	AssertOk(t, err, "Could not find the new endpoint")
	Assert(t, (foundEp.Endpoint.Status.MicroSegmentVlan == 101), "endpoint useg vlan did not match")

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

	// verify we can find the new endpoint
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0406.0800")
	AssertOk(t, err, "Could not find the new endpoint")
	Assert(t, (foundEp.Endpoint.Status.MicroSegmentVlan == 200), "endpoint useg vlan did not match")

	// delete second interface
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces = nwr.Spec.Interfaces[0:1]
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	// verify endpoint is gone
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0406.0800")
	Assert(t, (err != nil), "found endpoint for deleted interface", foundEp)

	// trigger a dummy and verify endpoint is not deleted
	nwr.ObjectMeta.GenerationID = "2"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0203")
	AssertOk(t, err, "Could not find the new endpoint")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&nwr)
	AssertOk(t, err, "Error deleting the workload")

	// verify endpoint is gone from the database
	_, ok = nw.FindEndpoint("testWorkload-0001.0201.0203")
	Assert(t, (ok == false), "Deleted endpoint still found in network db", "testWorkload-0001.0201.0203")
}

func TestWorkloadUpdateHost(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

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

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

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

	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.HomingHostName == host2.Name), "endpoint host did not match")
	Assert(t, (foundEp.Endpoint.Status.NodeUUID == snic2.Name), "endpoint params did not match")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&nwr)
	AssertOk(t, err, "Error deleting the workload")

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

	// find the second workload
	fwr2, err := stateMgr.FindWorkload("default", "testWorkload2")
	AssertOk(t, err, "Could not find the second workload")
	Assert(t, (fwr2.Workload.Status.PropagationStatus.Status == "Propagation Failed. Duplicate MAC address"), "Invalid workload status", fwr2.Workload.Status.PropagationStatus.Status)

	// verify endpoint is not created
	_, ok = nw.FindEndpoint("testWorkload2-0001.0203.0405")
	Assert(t, (ok == false), "Duplicate endpoint still found in network db", "testWorkload2-0001.0203.0405")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&wr)
	AssertOk(t, err, "Error deleting the workload")

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

	// verify we can find the endpoint associated with the host
	foundHost, err := stateMgr.FindHost("default", "testHost")
	AssertOk(t, err, "Could not find the host")
	Assert(t, (len(foundHost.Host.Spec.DSCs) == 1), "host params did not match")

	// delete the host
	err = stateMgr.ctrler.Host().Delete(&host)
	AssertOk(t, err, "Error deleting the host")

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

	foundEP, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	AssertOk(t, err, "could not find the endpoint")
	Assert(t, foundEP.Endpoint.Status.NodeUUID == snic.Name, "Endpoint nodeUUID did not match")

	// change host's mac address
	nhst := ref.DeepCopy(host).(cluster.Host)
	nhst.Spec.DSCs[0].MACAddress = "0002.0406.0800"
	err = stateMgr.ctrler.Host().Update(&nhst)
	AssertOk(t, err, "Error updating the host")

	// verify the endpoint is gone
	_, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	Assert(t, err != nil, "endpoint still found when host isnt associated with snic")

	// now associate by name
	nhst = ref.DeepCopy(nhst).(cluster.Host)
	nhst.Spec.DSCs[0].MACAddress = ""
	nhst.Spec.DSCs[0].ID = "test-snic"
	err = stateMgr.ctrler.Host().Update(&nhst)
	AssertOk(t, err, "Error updating the host")

	// verify we can find the endpoint
	foundEP, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	AssertOk(t, err, "could not find the endpoint")
	Assert(t, foundEP.Endpoint.Status.NodeUUID == snic.Name, "Endpoint nodeUUID did not match")

	// delete the host
	err = stateMgr.ctrler.DistributedServiceCard().Delete(&snic)
	AssertOk(t, err, "Error deleting the host")

	// verify endpoint is gone from the database
	_, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	Assert(t, err != nil, "endpoint still found when host isnt associated with snic")
}

func TestSmartNicCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

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

	// verify we can find the endpoint associated with the smartNic
	foundDistributedServiceCard, err := stateMgr.FindDistributedServiceCard("default", "testDistributedServiceCard")
	AssertOk(t, err, "Could not find the smartNic")
	Assert(t, (foundDistributedServiceCard.DistributedServiceCard.Status.PrimaryMAC == "0001.0203.0405"), "smartNic params did not match")

	foundDistributedServiceCard, err = stateMgr.FindDistributedServiceCardByMacAddr("0001.0203.0405")
	AssertOk(t, err, "Could not find the smartNic")

	// delete the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Delete(&snic)
	AssertOk(t, err, "Error deleting the smartNic")

	// verify endpoint is gone from the database
	_, err = stateMgr.FindDistributedServiceCard("default", "testDistributedServiceCard")
	Assert(t, (err != nil), "Deleted smartNic still found in db")
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
		TypeMeta: api.TypeMeta{Kind: "Interface"},
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
		TypeMeta: api.TypeMeta{Kind: "Interface"},
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
		TypeMeta: api.TypeMeta{Kind: "Interface"},
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

func TestWatchFilter(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	ometa := api.ObjectMeta{
		Name: "0000.0000.0001",
	}
	filterFn1 := stateMgr.GetWatchFilter("Network", &ometa)
	filterFn2 := stateMgr.GetWatchFilter("Endpoint", &ometa)

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
