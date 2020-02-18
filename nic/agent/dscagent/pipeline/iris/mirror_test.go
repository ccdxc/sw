// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleMirrorSession(t *testing.T) {
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

	if err := HandleCollector(infraAPI, telemetryClient, types.Create, col, 65); err != nil {
		t.Fatal(err)
	}
	err := HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Create, mirror, 65)
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
