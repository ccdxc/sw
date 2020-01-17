// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
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
	tnt, err := ag.FindNamespace(ns.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, tnt.Name == "testTenant", "Tenant names did not match", tnt)

	// verify duplicate tenant creations succeed
	err = ag.CreateNamespace(&ns)
	AssertOk(t, err, "Error creating duplicate tenant")

	// verify list api works.
	nsList := ag.ListNamespace()
	Assert(t, len(nsList) == existingNS+1, "Incorrect number of namespace")

	// delete the namespace and verify its gone from db
	err = ag.DeleteNamespace(ns.Tenant, ns.Name, ns.Name)
	AssertOk(t, err, "Error deleting network")
	_, err = ag.FindNamespace(ns.ObjectMeta)
	Assert(t, err != nil, "Tenant was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNamespace(ns.Tenant, ns.Name, ns.Name)
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
	namespace, err := ag.FindNamespace(ns.ObjectMeta)
	AssertOk(t, err, "Namespace was not found in DB")
	Assert(t, namespace.Name == "updateNamespace", "Namespace names did not match", namespace)

	nsSpec := netproto.NamespaceSpec{
		NamespaceType: "CUSTOMER",
	}

	ns.Spec = nsSpec

	err = ag.UpdateNamespace(&ns)
	AssertOk(t, err, "Error updating namespace")

	updNS := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant:          "updateTenant",
			Name:            "updateNamespace",
			ResourceVersion: "v3",
		},
	}
	err = ag.UpdateNamespace(&updNS)
	AssertOk(t, err, "Error updating namespace")
}

//--------------------- Corner Case Tests ---------------------//
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

	err = ag.DeleteNamespace(defaultNS.Tenant, defaultNS.Name, defaultNS.Name)
	Assert(t, err != nil, "default namespace deletes under default tenant should fail. It passed instead")

	// Delete a non-default tenant and ensure that the default ns under it is gone
	err = ag.DeleteTenant(tn.Name, tn.Name, tn.Name)
	AssertOk(t, err, "Non default tenant deletes should be disallowed.")

	_, err = ag.FindNamespace(api.ObjectMeta{Tenant: "testTenant", Namespace: "default", Name: "default"})
	Assert(t, err != nil, "deleting a non default tenant should automatically trigger the deletion of the default namespace under it.")
}

func TestNamespaceUpdateOnNonExistingNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// namespace
	defaultNS := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "NonExistingNamespace",
		},
	}

	err := ag.UpdateNamespace(&defaultNS)
	Assert(t, err != nil, "updates on non existing namespaces should fail")

}
