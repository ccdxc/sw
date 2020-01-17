// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleTunnel(t *testing.T) {
	t.Parallel()
	tunnel := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTunnel",
		},
		Spec: netproto.TunnelSpec{
			AdminStatus: "UP",
			Src:         "10.10.10.10",
			Dst:         "20.20.20.20",
		},
	}

	err := HandleTunnel(infraAPI, intfClient, types.Create, tunnel, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleTunnel(infraAPI, intfClient, types.Update, tunnel, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleTunnel(infraAPI, intfClient, types.Delete, tunnel, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleTunnel(infraAPI, intfClient, 42, tunnel, 65)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleTunnelInfraFailures(t *testing.T) {
	t.Parallel()
	tunnel := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTunnel",
		},
		Spec: netproto.TunnelSpec{
			AdminStatus: "UP",
			Src:         "10.10.10.10",
			Dst:         "20.20.20.20",
		},
	}
	i := newBadInfraAPI()
	err := HandleTunnel(i, intfClient, types.Create, tunnel, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleTunnel(i, intfClient, types.Update, tunnel, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleTunnel(i, intfClient, types.Delete, tunnel, 65)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
