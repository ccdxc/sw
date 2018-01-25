// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"fmt"
	"sync"
	"testing"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type mockDatapath struct {
	sync.Mutex
	netdb map[string]*netproto.Network
	epdb  map[string]*netproto.Endpoint
	sgdb  map[string]*netproto.SecurityGroup
	tndb  map[string]*netproto.Tenant
}

// SetAgent registers agent with datapath
func (dp *mockDatapath) SetAgent(ag DatapathIntf) error {
	return nil
}

func (dp *mockDatapath) CreateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) (*IntfInfo, error) {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta)
	dp.epdb[key] = ep
	return nil, nil
}

func (dp *mockDatapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta)
	dp.epdb[key] = ep
	return nil
}

func (dp *mockDatapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta)
	dp.epdb[key] = ep
	return nil
}

func (dp *mockDatapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta)
	dp.epdb[key] = ep
	return nil
}

func (dp *mockDatapath) DeleteLocalEndpoint(ep *netproto.Endpoint) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta)
	delete(dp.epdb, key)
	return nil
}

func (dp *mockDatapath) DeleteRemoteEndpoint(ep *netproto.Endpoint) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta)
	delete(dp.epdb, key)
	return nil
}

// CreateNetwork creates a network in datapath
func (dp *mockDatapath) CreateNetwork(nw *netproto.Network, tn *netproto.Tenant) error {
	return nil
}

// UpdateNetwork updates a network in datapath
func (dp *mockDatapath) UpdateNetwork(nw *netproto.Network) error {
	return nil
}

// DeleteNetwork deletes a network from datapath
func (dp *mockDatapath) DeleteNetwork(nw *netproto.Network) error {
	return nil
}

// CreateSecurityGroup creates a security group
func (dp *mockDatapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(sg.ObjectMeta)
	dp.sgdb[key] = sg
	return nil
}

// UpdateSecurityGroup updates a security group
func (dp *mockDatapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}

// DeleteSecurityGroup deletes a security group
func (dp *mockDatapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(sg.ObjectMeta)
	delete(dp.epdb, key)
	return nil
}

// AddSecurityRule adds a security rule
func (dp *mockDatapath) AddSecurityRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule, peersg *netproto.SecurityGroup) error {
	return nil
}

// DeleteSecurityRule deletes a security rule
func (dp *mockDatapath) DeleteSecurityRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule, peersg *netproto.SecurityGroup) error {
	return nil
}

// CreateTenant creates a tenant. Stubbed out to satisfy the interface
func (dp *mockDatapath) CreateTenant(tn *netproto.Tenant) error {
	return nil
}

// DeleteTenant deletes a tenant. Stubbed out to satisfy the interface
func (dp *mockDatapath) DeleteTenant(tn *netproto.Tenant) error {
	return nil
}

// UpdateTenant updates a tenant. Stubbed out to satisfy the interface
func (dp *mockDatapath) UpdateTenant(tn *netproto.Tenant) error {
	return nil
}

type mockCtrler struct {
	epdb map[string]*netproto.Endpoint
}

func (ctrler *mockCtrler) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	key := objectKey(epinfo.ObjectMeta)
	ctrler.epdb[key] = epinfo
	return epinfo, nil
}

func (ctrler *mockCtrler) EndpointAgeoutNotif(epinfo *netproto.Endpoint) error {
	return nil
}

func (ctrler *mockCtrler) EndpointDeleteReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	key := objectKey(epinfo.ObjectMeta)
	delete(ctrler.epdb, key)
	return epinfo, nil
}

// createNetAgent creates a netagent scaffolding
func createNetAgent(t *testing.T) (*NetAgent, *mockDatapath, *mockCtrler) {
	dp := &mockDatapath{
		epdb:  make(map[string]*netproto.Endpoint),
		netdb: make(map[string]*netproto.Network),
		sgdb:  make(map[string]*netproto.SecurityGroup),
		tndb:  make(map[string]*netproto.Tenant),
	}
	ct := &mockCtrler{
		epdb: make(map[string]*netproto.Endpoint),
	}

	// create new network agent
	nagent, err := NewNetAgent(dp, "", "some-unique-id")

	if err != nil {
		t.Fatalf("Error creating network agent. Err: %v", err)
		return nil, nil, nil
	}

	// fake controller intf
	nagent.RegisterCtrlerIf(ct)

	return nagent, dp, ct
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
			Tenant: "default",
			Name:   "default",
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
			Tenant: "default",
			Name:   "default",
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
			Tenant: "updateTenant",
			Name:   "updateTenant",
		},
	}

	err := ag.CreateTenant(tn)
	AssertOk(t, err, "Error creating tenant")

	// create network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "updateTenant",
			Name:   "updateNetwork",
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

func TestEndpointCreateDelete(t *testing.T) {
	// create netagent
	ag, dp, ct := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "default",
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
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
		},
	}

	// create the endpoint
	ep, _, err := ag.EndpointCreateReq(&epinfo)
	AssertOk(t, err, "Error creating endpoint")

	// verify both controller and datapath got called
	key := objectKey(epinfo.ObjectMeta)
	nep, ok := ag.endpointDB[key]
	Assert(t, ok, "Endpoint was not found in datapath", dp)
	Assert(t, proto.Equal(nep, ep), "Datapath endpoint did not match", nep)
	dep, ok := dp.epdb[key]
	Assert(t, ok, "Endpoint was not found in datapath", dp)
	Assert(t, proto.Equal(dep, ep), "Datapath endpoint did not match", dep)
	cep, ok := ct.epdb[key]
	Assert(t, ok, "Endpoint was not found in ctrler", dp)
	Assert(t, proto.Equal(cep, ep), "Datapath endpoint did not match", cep)

	// verify duplicate endpoint creations succeed
	_, _, err = ag.EndpointCreateReq(&epinfo)
	AssertOk(t, err, "Endpoint creation is not idempotent")

	// verify endpoint create on non-existing network fails
	ep2 := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testEndpoint2",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "invalid",
		},
	}
	_, _, err = ag.EndpointCreateReq(&ep2)
	Assert(t, (err != nil), "Endpoint create on non-existing network succeeded", ag)

	// verify list api works
	epList := ag.ListEndpoint()
	Assert(t, (len(epList) == 1), "Incorrect number of endpoints")

	// endpoint message
	depinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID2",
			WorkloadUUID: "testWorkloadUUID2",
			NetworkName:  "default",
		},
	}
	_, _, err = ag.EndpointCreateReq(&depinfo)
	Assert(t, (err != nil), "Conflicting endpoint creating succeeded", ag)

	// delete the endpoint
	err = ag.EndpointDeleteReq(&epinfo)
	AssertOk(t, err, "Endpoint delete failed")

	// verify endpoint is gone everywhere
	_, ok = ag.endpointDB[key]
	Assert(t, !ok, "Endpoint was still found in datapath", dp)
	_, ok = dp.epdb[key]
	Assert(t, !ok, "Endpoint was still found in datapath", dp)
	_, ok = ct.epdb[key]
	Assert(t, !ok, "Endpoint was still found in ctrler", dp)

	// verify non-existing endpoint can not be deleted
	err = ag.EndpointDeleteReq(&epinfo)
	Assert(t, (err != nil), "Deleting non-existing endpoint succeeded", ag)
}

func TestCtrlerEndpointCreateDelete(t *testing.T) {
	// create netagent
	ag, dp, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "default",
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
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
		},
	}

	// create the endpoint
	_, err = ag.CreateEndpoint(&epinfo)
	AssertOk(t, err, "Error creating endpoint")

	// verify datapath got called
	key := objectKey(epinfo.ObjectMeta)
	nep, ok := ag.endpointDB[key]
	Assert(t, ok, "Endpoint was not found in datapath", dp)
	Assert(t, proto.Equal(nep, &epinfo), "Datapath endpoint did not match", nep)
	dep, ok := dp.epdb[key]
	Assert(t, ok, "Endpoint was not found in datapath", dp)
	Assert(t, proto.Equal(dep, &epinfo), "Datapath endpoint did not match", dep)

	// verify duplicate endpoint creations succeed
	_, err = ag.CreateEndpoint(&epinfo)
	AssertOk(t, err, "Endpoint creation is not idempotent")

	// endpoint message
	depinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID2",
			WorkloadUUID: "testWorkloadUUID2",
			NetworkName:  "default",
		},
	}
	_, err = ag.CreateEndpoint(&depinfo)
	Assert(t, (err != nil), "Conflicting endpoint creating succeeded", ag)

	// delete the endpoint
	err = ag.DeleteEndpoint(&epinfo)
	AssertOk(t, err, "Endpoint delete failed")

	// verify endpoint is gone everywhere
	_, ok = ag.endpointDB[key]
	Assert(t, !ok, "Endpoint was still found in datapath", dp)
	_, ok = dp.epdb[key]
	Assert(t, !ok, "Endpoint was still found in datapath", dp)

	// verify non-existing endpoint can not be deleted
	err = ag.DeleteEndpoint(&epinfo)
	Assert(t, (err != nil), "Deleting non-existing endpoint succeeded", ag)
}

func TestSecurityGroupCreateDelete(t *testing.T) {
	// create netagent
	ag, dp, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security group
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "test-sg",
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

	// verify datapath has the security group
	_, ok := dp.sgdb[objectKey(sg.ObjectMeta)]
	Assert(t, ok, "Security group not found in datapath")

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "default",
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
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:   "testEndpointUUID",
			WorkloadUUID:   "testWorkloadUUID",
			NetworkName:    "default",
			SecurityGroups: []string{"test-sg"},
		},
	}

	// create endpoint referring to security group
	_, err = ag.CreateEndpoint(&epinfo)
	AssertOk(t, err, "Endpoint creation with security group failed")

	// try creating an endpoint with non-existing security group
	ep2 := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testEndpoint2",
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
	ag, dp, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "default",
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
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
		},
	}

	// create the endpoint
	_, err = ag.CreateEndpoint(&epinfo)
	AssertOk(t, err, "Error creating endpoint")

	// security group
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "test-sg",
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

	// verify endpoint got updated
	key := objectKey(epinfo.ObjectMeta)
	nep, ok := ag.endpointDB[key]
	Assert(t, ok, "Endpoint was not found in datapath", dp)
	Assert(t, proto.Equal(nep, &epupd), "Datapath endpoint did not match", nep)
	dep, ok := dp.epdb[key]
	Assert(t, ok, "Endpoint was not found in datapath", dp)
	Assert(t, proto.Equal(dep, &epupd), "Datapath endpoint did not match", dep)

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
	ag, dp, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security group
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "test-sg",
		},
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "unknown",
			Rules: []netproto.SecurityRule{
				{
					Direction: "Incoming",
					PeerGroup: "",
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
		},
	}
	err = ag.CreateSecurityGroup(&sg2)
	AssertOk(t, err, "Error creating security group")

	// verify datapath has the security group
	_, ok := dp.sgdb[objectKey(sg2.ObjectMeta)]
	Assert(t, ok, "Security group not found in datapath")

	// update first sg
	sg.Spec.Rules = []netproto.SecurityRule{
		{
			Direction: "Incoming",
			PeerGroup: "test-sg2",
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
			Tenant: "default",
			Name:   "default",
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
					Tenant: "default",
					Name:   fmt.Sprintf("testEndpoint-%d", idx),
				},
				Spec: netproto.EndpointSpec{
					EndpointUUID: "testEndpointUUID",
					WorkloadUUID: "testWorkloadUUID",
					NetworkName:  "default",
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
					Tenant: "default",
					Name:   fmt.Sprintf("testEndpoint-%d", idx),
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
			Tenant: "testTenant",
			Name:   "testTenant",
		},
	}

	// create tenant
	err := ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating tenant")
	tnt, err := ag.FindTenant(tn.ObjectMeta)
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
	_, err = ag.FindTenant(tn.ObjectMeta)
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
			Tenant: "updateTenant",
			Name:   "updateTenant",
		},
	}

	// create tenant
	err := ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating tenant")
	tnt, err := ag.FindTenant(tn.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, tnt.Name == "updateTenant", "Tenant names did not match", tnt)

	tnSpec := netproto.TenantSpec{
		Meta: &api.ObjectMeta{
			Namespace: "testNamespace",
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
			Tenant: "nonExistentNetwork",
			Name:   "default",
		},
	}

	// create network
	err := ag.CreateNetwork(&nt)
	Assert(t, err != nil, "Network create was expected to fail.")
}
