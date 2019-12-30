package state

import (
	"encoding/json"
	"fmt"
	"net"
	"os"
	"reflect"
	"testing"

	"github.com/pensando/sw/venice/ctrler/tpm"

	"github.com/pensando/sw/venice/globals"

	"github.com/golang/mock/gomock"
	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	agstate "github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/netagent/state/dependencies"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/protos/tpmprotos"
	mockdatapath "github.com/pensando/sw/nic/agent/tpa/datapath"
	"github.com/pensando/sw/nic/agent/tpa/state/types"
	"github.com/pensando/sw/venice/utils/emstore"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

const (
	emDbPath                = "/tmp/naples-tpagent.db"
	maxFlowExportCollectors = tpm.MaxNumExportPolicy * tpm.MaxNumCollectorsPerPolicy
)

func cleanup(t *testing.T, ag *PolicyState) {

	l, err := ag.store.List(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		},
	})

	if err == nil {
		for _, i := range l {
			err := ag.store.Delete(i)
			tu.AssertOk(t, err, fmt.Sprintf("failed to delete export policy %+v", i))
		}
	}

	ag.Close()
	os.Remove(emDbPath)
}

func createDataStore() (emstore.Emstore, error) {
	mstore, err := emstore.NewEmstore(emstore.BoltDBType, emDbPath)
	if err != nil {
		return nil, err
	}
	return mstore, nil
}

func mockGetMgmtIPAddr() string {
	conn, err := net.Dial("udp", "8.8.8.8:80")
	if err != nil {
		return "192.168.1.199"
	}
	defer conn.Close()

	localAddr := conn.LocalAddr().(*net.UDPAddr)
	return localAddr.IP.String()
}

func TestValidateMeta(t *testing.T) {
	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NamespaceDB: map[string]*netproto.Namespace{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	ag, err := NewTpAgent(na, mockGetMgmtIPAddr, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, ag)
	err = ag.CreateFlowExportPolicy(context.Background(), &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{},
	})
	tu.Assert(t, err != nil, fmt.Sprintf("create succeeded with invalid meta"))

	err = ag.CreateFlowExportPolicy(context.Background(), &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{},
		ObjectMeta: api.ObjectMeta{},
	})
	tu.Assert(t, err != nil, fmt.Sprintf("create succeeded with invalid meta"))
}

func TestStoreOps(t *testing.T) {
	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NamespaceDB: map[string]*netproto.Namespace{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	ag, err := NewTpAgent(na, mockGetMgmtIPAddr, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, ag)
	for i := 1; i < 4; i++ {
		pol := &types.FlowExportPolicyTable{
			FlowExportPolicy: &tpmprotos.FlowExportPolicy{
				TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
				ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("test-%d", i), Tenant: "default", Namespace: "default"},
			},
		}

		err = ag.store.Write(pol)
		tu.AssertOk(t, err, fmt.Sprintf("failed to write to store , %s", err))
	}

	for i := 1; i < 4; i++ {
		pol := &types.FlowExportPolicyTable{
			FlowExportPolicy: &tpmprotos.FlowExportPolicy{
				TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
				ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("test-%d", i), Tenant: "default", Namespace: "default"},
			},
			CollectorKeys: map[string]bool{"test": true},
			FlowRuleKeys:  map[string]bool{"test": true},
			Vrf:           uint64(i),
		}

		_, err = ag.store.Read(pol)
		tu.AssertOk(t, err, fmt.Sprintf("failed to write to store , %s", err))
	}
}

func TestFindNumExports(t *testing.T) {
	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NetworkDB:   map[string]*netproto.Network{},
		EndpointDB:  map[string]*netproto.Endpoint{},
		NamespaceDB: map[string]*netproto.Namespace{},
		VrfDB:       map[string]*netproto.Vrf{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}

	na.LateralDB = make(map[string][]string)
	key := na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "tenant"})
	na.TenantDB[key] = &netproto.Tenant{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.TenantSpec{Meta: &api.ObjectMeta{Tenant: "default"}},
		Status: netproto.TenantStatus{
			TenantID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Name: "default"}, api.TypeMeta{Kind: "namespace"})
	na.NamespaceDB[key] = &netproto.Namespace{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.NamespaceSpec{},
		Status: netproto.NamespaceStatus{
			NamespaceID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "vrf"})
	na.VrfDB[key] = &netproto.Vrf{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: uint64(101),
		},
	}

	for i := 1; i < maxFlowExportCollectors; i++ {
		name := fmt.Sprintf("ep3-%d", i)
		addr := fmt.Sprintf("192.168.3.%d/32", i)
		key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: name}, api.TypeMeta{Kind: "endpoint"})
		na.EndpointDB[key] = &netproto.Endpoint{
			ObjectMeta: api.ObjectMeta{Name: "ep1", Namespace: "default", Tenant: "default"},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{addr},
			},
			Status: netproto.EndpointStatus{},
		}

		name = fmt.Sprintf("ep4-%d", i)
		addr = fmt.Sprintf("192.168.4.%d/32", i)
		key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: name}, api.TypeMeta{Kind: "endpoint"})
		na.EndpointDB[key] = &netproto.Endpoint{
			ObjectMeta: api.ObjectMeta{Name: "ep1", Namespace: "default", Tenant: "default"},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{addr},
			},
			Status: netproto.EndpointStatus{},
		}
	}

	s, err := NewTpAgent(na, mockGetMgmtIPAddr, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))
	defer cleanup(t, s)

	for i := 1; i <= 9; i++ {
		spec := tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("test-%d", i), Tenant: "default", Namespace: "default"},
			Spec: tpmprotos.FlowExportPolicySpec{
				MatchRules: []netproto.MatchRule{
					{
						Src: &netproto.MatchSelector{
							Addresses: []string{"any"},
						},
						Dst: &netproto.MatchSelector{
							Addresses: []string{"any"},
							ProtoPorts: []*netproto.ProtoPort{
								{
									Protocol: "any",
									Port:     "any",
								},
							},
						},
					},
				},
				Interval:         "15s",
				TemplateInterval: "5m",
				Format:           "IPFIX",
				Exports: []monitoring.ExportConfig{
					{
						Destination: fmt.Sprintf("192.168.3.%d", i+1),
						Transport:   "udp/5555",
					},
					{
						Destination: fmt.Sprintf("192.168.4.%d", i+1),
						Transport:   "udp/6666",
					},
				},
			},
		}

		err := s.CreateFlowExportPolicy(context.Background(), &spec)

		if i <= 8 {
			tu.AssertOk(t, err, fmt.Sprintf("failed to find num exports, %s", err))
		} else {
			// > 9
			tu.Assert(t, err != nil, fmt.Sprintf("didn't fail for max collectors"))
		}

		pctx := &policyDb{
			state: s,
		}
		pctx.readCollectorTable()
		num := pctx.findNumCollectors()

		if i <= 8 {
			tu.Assert(t, num == 2*i, fmt.Sprintf("invalid num exports, require %d got %d", 2*i, num))
		} else {
			tu.Assert(t, num == 2*(i-1), fmt.Sprintf("invalid num exports, require %d got %d", 2*(i-1), num))
		}
	}
}

func TestValidatePolicy(t *testing.T) {
	destAddr := "192.168.3.1"
	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NetworkDB:   map[string]*netproto.Network{},
		EndpointDB:  map[string]*netproto.Endpoint{},
		NamespaceDB: map[string]*netproto.Namespace{},
		VrfDB:       map[string]*netproto.Vrf{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	key := na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "tenant"})
	na.TenantDB[key] = &netproto.Tenant{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.TenantSpec{Meta: &api.ObjectMeta{Tenant: "default"}},
		Status: netproto.TenantStatus{
			TenantID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Name: "default"}, api.TypeMeta{Kind: "namespace"})
	na.NamespaceDB[key] = &netproto.Namespace{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.NamespaceSpec{},
		Status: netproto.NamespaceStatus{
			NamespaceID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "vrf"})
	na.VrfDB[key] = &netproto.Vrf{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: uint64(101),
		},
	}

	ag, err := NewTpAgent(na, mockGetMgmtIPAddr, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))
	defer cleanup(t, ag)

	pol := &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
	}

	_, err = ag.validatePolicy(pol)
	tu.Assert(t, err != nil, fmt.Sprintf("nil target didnt fail"))

	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1010",
							},
						},
					},
				},
			},
			Interval:         "15s",
			TemplateInterval: "6m",
			Format:           "IPFIX",
			Exports:          []monitoring.ExportConfig{},
		}}

	_, err = ag.validatePolicy(pol)
	tu.Assert(t, err != nil, fmt.Sprintf("nil export didnt fail"))

	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{
			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			Exports: []monitoring.ExportConfig{
				{Transport: "UDP/1234"},
			},
		}}

	_, err = ag.validatePolicy(pol)
	tu.Assert(t, err != nil, fmt.Sprintf("nil destination didnt fail"))

	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{

			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destAddr,
				},
			},
		}}

	_, err = ag.validatePolicy(pol)
	tu.Assert(t, err != nil, fmt.Sprintf("nil transport didnt fail"))

	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default", Namespace: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1010",
							},
						},
					},
				},
			},
			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destAddr,
					Transport:   "UDP/1234",
				},
			},
		}}

	c, err := ag.validatePolicy(pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to find num exports"))
	tu.Assert(t, len(c) == 1, fmt.Sprintf("expected num exports: 1, got %+v", c))

	// check interval
	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1010",
							},
						},
					},
				},
			},
			Format:           "IPFIX",
			TemplateInterval: "5m",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destAddr,
					Transport:   "UDP/1234",
				},
			},
		}}

	testInterval := []struct {
		interval string
		pass     bool
	}{

		{interval: "25h", pass: true},
		{interval: "1s", pass: true},
	}

	for _, i := range testInterval {
		pol.Spec.Interval = i.interval
		_, err = ag.validatePolicy(pol)
		if i.pass {
			tu.AssertOk(t, err, fmt.Sprintf("failed to validate interval: %s", pol.Spec.Interval))
		} else {
			tu.Assert(t, err != nil, fmt.Sprintf("didnt fail interval: %s", pol.Spec.Interval))
		}
	}
}

func TestNewTpAgent(t *testing.T) {

	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NamespaceDB: map[string]*netproto.Namespace{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	ag, err := NewTpAgent(na, mockGetMgmtIPAddr, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, ag)

	pol := &types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		},
	}

	err = ag.store.Write(pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy"))

	obj, err := ag.store.Read(pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to read export policy"))
	rp, ok := obj.(*types.FlowExportPolicyTable)
	tu.Assert(t, ok == true, fmt.Sprintf("invalid export policy %+v", obj))
	tu.Assert(t, rp == pol, fmt.Sprintf("export policy  didn't match, %+v, %+v", rp, pol))

	pol2 := &types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "test2", Tenant: "default"},
		},
	}

	err = ag.store.Write(pol2)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy"))
	l, err := ag.store.List(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{},
		},
	})
	tu.AssertOk(t, err, fmt.Sprintf("failed to list export policy"))
	tu.Assert(t, len(l) == 2, fmt.Sprintf("invalid number of export policy, expectec 2 got %d, %+v", len(l), l))

	fd, err := os.Create(tpafile)
	tu.AssertOk(t, err, "failed to create %v", tpafile)
	fd.Close()
	defer os.Remove(tpafile)

	// check reboot cleanup
	ng, err := NewTpAgent(na, mockGetMgmtIPAddr, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))
	defer cleanup(t, ng)

}

func TestCreateFlowExportPolicy(t *testing.T) {
	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NetworkDB:   map[string]*netproto.Network{},
		EndpointDB:  map[string]*netproto.Endpoint{},
		NamespaceDB: map[string]*netproto.Namespace{},
		VrfDB:       map[string]*netproto.Vrf{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	key := na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "tenant"})
	na.TenantDB[key] = &netproto.Tenant{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.TenantSpec{Meta: &api.ObjectMeta{Tenant: "default"}},
		Status: netproto.TenantStatus{
			TenantID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Name: "default"}, api.TypeMeta{Kind: "namespace"})
	na.NamespaceDB[key] = &netproto.Namespace{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.NamespaceSpec{},
		Status: netproto.NamespaceStatus{
			NamespaceID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			VlanID: 42},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "vrf"})
	na.VrfDB[key] = &netproto.Vrf{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: uint64(101),
		},
	}

	for i := 0; i < maxFlowExportCollectors; i++ {
		name := fmt.Sprintf("ep%d", i+1)
		addr := fmt.Sprintf("192.168.3.%d/32", 10+i)
		key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: name}, api.TypeMeta{Kind: "endpoint"})
		na.EndpointDB[key] = &netproto.Endpoint{
			ObjectMeta: api.ObjectMeta{Name: "ep1", Namespace: "default", Tenant: "default"},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{addr},
			},
			Status: netproto.EndpointStatus{},
		}
	}

	s, err := NewTpAgent(na, mockGetMgmtIPAddr, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))
	defer cleanup(t, s)

	policyPrefix := "flowmon"
	for l := 0; l < tpm.MaxNumExportPolicy+1; l++ {
		pol := &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s-%d", policyPrefix, l), Tenant: "default", Namespace: "default"},

			Spec: tpmprotos.FlowExportPolicySpec{
				MatchRules: []netproto.MatchRule{
					{
						Src: &netproto.MatchSelector{
							Addresses: []string{"any"},
						},
						Dst: &netproto.MatchSelector{
							Addresses: []string{"any"},
							ProtoPorts: []*netproto.ProtoPort{
								{
									Protocol: "any",
									Port:     "any",
								},
							},
						},
					},
				},
				Interval:         "15s",
				TemplateInterval: "5m",
				Format:           "IPFIX",
				Exports: []monitoring.ExportConfig{
					{
						Destination: fmt.Sprintf("192.168.3.%d", 10+l),
						Transport:   "UDP/1234",
					},
				},
			},
		}

		err = s.CreateFlowExportPolicy(context.Background(), pol)
		if l < tpm.MaxNumExportPolicy {
			tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy %+v", pol))
		} else {
			tu.Assert(t, err != nil, fmt.Sprintf("created more than max (%d) policies", tpm.MaxNumExportPolicy))
		}
	}

	// list
	rp, err := s.ListFlowExportPolicy(context.Background())
	tu.AssertOk(t, err, fmt.Sprintf("failed to list export policy"))
	tu.Assert(t, len(rp) == tpm.MaxNumExportPolicy, fmt.Sprintf("expected %d export policy, got %d {%+v}", tpm.MaxNumExportPolicy, len(rp), rp))

	for l := 0; l < tpm.MaxNumExportPolicy; l++ {
		pol := &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s-%d", policyPrefix, l), Tenant: "default", Namespace: "default"},

			Spec: tpmprotos.FlowExportPolicySpec{
				MatchRules: []netproto.MatchRule{
					{
						Src: &netproto.MatchSelector{
							Addresses: []string{"any"},
						},
						Dst: &netproto.MatchSelector{
							Addresses: []string{"any"},
							ProtoPorts: []*netproto.ProtoPort{
								{
									Protocol: "any",
									Port:     "any",
								},
							},
						},
					},
				},
				Interval:         "15s",
				TemplateInterval: "5m",
				Format:           "IPFIX",
				Exports: []monitoring.ExportConfig{
					{
						Destination: fmt.Sprintf("192.168.3.%d", 10+l),
						Transport:   "UDP/1234",
					},
				},
			},
		}

		rp, err := s.GetFlowExportPolicy(context.Background(), &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s-%d", policyPrefix, l), Tenant: "default", Namespace: "default"},
		})

		tu.AssertOk(t, err, fmt.Sprintf("failed to read export policy %s-%d", policyPrefix, l))
		tu.Assert(t, rp.Name == pol.Name && rp.Namespace == pol.Namespace && rp.Tenant == pol.Tenant, "policy in db didn;t match, got %+v, expected:%+v", pol, rp)
		tu.Assert(t, reflect.DeepEqual(rp.Spec, pol.Spec), "policy in db didn;t match, got %+v, expected:%+v", pol, rp)
	}

	for l := 0; l < tpm.MaxNumExportPolicy; l++ {
		err = s.DeleteFlowExportPolicy(context.Background(), &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s-%d", policyPrefix, l), Tenant: "default", Namespace: "default"},
		})

		tu.AssertOk(t, err, fmt.Sprintf("failed to delete export policy"))
	}

	// verify all policie are removed
	pl, err := s.store.List(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "FlowExportPolicy",
			},
		},
	})
	tu.AssertOk(t, err, fmt.Sprintf("failed to list  export policy"))
	tu.Assert(t, len(pl) == 0, fmt.Sprintf("policies exists after delete, %+v", pl))
}

func TestCreateFlowExportPolicyWithMock(t *testing.T) {
	c := gomock.NewController(t)
	defer c.Finish()
	halMock := halproto.NewMockTelemetryClient(c)

	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NetworkDB:   map[string]*netproto.Network{},
		EndpointDB:  map[string]*netproto.Endpoint{},
		NamespaceDB: map[string]*netproto.Namespace{},
		VrfDB:       map[string]*netproto.Vrf{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)
	key := na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "tenant"})
	na.TenantDB[key] = &netproto.Tenant{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.TenantSpec{Meta: &api.ObjectMeta{Tenant: "default"}},
		Status: netproto.TenantStatus{
			TenantID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Name: "default"}, api.TypeMeta{Kind: "namespace"})
	na.NamespaceDB[key] = &netproto.Namespace{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.NamespaceSpec{},
		Status: netproto.NamespaceStatus{
			NamespaceID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			VlanID: 42},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "vrf"})
	na.VrfDB[key] = &netproto.Vrf{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: uint64(101),
		},
	}

	for i := 0; i < maxFlowExportCollectors; i++ {
		name := fmt.Sprintf("ep%d", i+1)
		addr := fmt.Sprintf("192.168.3.%d/32", 10+i)
		key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: name}, api.TypeMeta{Kind: "endpoint"})
		na.EndpointDB[key] = &netproto.Endpoint{
			ObjectMeta: api.ObjectMeta{Name: "ep1", Namespace: "default", Tenant: "default"},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{addr},
			},
			Status: netproto.EndpointStatus{},
		}
	}

	s, err := NewTpAgent(na, mockGetMgmtIPAddr, halMock)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, s)

	collResp := &halproto.CollectorResponseMsg{
		Response: []*halproto.CollectorResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Status: &halproto.CollectorStatus{
					Handle: uint64(105),
				},
			},
		},
	}

	flowResp := &halproto.FlowMonitorRuleResponseMsg{
		Response: []*halproto.FlowMonitorRuleResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Status: &halproto.FlowMonitorRuleStatus{
					Handle: uint64(105),
				},
			},
		},
	}

	halMock.EXPECT().CollectorCreate(gomock.Any(), gomock.Any()).Return(collResp, nil).Times(1)
	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).Times(1)

	policyPrefix := "mockflow"
	pol := &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: policyPrefix, Tenant: "default", Namespace: "default"},

		Spec: tpmprotos.FlowExportPolicySpec{
			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "any",
								Port:     "any",
							},
						},
					},
				},
			},
			Exports: []monitoring.ExportConfig{
				{
					Destination: fmt.Sprintf("192.168.3.11"),
					Transport:   "UDP/1234",
				},
			},
		},
	}

	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy %+v", pol))

	collDelResp := &halproto.CollectorDeleteResponseMsg{
		Response: []*halproto.CollectorDeleteResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}

	flowDelResp := &halproto.FlowMonitorRuleDeleteResponseMsg{
		Response: []*halproto.FlowMonitorRuleDeleteResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}

	halMock.EXPECT().CollectorDelete(gomock.Any(), gomock.Any()).Return(collDelResp, nil).Times(1)
	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(flowDelResp, nil).Times(1)

	err = s.DeleteFlowExportPolicy(context.Background(), pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to delete export policy %+v", pol))
}

func TestNetagentInfo(t *testing.T) {
	c := gomock.NewController(t)
	defer c.Finish()
	halMock := halproto.NewMockTelemetryClient(c)

	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NetworkDB:   map[string]*netproto.Network{},
		EndpointDB:  map[string]*netproto.Endpoint{},
		NamespaceDB: map[string]*netproto.Namespace{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	s, err := NewTpAgent(na, mockGetMgmtIPAddr, halMock)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, s)

	pol := &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "flowexport", Tenant: "default", Namespace: "default"},

		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1010",
							},
						},
					},
				},
			},
			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: "10.10.10.1",
					Transport:   "UDP/1234",
				},
			},
		},
	}

	key := na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "tenant"})
	na.TenantDB[key] = &netproto.Tenant{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.TenantSpec{Meta: &api.ObjectMeta{Tenant: "default"}},
		Status: netproto.TenantStatus{
			TenantID: uint64(501),
		},
	}

	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid namespace didn't fail"))

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Name: "default"}, api.TypeMeta{Kind: "namespace"})
	na.NamespaceDB[key] = &netproto.Namespace{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.NamespaceSpec{},
		Status: netproto.NamespaceStatus{
			NamespaceID: uint64(501),
		},
	}

	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid endpoint didn't fail"))

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "flowexport"}, api.TypeMeta{Kind: "endpoint"})
	na.EndpointDB[key] = &netproto.Endpoint{
		ObjectMeta: api.ObjectMeta{Name: "flowexport", Namespace: "default", Tenant: "default"},
		Spec: netproto.EndpointSpec{
			NetworkName:   "default",
			IPv4Addresses: []string{"10.10.10.1/32"},
		},
		Status: netproto.EndpointStatus{},
	}

	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid network didn't fail"))

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			VlanID: 42},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	pol.Spec.Exports[0].Transport = "IPIP/100"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid protocol didn't fail"))

	pol.Spec.Exports[0].Transport = "ICMP/1000"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid protocol didn't fail"))

	pol.Spec.Exports[0].Transport = "TCP/UDP"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid port didn't fail"))

	pol.Spec.Exports[0].Transport = "TCP/65536"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid port didn't fail"))

	pol.Spec.Exports[0].Transport = "UDP/1234"
	pol.Spec.Format = "NETFLOW"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid format didn't fail"))

}

func TestHalIPAddr(t *testing.T) {
	_, _, err := convertToHalIPAddr("nonExistingHost")
	tu.Assert(t, err != nil, fmt.Sprintf("invalid ip didn't fail"))

	_, _, err = convertToHalIPAddr("google.com")
	tu.AssertOk(t, err, fmt.Sprintf("failed to look up hostname"))

	_, _, err = convertToHalIPAddr("2001:0db8:85a3:0000:0000:8a2e:0370:7334")
	tu.AssertOk(t, err, fmt.Sprintf("failed to parse ipv6"))

	_, s, err := convertToHalIPAddr("2.2.2.2/24")
	tu.AssertOk(t, err, fmt.Sprintf("failed to parse CIDR"))
	tu.Assert(t, s == "2.2.2.2", "invalid addr", s)

	_, s, err = convertToHalIPAddr("2.2.2.2")
	tu.AssertOk(t, err, fmt.Sprintf("failed to parse ipv4 addr"))
	tu.Assert(t, s == "2.2.2.2", "invalid addr", s)

}

func TestTpaDebug(t *testing.T) {
	c := gomock.NewController(t)
	defer c.Finish()
	halMock := halproto.NewMockTelemetryClient(c)

	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))
	defer ds.Close()

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NetworkDB:   map[string]*netproto.Network{},
		EndpointDB:  map[string]*netproto.Endpoint{},
		NamespaceDB: map[string]*netproto.Namespace{},
		VrfDB:       map[string]*netproto.Vrf{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	s, err := NewTpAgent(na, mockGetMgmtIPAddr, halMock)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, s)

	pol1 := &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "flowexport1", Tenant: "default", Namespace: "default"},

		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
				{
					Dst: &netproto.MatchSelector{
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: "10.10.10.1",
					Transport:   "UDP/1234",
				},
				{
					Destination: "10.10.10.2",
					Transport:   "UDP/1234",
				},
			},
		},
	}
	key := na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "tenant"})
	na.TenantDB[key] = &netproto.Tenant{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.TenantSpec{Meta: &api.ObjectMeta{Tenant: "default"}},
		Status: netproto.TenantStatus{
			TenantID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Name: "default"}, api.TypeMeta{Kind: "namespace"})
	na.NamespaceDB[key] = &netproto.Namespace{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.NamespaceSpec{},
		Status: netproto.NamespaceStatus{
			NamespaceID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "flowexport"}, api.TypeMeta{Kind: "endpoint"})
	na.EndpointDB[key] = &netproto.Endpoint{
		ObjectMeta: api.ObjectMeta{Name: "flowexport", Namespace: "default", Tenant: "default"},
		Spec: netproto.EndpointSpec{
			NetworkName:   "default",
			IPv4Addresses: []string{"10.10.10.1/32"},
		},
		Status: netproto.EndpointStatus{},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "flowexport1"}, api.TypeMeta{Kind: "endpoint"})
	na.EndpointDB[key] = &netproto.Endpoint{
		ObjectMeta: api.ObjectMeta{Name: "flowexport", Namespace: "default", Tenant: "default"},
		Spec: netproto.EndpointSpec{
			NetworkName:   "default",
			IPv4Addresses: []string{"10.10.10.2/32"},
		},
		Status: netproto.EndpointStatus{},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			VlanID: 42},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "vrf"})
	na.VrfDB[key] = &netproto.Vrf{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: uint64(101),
		},
	}

	collResp := &halproto.CollectorResponseMsg{
		Response: []*halproto.CollectorResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Status: &halproto.CollectorStatus{
					Handle: uint64(105),
				},
			},
		},
	}

	flowResp := &halproto.FlowMonitorRuleResponseMsg{
		Response: []*halproto.FlowMonitorRuleResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Status: &halproto.FlowMonitorRuleStatus{
					Handle: uint64(105),
				},
			},
		},
	}

	halMock.EXPECT().CollectorCreate(gomock.Any(), gomock.Any()).Return(collResp, nil).AnyTimes()
	halMock.EXPECT().CollectorDelete(gomock.Any(), gomock.Any()).Return(nil, nil).AnyTimes()

	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).AnyTimes()
	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(nil, nil).AnyTimes()

	err = s.CreateFlowExportPolicy(context.Background(), pol1)
	tu.AssertOk(t, err, "failed to create policy")

	data, err := s.Debug(nil)
	tu.AssertOk(t, err, "failed to read debug")

	dbgInfo, ok := data.(*debugInfo)
	tu.Assert(t, ok == true, "invalid debug info")

	pdebug := dbgInfo.Policy[fmt.Sprintf("%s/%s/%s", pol1.Tenant, pol1.Namespace, pol1.Name)]
	tu.Assert(t, pdebug.FlowExportPolicy != nil, "failed to find policy")
	tu.Assert(t, len(pdebug.Collectors) == 2, "mismatch in collectors", pdebug.Collectors)
	tu.Assert(t, len(dbgInfo.CollectorTable) == 2, "invalid collectors", dbgInfo.CollectorTable)
	tu.Assert(t, len(dbgInfo.FlowRuleTable) == 2, "invalid num of flows", dbgInfo.FlowRuleTable)
}

func TestPolicyOps(t *testing.T) {
	c := gomock.NewController(t)
	defer c.Finish()
	halMock := halproto.NewMockTelemetryClient(c)

	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NetworkDB:   map[string]*netproto.Network{},
		EndpointDB:  map[string]*netproto.Endpoint{},
		NamespaceDB: map[string]*netproto.Namespace{},
		VrfDB:       map[string]*netproto.Vrf{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	key := na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "tenant"})
	na.TenantDB[key] = &netproto.Tenant{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.TenantSpec{Meta: &api.ObjectMeta{Tenant: "default"}},
		Status: netproto.TenantStatus{
			TenantID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Name: "default"}, api.TypeMeta{Kind: "namespace"})
	na.NamespaceDB[key] = &netproto.Namespace{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.NamespaceSpec{},
		Status: netproto.NamespaceStatus{
			NamespaceID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			VlanID: 42},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "vrf"})
	na.VrfDB[key] = &netproto.Vrf{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: uint64(101),
		},
	}

	for i := 0; i < maxFlowExportCollectors; i++ {
		name := fmt.Sprintf("ep%d", i+1)
		addr := fmt.Sprintf("192.168.3.%d/32", 10+i)
		key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: name}, api.TypeMeta{Kind: "endpoint"})
		na.EndpointDB[key] = &netproto.Endpoint{
			ObjectMeta: api.ObjectMeta{Name: "ep1", Namespace: "default", Tenant: "default"},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{addr},
			},
			Status: netproto.EndpointStatus{},
		}
	}

	s, err := NewTpAgent(na, mockGetMgmtIPAddr, halMock)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, s)

	collResp := &halproto.CollectorResponseMsg{
		Response: []*halproto.CollectorResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Status: &halproto.CollectorStatus{
					Handle: uint64(105),
				},
			},
		},
	}

	flowResp := &halproto.FlowMonitorRuleResponseMsg{
		Response: []*halproto.FlowMonitorRuleResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Status: &halproto.FlowMonitorRuleStatus{
					Handle: uint64(105),
				},
			},
		},
	}

	pol := &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "matchrule-1", Tenant: "default", Namespace: "default"},

		Spec: tpmprotos.FlowExportPolicySpec{
			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.1", "192.168.100.2", "192.168.100.3"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.200.1", "192.168.200.2", "192.168.200.3"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "5001",
							},
						},
					},
				},
			},
			Exports: []monitoring.ExportConfig{
				{
					Destination: fmt.Sprintf("192.168.3.11"),
					Transport:   "UDP/1234",
				},
				{
					Destination: fmt.Sprintf("192.168.3.12"),
					Transport:   "UDP/1234",
				},
			},
		},
	}

	halMock.EXPECT().CollectorCreate(gomock.Any(), gomock.Any()).Return(collResp, nil).Times(4)
	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).Times(9)

	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy %+v", pol))

	// create again
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, "policy exists, create didn't fail")

	collDelResp := &halproto.CollectorDeleteResponseMsg{
		Response: []*halproto.CollectorDeleteResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}

	flowDelResp := &halproto.FlowMonitorRuleDeleteResponseMsg{
		Response: []*halproto.FlowMonitorRuleDeleteResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}

	halMock.EXPECT().CollectorDelete(gomock.Any(), gomock.Any()).Return(collDelResp, nil).Times(3)
	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(flowDelResp, nil).Times(9)

	err = s.DeleteFlowExportPolicy(context.Background(), pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to delete export policy %+v", pol))

	// delete again
	err = s.DeleteFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, "deleted non-existing policy")

	// rule overlap
	halMock.EXPECT().CollectorCreate(gomock.Any(), gomock.Any()).Return(collResp, nil).Times(1)
	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).Times(9)

	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy %+v", pol))

	dup := &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "matchrule-overlap", Tenant: "default", Namespace: "default"},

		Spec: tpmprotos.FlowExportPolicySpec{
			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.1", "192.168.100.2", "192.168.100.3"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.200.1", "192.168.200.2", "192.168.200.3"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "5001",
							},
						},
					},
				},
			},
			Exports: []monitoring.ExportConfig{

				{
					Destination: fmt.Sprintf("192.168.3.12"),
					Transport:   "UDP/1234",
				},
				{
					Destination: fmt.Sprintf("192.168.3.14"),
					Transport:   "UDP/1234",
				},
			},
		},
	}

	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(nil, nil).Times(9)
	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).Times(9)

	err = s.CreateFlowExportPolicy(context.Background(), dup)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy %+v", dup))

	// delete, results in update
	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(nil, nil).Times(9)
	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).Times(9)

	err = s.DeleteFlowExportPolicy(context.Background(), dup)
	tu.AssertOk(t, err, fmt.Sprintf("failed to delete export policy %+v", dup))

	overlap := &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "matchrule-overlap", Tenant: "default", Namespace: "default"},

		Spec: tpmprotos.FlowExportPolicySpec{
			Interval:         "15s",
			TemplateInterval: "5m",
			Format:           "IPFIX",
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.200.1"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "5001",
							},
						},
					},
				},
			},
			Exports: []monitoring.ExportConfig{
				{
					Destination: fmt.Sprintf("192.168.3.11"),
					Transport:   "UDP/1234",
				},
				{
					Destination: fmt.Sprintf("192.168.3.12"),
					Transport:   "UDP/1234",
				},
			},
		},
	}

	// create becomes flow rule update
	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(nil, nil).Times(1)
	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).Times(1)

	err = s.CreateFlowExportPolicy(context.Background(), overlap)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy %+v", overlap))

	// deletebecaomes flow update
	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(nil, nil).Times(1)
	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).Times(1)

	err = s.DeleteFlowExportPolicy(context.Background(), overlap)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy %+v", overlap))
}

func TestMatchRule(t *testing.T) {
	c := gomock.NewController(t)
	defer c.Finish()
	halMock := halproto.NewMockTelemetryClient(c)

	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NetworkDB:   map[string]*netproto.Network{},
		EndpointDB:  map[string]*netproto.Endpoint{},
		NamespaceDB: map[string]*netproto.Namespace{},
		VrfDB:       map[string]*netproto.Vrf{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}
	na.LateralDB = make(map[string][]string)

	key := na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "tenant"})
	na.TenantDB[key] = &netproto.Tenant{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.TenantSpec{Meta: &api.ObjectMeta{Tenant: "default"}},
		Status: netproto.TenantStatus{
			TenantID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Name: "default"}, api.TypeMeta{Kind: "namespace"})
	na.NamespaceDB[key] = &netproto.Namespace{
		ObjectMeta: api.ObjectMeta{Tenant: "default"},
		Spec:       netproto.NamespaceSpec{},
		Status: netproto.NamespaceStatus{
			NamespaceID: uint64(501),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			VlanID: 42},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "vrf"})
	na.VrfDB[key] = &netproto.Vrf{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: uint64(101),
		},
	}

	for i := 0; i < maxFlowExportCollectors; i++ {
		name := fmt.Sprintf("ep%d", i+1)
		addr := fmt.Sprintf("192.168.100.%d/32", 10+i)
		key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: name}, api.TypeMeta{Kind: "endpoint"})
		na.EndpointDB[key] = &netproto.Endpoint{
			ObjectMeta: api.ObjectMeta{Name: "ep1", Namespace: "default", Tenant: "default"},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{addr},
			},
			Status: netproto.EndpointStatus{},
		}
	}

	s, err := NewTpAgent(na, mockGetMgmtIPAddr, halMock)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, s)

	collResp := &halproto.CollectorResponseMsg{
		Response: []*halproto.CollectorResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Status: &halproto.CollectorStatus{
					Handle: uint64(105),
				},
			},
		},
	}

	flowResp := &halproto.FlowMonitorRuleResponseMsg{
		Response: []*halproto.FlowMonitorRuleResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Status: &halproto.FlowMonitorRuleStatus{
					Handle: uint64(105),
				},
			},
		},
	}

	// example test from iota
	policyJSON := `{
  "kind": "FlowExportPolicy",
  "meta": {
    "name": "flow-export-ipfix-0",
    "namespace": "default",
    "tenant": "default"
  },
  "spec": {
    "exports": [
      {
        "destination": "192.168.100.12",
        "transport": "UDP/2055"
      }
    ],
    "format": "IPFIX",
    "interval": "1s",
    "template-interval": "5m",
    "match-rules": [
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "120"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.101"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.101"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "120"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "550"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.101"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.101"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "550"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "65535"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.101"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.101"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "65535"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "0"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.101"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.101"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "0"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "120"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.102"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.102"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "120"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "550"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.102"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.102"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "550"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "65535"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.102"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.102"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "65535"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "0"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.102"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.102"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "0"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "120"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.103"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.103"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "120"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "550"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.103"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.103"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "550"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "65535"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.103"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.103"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "65535"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "0"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.103"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.103"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "0"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "120"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.104"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.104"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "120"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "550"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.104"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.104"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "550"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "65535"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.104"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.104"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "65535"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "0.0.0.0/0"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "0"
            }
          ]
        },
        "source": {
          "addresses": [
            "192.168.100.104"
          ]
        }
      },
      {
        "destination": {
          "addresses": [
            "192.168.100.104"
          ],
          "proto-ports": [
            {
              "protocol": "tcp",
              "port": "0"
            }
          ]
        },
        "source": {
          "addresses": [
            "0.0.0.0/0"
          ]
        }
      }
    ]
  },
  "status": {}
}
`

	/*
		    "192.168.100.101:0.0.0.0/0:0:0:0:6:120:120:0:501",
		    "0.0.0.0/0:192.168.100.103:0:0:0:6:0:0:0:501",
		    "192.168.100.103:0.0.0.0/0:0:0:0:6:0:0:0:501",
		    "192.168.100.101:0.0.0.0/0:0:0:0:6:0:0:0:501",
		    "0.0.0.0/0:192.168.100.102:0:0:0:6:65535:65535:0:501",
		    "0.0.0.0/0:192.168.100.103:0:0:0:6:65535:65535:0:501",
			"0.0.0.0/0:192.168.100.101:0:0:0:6:550:550:0:501",
			"192.168.100.102:0.0.0.0/0:0:0:0:6:65535:65535:0:501",
			"192.168.100.104:0.0.0.0/0:0:0:0:6:120:120:0:501",
			"192.168.100.102:0.0.0.0/0:0:0:0:6:0:0:0:501",
			"192.168.100.103:0.0.0.0/0:0:0:0:6:65535:65535:0:501",
			"0.0.0.0/0:192.168.100.104:0:0:0:6:120:120:0:501",
			"192.168.100.102:0.0.0.0/0:0:0:0:6:120:120:0:501",
			"0.0.0.0/0:192.168.100.101:0:0:0:6:120:120:0:501",
			"192.168.100.104:0.0.0.0/0:0:0:0:6:550:550:0:501",
			"0.0.0.0/0:192.168.100.104:0:0:0:6:65535:65535:0:501",
			"192.168.100.104:0.0.0.0/0:0:0:0:6:0:0:0:501",
			"0.0.0.0/0:192.168.100.104:0:0:0:6:550:550:0:501",
			"192.168.100.102:0.0.0.0/0:0:0:0:6:550:550:0:501",
			"0.0.0.0/0:192.168.100.103:0:0:0:6:550:550:0:501",
			"0.0.0.0/0:192.168.100.101:0:0:0:6:65535:65535:0:501",
			"192.168.100.103:0.0.0.0/0:0:0:0:6:550:550:0:501",
			"0.0.0.0/0:192.168.100.102:0:0:0:6:0:0:0:501",
			"0.0.0.0/0:192.168.100.102:0:0:0:6:550:550:0:501",
			"192.168.100.101:0.0.0.0/0:0:0:0:6:550:550:0:501",
			"0.0.0.0/0:192.168.100.103:0:0:0:6:120:120:0:501",
			"0.0.0.0/0:192.168.100.101:0:0:0:6:0:0:0:501",
			"0.0.0.0/0:192.168.100.104:0:0:0:6:0:0:0:501",
			"192.168.100.103:0.0.0.0/0:0:0:0:6:120:120:0:501",
			"0.0.0.0/0:192.168.100.102:0:0:0:6:120:120:0:501",
			"192.168.100.101:0.0.0.0/0:0:0:0:6:65535:65535:0:501",
		    "192.168.100.104:0.0.0.0/0:0:0:0:6:65535:65535:0:501"

	*/
	pol := &tpmprotos.FlowExportPolicy{}

	err = json.Unmarshal([]byte(policyJSON), pol)
	tu.AssertOk(t, err, "failed to convert json to policy")

	halMock.EXPECT().CollectorCreate(gomock.Any(), gomock.Any()).Return(collResp, nil).AnyTimes()
	halMock.EXPECT().CollectorDelete(gomock.Any(), gomock.Any()).Return(nil, nil).AnyTimes()

	halMock.EXPECT().FlowMonitorRuleCreate(gomock.Any(), gomock.Any()).Return(flowResp, nil).AnyTimes()
	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(nil, nil).AnyTimes()

	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy"))

	data, err := s.Debug(nil)
	tu.AssertOk(t, err, "failed to read debug")

	dbgInfo, ok := data.(*debugInfo)
	tu.Assert(t, ok == true, fmt.Sprintf("invalid debug info, %T", data))

	pdebug := dbgInfo.Policy[fmt.Sprintf("%s/%s/%s", pol.Tenant, pol.Namespace, pol.Name)]
	tu.Assert(t, pdebug.FlowExportPolicy != nil, "failed to find policy")
	tu.Assert(t, len(pdebug.Collectors) == 1, "mismatch in collectors", pdebug.Collectors)
	tu.Assert(t, len(dbgInfo.CollectorTable) == 1, "invalid collectors", dbgInfo.CollectorTable)
	tu.Assert(t, len(dbgInfo.FlowRuleTable) == 32, "invalid num of flows", dbgInfo.FlowRuleTable)

	collDelResp := &halproto.CollectorDeleteResponseMsg{
		Response: []*halproto.CollectorDeleteResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}

	flowDelResp := &halproto.FlowMonitorRuleDeleteResponseMsg{
		Response: []*halproto.FlowMonitorRuleDeleteResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}

	halMock.EXPECT().CollectorDelete(gomock.Any(), gomock.Any()).Return(collDelResp, nil).AnyTimes()
	halMock.EXPECT().FlowMonitorRuleDelete(gomock.Any(), gomock.Any()).Return(flowDelResp, nil).AnyTimes()

	err = s.DeleteFlowExportPolicy(context.Background(), pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to delete export policy %+v", pol))

	data, err = s.Debug(nil)
	tu.AssertOk(t, err, "failed to read debug")

	dbgInfo, ok = data.(*debugInfo)
	tu.Assert(t, ok == true, "invalid debug info")

	pdebug, ok = dbgInfo.Policy[fmt.Sprintf("%s/%s/%s", pol.Tenant, pol.Namespace, pol.Name)]
	tu.Assert(t, ok == false, "didn't remove policy", pdebug)
	tu.Assert(t, len(pdebug.Collectors) == 0, "mismatch in collectors", pdebug.Collectors)
	tu.Assert(t, len(dbgInfo.CollectorTable) == 0, "invalid collectors", dbgInfo.CollectorTable)
	tu.Assert(t, len(dbgInfo.FlowRuleTable) == 0, "invalid num of flows", dbgInfo.FlowRuleTable)
}

func TestValidateFlowExportPolicy(t *testing.T) {
	testFlowExpSpecList := []struct {
		name   string
		policy monitoring.FlowExportPolicy
		fail   bool
	}{
		{
			name: "invalid protocol, should be UDP",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-protocol",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
						},
					},
				},
			},
		},

		{
			name: "empty transport",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-transport",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "",
						},
					},
				},
			},
		},

		{
			name: "empty destination",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-destination",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					Format:           "IPFIX",
					TemplateInterval: "5m",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "",
							Transport:   "UDP/1234",
						},
					},
				},
			},
		},

		{
			name: "duplicate targets",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "duplicate-targets",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					Format:           "IPFIX",
					TemplateInterval: "5m",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/1234",
						},
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/1234",
						},
					},
				},
			},
		},

		{
			name: "invalid interval",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-interval",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Format: "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/1234",
						},
					},
				},
			},
		},

		{
			name: "invalid format",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-format",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "NETFLOW",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/1234",
						},
					},
				},
			},
		},

		{
			name: "invalid transport, missing port",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "transport-missing-port",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "UDP",
						},
					},
				},
			},
		},

		{
			name: "invalid transport, missing protocol",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "transport-missing-protocol",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "1234",
						},
					},
				},
			},
		},

		{
			name: "invalid port (aaaa)",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-port",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/aaaa",
						},
					},
				},
			},
		},

		{
			name: "empty targets",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-targets",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
				},
			},
		},
		{
			name: "too many collectors",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-targets",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/1234",
						},
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/1235",
						},
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/1236",
						},
					},
				},
			},
		},

		{
			name: "valid policy",
			fail: false,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "valid-policy",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval:         "15s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "UDP/1234",
						},
					},
				},
			},
		},
	}

	for i := range testFlowExpSpecList {
		err := ValidateFlowExportPolicy(&testFlowExpSpecList[i].policy)

		if testFlowExpSpecList[i].fail == true {
			t.Logf(fmt.Sprintf("test [%v] returned %v", testFlowExpSpecList[i].name, err))
			tu.Assert(t, err != nil, "test [%v] returned %v", testFlowExpSpecList[i].name, err)
		} else {
			t.Log(fmt.Sprintf("test [%v] returned %v", testFlowExpSpecList[i].name, err))
			tu.AssertOk(t, err, "test [%v] returned %v", testFlowExpSpecList[i].name, err)
		}
	}
}
