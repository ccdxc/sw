// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleMirror(t *testing.T) {
	t.Skip("Skipped till we figure out a way to ensure the lateral objects are correctly handled in the absensce of venice configs")
	mirror := netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "Mirror"},
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

	err := HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Create, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleMirrorSession(infraAPI, telemetryClient, intfClient, epClient, types.Delete, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}
	//
	//err = HandleMirrorSession(infraAPI, intfClient, 42, mirror, 65)
	//if err == nil {
	//	t.Fatal("Invalid op must return a valid error.")
	//}
}

//func TestHandleMirrorInfraFailures(t *testing.T) {
//	t.Parallel()
//	mirror := netproto.Mirror{
//		TypeMeta: api.TypeMeta{Kind: "Mirror"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "testMirror",
//		},
//		Spec: netproto.MirrorSpec{
//			AdminStatus: "UP",
//			Src:         "10.10.10.10",
//			Dst:         "20.20.20.20",
//		},
//	}
//	i := newBadInfraAPI()
//	err := HandleMirror(i, intfClient, types.Create, mirror, 65)
//	if err == nil {
//		t.Fatalf("Must return a valid error. Err: %v", err)
//	}
//
//	err = HandleMirror(i, intfClient, types.Update, mirror, 65)
//	if err == nil {
//		t.Fatalf("Must return a valid error. Err: %v", err)
//	}
//
//	err = HandleMirror(i, intfClient, types.Delete, mirror, 65)
//	if err == nil {
//		t.Fatalf("Must return a valid error. Err: %v", err)
//	}
//}
