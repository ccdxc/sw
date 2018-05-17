// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"fmt"
	"testing"

	//"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	hal "github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type mockCtrler struct {
	epdb map[string]*netproto.Endpoint
}

func (ctrler *mockCtrler) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	key := objectKey(epinfo.ObjectMeta, epinfo.TypeMeta)
	ctrler.epdb[key] = epinfo
	return epinfo, nil
}

func (ctrler *mockCtrler) EndpointAgeoutNotif(epinfo *netproto.Endpoint) error {
	return nil
}

func (ctrler *mockCtrler) EndpointDeleteReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	key := objectKey(epinfo.ObjectMeta, epinfo.TypeMeta)
	delete(ctrler.epdb, key)
	return epinfo, nil
}

// createNetAgent creates a netagent scaffolding
func createNetAgent(t *testing.T) (*Nagent, *mockCtrler, *hal.Datapath) {
	dp, err := hal.NewHalDatapath("mock")
	if err != nil {
		t.Fatalf("could not create a mock datapath")
	}
	ct := &mockCtrler{
		epdb: make(map[string]*netproto.Endpoint),
	}

	// create new network agent
	nagent, err := NewNetAgent(dp, state.AgentMode_MANAGED, "", "some-unique-id")

	if err != nil {
		t.Fatalf("Error creating network agent. Err: %v", err)
		return nil, nil, nil
	}

	// fake controller intf
	nagent.RegisterCtrlerIf(ct)
	return nagent, ct, dp
}

func TestNetworkCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// make create network call
	err := ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")
	tnt, err := ag.FindNetwork(nt.ObjectMeta)
	AssertOk(t, err, "Network was not found in DB")
	Assert(t, (tnt.Spec.IPv4Subnet == "10.1.1.0/24"), "Network subnet did not match", tnt)

	// verify duplicate network creations succeed
	err = ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating duplicate network")

	// verify duplicate network name with different content does not succeed
	nnt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "20.2.2.0/24",
			IPv4Gateway: "20.2.2.254",
		},
	}
	err = ag.CreateNetwork(&nnt)
	Assert(t, (err != nil), "conflicting network creation succeeded")

	// verify list api works
	netList := ag.ListNetwork()
	Assert(t, (len(netList) == 1), "Incorrect number of networks")

	// delete the network and verify its gone from db
	err = ag.DeleteNetwork(&nt)
	AssertOk(t, err, "Error deleting network")
	_, err = ag.FindNetwork(nt.ObjectMeta)
	Assert(t, (err != nil), "Network was still found in database after deleting", ag)

	// verify you can not delete non-existing network
	err = ag.DeleteNetwork(&nt)
	Assert(t, (err != nil), "deleting non-existing network succeeded", ag)
}

func TestNetworkUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing tenant
	tn := &netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "updateTenant",
		},
	}

	err := ag.CreateTenant(tn)
	AssertOk(t, err, "Error creating tenant")

	// create backing namespace
	ns := &netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "updateTenant",
			Name:   "updateNamespace",
		},
	}

	err = ag.CreateNamespace(ns)
	AssertOk(t, err, "Error Creating Namespace")

	// create network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "updateTenant",
			Namespace: "updateNamespace",
			Name:      "updateNetwork",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.0.0.0/16",
			IPv4Gateway: "10.0.0.1",
			VlanID:      42,
		},
	}

	// create network
	err = ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")
	tnt, err := ag.FindNetwork(nt.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, tnt.Name == "updateNetwork", "Tenant names did not match", tnt)

	ntSpec := netproto.NetworkSpec{
		IPv4Subnet:  "192.168.1.1/24",
		IPv4Gateway: "192.168.1.254",
	}

	nt.Spec = ntSpec

	err = ag.UpdateNetwork(&nt)
	AssertOk(t, err, "Error updating network")
}

func TestCtrlerEndpointCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// make create network call
	err := ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")

	// endpoint message
	epinfo := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "10.0.0.1/16",
		},
	}

	// create the endpoint
	ep, _, err := ag.EndpointCreateReq(epinfo)
	AssertOk(t, err, "Error creating endpoint")
	var foundEp *netproto.Endpoint
	eps := ag.ListEndpoint()
	for _, e := range eps {
		if ep.Name == "testEndpoint" {
			foundEp = e
			break
		}
	}
	AssertEquals(t, epinfo, foundEp, "Agent should return the exact ep that we created.")

	//verify duplicate endpoint creations succeed
	_, _, err = ag.EndpointCreateReq(epinfo)
	AssertOk(t, err, "Endpoint creation is not idempotent")
	//
	// verify endpoint create on non-existing network fails
	ep2 := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint2",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "invalid",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "10.0.0.1/16",
		},
	}
	_, _, err = ag.EndpointCreateReq(ep2)
	Assert(t, err != nil, "Endpoint create on non-existing network succeeded", ag)

	// verify list api works
	epList := ag.ListEndpoint()
	Assert(t, len(epList) == 1, "Incorrect number of endpoints")
	//
	// endpoint message
	depinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID2",
			WorkloadUUID: "testWorkloadUUID2",
			NetworkName:  "default",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "10.0.0.1/16",
		},
	}
	_, _, err = ag.EndpointCreateReq(&depinfo)
	Assert(t, err != nil, "Conflicting endpoint creating succeeded", ag)

	// delete the endpoint
	err = ag.EndpointDeleteReq(epinfo)
	AssertOk(t, err, "Endpoint delete failed")

	// ensure that ep list returns 0 after delete
	// verify list api works
	epList = ag.ListEndpoint()
	AssertEquals(t, 0, len(epList), "Incorrect number of endpoints")

	// verify non-existing endpoint can not be deleted
	err = ag.EndpointDeleteReq(epinfo)
	Assert(t, err != nil, "Deleting non-existing endpoint succeeded", ag)
}

func TestSecurityGroupCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security group
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sg",
		},
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "unknown",
			Rules: []netproto.SecurityRule{
				{
					Direction: "Incoming",
					PeerGroup: "",
					Services: []netproto.SecurityRule_Service{
						{
							Protocol: "tcp",
							Port:     80,
						},
					},
					Action: "Allow",
				},
			},
		},
	}

	// create a security group
	err := ag.CreateSecurityGroup(&sg)
	AssertOk(t, err, "Error creating security group")

	// verify list api works
	sgList := ag.ListSecurityGroup()
	Assert(t, (len(sgList) == 1), "Incorrect number of sgs")

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// make create network call
	err = ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")

	// endpoint message
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:   "testEndpointUUID",
			WorkloadUUID:   "testWorkloadUUID",
			NetworkName:    "default",
			SecurityGroups: []string{"test-sg"},
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "10.0.0.1/24",
		},
	}

	// create endpoint referring to security group
	_, err = ag.CreateEndpoint(&epinfo)
	AssertOk(t, err, "Endpoint creation with security group failed")

	// try creating an endpoint with non-existing security group
	ep2 := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint2",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:   "testEndpointUUID",
			WorkloadUUID:   "testWorkloadUUID",
			NetworkName:    "default",
			SecurityGroups: []string{"test-sg", "unknown-sg"},
		},
	}
	_, err = ag.CreateEndpoint(&ep2)
	Assert(t, (err != nil), "Endpoint create with unknown sg succeeded", ep2)

	// delete sg
	err = ag.DeleteSecurityGroup(&sg)
	AssertOk(t, err, "Error deleting security group")
}

func TestEndpointUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// make create network call
	err := ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")

	// endpoint message
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "10.0.0.1/24",
		},
	}

	// create the endpoint
	_, err = ag.CreateEndpoint(&epinfo)
	AssertOk(t, err, "Error creating endpoint")

	// security group
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "test-sg",
			Namespace: "default",
		},
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "unknown",
			Rules: []netproto.SecurityRule{
				{
					Direction: "Incoming",
					PeerGroup: "",
					Services: []netproto.SecurityRule_Service{
						{
							Protocol: "tcp",
							Port:     80,
						},
					},
					Action: "Allow",
				},
			},
		},
	}

	// create a security group
	err = ag.CreateSecurityGroup(&sg)
	AssertOk(t, err, "Error creating security group")

	// update the endpoint
	epupd := epinfo
	epupd.Spec.SecurityGroups = []string{"test-sg"}
	err = ag.UpdateEndpoint(&epupd)
	AssertOk(t, err, "Error updating endpoint")

	// try changing the network of endpoint
	epupd2 := epupd
	epupd2.Spec.NetworkName = "unknown"
	err = ag.UpdateEndpoint(&epupd2)
	Assert(t, (err != nil), "Changing network name succeeded")

	// try updating security group to an unknown
	epupd2 = epupd
	epupd2.Spec.SecurityGroups = []string{"unknown"}
	err = ag.UpdateEndpoint(&epupd2)
	Assert(t, (err != nil), "Changing to non-existing security group succeeded")
}

func TestSecurityGroupUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security group
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sg",
		},
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "unknown",
			Rules: []netproto.SecurityRule{
				{
					Direction: "Incoming",
					PeerGroup: "",
					Action:    "Allow",
				},
			},
		},
	}

	// create a security group
	err := ag.CreateSecurityGroup(&sg)
	AssertOk(t, err, "Error creating security group")

	// create second security group that refers to first one
	sg2 := sg
	sg2.Name = "test-sg2"
	sg2.Spec.Rules = []netproto.SecurityRule{
		{
			Direction: "Incoming",
			PeerGroup: "test-sg",
			Action:    "Allow",
		},
	}
	err = ag.CreateSecurityGroup(&sg2)
	AssertOk(t, err, "Error creating security group")

	// update first sg
	sg.Spec.Rules = []netproto.SecurityRule{
		{
			Direction: "Incoming",
			PeerGroup: "test-sg2",
			Action:    "Allow",
		},
	}
	err = ag.UpdateSecurityGroup(&sg)
	AssertOk(t, err, "Error updating security group")

	// try to refer to a non-existing sg
	sg3 := sg2
	sg3.Spec.Rules = []netproto.SecurityRule{
		{
			Direction: "Incoming",
			PeerGroup: "unknown",
			Action:    "Allow",
		},
	}
	err = ag.UpdateSecurityGroup(&sg3)
	Assert(t, (err != nil), "Referring to unknown sg succeeded", sg3)

	// clear the peer group
	sg2.Spec.Rules = []netproto.SecurityRule{}
	err = ag.UpdateSecurityGroup(&sg2)
	AssertOk(t, err, "Error updating security group")

	// delete sg
	err = ag.DeleteSecurityGroup(&sg)
	AssertOk(t, err, "Error deleting security group")
	// delete sg
	err = ag.DeleteSecurityGroup(&sg2)
	AssertOk(t, err, "Error deleting security group")
}

func TestEndpointConcurrency(t *testing.T) {
	var concurrency = 100

	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// make create network call
	err := ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")

	waitCh := make(chan error, concurrency*2)

	// create endpoint
	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			// endpoint message
			epinfo := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      fmt.Sprintf("testEndpoint-%d", idx),
				},
				Spec: netproto.EndpointSpec{
					EndpointUUID: "testEndpointUUID",
					WorkloadUUID: "testWorkloadUUID",
					NetworkName:  "default",
				},
				Status: netproto.EndpointStatus{
					IPv4Address: "10.0.0.1/24",
				},
			}

			// create the endpoint
			_, eperr := ag.CreateEndpoint(&epinfo)
			waitCh <- eperr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error creating endpoint")
	}

	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			epinfo := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      fmt.Sprintf("testEndpoint-%d", idx),
				},
			}
			eperr := ag.DeleteEndpoint(&epinfo)
			waitCh <- eperr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error deleting endpoint")
	}

}

func TestTenantCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// tenant
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "default",
			Name:      "testTenant",
		},
	}

	// create tenant
	err := ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating tenant")
	tnt, err := ag.FindTenant(tn.Name)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, tnt.Name == "testTenant", "Tenant names did not match", tnt)

	// verify duplicate tenant creations succeed
	err = ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating duplicate tenant")

	// verify list api works. 2 accounts for the default tenant that gets created at agent startup
	tenantList := ag.ListTenant()
	Assert(t, len(tenantList) == 2, "Incorrect number of tenants")

	// delete the network and verify its gone from db
	err = ag.DeleteTenant(&tn)
	AssertOk(t, err, "Error deleting network")
	_, err = ag.FindTenant(tn.Name)
	Assert(t, err != nil, "Tenant was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteTenant(&tn)
	Assert(t, err != nil, "deleting non-existing network succeeded", ag)
}

func TestTenantUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// tenant
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "updateTenant",
			Namespace: "updateTenant",
			Name:      "updateTenant",
		},
	}

	// create tenant
	err := ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating tenant")
	tnt, err := ag.FindTenant(tn.Name)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, tnt.Name == "updateTenant", "Tenant names did not match", tnt)

	tnSpec := netproto.TenantSpec{
		Meta: &api.ObjectMeta{
			ResourceVersion: "v2",
		},
	}

	tn.Spec = tnSpec

	err = ag.UpdateTenant(&tn)
	AssertOk(t, err, "Error updating tenant")
}

func TestNetworkCreateOnNonExistentTenant(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "nonExistentNetwork",
			Namespace: "nonExistentNetwork",
			Name:      "default",
		},
	}

	// create network
	err := ag.CreateNetwork(&nt)
	Assert(t, err != nil, "Network create was expected to fail.")
}

// Tests lif, uplink and ENIC creates and deletes
func TestInterfacesCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	existingIfLen := len(ag.ListInterface())
	// lif
	lif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testLif",
		},
		Spec: netproto.InterfaceSpec{
			Type: "LIF",
		},
	}

	// ENIC
	enic := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type: "ENIC",
		},
	}

	// uplink
	uplink := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testUplink",
		},
		Spec: netproto.InterfaceSpec{
			Type: "UPLINK",
		},
	}

	// create lif
	err := ag.CreateInterface(lif)
	AssertOk(t, err, "Error creating lif")
	intf, err := ag.FindInterface(lif.ObjectMeta)
	AssertOk(t, err, "LIF was not found in DB")
	Assert(t, intf.Name == "testLif", "Lif names did not match", intf)

	// create enic
	err = ag.CreateInterface(enic)
	AssertOk(t, err, "Error creating ENIC")
	intf, err = ag.FindInterface(enic.ObjectMeta)
	AssertOk(t, err, "ENIC was not found in DB")
	Assert(t, intf.Name == "testEnic", "Enic names did not match", intf)

	// create uplink
	err = ag.CreateInterface(uplink)
	AssertOk(t, err, "Error creating uplink")
	intf, err = ag.FindInterface(uplink.ObjectMeta)
	AssertOk(t, err, "Uplink was not found in DB")
	Assert(t, intf.Name == "testUplink", "Tenant names did not match", intf)

	// verify list api works
	intfList := ag.ListInterface()
	Assert(t, len(intfList) == 3+existingIfLen, "Incorrect number of interfaces")

	// delete lif
	err = ag.DeleteInterface(lif)
	AssertOk(t, err, "Error deleting lif")
	intf, err = ag.FindInterface(lif.ObjectMeta)
	Assert(t, err != nil, "LIF found despite delete")

	// delete enic
	err = ag.DeleteInterface(enic)
	AssertOk(t, err, "Error deleting ENIC")
	intf, err = ag.FindInterface(enic.ObjectMeta)
	Assert(t, err != nil, "ENIC found despite delete")

	// delete uplink
	err = ag.DeleteInterface(uplink)
	AssertOk(t, err, "Error creating uplink")
	intf, err = ag.FindInterface(uplink.ObjectMeta)
	Assert(t, err != nil, "Uplink found despite delete")

	// verify list api works returns 0.
	intfList = ag.ListInterface()
	Assert(t, len(intfList) == existingIfLen, "Incorrect number of interfaces")
}

func TestInterfaceUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	existingIfLen := len(ag.ListInterface())

	// lif
	lif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testLif",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "LIF",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}

	// ENIC
	enic := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "ENIC",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}

	// uplink
	uplink := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testUplink",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "UPLINK",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}

	// create lif
	err := ag.CreateInterface(lif)
	AssertOk(t, err, "Error creating lif")
	intf, err := ag.FindInterface(lif.ObjectMeta)
	AssertOk(t, err, "LIF was not found in DB")
	Assert(t, intf.Name == "testLif", "Lif names did not match", intf)

	// create enic
	err = ag.CreateInterface(enic)
	AssertOk(t, err, "Error creating ENIC")
	intf, err = ag.FindInterface(enic.ObjectMeta)
	AssertOk(t, err, "ENIC was not found in DB")
	Assert(t, intf.Name == "testEnic", "Enic names did not match", intf)

	// create uplink
	err = ag.CreateInterface(uplink)
	AssertOk(t, err, "Error creating uplink")
	intf, err = ag.FindInterface(uplink.ObjectMeta)
	AssertOk(t, err, "Uplink was not found in DB")
	Assert(t, intf.Name == "testUplink", "Tenant names did not match", intf)

	// update interfaces statuses to be down
	downLif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testLif",
		},
		Spec: netproto.InterfaceSpec{
			AdminStatus: "DOWN",
		},
	}

	downEnic := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			AdminStatus: "DOWN",
		},
	}

	downUplink := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testUplink",
		},
		Spec: netproto.InterfaceSpec{
			AdminStatus: "DOWN",
		},
	}

	// update lif
	err = ag.UpdateInterface(downLif)
	AssertOk(t, err, "Error updating lif")
	intf, err = ag.FindInterface(downLif.ObjectMeta)
	AssertOk(t, err, "LIF not found in DB")
	AssertEquals(t, "DOWN", intf.Spec.AdminStatus, "Expected LIF to be down")

	// update enic
	err = ag.UpdateInterface(downEnic)
	AssertOk(t, err, "Error updating ENIC")
	intf, err = ag.FindInterface(downEnic.ObjectMeta)
	AssertOk(t, err, "ENIC not found in DB")
	AssertEquals(t, "DOWN", intf.Spec.AdminStatus, "Expected ENIC to be down")

	// update uplink
	err = ag.UpdateInterface(downUplink)
	AssertOk(t, err, "Error updating uplink")
	intf, err = ag.FindInterface(downUplink.ObjectMeta)
	AssertOk(t, err, "Uplink not found in DB")
	AssertEquals(t, "DOWN", intf.Spec.AdminStatus, "Expected Uplink to be down")

	// verify list api works.
	intfList := ag.ListInterface()
	Assert(t, len(intfList) == 3+existingIfLen, "Incorrect number of interfaces")
}

func TestDuplicateInterfaceCreate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// lif
	lif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testLif",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "LIF",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}

	// create lif
	err := ag.CreateInterface(lif)
	AssertOk(t, err, "Error creating lif")
	intf, err := ag.FindInterface(lif.ObjectMeta)
	AssertOk(t, err, "LIF was not found in DB")
	Assert(t, intf.Name == "testLif", "Lif names did not match", intf)

	// create lif again
	err = ag.CreateInterface(lif)
	AssertOk(t, err, "Duplicate interface create failed when we expect it to be successful.")

	// verify duplicate interface name with different content does not succeed
	dupLif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testLif",
		},
		Spec: netproto.InterfaceSpec{
			Type: "ENIC",
		},
	}
	err = ag.CreateInterface(dupLif)
	Assert(t, err != nil, "Duplicate interface create successful when we expect it to fail.")
}

// Tests corner cases. Like updating non existent interfaces, creating an interface on non-existent tenant
func TestNonExistentInterfaceObjects(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// lif
	badLif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "nonExistentInterface",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "LIF",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}

	// update non existing interface
	err := ag.UpdateInterface(badLif)
	Assert(t, err != nil, "Non existent interface updates should fail, it passed instead")

	// deleting non existing interface
	err = ag.DeleteInterface(badLif)
	Assert(t, err != nil, "Non existent interface deletes should fail, it passed instead")

	// non existing tenant update
	badLif.Tenant = "nonExistentTenant"

	// create interface on non existing tenant
	err = ag.CreateInterface(badLif)
	Assert(t, err != nil, "Non existent tenant interface creates should fail, it passed instead")

	// update interface on non existing tenant
	err = ag.UpdateInterface(badLif)
	Assert(t, err != nil, "Non existent tenant interface updates should fail, it passed instead")

	// delete interface on non existing interface
	err = ag.DeleteInterface(badLif)
	Assert(t, err != nil, "Non existent tenant interface deletes should fail, it passed instead")

}

func TestNamespaceCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// namespace
	ns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testTenant",
		},
	}

	// create backing tenant
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testTenant",
		},
	}
	err := ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating tenant")

	existingNS := len(ag.ListNamespace())

	// create namespace
	err = ag.CreateNamespace(&ns)
	AssertOk(t, err, "Error creating namespace")
	tnt, err := ag.FindNamespace(ns.Tenant, ns.Name)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, tnt.Name == "testTenant", "Tenant names did not match", tnt)

	// verify duplicate tenant creations succeed
	err = ag.CreateNamespace(&ns)
	AssertOk(t, err, "Error creating duplicate tenant")

	// verify list api works.
	nsList := ag.ListNamespace()
	Assert(t, len(nsList) == existingNS+1, "Incorrect number of namespace")

	// delete the namespace and verify its gone from db
	err = ag.DeleteNamespace(&ns)
	AssertOk(t, err, "Error deleting network")
	_, err = ag.FindNamespace(ns.Tenant, ns.Name)
	Assert(t, err != nil, "Tenant was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNamespace(&ns)
	Assert(t, err != nil, "deleting non-existing network succeeded", ag)
}

func TestNamespaceUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// namespace
	ns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "updateTenant",
			Name:   "updateNamespace",
		},
	}

	// create backing tenant
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "updateTenant",
		},
	}
	err := ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating tenant")

	// create namespace
	err = ag.CreateNamespace(&ns)
	AssertOk(t, err, "Error creating namespace")
	namespace, err := ag.FindNamespace(ns.Tenant, ns.Name)
	AssertOk(t, err, "Namespace was not found in DB")
	Assert(t, namespace.Name == "updateNamespace", "Namespace names did not match", namespace)

	nsSpec := netproto.NamespaceSpec{
		Meta: &api.ObjectMeta{
			ResourceVersion: "v2",
		},
	}

	ns.Spec = nsSpec

	err = ag.UpdateNamespace(&ns)
	AssertOk(t, err, "Error updating namespace")
}

func TestNatPoolCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// namespace
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")
	natPool, err := ag.FindNatPool(np.ObjectMeta)
	AssertOk(t, err, "Nat Pool was not found in DB")
	Assert(t, natPool.Name == "testNatPool", "NatPool names did not match", natPool)

	// verify duplicate tenant creations succeed
	err = ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating duplicate nat pool")

	// verify list api works.
	npList := ag.ListNatPool()
	Assert(t, len(npList) == 1, "Incorrect number of nat pools")

	// delete the natpool and verify its gone from db
	err = ag.DeleteNatPool(&np)
	AssertOk(t, err, "Error deleting nat pool")
	_, err = ag.FindNatPool(np.ObjectMeta)
	Assert(t, err != nil, "Nat Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNatPool(&np)
	Assert(t, err != nil, "deleting non-existing nat pool succeeded", ag)
}

func TestNatPoolUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// namespace
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "updateNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	updatedNp := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "updateNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "192.168.1.1 - 192.168.1.100",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")
	natPool, err := ag.FindNatPool(np.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, natPool.Name == "updateNatPool", "Nat Pool names did not match", natPool)

	err = ag.UpdateNatPool(&updatedNp)
	AssertOk(t, err, "Error updating nat pool")
}

func TestNatPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.0 - 192.168.1.1",
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error creating nat policy")
	natPool, err := ag.FindNatPolicy(natPolicy.ObjectMeta)
	AssertOk(t, err, "Nat Policy was not found in DB")
	Assert(t, natPool.Name == "testNatPolicy", "NatPolicy names did not match", natPool)

	// verify duplicate tenant creations succeed
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error creating duplicate nat policy")

	// verify list api works.
	npList := ag.ListNatPolicy()
	Assert(t, len(npList) == 1, "Incorrect number of nat policies")

	// delete the nat policy and verify its gone from db
	err = ag.DeleteNatPolicy(&natPolicy)
	AssertOk(t, err, "Error deleting nat policy")
	_, err = ag.FindNatPolicy(np.ObjectMeta)
	Assert(t, err != nil, "Nat Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNatPolicy(&natPolicy)
	Assert(t, err != nil, "deleting non-existing nat policy succeeded", ag)
}

func TestNatPolicyCreateOnRemoteNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing namespace and nat pool
	remoteNS := &netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "remoteNS",
		},
	}

	err := ag.CreateNamespace(remoteNS)
	AssertOk(t, err, "could not create remote namespace")

	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "remoteNS",
			Name:      "remoteNSNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err = ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.0 - 192.168.1.1",
					},
					NatPool: "remoteNS/remoteNSNatPool",
					Action:  "DNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error creating nat policy")
	natPool, err := ag.FindNatPolicy(natPolicy.ObjectMeta)
	AssertOk(t, err, "Nat Policy was not found in DB")
	Assert(t, natPool.Name == "testNatPolicy", "NatPolicy names did not match", natPool)
}

func TestNatPolicyOnNonExistentLocalNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.0 - 192.168.1.1",
					},
					NatPool: "localNonExistentNatPool",
					Action:  "DNAT",
				},
			},
		},
	}

	// create nat policy
	err := ag.CreateNatPolicy(&natPolicy)
	Assert(t, err != nil, "Nat Policy create with a non existent local nat rule must fail validation. It passed instead")
}

func TestNatPolicyOnMatchAllDst(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing nat pool
	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Nat Policy create with missing Dst Selector should pass")
}

func TestNatPolicyOnMatchAllSrc(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing nat pool
	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.0 - 192.168.1.1",
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Nat Policy create with missing Src Selector should pass")
}

func TestNatPolicyOnMatchAll(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing nat pool
	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Nat Policy create with missing Src Selector should pass")
}

func TestNatPolicyOnNonExistentRemoteNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing namespace and nat pool
	remoteNS := &netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "remoteNS",
		},
	}

	err := ag.CreateNamespace(remoteNS)
	AssertOk(t, err, "could not create remote namespace")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.0 - 192.168.1.1",
					},
					NatPool: "remoteNS/nonExistentRemoteNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	Assert(t, err != nil, "Nat Policy creation on non existent remote nat pool should fail validation. It passed instead.")
}

func TestNatPolicyUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing nat pool
	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")
	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.0 - 192.168.1.1",
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error creating nat policy")
	natPool, err := ag.FindNatPolicy(natPolicy.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, natPool.Name == "testNatPolicy", "Nat Pool names did not match", natPool)

	npSpec := netproto.NatPolicySpec{
		Rules: []netproto.NatRule{
			{
				NatPool: "updatedNatPool",
			},
		},
	}

	natPolicy.Spec = npSpec

	err = ag.UpdateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error updating nat policy")
}

func TestRouteCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	rt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testRoute",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.1.1.0/24",
			Interface: "default-uplink-1",
			GatewayIP: "10.1.1.1",
		},
	}

	// create route
	err := ag.CreateRoute(&rt)
	AssertOk(t, err, "Error creating nroute")
	route, err := ag.FindRoute(rt.ObjectMeta)
	AssertOk(t, err, "Route was not found in DB")
	Assert(t, route.Name == "testRoute", "Route names did not match", route)

	// verify duplicate route creations succeed
	err = ag.CreateRoute(&rt)
	AssertOk(t, err, "Error creating duplicate route")

	// verify list api works.
	rtList := ag.ListRoute()
	Assert(t, len(rtList) == 1, "Incorrect number of routes")

	// delete the route and verify its gone from db
	err = ag.DeleteRoute(&rt)
	AssertOk(t, err, "Error deleting route")
	_, err = ag.FindNatPolicy(rt.ObjectMeta)
	Assert(t, err != nil, "Route was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteRoute(&rt)
	Assert(t, err != nil, "deleting non-existing route succeeded", ag)
}

func TestNatBindingCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	// namespace
	nb := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "preCreatedNatPool",
			IPAddress:   "10.1.1.1",
		},
	}

	// create backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.0.0.1-10.1.1.100",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// create nat binding
	err = ag.CreateNatBinding(&nb)
	AssertOk(t, err, "Error creating nat binding")
	natPool, err := ag.FindNatBinding(nb.ObjectMeta)
	AssertOk(t, err, "Nat Pool was not found in DB")
	Assert(t, natPool.Name == "testNatBinding", "NatBinding names did not match", natPool)

	// verify duplicate tenant creations succeed
	err = ag.CreateNatBinding(&nb)
	AssertOk(t, err, "Error creating duplicate nat pool")

	// verify list api works.
	nbList := ag.ListNatBinding()
	Assert(t, len(nbList) == 1, "Incorrect number of nat pools")

	// delete the natpool and verify its gone from db
	err = ag.DeleteNatBinding(&nb)
	AssertOk(t, err, "Error deleting nat pool")
	_, err = ag.FindNatBinding(nb.ObjectMeta)
	Assert(t, err != nil, "Nat Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNatBinding(&nb)
	Assert(t, err != nil, "deleting non-existing nat pool succeeded", ag)
}

func TestRouteUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// route
	rt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testRoute",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.1.1.0/24",
			GatewayIP: "10.1.1.1",
			Interface: "default-uplink-1",
		},
	}

	// create nat policy
	err := ag.CreateRoute(&rt)
	AssertOk(t, err, "Error creating route")
	route, err := ag.FindRoute(rt.ObjectMeta)
	AssertOk(t, err, "Route not found in DB")
	Assert(t, route.Name == "testRoute", "Route names did not match", route)

	rtSpec := netproto.RouteSpec{
		Interface: "default-uplink-3",
	}

	rt.Spec = rtSpec

	err = ag.UpdateRoute(&rt)
	AssertOk(t, err, "Error updating route")
}

func TestNatBindingUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	// namespace
	nb := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "updateNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "preCreatedNatPool",
			IPAddress:   "10.1.1.1",
		},
	}
	// create backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.0.0.1-10.1.1.100",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// create nat pool
	err = ag.CreateNatBinding(&nb)
	AssertOk(t, err, "Error creating nat binding")
	natBinding, err := ag.FindNatBinding(nb.ObjectMeta)
	AssertOk(t, err, "NatBinding was not found in DB")
	Assert(t, natBinding.Name == "updateNatBinding", "Nat Binding names did not match", natBinding)

	nbSpec := netproto.NatBindingSpec{
		NatPoolName: "updateNatPool",
		IPAddress:   "192.168.1.1",
	}

	nb.Spec = nbSpec

	err = ag.UpdateNatBinding(&nb)
	AssertOk(t, err, "Error updating nat pool")
}

func TestNatBidingToRemoteNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	// create backing remote Namespace and NatPool
	rns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "remoteNamespace",
		},
	}
	err := ag.CreateNamespace(&rns)
	AssertOk(t, err, "Could not create remote namespace")

	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "remoteNamespace",
			Name:      "remoteNamespaceNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.0.0.1-10.1.1.100",
		},
	}

	// create nat pool
	err = ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	nb := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "updateNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "remoteNamespace/remoteNamespaceNatPool",
			IPAddress:   "10.1.1.1",
		},
	}

	// create nat binding
	err = ag.CreateNatBinding(&nb)
	AssertOk(t, err, "Error creating nat binding")
	natPool, err := ag.FindNatBinding(nb.ObjectMeta)
	AssertOk(t, err, "NatBinding was not found in DB")
	Assert(t, natPool.Name == "updateNatBinding", "Nat Pool names did not match", natPool)
}

func TestNatPoolBindingOnNonExistentNatPools(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	// nat binding
	nb := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "updateNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "nonexistentNs/remoteNamespaceNatPool",
			IPAddress:   "10.1.1.1",
		},
	}

	// create nat binding
	err := ag.CreateNatBinding(&nb)
	Assert(t, err != nil, "Nat Bindings with nat pools on non existent namespaces should fail")

	nb = netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "updateNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "badformat/random/nonexistentNs/remoteNamespaceNatPool",
			IPAddress:   "10.1.1.1",
		},
	}

	// create nat binding
	err = ag.CreateNatBinding(&nb)
	Assert(t, err != nil, "Nat Bindings with nat pools on non existent namespaces should fail")

}

func TestIPSecPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
		},
	}
	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
		},
	}
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
					SAName: "testIPSecSADecrypt",
					SAType: "DECRYPT",
					SPI:    42,
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating IPSec policy")
	foundIPSecPolicy, err := ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	AssertOk(t, err, "IPSec Policy was not found in DB")
	Assert(t, foundIPSecPolicy.Name == "testIPSecPolicy", "IPSecPolicy names did not match", foundIPSecPolicy)

	// verify duplicate tenant creations succeed
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating duplicate IPSec policy")

	// verify list api works.
	npList := ag.ListIPSecPolicy()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec policies")

	// delete the nat policy and verify its gone from db
	err = ag.DeleteIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error deleting nat policy")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "deleting non-existing nat policy succeeded", ag)
}

func TestIPSecSAEncryptCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
		},
	}
	dupSAEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           42,
		},
	}
	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	foundIPSecSA, err := ag.FindIPSecSAEncrypt(saEncrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA  ecnrypt was not found in DB")
	Assert(t, foundIPSecSA.Name == "testIPSecSAEncrypt", "IPSecSA encrypt names did not match", foundIPSecSA)

	// verify duplicate sa encrypt creations succeed
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating duplicate IPSec SA Encrypt rule")

	// verify duplicate sa encrypt with changed spec doesn't succeed.
	err = ag.CreateIPSecSAEncrypt(&dupSAEncrypt)
	Assert(t, err != nil, "Error creating duplicate IPSec SA Encrypt rule")

	// verify list api works.
	npList := ag.ListIPSecSAEncrypt()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec SA Encrypt rules")

	// delete the ipsec sa encrypt and verify its gone from db
	err = ag.DeleteIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error deleting IPSec Encrypt rule")
	_, err = ag.FindIPSecSAEncrypt(saEncrypt.ObjectMeta)
	Assert(t, err != nil, "IPSec SA rule was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteIPSecSAEncrypt(&saEncrypt)
	Assert(t, err != nil, "deleting non-existing IPSec SA rule succeeded", ag)
}

func TestIPSecSAEncryptUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
		},
	}
	saEncryptUpdate := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           42,
		},
	}
	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	foundSA, err := ag.FindIPSecSAEncrypt(saEncrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA Encrypt rule was not found in DB")
	Assert(t, foundSA.Name == "testIPSecSAEncrypt", "IPSec SA Encrypt rule names did not match", foundSA)

	err = ag.UpdateIPSecSAEncrypt(&saEncryptUpdate)
	AssertOk(t, err, "Error updating IPSec SA Encrypt rule")
}

func TestIPSecSADecryptCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                1,
		},
	}

	dupSADecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                42,
		},
	}
	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	foundIPSecSA, err := ag.FindIPSecSADecrypt(saDecrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA  ecnrypt was not found in DB")
	Assert(t, foundIPSecSA.Name == "testIPSecSADecrypt", "IPSecSA encrypt names did not match", foundIPSecSA)

	// verify duplicate sa encrypt creations succeed
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating duplicate IPSec SA Decrypt rule")

	// verify duplicate sa encrypt with changed spec doesn't succeed.
	err = ag.CreateIPSecSADecrypt(&dupSADecrypt)
	Assert(t, err != nil, "Error creating duplicate IPSec SA Decrypt rule")

	// verify list api works.
	npList := ag.ListIPSecSADecrypt()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec SA Decrypt rules")

	// delete the ipsec sa encrypt and verify its gone from db
	err = ag.DeleteIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error deleting IPSec Decrypt rule")
	_, err = ag.FindIPSecSADecrypt(saDecrypt.ObjectMeta)
	Assert(t, err != nil, "IPSec SA rule was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteIPSecSADecrypt(&saDecrypt)
	Assert(t, err != nil, "deleting non-existing IPSec SA rule succeeded", ag)
}

func TestIPSecSADecryptUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                1,
		},
	}
	saDecryptUpdate := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                42,
		},
	}
	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	foundSA, err := ag.FindIPSecSADecrypt(saDecrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA Decrypt rule was not found in DB")
	Assert(t, foundSA.Name == "testIPSecSADecrypt", "IPSec SA Decrypt rule names did not match", foundSA)

	err = ag.UpdateIPSecSADecrypt(&saDecryptUpdate)
	AssertOk(t, err, "Error updating IPSec SA Decrypt rule")
}

func TestIPSecPolicyCreateDeleteOnRemoteSARule(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create remote NS
	rns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "remoteNS",
		},
	}
	err := ag.CreateNamespace(&rns)
	AssertOk(t, err, "Could not create remote namespace")

	// Create remote NS backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "remoteNS",
			Name:      "kg2-ipsec-sa-encrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			LocalGwIP:     "20.1.1.1",
			RemoteGwIP:    "20.1.1.2",
			SPI:           1,
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "remoteNS",
			Name:      "kg2-ipsec-sa-decrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			DecryptAlgo:   "AES_GCM_256",
			DecryptionKey: "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			SPI:           1,
		},
	}
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0-10.0.255.255",
					},
					Dst: &netproto.MatchSelector{
						Address: "10.0.0.0-10.0.255.255",
					},
					SAName: "remoteNS/kg2-ipsec-sa-encrypt",
					SAType: "ENCRYPT",
				},
				{
					Src: &netproto.MatchSelector{
						Address: "20.1.1.2 - 20.1.1.2",
					},
					Dst: &netproto.MatchSelector{
						Address: "20.1.1.1 - 20.1.1.1",
					},
					SPI:    1,
					SAName: "remoteNS/kg2-ipsec-sa-decrypt",
					SAType: "DECRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating IPSec policy")
	foundIPSecPolicy, err := ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	AssertOk(t, err, "IPSec Policy was not found in DB")
	Assert(t, foundIPSecPolicy.Name == "kg2-ipsec-decrypt-policy", "IPSecPolicy names did not match", foundIPSecPolicy)

	// verify duplicate tenant creations succeed
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating duplicate IPSec policy")

	// verify list api works.
	npList := ag.ListIPSecPolicy()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec policies")
}

func TestIPSecPolicyCreateDeleteOnNonExistentSARule(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
		},
	}

	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-sa-decrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			DecryptAlgo:   "AES_GCM_256",
			DecryptionKey: "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			SPI:           1,
		},
	}
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0-10.0.255.255",
					},
					Dst: &netproto.MatchSelector{
						Address: "10.0.0.0-10.0.255.255",
					},
					SAName: "nonExistentEncryptSA",
					SAType: "ENCRYPT",
				},
				{
					Src: &netproto.MatchSelector{
						Address: "20.1.1.2 - 20.1.1.2",
					},
					Dst: &netproto.MatchSelector{
						Address: "20.1.1.1 - 20.1.1.1",
					},
					SPI:    1,
					SAName: "remoteNS/kg2-ipsec-sa-decrypt",
					SAType: "DECRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec policy creates on non existen encrypt SA Rules should fail")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Policy was not found in DB")

	ipSecPolicy = netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "20.1.1.2 - 20.1.1.2",
					},
					Dst: &netproto.MatchSelector{
						Address: "20.1.1.1 - 20.1.1.1",
					},
					SPI:    1,
					SAName: "nonExistentDecryptPolicy",
					SAType: "DECRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec policy creates on non existent decrypt SA Rules should fail")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Policy was not found in DB")

	// IPSec POlicy with missing decrypt SA
	ipSecPolicy = netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "20.1.1.2 - 20.1.1.2",
					},
					Dst: &netproto.MatchSelector{
						Address: "20.1.1.1 - 20.1.1.1",
					},
					SPI:    1,
					SAName: "nonExistentDecryptSA",
					SAType: "DECRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec policy creates on non existent decrypt SA Rules should fail")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Policy was not found in DB")

	// IPSec Policy with SPI Value in encrypt policy
	ipSecPolicy = netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "spi-on-encrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "20.1.1.2 - 20.1.1.2",
					},
					Dst: &netproto.MatchSelector{
						Address: "20.1.1.1 - 20.1.1.1",
					},
					SPI:    1,
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec policy creates on non existent decrypt SA Rules should fail")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Policy was not found in DB")
}

func TestDefaultNamespaceTenantDeleteCornerCases(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// namespace
	defaultNS := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "default",
		},
	}

	// create backing tenant
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testTenant",
		},
	}
	err := ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating tenant")

	err = ag.DeleteNamespace(&defaultNS)
	Assert(t, err != nil, "default namespace deletes under default tenant should fail. It passed instead")

	// Delete a non-default tenant and ensure that the default ns under it is gone
	err = ag.DeleteTenant(&tn)
	AssertOk(t, err, "Non default tenant deletes should be disallowed.")

	_, err = ag.FindNamespace("testTenant", "default")
	Assert(t, err != nil, "deleting a non default tenant should automatically trigger the deletion of the default namespace under it.")
}

func TestIPSecPolicyCreateInvalidRule(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
		},
	}
	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "BADRULE TYPE",
				},
			},
		},
	}
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec Policy with invalid rules should fail")

}

func TestIPSecPolicyUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
		},
	}
	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "10.0.0.0 - 10.0.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}

	ipSecPolicyUpdate := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Address: "20.0.0.0 -  20.1.1.0",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "IPSec Policy creation failed")

	err = ag.UpdateIPSecPolicy(&ipSecPolicyUpdate)
	AssertOk(t, err, "IPSec Policy update failed")

}
