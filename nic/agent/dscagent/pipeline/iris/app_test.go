// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleApp(t *testing.T) {
	t.Parallel()
	dns := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
					MaxMessageLength:       42,
				},
			},
		},
	}

	err := HandleApp(infraAPI, types.Create, dns)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleApp(infraAPI, types.Update, dns)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleApp(infraAPI, types.Delete, dns)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleApp(infraAPI, 42, dns)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleAppInfraFailures(t *testing.T) {
	t.Parallel()
	dns := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
					MaxMessageLength:       42,
				},
			},
		},
	}

	i := newBadInfraAPI()
	err := HandleApp(i, types.Create, dns)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleApp(i, types.Update, dns)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleApp(i, types.Delete, dns)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
