package state

import (
	"testing"

	"github.com/pensando/sw/venice/utils/emstore"

	agstate "github.com/pensando/sw/nic/agent/netagent/state"
	tu "github.com/pensando/sw/venice/utils/testutils"

	"fmt"
	"os"
	"reflect"

	"github.com/golang/mock/gomock"
	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/agent/netagent/state/dependencies"
	mockdatapath "github.com/pensando/sw/nic/agent/tpa/datapath"
	"github.com/pensando/sw/nic/agent/tpa/protos"
	"github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
)

const emDbPath = "/tmp/naples-tpagent.db"

func cleanup(t *testing.T, ag *PolicyState) {
	l, err := ag.store.List(&tpaprotos.FlowExportPolicyObj{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{},
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

func TestValidateMeta(t *testing.T) {

	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NamespaceDB: map[string]*netproto.Namespace{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}

	ag, err := NewTpAgent(na, mockdatapath.MockHal())
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

func TestFindNumExports(t *testing.T) {

	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NamespaceDB: map[string]*netproto.Namespace{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}

	ag, err := NewTpAgent(na, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, ag)

	spec := monitoring.FlowExportTarget{

		Interval: "15s",
		Format:   "IPFIX",
		Exports: []monitoring.ExportConfig{
			{
				Destination: fmt.Sprintf("192.168.3.1"),
				Transport:   "TCP/1234",
			},
			{
				Destination: fmt.Sprintf("192.168.3.2"),
				Transport:   "TCP/1234",
			},
		},
	}

	for i := 1; i < 4; i++ {
		pol := &tpaprotos.FlowExportPolicyObj{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("test-%d", i), Tenant: "default", Namespace: "default"},
			P: &tpmprotos.FlowExportPolicy{
				Spec: tpmprotos.FlowExportPolicySpec{
					Targets: []monitoring.FlowExportTarget{spec, spec},
				},
			},
		}
		err = ag.store.Write(pol)
		n, err := ag.findNumExports(nil)
		tu.AssertOk(t, err, fmt.Sprintf("failed to find num exports, %s", err))
		tu.Assert(t, n == i*4, fmt.Sprintf("invalid num exports, require %d got %d", i*4, n))

		// exclude the current policy
		n, err = ag.findNumExports(&api.ObjectMeta{Name: fmt.Sprintf("test-%d", i), Tenant: "default", Namespace: "default"})
		tu.AssertOk(t, err, fmt.Sprintf("failed to find num exports, %s", err))
		tu.Assert(t, n == (i-1)*4, fmt.Sprintf("invalid num exports, require %d got %d", (i-1)*4, n))
	}
}

func TestValidatePolicy(t *testing.T) {
	destAddr := "192.168.3.1"
	ds, err := createDataStore()
	tu.AssertOk(t, err, fmt.Sprintf("failed to create database"))

	na := &agstate.Nagent{
		TenantDB:    map[string]*netproto.Tenant{},
		NamespaceDB: map[string]*netproto.Namespace{},
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
	}

	ag, err := NewTpAgent(na, mockdatapath.MockHal())
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
			Targets: []monitoring.FlowExportTarget{
				{
					Interval: "15s",
					Format:   "IPFIX",
					Exports:  []monitoring.ExportConfig{},
				},
			},
		}}

	_, err = ag.validatePolicy(pol)
	tu.Assert(t, err != nil, fmt.Sprintf("nil export didnt fail"))

	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{
			Targets: []monitoring.FlowExportTarget{
				{
					Interval: "15s",
					Format:   "IPFIX",
					Exports: []monitoring.ExportConfig{
						{Transport: "TCP/1234"},
					},
				},
			},
		}}

	_, err = ag.validatePolicy(pol)
	tu.Assert(t, err != nil, fmt.Sprintf("nil destination didnt fail"))

	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{
			Targets: []monitoring.FlowExportTarget{
				{
					Interval: "15s",
					Format:   "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: destAddr,
						},
					},
				},
			},
		}}

	_, err = ag.validatePolicy(pol)
	tu.Assert(t, err != nil, fmt.Sprintf("nil transport didnt fail"))

	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{
			Targets: []monitoring.FlowExportTarget{
				{
					Interval: "15s",
					Format:   "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: destAddr,
							Transport:   "TCP/1234",
						},
					},
				},
			},
		}}

	c, err := ag.validatePolicy(pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to find num exports"))
	tu.Assert(t, c == 1, fmt.Sprintf("expected num exports: 1, got %d", c))

	// check interval
	pol = &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
		Spec: tpmprotos.FlowExportPolicySpec{
			Targets: []monitoring.FlowExportTarget{
				{
					Format: "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: destAddr,
							Transport:   "TCP/1234",
						},
					},
				},
			},
		}}

	testInterval := []struct {
		interval string
		pass     bool
	}{
		{interval: "15ns", pass: false},
		{interval: "15us", pass: false},
		{interval: "15µs", pass: false},
		{interval: "999ms", pass: false},
		{interval: "25h", pass: false},
		{interval: "1s", pass: true},
	}

	for _, i := range testInterval {
		pol.Spec.Targets[0].Interval = i.interval
		_, err = ag.validatePolicy(pol)
		if i.pass {
			tu.AssertOk(t, err, fmt.Sprintf("failed to validate interval: %s", pol.Spec.Targets[0].Interval))
		} else {
			tu.Assert(t, err != nil, fmt.Sprintf("didnt fail interval: %s", pol.Spec.Targets[0].Interval))
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

	ag, err := NewTpAgent(na, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, ag)

	pol := &tpaprotos.FlowExportPolicyObj{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "default"},
	}

	err = ag.store.Write(pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy"))

	obj, err := ag.store.Read(pol)
	tu.AssertOk(t, err, fmt.Sprintf("failed to read export policy"))
	rp, ok := obj.(*tpaprotos.FlowExportPolicyObj)
	tu.Assert(t, ok == true, fmt.Sprintf("invalid export policy %+v", obj))
	tu.Assert(t, rp == pol, fmt.Sprintf("export policy  didn't match, %+v, %+v", rp, pol))

	pol2 := &tpaprotos.FlowExportPolicyObj{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test2", Tenant: "default"},
	}

	err = ag.store.Write(pol2)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy"))
	l, err := ag.store.List(&tpaprotos.FlowExportPolicyObj{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{},
	})
	tu.AssertOk(t, err, fmt.Sprintf("failed to list export policy"))
	tu.Assert(t, len(l) == 2, fmt.Sprintf("invalid number of export policy, expectec 2 got %d, %+v", len(l), l))
}

func TestCreateFlowExportPolicy(t *testing.T) {
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
			IPv4Subnet: "",
		},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	for i := 0; i < maxFlowExportCollectors; i++ {
		name := fmt.Sprintf("ep%d", i+1)
		addr := fmt.Sprintf("192.168.3.%d/32", 10+i)
		key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: name}, api.TypeMeta{Kind: "endpoint"})
		na.EndpointDB[key] = &netproto.Endpoint{
			ObjectMeta: api.ObjectMeta{Name: "ep1", Namespace: "default", Tenant: "default"},
			Spec: netproto.EndpointSpec{
				NetworkName: "default",
				IPv4Address: addr,
			},
			Status: netproto.EndpointStatus{},
		}
	}

	s, err := NewTpAgent(na, mockdatapath.MockHal())
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))
	defer cleanup(t, s)

	policyPrefix := "flowmon"
	for l := 0; l < maxFlowExportCollectors+1; l++ {
		pol := &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s-%d", policyPrefix, l), Tenant: "default", Namespace: "default"},

			Spec: tpmprotos.FlowExportPolicySpec{
				Targets: []monitoring.FlowExportTarget{
					{
						Interval: "15s",
						Format:   "IPFIX",
						Exports: []monitoring.ExportConfig{
							{
								Destination: fmt.Sprintf("192.168.3.%d", 10+l),
								Transport:   "TCP/1234",
							},
						},
					},
				},
			},
		}

		err = s.CreateFlowExportPolicy(context.Background(), pol)
		if l < maxFlowExportCollectors {
			tu.AssertOk(t, err, fmt.Sprintf("failed to create export policy %+v", pol))
		} else {
			tu.Assert(t, err != nil, fmt.Sprintf("created more than max (%d) policies", maxFlowExportCollectors))
		}
	}

	// list
	rp, err := s.ListFlowExportPolicy(context.Background())
	tu.AssertOk(t, err, fmt.Sprintf("failed to list export policy"))
	tu.Assert(t, len(rp) == maxFlowExportCollectors, fmt.Sprintf("expected %d export policy, got %d {%+v}", maxFlowExportCollectors, len(rp), rp))

	for l := 0; l < maxFlowExportCollectors; l++ {
		pol := &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s-%d", policyPrefix, l), Tenant: "default", Namespace: "default"},

			Spec: tpmprotos.FlowExportPolicySpec{
				Targets: []monitoring.FlowExportTarget{
					{
						Interval: "15s",
						Format:   "IPFIX",
						Exports: []monitoring.ExportConfig{
							{
								Destination: fmt.Sprintf("192.168.3.%d", 10+l),
								Transport:   "TCP/1234",
							},
						},
					},
				},
			},
		}

		rp, err := s.GetFlowExportPolicy(context.Background(), &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s-%d", policyPrefix, l), Tenant: "default", Namespace: "default"},
		})

		tu.AssertOk(t, err, fmt.Sprintf("failed to read export policy %s-%d", policyPrefix, l))
		tu.Assert(t, reflect.DeepEqual(rp, pol), "policy in db didn;t match, got %+v, expected:%+v", pol, rp)
	}

	for l := 0; l < maxFlowExportCollectors; l++ {
		err = s.DeleteFlowExportPolicy(context.Background(), &tpmprotos.FlowExportPolicy{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s-%d", policyPrefix, l), Tenant: "default", Namespace: "default"},
		})

		tu.AssertOk(t, err, fmt.Sprintf("failed to delete export policy"))
	}

	// verify all policie are removed
	pl, err := s.store.List(&tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"}})
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
		Solver:      dependencies.NewDepSolver(),
		Store:       ds,
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

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			IPv4Subnet: "",
		},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	for i := 0; i < maxFlowExportCollectors; i++ {
		name := fmt.Sprintf("ep%d", i+1)
		addr := fmt.Sprintf("192.168.3.%d/32", 10+i)
		key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: name}, api.TypeMeta{Kind: "endpoint"})
		na.EndpointDB[key] = &netproto.Endpoint{
			ObjectMeta: api.ObjectMeta{Name: "ep1", Namespace: "default", Tenant: "default"},
			Spec: netproto.EndpointSpec{
				NetworkName: "default",
				IPv4Address: addr,
			},
			Status: netproto.EndpointStatus{},
		}
	}

	s, err := NewTpAgent(na, halMock)
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
		ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("%s", policyPrefix), Tenant: "default", Namespace: "default"},

		Spec: tpmprotos.FlowExportPolicySpec{
			Targets: []monitoring.FlowExportTarget{
				{
					Interval: "15s",
					Format:   "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: fmt.Sprintf("192.168.3.11"),
							Transport:   "UDP/1234",
						},
					},
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

	s, err := NewTpAgent(na, halMock)
	tu.AssertOk(t, err, fmt.Sprintf("failed to create telemetry agent"))

	defer cleanup(t, s)

	pol := &tpmprotos.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "flowexport", Tenant: "default", Namespace: "default"},

		Spec: tpmprotos.FlowExportPolicySpec{
			Targets: []monitoring.FlowExportTarget{
				{
					Interval: "15s",
					Format:   "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.10.10.1",
							Transport:   "UDP/1234",
						},
					},
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
			NetworkName: "default",
			IPv4Address: "10.10.10.1/32",
		},
		Status: netproto.EndpointStatus{},
	}

	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid network didn't fail"))

	key = na.Solver.ObjectKey(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "default"}, api.TypeMeta{Kind: "network"})
	na.NetworkDB[key] = &netproto.Network{
		ObjectMeta: api.ObjectMeta{Name: "default", Namespace: "default", Tenant: "default"},
		Spec: netproto.NetworkSpec{
			IPv4Subnet: "",
		},
		Status: netproto.NetworkStatus{
			NetworkID: uint64(101),
		},
	}

	pol.Spec.Targets[0].Exports[0].Transport = "IPIP/100"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid protocol didn't fail"))

	pol.Spec.Targets[0].Exports[0].Transport = "ICMP/1000"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid protocol didn't fail"))

	pol.Spec.Targets[0].Exports[0].Transport = "TCP/UDP"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid port didn't fail"))

	pol.Spec.Targets[0].Exports[0].Transport = "TCP/65536"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid port didn't fail"))

	pol.Spec.Targets[0].Exports[0].Transport = "UDP/1234"
	pol.Spec.Targets[0].Format = "NETFLOW"
	err = s.CreateFlowExportPolicy(context.Background(), pol)
	tu.Assert(t, err != nil, fmt.Sprintf("invalid format didn't fail"))
}

func TestHalIPAddr(t *testing.T) {
	_, _, err := convertToHalIPAddr("test")
	tu.Assert(t, err != nil, fmt.Sprintf("invalid ip didn't fail"))

	_, _, err = convertToHalIPAddr("google.com")
	tu.AssertOk(t, err, fmt.Sprintf("failed to look up hostname"))

	_, _, err = convertToHalIPAddr("2001:0db8:85a3:0000:0000:8a2e:0370:7334")
	tu.AssertOk(t, err, fmt.Sprintf("failed to parse ipv6"))
}
