// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestPortCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	port := netproto.Port{
		TypeMeta: api.TypeMeta{Kind: "Port"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPort",
		},
		Spec: netproto.PortSpec{
			Speed:        "SPEED_100G",
			Type:         "TYPE_ETHERNET",
			AdminStatus:  "UP",
			BreakoutMode: "BREAKOUT_NONE",
			Lanes:        4,
		},
	}

	preExistingPortCount := len(ag.ListPort())

	// create port
	err := ag.CreatePort(&port)
	AssertOk(t, err, "Error creating port")
	p, err := ag.FindPort(port.ObjectMeta)
	AssertOk(t, err, "Port was not found in DB")
	Assert(t, p.Name == "testPort", "Port names did not match", port)

	// verify duplicate port creations succeed
	err = ag.CreatePort(&port)
	AssertOk(t, err, "Error creating duplicate port")

	// verify list api works.
	rtList := ag.ListPort()
	Assert(t, len(rtList) == preExistingPortCount+1, "Incorrect number of ports")

	// delete the port and verify its gone from db
	err = ag.DeletePort(port.Tenant, port.Namespace, port.Name)
	AssertOk(t, err, "Error deleting port")
	_, err = ag.FindPort(port.ObjectMeta)
	Assert(t, err != nil, "Port was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeletePort(port.Tenant, port.Namespace, port.Name)
	Assert(t, err != nil, "deleting non-existing port succeeded", ag)
}

func TestPortUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// port
	port := netproto.Port{
		TypeMeta: api.TypeMeta{Kind: "Port"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPort",
		},
		Spec: netproto.PortSpec{
			Speed:        "SPEED_100G",
			Type:         "TYPE_ETHERNET",
			AdminStatus:  "UP",
			BreakoutMode: "BREAKOUT_NONE",
			Lanes:        4,
		},
	}

	// create nat policy
	err := ag.CreatePort(&port)
	AssertOk(t, err, "Error creating port")
	p, err := ag.FindPort(port.ObjectMeta)
	AssertOk(t, err, "Port not found in DB")
	Assert(t, p.Name == "testPort", "Port names did not match", port)

	portSpec := netproto.PortSpec{
		AdminStatus: "DOWN",
	}

	port.Spec = portSpec

	err = ag.UpdatePort(&port)
	AssertOk(t, err, "Error updating port")

}

//--------------------- Corner Case Tests ---------------------//
func TestPortCreateOnNonExistingNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	rt := netproto.Port{
		TypeMeta: api.TypeMeta{Kind: "Port"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "BadNamsepace",
			Name:      "testPort",
		},
		Spec: netproto.PortSpec{
			Speed:        "SPEED_100G",
			Type:         "TYPE_ETHERNET",
			AdminStatus:  "UP",
			BreakoutMode: "BREAKOUT_NONE",
			Lanes:        4,
		},
	}

	// create port
	err := ag.CreatePort(&rt)
	Assert(t, err != nil, "port create on non existent namespace should fail")
}

func TestPortUpdateOnNonExistentPort(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	rt := netproto.Port{
		TypeMeta: api.TypeMeta{Kind: "Port"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPort",
		},
		Spec: netproto.PortSpec{
			Speed:        "SPEED_100G",
			Type:         "TYPE_ETHERNET",
			AdminStatus:  "UP",
			BreakoutMode: "BREAKOUT_NONE",
			Lanes:        4,
		},
	}

	// create port
	err := ag.UpdatePort(&rt)
	Assert(t, err != nil, "port udpate with non existent port should fail")
}
