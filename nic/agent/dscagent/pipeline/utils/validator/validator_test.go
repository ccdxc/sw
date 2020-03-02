package validator

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestValidateFlowExportPolicy(t *testing.T) {
	netflow := netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "Netflow"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetflow",
		},
		Spec: netproto.FlowExportPolicySpec{
			VrfName:  "default",
			Interval: "30s",
			Exports: []netproto.ExportConfig{
				{
					Destination: "192.168.100.101",
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
				{
					Destination: "192.168.100.101",
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
				{
					Destination: "192.168.100.101",
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
				{
					Destination: "192.168.100.101",
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
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
	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		t.Fatal(err)
	}
	collectorToKeys := map[string]int{
		"default-192.168.100.101-2055": 1,
		"default-192.168.100.102-2055": 1,
		"default-192.168.100.103-2055": 1,
		"default-192.168.100.104-2055": 1,
		"default-192.168.100.105-2055": 1,
		"default-192.168.100.106-2055": 1,
		"default-192.168.100.107-2055": 1,
		"default-192.168.100.108-2055": 1,
		"default-192.168.100.109-2055": 1,
		"default-192.168.100.110-2055": 1,
		"default-192.168.100.111-2055": 1,
		"default-192.168.100.112-2055": 1,
		"default-192.168.100.113-2055": 1,
		"default-192.168.100.114-2055": 1,
		"default-192.168.100.115-2055": 1,
		"default-192.168.100.116-2055": 1,
	}
	// Make sure creates do not exceed the max collector per flow limit
	_, err := ValidateFlowExportPolicy(infraAPI, netflow, types.Create, collectorToKeys)
	if err == nil {
		t.Fatalf("Must return an error. %v", err)
	}
	netflow.Spec.Exports = []netproto.ExportConfig{
		{
			Destination: "192.168.100.117",
			Transport: &netproto.ProtoPort{
				Protocol: "udp",
				Port:     "2055",
			},
		},
	}
	_, err = ValidateFlowExportPolicy(infraAPI, netflow, types.Create, collectorToKeys)
	if err == nil {
		t.Fatalf("Must return an error. %v", err)
	}
	delete(collectorToKeys, "default-192.168.100.116-2055")

	// Make sure create is allowed
	_, err = ValidateFlowExportPolicy(infraAPI, netflow, types.Create, collectorToKeys)
	if err != nil {
		t.Fatal(err)
	}

	// Make sure update doesn't exceed the max collector limit
	netflow.Spec.Exports = []netproto.ExportConfig{
		{
			Destination: "192.168.100.115",
			Transport: &netproto.ProtoPort{
				Protocol: "udp",
				Port:     "2055",
			},
		},
	}
	dat, _ = netflow.Marshal()

	if err := infraAPI.Store(netflow.Kind, netflow.GetKey(), dat); err != nil {
		t.Fatal(err)
	}

	collectorToKeys["default-192.168.100.116-2055"] = 1
	netflow.Spec.Exports = []netproto.ExportConfig{
		{
			Destination: "192.168.100.115",
			Transport: &netproto.ProtoPort{
				Protocol: "udp",
				Port:     "2055",
			},
		},
		{
			Destination: "192.168.100.117",
			Transport: &netproto.ProtoPort{
				Protocol: "udp",
				Port:     "2055",
			},
		},
	}
	_, err = ValidateFlowExportPolicy(infraAPI, netflow, types.Update, collectorToKeys)
	if err == nil {
		t.Fatalf("Must return an error. %v", err)
	}
	netflow.Spec.Exports = []netproto.ExportConfig{
		{
			Destination: "192.168.100.117",
			Transport: &netproto.ProtoPort{
				Protocol: "udp",
				Port:     "2055",
			},
		},
	}
	_, err = ValidateFlowExportPolicy(infraAPI, netflow, types.Update, collectorToKeys)
	if err != nil {
		t.Fatal(err)
	}

	// Make sure a collector that is referenced by multiple flows
	// is not removed in update
	collectorToKeys["default-192.168.100.115-2055"] = 2
	_, err = ValidateFlowExportPolicy(infraAPI, netflow, types.Update, collectorToKeys)
	if err == nil {
		t.Fatalf("Must return an error. %v", err)
	}
	err = infraAPI.Delete(vrf.Kind, vrf.GetKey())
	if err != nil {
		t.Fatal(err)
	}
}

func TestValidateMirrorSession(t *testing.T) {
	mirror := netproto.MirrorSession{
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
					ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.109"},
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
	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		t.Fatal(err)
	}
	mirrorDestToKeys := map[string]int{
		"default-192.168.100.101": 1,
		"default-192.168.100.102": 1,
		"default-192.168.100.103": 1,
		"default-192.168.100.104": 1,
		"default-192.168.100.105": 1,
		"default-192.168.100.106": 1,
		"default-192.168.100.107": 1,
		"default-192.168.100.108": 1,
	}
	// Make sure creates do not exceed the max mirror session limit
	_, err := ValidateMirrorSession(infraAPI, mirror, types.Create, mirrorDestToKeys)
	if err == nil {
		t.Fatalf("Must return an error. %v", err)
	}
	delete(mirrorDestToKeys, "default-192.168.100.108")

	// Make sure create is allowed
	_, err = ValidateMirrorSession(infraAPI, mirror, types.Create, mirrorDestToKeys)
	if err != nil {
		t.Fatal(err)
	}

	// Make sure update doesn't exceed the max mirror session limit
	mirror.Spec.Collectors = []netproto.MirrorCollector{
		{
			ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.107"},
		},
	}
	dat, _ = mirror.Marshal()

	if err := infraAPI.Store(mirror.Kind, mirror.GetKey(), dat); err != nil {
		t.Fatal(err)
	}

	mirrorDestToKeys["default-192.168.100.108"] = 1
	mirror.Spec.Collectors = []netproto.MirrorCollector{
		{
			ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.107"},
		},
		{
			ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.109"},
		},
	}
	_, err = ValidateMirrorSession(infraAPI, mirror, types.Update, mirrorDestToKeys)
	if err == nil {
		t.Fatalf("Must return an error. %v", err)
	}
	mirror.Spec.Collectors = []netproto.MirrorCollector{
		{
			ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.109"},
		},
	}
	_, err = ValidateMirrorSession(infraAPI, mirror, types.Update, mirrorDestToKeys)
	if err != nil {
		t.Fatal(err)
	}

	// Make sure a collector that is referenced by multiple mirror sessions or interface
	// erspan is not removed in update
	mirrorDestToKeys["default-192.168.100.107"] = 2
	_, err = ValidateMirrorSession(infraAPI, mirror, types.Update, mirrorDestToKeys)
	if err == nil {
		t.Fatalf("Must return an error. %v", err)
	}
	err = infraAPI.Delete(vrf.Kind, vrf.GetKey())
	if err != nil {
		t.Fatal(err)
	}
}
