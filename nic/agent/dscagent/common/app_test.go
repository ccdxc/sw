package common

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

	err := handleApp(infraAPI, types.Create, dns)
	if err != nil {
		t.Fatal(err)
	}

	err = handleApp(infraAPI, types.Update, dns)
	if err != nil {
		t.Fatal(err)
	}

	err = handleApp(infraAPI, types.Delete, dns)
	if err != nil {
		t.Fatal(err)
	}

	err = handleApp(infraAPI, 42, dns)
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
	err := handleApp(i, types.Create, dns)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = handleApp(i, types.Update, dns)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = handleApp(i, types.Delete, dns)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
