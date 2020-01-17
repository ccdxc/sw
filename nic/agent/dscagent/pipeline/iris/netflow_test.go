// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleNetflow(t *testing.T) {
	t.Skip("Skipped till we figure out a way to ensure the lateral objects are correctly handled in the absensce of venice configs")
	t.Parallel()
	netflow := netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "Netflow"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetflow",
		},
		Spec: netproto.FlowExportPolicySpec{
			Interval: "30s",
			Exports: []netproto.ExportConfig{
				{
					Destination: "192.168.100.103",
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
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
		Status: netproto.FlowExportPolicyStatus{FlowExportPolicyID: 1},
	}

	err := HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Create, netflow, 65)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Delete, netflow, 65)
	if err != nil {
		t.Fatal(err)
	}
	//
	//err = HandleFlowExportPolicy(infraAPI, intfClient, 42, netflow, 65)
	//if err == nil {
	//	t.Fatal("Invalid op must return a valid error.")
	//}
}

//func TestHandleNetflowInfraFailures(t *testing.T) {
//	t.Parallel()
//	netflow := netproto.Netflow{
//		TypeMeta: api.TypeMeta{Kind: "Netflow"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "testNetflow",
//		},
//		Spec: netproto.NetflowSpec{
//			AdminStatus: "UP",
//			Src:         "10.10.10.10",
//			Dst:         "20.20.20.20",
//		},
//	}
//	i := newBadInfraAPI()
//	err := HandleNetflow(i, intfClient, types.Create, netflow, 65)
//	if err == nil {
//		t.Fatalf("Must return a valid error. Err: %v", err)
//	}
//
//	err = HandleNetflow(i, intfClient, types.Update, netflow, 65)
//	if err == nil {
//		t.Fatalf("Must return a valid error. Err: %v", err)
//	}
//
//	err = HandleNetflow(i, intfClient, types.Delete, netflow, 65)
//	if err == nil {
//		t.Fatalf("Must return a valid error. Err: %v", err)
//	}
//}
