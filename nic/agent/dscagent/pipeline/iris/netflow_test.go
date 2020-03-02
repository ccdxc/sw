// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleNetflowCollector(t *testing.T) {
	netflows := []netproto.FlowExportPolicy{
		{
			TypeMeta: api.TypeMeta{Kind: "Netflow"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testNetflow1",
			},
			Spec: netproto.FlowExportPolicySpec{
				Interval: "30s",
				Exports: []netproto.ExportConfig{
					{
						Destination: "192.168.100.101",
						Transport: &netproto.ProtoPort{
							Protocol: "udp",
							Port:     "2055",
						},
					},
				},
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Netflow"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testNetflow2",
			},
			Spec: netproto.FlowExportPolicySpec{
				Interval: "30s",
				Exports: []netproto.ExportConfig{
					{
						Destination: "192.168.100.101",
						Transport: &netproto.ProtoPort{
							Protocol: "udp",
							Port:     "2055",
						},
					},
				},
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Netflow"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testNetflow3",
			},
			Spec: netproto.FlowExportPolicySpec{
				Interval: "30s",
				Exports: []netproto.ExportConfig{
					{
						Destination: "192.168.100.101",
						Transport: &netproto.ProtoPort{
							Protocol: "udp",
							Port:     "2055",
						},
					},
				},
			},
		},
	}
	for _, netflow := range netflows {
		if err := HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Create, netflow, 65); err != nil {
			t.Fatal(err)
		}
	}
	// Make sure only 1 collector is created i.e "-192.168.100.101-2055"
	if len(CollectorToNetflow) != 1 {
		t.Fatalf("Expected 1 key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	netflowKeys, ok := CollectorToNetflow["-192.168.100.101-2055"]
	if !ok {
		t.Fatalf("Expected -192.168.100.101-2055 to be key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	if len(netflowKeys.NetflowKeys) != 3 {
		t.Fatalf("Expected 3 netflows. %v", netflowKeys.NetflowKeys)
	}

	// Update first netflow to a different IP and make sure a new key is generated
	netflows[0].Spec.Exports[0].Destination = "192.168.100.102"
	if err := HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Update, netflows[0], 65); err != nil {
		t.Fatal(err)
	}
	if len(CollectorToNetflow) != 2 {
		t.Fatalf("Expected 2 key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	netflowKeys, ok = CollectorToNetflow["-192.168.100.101-2055"]
	if !ok {
		t.Fatalf("Expected -192.168.100.101-2055 to be key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	if len(netflowKeys.NetflowKeys) != 2 {
		t.Fatalf("Expected 2 netflows. %v", netflowKeys.NetflowKeys)
	}
	netflowKeys, ok = CollectorToNetflow["-192.168.100.102-2055"]
	if !ok {
		t.Fatalf("Expected -192.168.100.102-2055 to be key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	if len(netflowKeys.NetflowKeys) != 1 {
		t.Fatalf("Expected 1 netflows. %v", netflowKeys.NetflowKeys)
	}

	// Update the last netflow to have a different port
	netflows[2].Spec.Exports[0].Transport.Port = "2056"
	if err := HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Update, netflows[2], 65); err != nil {
		t.Fatal(err)
	}
	if len(CollectorToNetflow) != 3 {
		t.Fatalf("Expected 3 key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	netflowKeys, ok = CollectorToNetflow["-192.168.100.101-2056"]
	if !ok {
		t.Fatalf("Expected -192.168.100.101-2056 to be key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	if len(netflowKeys.NetflowKeys) != 1 {
		t.Fatalf("Expected 1 netflows. %v", netflowKeys.NetflowKeys)
	}

	// Update the second netflow
	netflows[1].Spec.Exports[0].Transport.Port = "2056"
	if err := HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Update, netflows[1], 65); err != nil {
		t.Fatal(err)
	}
	if len(CollectorToNetflow) != 2 {
		t.Fatalf("Expected 2 key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	if _, ok = CollectorToNetflow["-192.168.100.101-2055"]; ok {
		t.Fatalf("Expected -192.168.100.101-2055 to be deleted. %v", CollectorToNetflow)
	}

	// Update the first one
	netflows[0].Spec.Exports[0].Destination = "192.168.100.101"
	netflows[0].Spec.Exports[0].Transport.Port = "2056"
	if err := HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Update, netflows[0], 65); err != nil {
		t.Fatal(err)
	}
	if len(CollectorToNetflow) != 1 {
		t.Fatalf("Expected 2 key in CollectorToNetflow, %v", CollectorToNetflow)
	}
	if _, ok = CollectorToNetflow["-192.168.100.101-2056"]; !ok {
		t.Fatalf("Expected -192.168.100.101-2056 to be in the map. %v", CollectorToNetflow)
	}

	// Delete the flows
	for _, netflow := range netflows {
		if err := HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Delete, netflow, 65); err != nil {
			t.Fatal(err)
		}
	}
	if len(CollectorToNetflow) != 0 {
		t.Fatalf("Expected 0 keys in CollectorToNetflow, %v", CollectorToNetflow)
	}
}

func TestHandleNetflowUpdates(t *testing.T) {
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
					Destination: "192.168.100.101",
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}
	internalCol := "192.168.100.101"
	internalCol1 := "192.168.100.103"
	var col1Count, col2Count int
	if _, ok := lateralDB[internalCol]; ok {
		col1Count = len(lateralDB[internalCol])
	}
	if _, ok := lateralDB[internalCol1]; ok {
		col2Count = len(lateralDB[internalCol1])
	}
	err := HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Create, netflow, 65)
	if err != nil {
		t.Fatal(err)
	}

	if _, ok := lateralDB[internalCol]; !ok {
		t.Fatalf("192.168.100.101 collector not created. DB %v", lateralDB)
	}
	if len(lateralDB[internalCol]) != col1Count+1 {
		t.Fatalf("Collector keys not populated for 192.168.100.101. %v", lateralDB[internalCol])
	}
	netflow.Spec.Exports[0].Destination = "192.168.100.103"
	err = HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Update, netflow, 65)
	if err != nil {
		t.Fatal(err)
	}

	if _, ok := lateralDB[internalCol]; ok && len(lateralDB[internalCol]) != col1Count {
		t.Fatalf("192.168.100.101 should be removed. DB %v", lateralDB)
	}
	if _, ok := lateralDB[internalCol1]; !ok {
		t.Fatalf("192.168.100.103 collector not created. DB %v", lateralDB)
	}
	if len(lateralDB[internalCol1]) != col2Count+1 {
		t.Fatalf("Collector keys not populated. %v", lateralDB[internalCol1])
	}
	err = HandleFlowExportPolicy(infraAPI, telemetryClient, intfClient, epClient, types.Delete, netflow, 65)
	if err != nil {
		t.Fatal(err)
	}
	if _, ok := lateralDB[internalCol]; ok && len(lateralDB[internalCol]) != col1Count {
		t.Fatalf("192.168.100.101 should be removed. DB %v", lateralDB)
	}
	if _, ok := lateralDB[internalCol1]; ok && len(lateralDB[internalCol1]) != col2Count {
		t.Fatalf("192.168.100.101 should be removed. DB %v", lateralDB)
	}
}

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
