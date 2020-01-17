// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//

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
			VlanID: 42,
		},
	}

	// make create network call
	err := ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")
	tnt, err := ag.FindNetwork(nt.ObjectMeta)
	AssertOk(t, err, "Network was not found in DB")
	Assert(t, tnt.Spec.VlanID == 42, "Network VLAN didn't match", tnt)

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
			VlanID: 84,
		},
	}
	err = ag.CreateNetwork(&nnt)
	Assert(t, (err != nil), "conflicting network creation succeeded")

	// verify list api works
	netList := ag.ListNetwork()
	Assert(t, len(netList) == 2, "Incorrect number of networks")

	// delete the network and verify its gone from db
	err = ag.DeleteNetwork(nt.Tenant, nt.Namespace, nt.Name)
	AssertOk(t, err, "Error deleting network")
	_, err = ag.FindNetwork(nt.ObjectMeta)
	Assert(t, err != nil, "Network was still found in database after deleting", ag)

	// verify you can not delete non-existing network
	err = ag.DeleteNetwork(nt.Tenant, nt.Namespace, nt.Name)
	Assert(t, err != nil, "deleting non-existing network succeeded", ag)
}

func TestNetworkDuplicatePrefix(t *testing.T) {
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
			VlanID: 42,
		},
	}

	// make create network call
	err := ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")
	tnt, err := ag.FindNetwork(nt.ObjectMeta)
	AssertOk(t, err, "Network was not found in DB")
	Assert(t, tnt.Spec.VlanID == 42, "Network VLAN didn't match", tnt)

	dupNt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "dupNet",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}

	// make create network call
	err = ag.CreateNetwork(&dupNt)
	Assert(t, err != nil, "Network Create with duplicate prefix succeeded")
}

func TestNetworkDuplicateVLanID(t *testing.T) {
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
			VlanID: 42,
		},
	}

	// make create network call
	err := ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")
	tnt, err := ag.FindNetwork(nt.ObjectMeta)
	AssertOk(t, err, "Network was not found in DB")
	Assert(t, tnt.Spec.VlanID == 42, "Network VLAN didn't match", tnt)

	dupNt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "dupNet",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}

	// make create network call
	err = ag.CreateNetwork(&dupNt)
	Assert(t, err != nil, "Network Create with duplicate vlan id succeeded")
}

func TestNetworkDuplicateEmptyPrefix(t *testing.T) {
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
			VlanID: 42,
		},
	}

	// make create network call
	err := ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")
	_, err = ag.FindNetwork(nt.ObjectMeta)
	AssertOk(t, err, "Network was not found in DB")

	dupNt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "dupNet",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 84,
		},
	}

	// make create network call
	err = ag.CreateNetwork(&dupNt)
	AssertOk(t, err, "Network Create with empty prefixes should succeed")
}

func TestNetworkDuplicatePrefixAcrossDifferentNamespaces(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing vrf
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testVrf",
		},
	}

	err := ag.CreateVrf(&vrf)
	AssertOk(t, err, "Creating vrf failed")

	// network message
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			VrfName: "testVrf",
			VlanID:  42,
		},
	}

	// make create network call
	err = ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")
	tnt, err := ag.FindNetwork(nt.ObjectMeta)
	AssertOk(t, err, "Network was not found in DB")
	Assert(t, tnt.Spec.VlanID == 42, "Network VLAN didn't match", tnt)

	ns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "newNamespace",
		},
		Spec: netproto.NamespaceSpec{
			NamespaceType: "CUSTOMER",
		},
	}
	err = ag.CreateNamespace(&ns)
	AssertOk(t, err, "Namespace creation must be successful")

	dupNt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "newNamespace",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}

	// make create network call
	err = ag.CreateNetwork(&dupNt)
	AssertOk(t, err, "Network Create with duplicate prefix across different namespaces must succeed")
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
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}

	// create network
	err = ag.CreateNetwork(&nt)
	AssertOk(t, err, "Error creating network")
	tnt, err := ag.FindNetwork(nt.ObjectMeta)
	AssertOk(t, err, "Tenant was not found in DB")
	Assert(t, tnt.Name == "updateNetwork", "Tenant names did not match", tnt)

	ntSpec := netproto.NetworkSpec{
		VlanID: 88,
	}

	nt.Spec = ntSpec

	err = ag.UpdateNetwork(&nt)
	AssertOk(t, err, "Error updating network")

	ntUpd := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "updateTenant",
			Namespace: "updateNamespace",
			Name:      "updateNetwork",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 100,
		},
	}
	err = ag.UpdateNetwork(&ntUpd)
	AssertOk(t, err, "Error updating network")

}

//--------------------- Corner Case Tests ---------------------//

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

func TestNetworkUpdateOnNonExistingNetwork(t *testing.T) {
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
			VlanID: 42,
		},
	}

	// make create network call
	err := ag.UpdateNetwork(&nt)
	Assert(t, err != nil, "network update on non existent network should fail")
}

func TestDeleteDefaultUntaggedNw(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	err := ag.DeleteNetwork("default", "default", "_internal_untagged_nw")
	Assert(t, err != nil, "Deleting internal untagged networks must fail")

}
