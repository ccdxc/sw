// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//

func TestTenantCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// tenant
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testTenant",
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
	err = ag.DeleteTenant(tn.Name, tn.Name, tn.Name)
	AssertOk(t, err, "Error deleting tenant")
	_, err = ag.FindTenant(tn.ObjectMeta)
	Assert(t, err != nil, "Tenant was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteTenant(tn.Name, tn.Name, tn.Name)
	Assert(t, err != nil, "deleting non-existing tenant succeeded", ag)
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
	tnt, err := ag.FindTenant(tn.ObjectMeta)
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

	updTn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Tenant:          "updateTenant",
			Namespace:       "updateTenant",
			Name:            "updateTenant",
			ResourceVersion: "v2",
		},
	}
	err = ag.UpdateTenant(&updTn)
	AssertOk(t, err, "Error updating tenant")
}

//--------------------- Corner Case Tests ---------------------//

func TestTenantUpdateOnNonExistingTenant(t *testing.T) {
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
	err := ag.UpdateTenant(&tn)
	Assert(t, err != nil, "tenant updates on non existent tenant should fail")
}

func TestDefaultTenantDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// tenant
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
		},
	}

	// create tenant
	err := ag.DeleteTenant(tn.Name, tn.Name, tn.Name)
	Assert(t, err != nil, "default tenants should not be able to be deleted")
}
