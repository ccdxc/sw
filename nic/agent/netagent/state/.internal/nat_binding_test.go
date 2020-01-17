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
func TestNatBindingCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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
	err = ag.DeleteNatBinding(nb.Tenant, nb.Namespace, nb.Name)
	AssertOk(t, err, "Error deleting nat pool")
	_, err = ag.FindNatBinding(nb.ObjectMeta)
	Assert(t, err != nil, "Nat Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNatBinding(nb.Tenant, nb.Namespace, nb.Name)
	Assert(t, err != nil, "deleting non-existing nat pool succeeded", ag)
}

func TestNatBindingUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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
	AssertOk(t, err, "Error updating nat binding")

	updNB := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "updateNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "preCreatedNatPool",
			IPAddress:   "192.168.1.1",
		},
	}
	err = ag.UpdateNatBinding(&updNB)
	AssertOk(t, err, "Error updating nat binding")

}

//--------------------- Corner Case Tests ---------------------//
func TestNatBidingToRemoteNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
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
	ag, _, _ := state.createNetAgent(t)
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

func TestNatBindingOnNonExistentNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	// nat binding
	nb := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "NonExistentNamespace",
			Name:      "updateNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "natPool",
			IPAddress:   "10.1.1.1",
		},
	}

	// create nat binding
	err := ag.CreateNatBinding(&nb)
	Assert(t, err != nil, "Nat Bindings with nat pools on non existent namespaces should fail")
}

func TestNatBindingDatapathFailure(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	// nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "NatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.0.0.1 - 10.1.1.1",
		},
	}

	// create nat binding
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "nat pool creation failed")

	nb := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "default/NatPool",
			IPAddress:   "BadIPAddress",
		},
	}

	// create nat binding
	err = ag.CreateNatBinding(&nb)
	Assert(t, err != nil, "Nat Bindings with bad ip address should fail")
}

func TestNatBindingUpdateOnNonExistentNatBinding(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	// nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "NatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.0.0.1 - 10.1.1.1",
		},
	}

	// create nat binding
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "nat pool creation failed")

	nb := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatBinding",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "default/NatPool",
			IPAddress:   "10.0.0.1",
		},
	}

	// create nat binding
	err = ag.UpdateNatBinding(&nb)
	Assert(t, err != nil, "Nat Binding update with non existing binding should fail")
}
