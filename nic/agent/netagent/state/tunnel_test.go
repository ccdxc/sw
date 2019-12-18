package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestTunnelCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	tun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}

	// create tunnel
	err := ag.CreateTunnel(&tun)
	AssertOk(t, err, "Error creating tunnel")
	tunnel, err := ag.FindTunnel(tun.ObjectMeta)
	AssertOk(t, err, "Tunnel was not found in DB")
	Assert(t, tunnel.Name == "testTunnel", "Tunnel names did not match", tunnel)

	// verify duplicate tunnel creations succeed
	err = ag.CreateTunnel(&tun)
	AssertOk(t, err, "Error creating duplicate tunnel")

	// verify list api works.
	rtList := ag.ListTunnel()
	Assert(t, len(rtList) == 1, "Incorrect number of tunnels")

	// delete the tunnel and verify its gone from db
	err = ag.DeleteTunnel(tun.Tenant, tun.Namespace, tun.Name)
	AssertOk(t, err, "Error deleting tunnel")
	_, err = ag.FindTunnel(tun.ObjectMeta)
	Assert(t, err != nil, "Tunnel was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteTunnel(tun.Tenant, tun.Namespace, tun.Name)
	Assert(t, err != nil, "deleting non-existing tunnel succeeded", ag)
}

func TestTunnelUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// tunnel
	tun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}

	// create nat policy
	err := ag.CreateTunnel(&tun)
	AssertOk(t, err, "Error creating tunnel")
	tunnel, err := ag.FindTunnel(tun.ObjectMeta)
	AssertOk(t, err, "Tunnel not found in DB")
	Assert(t, tunnel.Name == "testTunnel", "Tunnel names did not match", tunnel)
	updtun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}

	err = ag.UpdateTunnel(&updtun)
	AssertOk(t, err, "Error updating tunnel")

}

//--------------------- Corner Case Tests ---------------------//
func TestTunnelCreateOnNonExistingNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	tun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "BadNamsepace",
			Name:      "testTunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}

	// create tunnel
	err := ag.CreateTunnel(&tun)
	Assert(t, err != nil, "tunnel create on non existent namespace should fail")
}

func TestTunnelUpdateOnNonExistentTunnel(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	tun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}

	// create tunnel
	err := ag.UpdateTunnel(&tun)
	Assert(t, err != nil, "tunnel update with non existent tunnel should fail")
}
