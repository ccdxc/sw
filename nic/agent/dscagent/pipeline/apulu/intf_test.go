// +build apulu

package apulu

import (
	"testing"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func addDummyVrf() error {
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testVrf",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
	}
	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		return err
	}
	return nil
}

func TestHandleInterface(t *testing.T) {
	t.Parallel()
	intf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testInterface",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.InterfaceSpec{
			Type:        "L3",
			AdminStatus: "UP",
			VrfName:     "testVrf",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID:   42,
			InterfaceUUID: uuid.NewV4().String(),
			IFUplinkStatus: netproto.InterfaceUplinkStatus{
				PortID: 42,
			},
		},
	}

	if err := addDummyVrf(); err != nil {
		t.Fatal(err)
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

func TestHandleInterfaceInfraFailures(t *testing.T) {
	t.Skip("convertInterface return nil in case of badinfra api")
	t.Parallel()
	intf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testInterface",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.InterfaceSpec{
			Type:        "L3",
			AdminStatus: "UP",
			VrfName:     "testVrf",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID:   42,
			InterfaceUUID: uuid.NewV4().String(),
		},
	}
	if err := addDummyVrf(); err != nil {
		t.Fatal(err)
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
