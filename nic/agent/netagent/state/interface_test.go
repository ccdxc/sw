// +build ignore

package state

import (
	"math"
	"testing"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// Tests lif, uplink and ENIC creates and deletes
func TestInterfacesCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	existingIfLen := len(ag.ListInterface())

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

	// create enic
	err := ag.CreateInterface(enic)
	AssertOk(t, err, "Error creating ENIC")
	intf, err := ag.FindInterface(enic.ObjectMeta)
	AssertOk(t, err, "ENIC was not found in DB")
	Assert(t, intf.Name == "testEnic", "Enic names did not match", intf)

	// verify list api works
	intfList := ag.ListInterface()
	Assert(t, len(intfList) == 1+existingIfLen, "Incorrect number of interfaces")

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

	// create enic
	err := ag.CreateInterface(enic)
	AssertOk(t, err, "Error creating ENIC")
	intf, err := ag.FindInterface(enic.ObjectMeta)
	AssertOk(t, err, "ENIC was not found in DB")
	Assert(t, intf.Name == "testEnic", "Enic names did not match", intf)

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

	// update enic
	err = ag.UpdateInterface(downEnic)
	AssertOk(t, err, "Error updating ENIC")
	intf, err = ag.FindInterface(downEnic.ObjectMeta)
	AssertOk(t, err, "ENIC not found in DB")
	AssertEquals(t, "DOWN", intf.Spec.AdminStatus, "Expected ENIC to be down")

	// verify list api works.
	intfList := ag.ListInterface()
	Assert(t, len(intfList) == 1+existingIfLen, "Incorrect number of interfaces")
}

func TestDuplicateInterfaceCreate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// enic
	enic := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testENIC",
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
	err := ag.CreateInterface(enic)
	AssertOk(t, err, "Error creating enic")
	intf, err := ag.FindInterface(enic.ObjectMeta)
	AssertOk(t, err, "ENIC was not found in DB")
	Assert(t, intf.Name == "testENIC", "ENIC names did not match", intf)

	// create enic again
	err = ag.CreateInterface(enic)
	AssertOk(t, err, "Duplicate interface create failed when we expect it to be successful.")

	// verify duplicate interface name with different content does not succeed
	dupLif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testENIC",
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

func TestInterfaceUpdateHandlerUPToDOWNTransition(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	mockLif, ok := ag.findIntfByName("lif1")

	// Trigger a mock If down
	hwLifResp := &halproto.LifGetResponse{
		ApiStatus: 0,
		Spec: &halproto.LifSpec{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
		},
		Status: &halproto.LifStatus{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
			LifStatus: halproto.IfStatus_IF_STATUS_DOWN,
		},
	}
	AssertEquals(t, true, ok, "Failed to find lif1")
	err := ag.LifUpdateHandler(hwLifResp)
	AssertOk(t, err, "LifUpdateHandler failed.")

	updatedMockLif, ok := ag.findIntfByName("lif1")
	AssertEquals(t, true, ok, "Failed to find lif1")
	AssertEquals(t, "DOWN", updatedMockLif.Status.OperStatus, "Oper status doesn't reflect down status")
}

func TestInterfaceUpdateHandlerDOWNToUPTransition(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	mockLif, ok := ag.findIntfByName("lif1")

	// Trigger a mock If down
	hwLifResp := &halproto.LifGetResponse{
		ApiStatus: 0,
		Spec: &halproto.LifSpec{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
		},
		Status: &halproto.LifStatus{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
			LifStatus: halproto.IfStatus_IF_STATUS_DOWN,
		},
	}
	AssertEquals(t, true, ok, "Failed to find lif1")
	err := ag.LifUpdateHandler(hwLifResp)
	AssertOk(t, err, "LifUpdateHandler failed.")

	updatedHwLifResp := &halproto.LifGetResponse{
		ApiStatus: 0,
		Spec: &halproto.LifSpec{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
		},
		Status: &halproto.LifStatus{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
			LifStatus: halproto.IfStatus_IF_STATUS_UP,
		},
	}
	err = ag.LifUpdateHandler(updatedHwLifResp)
	AssertOk(t, err, "LifUpdateHandler failed.")

	updatedMockLif, ok := ag.findIntfByName("lif1")
	AssertEquals(t, true, ok, "Failed to find lif1")
	AssertEquals(t, "UP", updatedMockLif.Status.OperStatus, "Oper status doesn't reflect up status")
}

func TestInterfaceUpdateHandlerNameChanges(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	mockLif, ok := ag.findIntfByName("lif1")

	// Trigger a mock If down
	hwLifResp := &halproto.LifGetResponse{
		ApiStatus: 0,
		Spec: &halproto.LifSpec{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
			Name: "gandalf-the-grey",
		},
		Status: &halproto.LifStatus{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
			LifStatus: halproto.IfStatus_IF_STATUS_DOWN,
		},
	}
	AssertEquals(t, true, ok, "Failed to find lif1")
	err := ag.LifUpdateHandler(hwLifResp)
	AssertOk(t, err, "LifUpdateHandler failed.")

	AssertEquals(t, "gandalf-the-grey", mockLif.Status.IFHostStatus.HostIfName, "IF Names did not match")

	updatedHwLifResp := &halproto.LifGetResponse{
		ApiStatus: 0,
		Spec: &halproto.LifSpec{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
			Name: "gandalf-the-white",
		},
		Status: &halproto.LifStatus{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: mockLif.Status.InterfaceID,
				},
			},
			LifStatus: halproto.IfStatus_IF_STATUS_UP,
		},
	}
	err = ag.LifUpdateHandler(updatedHwLifResp)
	AssertOk(t, err, "LifUpdateHandler failed.")

	updatedMockLif, ok := ag.findIntfByName("lif1")
	AssertEquals(t, true, ok, "Failed to find lif1")
	AssertEquals(t, "UP", updatedMockLif.Status.OperStatus, "Oper status doesn't reflect up status")
	AssertEquals(t, "gandalf-the-white", mockLif.Status.IFHostStatus.HostIfName, "IF Names did not match")
}

func TestInterfaceGetByID(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	intf, err := ag.GetInterfaceByID(1)
	AssertOk(t, err, "Getting interface by id must work")
	AssertEquals(t, uint64(1), intf.Status.InterfaceID, "Interface id must match")
}

func TestInterfaceGetByIDFailure(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	intf, err := ag.GetInterfaceByID(math.MaxInt64)
	Assert(t, err != nil, "Getting interface on non existing interface must error out")
	AssertEquals(t, (*netproto.Interface)(nil), intf, "Interface must be nil")
}
