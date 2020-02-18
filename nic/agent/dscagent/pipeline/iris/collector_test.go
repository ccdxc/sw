// +build iris

package iris

import (
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestMirrorSessionIDRefcouting(t *testing.T) {
	cols := []netproto.Collector{
		{
			TypeMeta: api.TypeMeta{Kind: "Collector"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testCollector1",
			},
			Spec: netproto.CollectorSpec{
				VrfName:     "default",
				Destination: "192.168.100.101",
			},
			Status: netproto.CollectorStatus{Collector: 1},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Collector"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testCollector2",
			},
			Spec: netproto.CollectorSpec{
				VrfName:     "default",
				Destination: "192.168.100.102",
			},
			Status: netproto.CollectorStatus{Collector: 2},
		},
	}
	mirrors := []netproto.MirrorSession{
		{
			TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testMirror1",
			},
			Spec: netproto.MirrorSessionSpec{
				VrfName:    "default",
				PacketSize: 128,
				Collectors: []netproto.MirrorCollector{
					{
						ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.101"},
					},
				},
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testMirror2",
			},
			Spec: netproto.MirrorSessionSpec{
				VrfName:    "default",
				PacketSize: 128,
				Collectors: []netproto.MirrorCollector{
					{
						ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.101"},
					},
				},
			},
		},
	}
	// Create collectors
	for _, col := range cols {
		err := HandleCollector(infraAPI, telemetryClient, types.Create, col, 65)
		if err != nil {
			t.Fatal(err)
		}
	}
	// Create mirror sessions
	for _, mirror := range mirrors {
		err := HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Create, mirror, 65)
		if err != nil {
			t.Fatal(err)
		}
	}
	// Verify that first collector delete is erred out
	if err := HandleCollector(infraAPI, telemetryClient, types.Delete, cols[0], 65); err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
	// Verify that second collector delete is successful
	if err := HandleCollector(infraAPI, telemetryClient, types.Delete, cols[1], 65); err != nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
	// Verify the mappings
	mirrorIDs, ok := mirrorDestToIDMapping["default-192.168.100.101"]
	if !ok {
		t.Fatalf("Expected default-192.168.100.101 to be a key in mirrorDestToIDMapping, %v", mirrorDestToIDMapping)
	}
	if len(mirrorIDs.mirrorKeys) != 2 {
		t.Fatalf("Expected 2 entries in mirrorKeys %v", mirrorIDs.mirrorKeys)
	}
	if !reflect.DeepEqual(mirrorIDs.mirrorKeys, []string{"default/default/testMirror1", "default/default/testMirror2"}) {
		t.Fatalf("Unexpected mirror %v", mirrorIDs.mirrorKeys)
	}

	// Remove one mirror sessions and verify the mappings
	err := HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirrors[0], 65)
	if err != nil {
		t.Fatal(err)
	}

	mirrorIDs, ok = mirrorDestToIDMapping["default-192.168.100.101"]
	if !ok {
		t.Fatalf("Expected default-192.168.100.101 to be a key in mirrorDestToIDMapping, %v", mirrorDestToIDMapping)
	}
	if len(mirrorIDs.mirrorKeys) != 1 {
		t.Fatalf("Expected 1 entries in mirrorKeys %v", mirrorIDs.mirrorKeys)
	}
	if !reflect.DeepEqual(mirrorIDs.mirrorKeys, []string{"default/default/testMirror2"}) {
		t.Fatalf("Unexpected mirror %v", mirrorIDs.mirrorKeys)
	}

	// Remove the last mirror session and verify that no mappings are present
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirrors[1], 65)
	if err != nil {
		t.Fatal(err)
	}

	// Verify that first collector delete is successful
	if err := HandleCollector(infraAPI, telemetryClient, types.Delete, cols[0], 65); err != nil {
		t.Fatal(err)
	}

	if _, ok = mirrorDestToIDMapping["default-192.168.100.101"]; ok {
		t.Fatalf("Expected mirrorDestToIDMapping to not have default-192.168.100.101 %v", mirrorDestToIDMapping["default-192.168.100.101"])
	}
}

func TestHandleCollector(t *testing.T) {
	col := netproto.Collector{
		TypeMeta: api.TypeMeta{Kind: "Collector"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testCollector",
		},
		Spec: netproto.CollectorSpec{
			Destination: "192.168.100.101",
		},
		Status: netproto.CollectorStatus{Collector: 1},
	}

	err := HandleCollector(infraAPI, telemetryClient, types.Create, col, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleCollector(infraAPI, telemetryClient, types.Update, col, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleCollector(infraAPI, telemetryClient, types.Delete, col, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleCollector(infraAPI, telemetryClient, 42, col, 65)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleCollectorInfraFailures(t *testing.T) {
	col := netproto.Collector{
		TypeMeta: api.TypeMeta{Kind: "Collector"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testCollector",
		},
		Spec: netproto.CollectorSpec{
			Destination: "192.168.100.101",
		},
		Status: netproto.CollectorStatus{Collector: 1},
	}

	i := newBadInfraAPI()
	err := HandleCollector(i, telemetryClient, types.Create, col, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleCollector(i, telemetryClient, types.Update, col, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleCollector(i, telemetryClient, types.Delete, col, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
