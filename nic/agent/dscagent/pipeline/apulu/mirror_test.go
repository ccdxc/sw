// +build apulu

package apulu

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

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
		},
		Status: netproto.MirrorSessionStatus{MirrorSessionID: 1},
	}

	err := HandleMirrorSession(infraAPI, mirrorClient, types.Create, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleMirrorSession(infraAPI, mirrorClient, types.Update, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleMirrorSession(infraAPI, mirrorClient, types.Delete, mirror, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleMirrorSession(infraAPI, mirrorClient, 42, mirror, 65)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleMirrorSessionInfraFailures(t *testing.T) {
	t.Parallel()
	mirror := netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testMirror",
		},
		Spec: netproto.MirrorSessionSpec{
			PacketSize: 128,
		},
	}
	i := newBadInfraAPI()
	err := HandleMirrorSession(i, mirrorClient, types.Create, mirror, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleMirrorSession(i, mirrorClient, types.Update, mirror, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleMirrorSession(i, mirrorClient, types.Delete, mirror, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
