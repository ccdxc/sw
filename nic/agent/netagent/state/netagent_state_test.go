// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"fmt"
	"sync"
	"testing"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/protos"
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

	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	dp.epdb[key] = ep
	return nil, nil
}

func (dp *mockDatapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup, intf *netproto.Interface, ns *netproto.Namespace) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	dp.epdb[key] = ep
	return nil
}

func (dp *mockDatapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	dp.epdb[key] = ep
	return nil
}

func (dp *mockDatapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	dp.epdb[key] = ep
	return nil
}

func (dp *mockDatapath) DeleteLocalEndpoint(ep *netproto.Endpoint) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	delete(dp.epdb, key)
	return nil
}

func (dp *mockDatapath) DeleteRemoteEndpoint(ep *netproto.Endpoint) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	delete(dp.epdb, key)
	return nil
}

// CreateNetwork creates a network in datapath
func (dp *mockDatapath) CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error {
	return nil
}

// UpdateNetwork updates a network in datapath
func (dp *mockDatapath) UpdateNetwork(nw *netproto.Network, ns *netproto.Namespace) error {
	return nil
}

// DeleteNetwork deletes a network from datapath
func (dp *mockDatapath) DeleteNetwork(nw *netproto.Network, ns *netproto.Namespace) error {
	return nil
}

// CreateSecurityGroup creates a security group
func (dp *mockDatapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	dp.Lock()
	defer dp.Unlock()

	key := objectKey(sg.ObjectMeta, sg.TypeMeta)
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

	key := objectKey(sg.ObjectMeta, sg.TypeMeta)
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

// CreateVrf creates a vrf. Stubbed out to satisfy the interface
func (dp *mockDatapath) CreateVrf(vrfID uint64) error {
	return nil
}

// DeleteVrf deletes a vrf. Stubbed out to satisfy the interface
func (dp *mockDatapath) DeleteVrf(vrfID uint64) error {
	return nil
}

// UpdateVrf updates a vrf. Stubbed out to satisfy the interface
func (dp *mockDatapath) UpdateVrf(vrfID uint64) error {
	return nil
}

// CreateInterface creates an interface. Stubbed out to satisfy datapath interface.
func (dp *mockDatapath) CreateInterface(intf *netproto.Interface, lif *netproto.Interface, ns *netproto.Namespace) error {
	return nil
}

// DeleteInterface deletes. Stubbed out to satisfy datapath interface.
func (dp *mockDatapath) DeleteInterface(intf *netproto.Interface, ns *netproto.Namespace) error {
	return nil
}

// UpdateInterface updates an interface. Stubbed out to satisfy datapath interface.
func (dp *mockDatapath) UpdateInterface(intf *netproto.Interface, ns *netproto.Namespace) error {
	return nil
}

// CreateNatPool creates a NAT Pool in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) CreateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {

	return nil
}

// UpdateNatPool updates a NAT Pool in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) UpdateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {

	return nil
}

// DeleteNatPool deletes a NAT Pool in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) DeleteNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {

	return nil
}

// CreateNatPolicy creates a NAT Policy in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) CreateNatPolicy(np *netproto.NatPolicy, npLUT map[string]*NatPoolRef, ns *netproto.Namespace) error {

	return nil
}

// UpdateNatPolicy updates a NAT Policy in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) UpdateNatPolicy(np *netproto.NatPolicy, ns *netproto.Namespace) error {

	return nil
}

// DeleteNatPolicy deletes a NAT Policy in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) DeleteNatPolicy(np *netproto.NatPolicy, ns *netproto.Namespace) error {

	return nil
}

// CreateRoute creates a Route in the datapath. Stubbed out to satisfy the interface
func (dp *mockDatapath) CreateRoute(rt *netproto.Route, ns *netproto.Namespace) error {

	return nil
}

// CreateNatBinding creates a NAT Binding in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) CreateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, ns *netproto.Namespace) (*netproto.NatBinding, error) {

	return nb, nil
}

// UpdateRoute updates a Route in the datapath. Stubbed out to satisfy the interface
func (dp *mockDatapath) UpdateRoute(rt *netproto.Route, ns *netproto.Namespace) error {

	return nil
}

// UpdateNatBinding updates a NAT Binding in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) UpdateNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error {

	return nil
}

// DeleteRoute deletes a Route in the datapath. Stubbed out to satisfy the interface
func (dp *mockDatapath) DeleteRoute(rt *netproto.Route, ns *netproto.Namespace) error {

	return nil
}

// DeleteNatBinding deletes a NAT Binding in the datapath. Stubbed out to satisfy datapath interface
func (dp *mockDatapath) DeleteNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error {

	return nil
}

// CreateIPSecPolicy creates an IPSec Policy in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) CreateIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace, ipSecLUT map[string]*IPSecRuleRef) error {
	return nil
}

// UpdateIPSecPolicy updates an IPSec Policy in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) UpdateIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecPolicy deletes an IPSec Policy in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) DeleteIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace) error {
	return nil
}

// CreateIPSecSAEncrypt creates an IPSec Encrypt SA in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) CreateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// UpdateIPSecSAEncrypt updates an IPSec Encrypt SA in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) UpdateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecSAEncrypt deletes an IPSec Encrypt SA in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) DeleteIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// CreateIPSecSADecrypt creates an IPSec Decrypt SA in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) CreateIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

// UpdateIPSecSADecrypt updates an IPSec Decrypt SA in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) UpdateIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecSADecrypt deletes an IPSec Decrypt SA in the datapath. Stubbed out to satisfy the datapath Interface
func (dp *mockDatapath) DeleteIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

func (dp *mockDatapath) ListInterfaces() (*halproto.LifGetResponseMsg, *halproto.InterfaceGetResponseMsg, error) {
	var lifs halproto.LifGetResponseMsg
	var uplinks halproto.InterfaceGetResponseMsg
	mockLifs := []*halproto.LifGetResponse{
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.LifSpec{
				KeyOrHandle: &halproto.LifKeyHandle{
					KeyOrHandle: &halproto.LifKeyHandle_LifId{
						LifId: 1,
					},
				},
			},
		},
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.LifSpec{
				KeyOrHandle: &halproto.LifKeyHandle{
					KeyOrHandle: &halproto.LifKeyHandle_LifId{
						LifId: 2,
					},
				},
			},
		},
	}
	lifs.Response = append(lifs.Response, mockLifs...)

	mockUplinks := []*halproto.InterfaceGetResponse{
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.InterfaceSpec{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: 3,
					},
				},
				Type: halproto.IfType_IF_TYPE_UPLINK,
			},
		},
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.InterfaceSpec{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: 4,
					},
				},
				Type: halproto.IfType_IF_TYPE_UPLINK,
			},
		},
	}
	uplinks.Response = append(uplinks.Response, mockUplinks...)

	return &lifs, &uplinks, nil
}

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
	nagent, err := NewNetAgent(dp, state.AgentMode_MANAGED, "", "some-unique-id")

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
	}

	// create the endpoint
	ep, _, err := ag.EndpointCreateReq(&epinfo)
	AssertOk(t, err, "Error creating endpoint")

	// verify both controller and datapath got called
	key := objectKey(epinfo.ObjectMeta, epinfo.TypeMeta)
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
			Tenant:    "default",
			Name:      "testEndpoint2",
			Namespace: "default",
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
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
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
			Interface:    "default-uplink-1",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "10.1.1.1/24",
		},
	}

	// create the endpoint
	_, err = ag.CreateEndpoint(&epinfo)
	AssertOk(t, err, "Error creating endpoint")

	// verify datapath got called
	key := objectKey(epinfo.ObjectMeta, epinfo.TypeMeta)
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
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
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
	_, ok := dp.sgdb[objectKey(sg.ObjectMeta, sg.TypeMeta)]
	Assert(t, ok, "Security group not found in datapath")

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
	ag, dp, _ := createNetAgent(t)
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
	key := objectKey(epinfo.ObjectMeta, epinfo.TypeMeta)
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
	_, ok := dp.sgdb[objectKey(sg2.ObjectMeta, sg2.TypeMeta)]
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

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")
	natPool, err := ag.FindNatPool(np.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, natPool.Name == "updateNatPool", "Nat Pool names did not match", natPool)

	npSpec := netproto.NatPoolSpec{
		IPRange: "192.168.1.1-192.168.1.100",
	}

	np.Spec = npSpec

	err = ag.UpdateNatPool(&np)
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

	updatedSADecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "updatedSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_128",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
		},
	}
	err = ag.CreateIPSecSADecrypt(&updatedSADecrypt)
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

	// create nat policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating IPsec policy")
	natPool, err := ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, natPool.Name == "testIPSecPolicy", "IPSec policy names did not match", natPool)

	ipSpec := netproto.IPSecPolicySpec{
		Rules: []netproto.IPSecRule{
			{
				SAName: "updatedDecryptSA",
			},
		},
	}

	ipSecPolicy.Spec = ipSpec

	err = ag.UpdateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error updating nat policy")

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
		},
	}
	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	foundSA, err := ag.FindIPSecSAEncrypt(saEncrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA Encrypt rule was not found in DB")
	Assert(t, foundSA.Name == "testIPSecSAEncrypt", "IPSec SA Encrypt rule names did not match", foundSA)

	ipSpec := netproto.IPSecSAEncryptSpec{
		Protocol: "AH",
	}

	saEncrypt.Spec = ipSpec

	err = ag.UpdateIPSecSAEncrypt(&saEncrypt)
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
		},
	}
	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	foundSA, err := ag.FindIPSecSADecrypt(saDecrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA Decrypt rule was not found in DB")
	Assert(t, foundSA.Name == "testIPSecSADecrypt", "IPSec SA Decrypt rule names did not match", foundSA)

	ipSpec := netproto.IPSecSADecryptSpec{
		Protocol: "AH",
	}

	saDecrypt.Spec = ipSpec

	err = ag.UpdateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error updating IPSec SA Decrypt rule")
}
