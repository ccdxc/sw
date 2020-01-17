// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/nic/agent/netagent/state"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestNatPoolCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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
	err = ag.DeleteNatPool(np.Tenant, np.Namespace, np.Name)
	AssertOk(t, err, "Error deleting nat pool")
	_, err = ag.FindNatPool(np.ObjectMeta)
	Assert(t, err != nil, "Nat Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNatPool(np.Tenant, np.Namespace, np.Name)
	Assert(t, err != nil, "deleting non-existing nat pool succeeded", ag)
}

func TestNatPoolUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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

//--------------------- Corner Case Tests ---------------------//

func TestNatPoolCreateOnNonExistingNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// namespace
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "NonExistentNamespace",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	Assert(t, err != nil, "nat pool creation on non-existent namespace should fail")
}

func TestNatPoolDatapathFailure(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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
			IPRange: "10.1.2.1-BadIPRange",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	Assert(t, err != nil, "nat pool creation on invalid ip range should fail")
}

func TestNatPoolUpdateOnNonExistentNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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
			IPRange: "10.1.2.1-10.1.3.1",
		},
	}

	// create nat pool
	err := ag.UpdateNatPool(&np)
	Assert(t, err != nil, "nat pool update on non existing nat pool should fail")
}
