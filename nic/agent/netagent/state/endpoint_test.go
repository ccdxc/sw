package state

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
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
			MacAddress:   "4242.4242.4242",
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

func TestLocalEndpointUpdate(t *testing.T) {
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
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
			NodeUUID:     ag.NodeUUID,
			MacAddress:   "4242.4242.4242",
		},
	}

	// update the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "Endpoint create failed.")

	ep, err := ag.FindEndpoint(epinfo.ObjectMeta)
	AssertOk(t, err, "EP not found")
	Assert(t, ep.Name == epinfo.Name, "Endpoints didn't match")

	epinfo.Spec.WorkloadName = "updatedWorkloadName"
	err = ag.UpdateEndpoint(epinfo)
	AssertOk(t, err, "Local endpoint update failed")
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
			MacAddress:   "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(&epinfo)
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
		},
	}

	// create a security group
	err = ag.CreateSecurityGroup(&sg)
	AssertOk(t, err, "Error creating security group")

	// update the remote endpoint
	epupd := epinfo
	epupd.Spec.SecurityGroups = []string{"test-sg"}
	err = ag.UpdateEndpoint(&epupd)
	AssertOk(t, err, "Error updating endpoint")

	// update the remote endpoint
	epupd.Spec.SecurityGroups = []string{"test-sg"}
	epupd.Spec.NodeUUID = ag.NodeUUID
	err = ag.UpdateEndpoint(&epupd)
	AssertOk(t, err, "Error updating endpoint")

	// try changing the network of endpoint
	epupd2 := epupd
	epupd2.Spec.NetworkName = "unknown"
	err = ag.UpdateEndpoint(&epupd2)
	Assert(t, (err != nil), "Changing network to non-existing network succeeded")

	// try updating security group to an unknown
	epupd2 = epupd
	epupd2.Spec.SecurityGroups = []string{"unknown"}
	err = ag.UpdateEndpoint(&epupd2)
	Assert(t, (err != nil), "Changing to non-existing security group succeeded")

	// create another network
	nt2 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "test",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
			VlanID:      1,
		},
	}

	// make create network call
	err = ag.CreateNetwork(&nt2)
	AssertOk(t, err, "Error creating network")

	// move endpoint to new network
	epupd3 := epinfo
	epupd3.Spec.NetworkName = "test"
	err = ag.UpdateEndpoint(&epupd3)
	AssertOk(t, err, "Error updating endpoint")

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
					MacAddress:   "4242.4242.4242",
				},
			}

			// create the endpoint
			eperr := ag.CreateEndpoint(&epinfo)
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
			eperr := ag.DeleteEndpoint(epinfo.Tenant, epinfo.Namespace, epinfo.Name)
			waitCh <- eperr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error deleting endpoint")
	}
}
func TestLocalEndpointPointingToAnyLif(t *testing.T) {
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
			NodeUUID:     ag.NodeUUID,
			MacAddress:   "4242.4242.4242",
		}}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "creating a local ep pointing to any lif failed")
}

func TestLocalEndpointPointingToPredefinedLIF(t *testing.T) {
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
			//WorkloadUUID:  "testWorkloadUUID",
			NetworkName:   "default",
			InterfaceType: "lif",
			Interface:     "lif1",
			NodeUUID:      ag.NodeUUID,
			MacAddress:    "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "creating local ep pointing to pre defined lif failed")
}

func TestRemoteEndpointPointingToAnyUplink(t *testing.T) {
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
			NodeUUID:     "different-uuid-than-agent",
			MacAddress:   "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "creating a local ep pointing to any lif failed")
}

func TestRemoteEndpointPointingToPredefinedUplink(t *testing.T) {
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
			//WorkloadUUID:  "testWorkloadUUID",
			NetworkName:   "default",
			InterfaceType: "uplink",
			Interface:     "uplink128",
			NodeUUID:      "some-different-uuid-than-agent",
			MacAddress:    "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "creating local ep pointing to pre defined lif failed")
}

func TestRemoteEndpointPointingToLocalTunnel(t *testing.T) {
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

	// create a tunnel in the same namespace
	tun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "public-tunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}
	// make create tunnel call
	err = ag.CreateTunnel(&tun)
	AssertOk(t, err, "Error creating tunnel")

	// endpoint message
	epinfo := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:  "testEndpointUUID",
			WorkloadUUID:  "testWorkloadUUID",
			NetworkName:   "default",
			InterfaceType: "tunnel",
			Interface:     "public-tunnel",
			NodeUUID:      "remote",
			MacAddress:    "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "creating a remote ep pointing to a tunnel failed")
}

func TestRemoteEndpointPointingToRemoteTunnel(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create a namespace
	publicNS := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "public-ns",
		},
	}
	err := ag.CreateNamespace(&publicNS)
	AssertOk(t, err, "error creating namespace")

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

	// create a tunnel in the same namespace
	tun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public-ns",
			Name:      "public-tunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}
	// make create tunnel call
	err = ag.CreateTunnel(&tun)
	AssertOk(t, err, "Error creating tunnel")

	// endpoint message
	epinfo := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:  "testEndpointUUID",
			WorkloadUUID:  "testWorkloadUUID",
			NetworkName:   "default",
			Interface:     "public-ns/public-tunnel",
			InterfaceType: "tunnel",
			NodeUUID:      "remote",
			MacAddress:    "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "creating a remote ep pointing to a tunnel failed")
}

func TestFindLocalEndpoint(t *testing.T) {
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
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
			NodeUUID:     ag.NodeUUID,
			MacAddress:   "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "creating a local endpoint failed")
	_, err = ag.FindLocalEndpoint("default", "default")
	AssertOk(t, err, "failed to find local endpoint")
	err = ag.DeleteEndpoint("default", "default", "default")
	AssertOk(t, err, "failed to delete endpoint")

	epr, err := ag.FindLocalEndpoint("default", "default")
	Assert(t, err != nil, "unknown local endpoint", epr)
}

//--------------------- Corner Case Tests ---------------------//

func TestEndpointCreateOnNonExistentNamespace(t *testing.T) {
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
			Namespace: "nonExistentNamespace",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
			MacAddress:   "4242.4242.4242",
		},
	}

	// create the endpoint
	_, _, err = ag.EndpointCreateReq(epinfo)
	Assert(t, err != nil, "Creating an endpoint on non-existent Namespace should fail.")
}

func TestRemoteEndpointOnNonExistentInterface(t *testing.T) {
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
			Interface:    "bad-interface",
			NodeUUID:     "remote",
			MacAddress:   "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	Assert(t, err != nil, "Creating an endpoint with non existent interfaces should fail.")
}

func TestNonExistentEndpointUpdate(t *testing.T) {
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
			Name:      "NonExistentEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
			Interface:    "bad-interface",
			NodeUUID:     ag.NodeUUID,
			MacAddress:   "4242.4242.4242",
		},
	}

	// update the endpoint
	err = ag.UpdateEndpoint(epinfo)
	Assert(t, err != nil, "Creating an endpoint with non existent interfaces should fail.")
}

func TestRemoteEndpointOnNonExistentRemoteTunnel(t *testing.T) {
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
			Interface:    "remoteNS/nonExistentRemoteNatPool",
			NodeUUID:     "remote",
			MacAddress:   "4242.4242.4242",
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	Assert(t, err != nil, "remote ep creates on non-existent tunnels should fail")
}

func TestEndpointCreateInvalidIPAddress(t *testing.T) {
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
			EndpointUUID:  "testEndpointUUID",
			WorkloadUUID:  "testWorkloadUUID",
			NetworkName:   "default",
			NodeUUID:      "remote",
			MacAddress:    "4242.4242.4242",
			IPv4Addresses: []string{"A.B.C.D"},
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	Assert(t, err != nil, "ep creates with invalid IP Address must fail")
}

func TestEndpointCreateCIDRIP(t *testing.T) {
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
			EndpointUUID:  "testEndpointUUID",
			WorkloadUUID:  "testWorkloadUUID",
			NetworkName:   "default",
			NodeUUID:      "remote",
			MacAddress:    "4242.4242.4242",
			IPv4Addresses: []string{"10.1.1.1/24"},
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "ep creates with CIDR ipv4 addresses must succeed")
}

func TestEndpointCreateIP(t *testing.T) {
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
			EndpointUUID:  "testEndpointUUID",
			WorkloadUUID:  "testWorkloadUUID",
			NetworkName:   "default",
			NodeUUID:      "remote",
			MacAddress:    "4242.4242.4242",
			IPv4Addresses: []string{"10.1.1.1"},
		},
	}

	// create the endpoint
	err = ag.CreateEndpoint(epinfo)
	AssertOk(t, err, "ep creates with CIDR ipv4 addresses must succeed")
	foundEP, err := ag.FindEndpoint(epinfo.ObjectMeta)
	AssertOk(t, err, "Failed to find the endpoint. Err: %v", err)
	AssertEquals(t, []string{"10.1.1.1/32"}, foundEP.Spec.IPv4Addresses, "Endpoints specifying just the IP Addresses must have a /32 prefix")
}
