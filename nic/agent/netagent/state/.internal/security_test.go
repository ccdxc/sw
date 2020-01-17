// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/nic/agent/netagent/state"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestSecurityGroupCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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
			IPv4Addresses:  []string{"10.0.0.1/24"},
			MacAddress:     "4242.4242.4242",
		},
	}

	// create endpoint referring to security group
	err = ag.CreateEndpoint(&epinfo)
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
	err = ag.CreateEndpoint(&ep2)
	Assert(t, err != nil, "Endpoint create with unknown sg succeeded", ep2)

	// delete sg
	err = ag.DeleteSecurityGroup(sg.Tenant, sg.Namespace, sg.Name)
	AssertOk(t, err, "Error deleting security group")
}

func TestSecurityGroupUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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
		},
	}

	// create a security group
	err := ag.CreateSecurityGroup(&sg)
	AssertOk(t, err, "Error creating security group")

	// update the sg spec
	updatedSGSpec := netproto.SecurityGroupSpec{
		SecurityProfile: "testProfile",
	}

	sg.Spec = updatedSGSpec

	err = ag.UpdateSecurityGroup(&sg)
	AssertOk(t, err, "Error updating security group")
}
