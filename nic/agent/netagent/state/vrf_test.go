// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestVrfCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// vrf
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNamespace",
			Name:      "testTenant",
		},
	}

	// create backing tenant and namespace
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testTenant",
		},
	}
	err := ag.CreateTenant(&tn)
	AssertOk(t, err, "Error creating tenant")

	ns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testNamespace",
		},
	}
	err = ag.CreateNamespace(&ns)
	AssertOk(t, err, "Error creating namespace")

	existingNS := len(ag.ListVrf())

	// create vrf
	err = ag.CreateVrf(&vrf)
	AssertOk(t, err, "Error creating vrf")
	tnt, err := ag.FindVrf(vrf.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, tnt.Name == "testTenant", "Tenant names did not match", tnt)

	// verify duplicate tenant creatiovrf succeed
	err = ag.CreateVrf(&vrf)
	AssertOk(t, err, "Error creating duplicate tenant")

	// verify list api works.
	vrfList := ag.ListVrf()
	Assert(t, len(vrfList) == existingNS+1, "Incorrect number of vrf")

	// delete the vrf and verify its gone from db
	err = ag.DeleteVrf(vrf.Tenant, vrf.Namespace, vrf.Name)
	AssertOk(t, err, "Error deleting network")
	_, err = ag.FindVrf(vrf.ObjectMeta)
	Assert(t, err != nil, "Tenant was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteVrf(vrf.Tenant, vrf.Namespace, vrf.Name)
	Assert(t, err != nil, "deleting non-existing network succeeded", ag)
}

func TestVrfUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// vrf
	vrf := &netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "updateTenant",
			Namespace: "updateNamespace",
			Name:      "updateVrf",
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

	ns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "updateTenant",
			Name:   "updateNamespace",
		},
	}
	err = ag.CreateNamespace(&ns)
	AssertOk(t, err, "Error creating namespace")

	// create vrf
	err = ag.CreateVrf(vrf)
	AssertOk(t, err, "Error creating vrf")
	vrf, err = ag.FindVrf(vrf.ObjectMeta)
	AssertOk(t, err, "Vrf was not found in DB")
	Assert(t, vrf.Name == "updateVrf", "Vrf names did not match", vrf)

	vrfSpec := netproto.VrfSpec{
		VrfType: "CUSTOMER",
	}

	vrf.Spec = vrfSpec

	err = ag.UpdateVrf(vrf)
	AssertOk(t, err, "Error updating vrf")

	updNS := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:          "updateTenant",
			Namespace:       "updateNamespace",
			Name:            "updateVrf",
			ResourceVersion: "v3",
		},
	}
	err = ag.UpdateVrf(&updNS)
	AssertOk(t, err, "Error updating vrf")
}

//--------------------- Corner Case Tests ---------------------//
func TestDefaultVrfDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// vrf
	defaultVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
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

	err = ag.DeleteVrf(defaultVrf.Tenant, defaultVrf.Namespace, defaultVrf.Name)
	Assert(t, err != nil, "default vrf deletes should fail. It passed instead")
}

func TestVrfUpdateOnNonExistingVrf(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// vrf
	defaultNS := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "NonExistingVrf",
		},
	}

	err := ag.UpdateVrf(&defaultNS)
	Assert(t, err != nil, "updates on non existing vrfs should fail")

}
