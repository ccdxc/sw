// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"testing"
	"time"

	"github.com/Sirupsen/logrus"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/utils/memdb"
	. "github.com/pensando/sw/utils/testutils"
)

// createNetwork utility function to create a network
func createNetwork(stateMgr *Statemgr, tenant, net, subnet, gw string) error {
	// network params
	np := network.Network{
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

	// create a network
	return stateMgr.CreateNetwork(&np)
}

func createSg(stateMgr *Statemgr, tenant, sgname string, selectors []string) (*network.SecurityGroup, error) {
	// sg object
	sg := network.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tenant,
			Name:   sgname,
		},
		Spec: network.SecurityGroupSpec{
			WorkloadSelector: selectors,
		},
	}

	// create sg
	err := stateMgr.CreateSecurityGroup(&sg)

	return &sg, err
}

// TestNetworkCreateDelete tests network create
func TestNetworkCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr()
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
	Assert(t, (nw.Spec.IPv4Subnet == "10.1.1.0/24"), "Network subnet did not match", nw)
	Assert(t, (nw.Spec.IPv4Gateway == "10.1.1.254"), "Network gateway did not match", nw)
	Assert(t, (nw.TypeMeta.Kind == "Network"), "Network type meta did not match", nw)

	// verify can not create networks with invalid params
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.255")
	Assert(t, (err != nil), "Network with invalid gateway got created", "10.1.1.255")
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.0")
	Assert(t, (err != nil), "Network with invalid gateway got created", "10.1.1.0")
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.2.254")
	Assert(t, (err != nil), "Network with invalid gateway got created", "10.1.2.254")
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.254/24")
	Assert(t, (err != nil), "Network with invalid gateway got created", "10.1.1.254/24")
	err = createNetwork(stateMgr, "default", "invalid", "10.1.1.10-20/24", "10.1.1.254")
	Assert(t, (err != nil), "Network with invalid subnet got created", "10.1.1.10-20/24")

	// verify we cant create duplicate networks
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	Assert(t, (err != nil), "Duplicate network creation succeeded")

	// delete the network
	err = stateMgr.DeleteNetwork("default", "default")
	AssertOk(t, err, "Failed to delete the network")

	// verify network is deleted from the db
	_, err = stateMgr.FindNetwork("default", "default")
	Assert(t, (err != nil), "Network still found after its deleted")
}

func TestNetworkListWatch(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// start a watch
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	stateMgr.WatchObjects("Network", watchChan)

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	// verify we get a watch event
	select {
	case wnt, ok := <-watchChan:
		Assert(t, ok, "Error reading from channel", wnt)
		Assert(t, (wnt.Obj.GetObjectMeta().Name == "default"), "Received invalid network", wnt)
		logrus.Infof("Received network watch {%+v}", wnt)
	case <-time.After(time.Second):
		t.Fatalf("Timed out while waiting for channel event")
	}

	// verify the list works
	nets, err := stateMgr.ListNetworks()
	AssertOk(t, err, "Error listing networks")
	Assert(t, (len(nets) == 1), "Incorrect number of networks received", nets)
	Assert(t, (nets[0].Network.Name == "default"), "Incorrect network received", nets[0])
}

// TestEndpointCreateDelete tests network create
func TestEndpointCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	// endpoint params
	epinfo := network.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "",
			Tenant:    "default",
		},
		Spec: network.EndpointSpec{},
		Status: network.EndpointStatus{
			EndpointUUID:   "testEndpointUUID",
			WorkloadUUID:   "testContainerUUID",
			WorkloadName:   "testContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	// find the network
	nw, err := stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	eps, err := nw.CreateEndpoint(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)
	Assert(t, (eps.TypeMeta.Kind == "Endpoint"), "Endpoint type meta did not match", eps)
	Assert(t, (eps.Status.IPv4Address == "10.1.1.1/24"), "Endpoint address did not match", eps)
	Assert(t, (eps.Status.IPv4Gateway == "10.1.1.254"), "Endpoint gateway did not match", eps)

	// verify we can find the endpoint
	foundEp, ok := nw.FindEndpoint("testEndpoint")
	Assert(t, ok, "Could not find the endpoint", "testEndpoint")
	Assert(t, (foundEp == eps), "Created and found eps did not match", []*EndpointState{eps, foundEp})

	// verify can not delete a network when it still has endpoints
	err = stateMgr.DeleteNetwork("default", "default")
	Assert(t, (err != nil), "Was able to delete network with endpoint")

	// verify you cant create duplicate endpoints
	_, err = nw.CreateEndpoint(&epinfo)
	Assert(t, (err != nil), "Was able to create duplicate endpoint", epinfo)

	// verify creating new EP after ip addr have run out fails
	newEP := network.Endpoint{
		ObjectMeta: api.ObjectMeta{
			Name:      "newEndpoint",
			Namespace: "",
			Tenant:    "default",
		},
		Spec: network.EndpointSpec{},
		Status: network.EndpointStatus{
			EndpointUUID:   "newEndpointUUID",
			WorkloadUUID:   "newContainerUUID",
			WorkloadName:   "newContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}
	nep, err := nw.CreateEndpoint(&newEP)
	AssertOk(t, err, "Endpoint failed")
	Assert(t, (nep.Status.IPv4Address == "10.1.1.2/24"), "Endpoint address did not match", eps)

	// delete the endpoint
	deletedEp, err := nw.DeleteEndpoint(&epinfo.ObjectMeta)
	Assert(t, (err == nil), "Error deleting the endpoint", epinfo)
	Assert(t, (deletedEp == eps), "Deleted and created eps did not match", []*EndpointState{eps, deletedEp})

	// verify endpoint is gone from the database
	_, ok = nw.FindEndpoint("testEndpoint")
	Assert(t, (ok == false), "Deleted endpoint still found in network db", "testEndpoint")

	// verify deleting non existing endpoint returns an error
	_, err = nw.DeleteEndpoint(&epinfo.ObjectMeta)
	Assert(t, (err != nil), "Deleting non existing endpoint succeded", epinfo)

	// delete the second endpoint
	_, err = nw.DeleteEndpoint(&newEP.ObjectMeta)
	Assert(t, (err == nil), "Error deleting the endpoint", epinfo)
}

func TestEndpointCreateFailure(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/30", "10.1.1.2")
	AssertOk(t, err, "Error creating the network")

	// endpoint params
	epinfo := network.Endpoint{
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "",
			Tenant:    "default",
		},
		Status: network.EndpointStatus{
			EndpointUUID:   "testEndpointUUID",
			WorkloadUUID:   "testContainerUUID",
			WorkloadName:   "testContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	// find the network
	nw, err := stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	eps, err := nw.CreateEndpoint(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)
	Assert(t, (eps.Status.IPv4Address == "10.1.1.1/30"), "Endpoint address did not match", eps)

	// verify creating new EP after ip addr have run out fails
	newEP := network.Endpoint{
		ObjectMeta: api.ObjectMeta{
			Name:      "newEndpoint",
			Namespace: "",
			Tenant:    "default",
		},
		Status: network.EndpointStatus{
			EndpointUUID:   "newEndpointUUID",
			WorkloadUUID:   "newContainerUUID",
			WorkloadName:   "newContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}
	nep, err := nw.CreateEndpoint(&newEP)
	Assert(t, (err != nil), "Endpoint creation should have failed", nep)

	// delete the first endpoint
	_, err = nw.DeleteEndpoint(&epinfo.ObjectMeta)
	Assert(t, (err == nil), "Error deleting the endpoint", epinfo)

	// verify now the creation goes thru
	nep, err = nw.CreateEndpoint(&newEP)
	AssertOk(t, err, "Second endpoint creation failed")
	Assert(t, (nep.Status.IPv4Address == "10.1.1.1/30"), "Endpoint address did not match", nep)
}

// TestEndpointListWatch tests ep watch and list
func TestEndpointListWatch(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	// start a watch on endpoint
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	stateMgr.WatchObjects("Endpoint", watchChan)

	// endpoint params
	epinfo := network.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "",
			Tenant:    "default",
		},
		Spec: network.EndpointSpec{},
		Status: network.EndpointStatus{
			EndpointUUID:   "testEndpointUUID",
			WorkloadUUID:   "testContainerUUID",
			WorkloadName:   "testContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	// find the network
	nw, err := stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	eps, err := nw.CreateEndpoint(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)

	// verify we get a watch event
	select {
	case wep, ok := <-watchChan:
		Assert(t, ok, "Error reading from channel", wep)
		Assert(t, (wep.Obj.GetObjectKind() == "Endpoint"), "Watch rcvd invalid object kind", wep)
		Assert(t, (wep.Obj.GetObjectMeta().Name == eps.Endpoint.Name), "Received invalid endpoint", wep)
		logrus.Infof("Received endpoint watch {%+v}", wep)
	case <-time.After(time.Second):
		t.Fatalf("Timed out while waiting for channel event")
	}

	// verify the list works
	nets, err := stateMgr.ListNetworks()
	AssertOk(t, err, "Error listing networks")
	Assert(t, (len(nets) == 1), "Incorrect number of networks received", nets)
	for _, nt := range nets {
		endps := nt.ListEndpoints()
		Assert(t, (len(endps) == 1), "Invalid number of endpoints received", eps)
		Assert(t, (endps[0].Endpoint.Name == eps.Endpoint.Name), "Invalid endpoint params received", endps[0])
	}

}

func TestSgCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create sg
	sg, err := createSg(stateMgr, "default", "testSg", []string{"env:production", "app:procurement"})
	AssertOk(t, err, "Error creating security group")

	// verify we can find the sg
	sgs, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Could not find the security group")
	AssertEquals(t, sgs.Spec.WorkloadSelector, sg.Spec.WorkloadSelector, "Security group params did not match")

	// delete the security group
	err = stateMgr.DeleteSecurityGroup("default", "testSg")
	AssertOk(t, err, "Error deleting security group")

	// verify the sg is gone
	_, err = stateMgr.FindSecurityGroup("default", "testSg")
	Assert(t, (err != nil), "Security group still found after deleting")
}

func TestSgAttachEndpoint(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create sg
	_, err = createSg(stateMgr, "default", "testSg", []string{"env:production", "app:procurement"})
	AssertOk(t, err, "Error creating security group")
	sg, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Error finding sg")

	// create a network
	err = createNetwork(stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	// endpoint object
	epinfo := network.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "",
			Tenant:    "default",
		},
		Spec: network.EndpointSpec{},
		Status: network.EndpointStatus{
			EndpointUUID:       "testEndpointUUID",
			WorkloadUUID:       "testContainerUUID",
			WorkloadName:       "testContainerName",
			Network:            "default",
			HomingHostAddr:     "192.168.1.1",
			HomingHostName:     "testHost",
			WorkloadAttributes: []string{"env:production", "app:procurement"},
		},
	}

	// find the network
	nw, err := stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	eps, err := nw.CreateEndpoint(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)

	// verify endpoint is associated with the sg
	Assert(t, (len(eps.Status.SecurityGroups) == 1), "Sg was not linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 1), "Sg was not linked to endpoint", eps)
	Assert(t, (eps.Status.SecurityGroups[0] == sg.Name), "Sg was not linked to endpoint", eps)
	Assert(t, (len(sg.Status.Workloads) == 1), "Endpoint is not linked to sg", sg)
	Assert(t, (len(sg.endpoints) == 1), "Endpoint is not linked to sg", sg)
	Assert(t, (sg.Status.Workloads[0] == eps.Name), "Endpoint is not linked to sg", sg)

	// create a new sg
	_, err = createSg(stateMgr, "default", "testSg2", []string{"env:production", "app:procurement"})
	AssertOk(t, err, "Error creating security group")
	sg2, err := stateMgr.FindSecurityGroup("default", "testSg2")
	AssertOk(t, err, "Error finding sg")

	// verify endpoint is associated with the new sg too..
	Assert(t, (len(eps.Status.SecurityGroups) == 2), "Sg was not linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 2), "Sg was not linked to endpoint", eps)
	Assert(t, (eps.Status.SecurityGroups[1] == sg2.Name), "Sg was not linked to endpoint", eps)
	Assert(t, (len(sg2.Status.Workloads) == 1), "Endpoint is not linked to sg", sg2)
	Assert(t, (len(sg2.endpoints) == 1), "Endpoint is not linked to sg", sg2)
	Assert(t, (sg2.Status.Workloads[0] == eps.Name), "Endpoint is not linked to sg", sg2)

	// delete the second sg
	err = stateMgr.DeleteSecurityGroup("default", "testSg2")
	AssertOk(t, err, "Error deleting security group")

	// verify sg is removed from endpoint
	Assert(t, (len(eps.Status.SecurityGroups) == 1), "Sg is still linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 1), "Sg is still linked to endpoint", eps)
	Assert(t, (eps.Status.SecurityGroups[0] == sg.Name), "Sg is still linked to endpoint", eps)

	// delete the endpoint
	_, err = nw.DeleteEndpoint(&eps.ObjectMeta)
	AssertOk(t, err, "Error deleting endpoint")

	// verify endpoint is removed from sg
	Assert(t, (len(sg.Status.Workloads) == 0), "Endpoint is still linked to sg", sg)
}

func TestSgpolicyCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create sg
	_, err = createSg(stateMgr, "default", "procurement", []string{"env:production", "app:procurement"})
	AssertOk(t, err, "Error creating security group")
	_, err = createSg(stateMgr, "default", "catalog", []string{"env:production", "app:catalog"})
	AssertOk(t, err, "Error creating security group")

	// sg policy
	sgp := network.Sgpolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "test-sgpolicy",
		},
		Spec: network.SgpolicySpec{
			AttachGroups: []string{"procurement"},
			InRules: []network.SGRule{
				{
					Ports:     "tcp/80",
					Action:    "allow",
					PeerGroup: "catalog",
				},
			},
		},
	}

	// create sg policy
	err = stateMgr.CreateSgpolicy(&sgp)
	AssertOk(t, err, "Error creating the sg policy")

	// verify we can find the sg policy
	sgps, err := stateMgr.FindSgpolicy("default", "test-sgpolicy")
	AssertOk(t, err, "Could not find the sg policy")
	AssertEquals(t, sgps.Spec.AttachGroups, sgp.Spec.AttachGroups, "Security policy params did not match")
	Assert(t, (len(sgps.groups) == 1), "Sg was not added to sgpolicy", sgps)
	Assert(t, sgps.groups["procurement"].Name == "procurement", "Sgpolicy is not linked to sg", sgps)

	// verify sg has the policy info
	prsg, err := stateMgr.FindSecurityGroup("default", "procurement")
	AssertOk(t, err, "Could not find security group")
	Assert(t, (len(prsg.policies) == 1), "sgpolicy was not added to sg", prsg)
	Assert(t, (prsg.policies[sgps.Name].Name == sgps.Name), "Sg is not linked to sgpolicy", prsg)
	Assert(t, (len(prsg.Status.Policies) == 1), "Policy not found in sg status", prsg)
	Assert(t, (prsg.Status.Policies[0] == sgps.Name), "Policy not found in sg status", prsg)

	// verify we can not attach a policy to unknown sg
	sgp2 := network.Sgpolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "sgpolicy2",
		},
		Spec: network.SgpolicySpec{
			AttachGroups: []string{"unknown"},
			InRules: []network.SGRule{
				{
					Ports:     "tcp/80",
					Action:    "allow",
					PeerGroup: "catalog",
				},
			},
		},
	}
	err = stateMgr.CreateSgpolicy(&sgp2)
	Assert(t, (err != nil), "Policy creation with unknown attachment suceeded")

	// delete the sg policy
	err = stateMgr.DeleteSgpolicy("default", "test-sgpolicy")
	AssertOk(t, err, "Error deleting security policy")

	// verify the sg policy is gone
	_, err = stateMgr.FindSgpolicy("default", "test-sgpolicy")
	Assert(t, (err != nil), "Security policy still found after deleting")

	// verify sgpolicy is unlinked from sg
	Assert(t, (len(prsg.policies) == 0), "sgpolicy was not removed sg", prsg)
	Assert(t, (len(prsg.Status.Policies) == 0), "sgpolicy was not removed sg", prsg)
}
