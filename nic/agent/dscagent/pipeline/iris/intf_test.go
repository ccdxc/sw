// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestMirrorSessionToInterfaceMapping(t *testing.T) {
	t.Parallel()
	// Mirror sessions
	mirrors := []netproto.MirrorSession{
		{
			TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testMirror1",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testMirror2",
			},
		},
	}
	// Create mirror sessions
	for _, mirror := range mirrors {
		err := HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Create, mirror, 65)
		if err != nil {
			t.Fatal(err)
		}
	}
	// Interface
	intf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testInterface1",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "UPLINK_ETH",
			AdminStatus: "UP",
			TxMirrorSessions: []*netproto.MirrorSession{
				&mirrors[0],
			},
		},
	}
	// Create interface with 1st mirror as Tx mirror session
	err := HandleInterface(infraAPI, intfClient, types.Create, intf)
	if err != nil {
		t.Fatal(err)
	}
	// Mirror session removal should fail
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirrors[0], 65)
	if err == nil {
		t.Fatal(err)
	}
	// Mirror session removal of other mirror session should go through
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirrors[1], 65)
	if err != nil {
		t.Fatal(err)
	}
	// Create the mirror session back for other mirror
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Create, mirrors[1], 65)
	if err != nil {
		t.Fatal(err)
	}
	// Attach the mirror session to interface
	intf.Spec.TxMirrorSessions = []*netproto.MirrorSession{
		&mirrors[1],
	}
	err = HandleInterface(infraAPI, intfClient, types.Update, intf)
	if err != nil {
		t.Fatal(err)
	}
	// Mirror session removal of other mirror session should err
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirrors[1], 65)
	if err == nil {
		t.Fatal(err)
	}
	// Mirror session removal of first mirror session should go through
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirrors[0], 65)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleInterfaceUplinkEth(t *testing.T) {
	t.Parallel()
	intf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testInterface",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "UPLINK_ETH",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: 42,
			IFUplinkStatus: netproto.InterfaceUplinkStatus{
				PortID: 42,
			},
		},
	}

	err := HandleInterface(infraAPI, intfClient, types.Create, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Update, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Delete, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, 42, intf)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleInterfaceUplinkMgmt(t *testing.T) {
	t.Parallel()
	intf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testInterface",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "UPLINK_MGMT",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: 42,
		},
	}

	err := HandleInterface(infraAPI, intfClient, types.Create, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Update, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Delete, intf)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleInterfaceUplinkEthInfraFailures(t *testing.T) {
	t.Parallel()
	intf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testInterface",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "UPLINK_ETH",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: 42,
		},
	}
	i := newBadInfraAPI()
	err := HandleInterface(i, intfClient, types.Create, intf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleInterface(i, intfClient, types.Update, intf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleInterface(i, intfClient, types.Delete, intf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
