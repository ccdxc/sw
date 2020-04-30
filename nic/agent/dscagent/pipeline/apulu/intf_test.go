// +build apulu

package apulu

import (
	"testing"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func addDummyVrf() (string, error) {
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

	return vrf.UUID, infraAPI.Store(vrf.Kind, vrf.GetKey(), dat)
}

func TestHandleInterface(t *testing.T) {
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

	if _, err := addDummyVrf(); err != nil {
		t.Fatal(err)
	}
	err := HandleInterface(infraAPI, intfClient, subnetClient, types.Create, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, subnetClient, types.Update, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, subnetClient, types.Delete, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, subnetClient, 42, intf)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleLoopbackInterface(t *testing.T) {
	ipam := netproto.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPAM",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.IPAMPolicySpec{
			DHCPRelay: &netproto.DHCPRelayPolicy{
				Servers: []*netproto.DHCPServer{
					{
						IPAddress: "192.168.100.101",
					},
				},
			},
		},
	}

	intf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testInterface",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.InterfaceSpec{
			Type:        "LOOPBACK",
			AdminStatus: "UP",
			VrfName:     "testVrf",
			IPAddress:   "1.1.1.1/32",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID:   42,
			InterfaceUUID: uuid.NewV4().String(),
			IFUplinkStatus: netproto.InterfaceUplinkStatus{
				PortID: 42,
			},
		},
	}

	uid, err := addDummyVrf()
	if err != nil {
		t.Fatal(err)
	}
	vrfuid, err := uuid.FromString(uid)
	if err != nil {
		t.Fatal(err)
	}
	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Create, ipam, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, subnetClient, types.Create, intf)
	if err != nil {
		t.Fatal(err)
	}

	intf.Spec.IPAddress = "2.2.2.2/32"
	err = HandleInterface(infraAPI, intfClient, subnetClient, types.Update, intf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, subnetClient, types.Delete, intf)
	if err != nil {
		t.Fatal(err)
	}

	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Delete, ipam, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
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
	if _, err := addDummyVrf(); err != nil {
		t.Fatal(err)
	}
	i := newBadInfraAPI()
	err := HandleInterface(i, intfClient, subnetClient, types.Create, intf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleInterface(i, intfClient, subnetClient, types.Update, intf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleInterface(i, intfClient, subnetClient, types.Delete, intf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
