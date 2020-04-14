// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleMirrorSessionUpdates(t *testing.T) {
	mirror := netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testMirror",
		},
		Spec: netproto.MirrorSessionSpec{
			PacketSize: 128,
			Collectors: []netproto.MirrorCollector{
				{
					ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.101"},
				},
			},
		},
	}
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}
	err := HandleVrf(infraAPI, vrfClient, types.Create, vrf)
	if err != nil {
		t.Fatal(err)
	}

	cols, err := infraAPI.List("Collector")
	if err != nil {
		t.Fatal(err)
	}
	oldCount := len(cols)
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Create, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}

	cols, err = infraAPI.List("Collector")
	if err != nil {
		t.Fatal(err)
	}
	if len(cols) != oldCount+1 {
		t.Fatalf("Expected %d collector, got %d", oldCount+1, len(cols))
	}
	mirror.Spec.Collectors[0].ExportCfg.Destination = "192.168.100.103"
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Update, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}
	cols, err = infraAPI.List("Collector")
	if err != nil {
		t.Fatal(err)
	}
	if len(cols) != oldCount+1 {
		t.Fatalf("Expected %d collector, got %d", oldCount+1, len(cols))
	}
	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}
	cols, err = infraAPI.List("Collector")
	if err != nil {
		t.Fatal(err)
	}
	if len(cols) != oldCount {
		t.Fatalf("Expected %d collector, got %d", oldCount, len(cols))
	}
	err = HandleVrf(infraAPI, vrfClient, types.Delete, vrf)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleMirrorSession(t *testing.T) {
	mirror := netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testMirror",
		},
		Spec: netproto.MirrorSessionSpec{
			PacketSize: 128,
			Collectors: []netproto.MirrorCollector{
				{
					ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.101"},
				},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.103"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.101"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "120",
							},
							{
								Protocol: "udp",
								Port:     "10001-10020",
							},
							{
								Protocol: "icmp",
							},
						},
					},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.101"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.103"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "120",
							},
						},
					},
				},
			},
		},
		Status: netproto.MirrorSessionStatus{MirrorSessionID: 1},
	}
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}
	err := HandleVrf(infraAPI, vrfClient, types.Create, vrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Create, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, 42, mirror, 65)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleMirrorInfraFailures(t *testing.T) {
	mirror := netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testMirror",
		},
		Spec: netproto.MirrorSessionSpec{
			PacketSize: 128,
			Collectors: []netproto.MirrorCollector{
				{
					ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.101"},
				},
			},
		},
	}

	i := newBadInfraAPI()
	err := HandleMirrorSession(i, telemetryClient, intfClient, epClient, types.Create, mirror, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleMirrorSession(i, telemetryClient, intfClient, epClient, types.Update, mirror, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleMirrorSession(i, telemetryClient, intfClient, epClient, types.Delete, mirror, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
