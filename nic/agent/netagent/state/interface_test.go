package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

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

	// verify list api works
	intfList := ag.ListInterface()
	Assert(t, len(intfList) == 2+existingIfLen, "Incorrect number of interfaces")

	// delete lif
	err = ag.DeleteInterface(lif.Tenant, lif.Namespace, lif.Name)
	AssertOk(t, err, "Error deleting lif")
	intf, err = ag.FindInterface(lif.ObjectMeta)
	Assert(t, err != nil, "LIF found despite delete")

	// delete enic
	err = ag.DeleteInterface(enic.Tenant, enic.Namespace, enic.Name)
	AssertOk(t, err, "Error deleting ENIC")
	intf, err = ag.FindInterface(enic.ObjectMeta)
	Assert(t, err != nil, "ENIC found despite delete")

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

	// verify list api works.
	intfList := ag.ListInterface()
	Assert(t, len(intfList) == 2+existingIfLen, "Incorrect number of interfaces")
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
	err = ag.DeleteInterface(badLif.Tenant, badLif.Namespace, badLif.Name)
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
	err = ag.DeleteInterface(badLif.Tenant, badLif.Namespace, badLif.Name)
	Assert(t, err != nil, "Non existent tenant interface deletes should fail, it passed instead")

}
