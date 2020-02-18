// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleInterfaceCollectors(t *testing.T) {
	collectorToIDMap := map[string]uint64{
		"testCollector1": uint64(1),
		"testCollector2": uint64(1),
		"testCollector3": uint64(2),
	}
	intf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testInterface",
		},
		Spec: netproto.InterfaceSpec{
			Type:         "UPLINK_MGMT",
			AdminStatus:  "UP",
			TxCollectors: []string{"testCollector1", "testCollector2"},
			RxCollectors: []string{"testCollector3"},
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: 42,
		},
	}

	err := HandleInterface(infraAPI, intfClient, types.Create, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Update, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Delete, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleInterfaceUplinkEth(t *testing.T) {
	collectorToIDMap := make(map[string]uint64)
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

	err := HandleInterface(infraAPI, intfClient, types.Create, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Update, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Delete, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, 42, intf, collectorToIDMap)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleInterfaceUplinkMgmt(t *testing.T) {
	collectorToIDMap := make(map[string]uint64)
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

	err := HandleInterface(infraAPI, intfClient, types.Create, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Update, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleInterface(infraAPI, intfClient, types.Delete, intf, collectorToIDMap)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleInterfaceUplinkEthInfraFailures(t *testing.T) {
	collectorToIDMap := make(map[string]uint64)
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
	err := HandleInterface(i, intfClient, types.Create, intf, collectorToIDMap)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleInterface(i, intfClient, types.Update, intf, collectorToIDMap)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleInterface(i, intfClient, types.Delete, intf, collectorToIDMap)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
