package rpcserver

import (
	"fmt"
	"os"
	"testing"

	apiProtos "github.com/pensando/sw/api/generated/monitoring"
	tpmProtos "github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
	vLog "github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/rpckit"
	tu "github.com/pensando/sw/venice/utils/testutils"

	"context"
	"reflect"

	"github.com/pensando/sw/api"
)

const listenURL = "127.0.0.1:"
const defaultCollectInterval = "30s"

func TestMain(m *testing.M) {
	rpcLog = vLog.WithContext("pkg", "TEST-"+pkgName)
	// call flag.Parse() here if TestMain uses flags
	os.Exit(m.Run())
}

func TestNewRPCServer(t *testing.T) {
	_, err := NewRPCServer("", nil, defaultCollectInterval)
	tu.Assert(t, err != nil, "didn't fail for invalid URL")

	f, err := NewRPCServer(listenURL, nil, defaultCollectInterval)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

}

func TestSetCollectionInterval(t *testing.T) {
	policyDb := memdb.NewMemdb()
	f, err := NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	interval := "300s"
	err = f.SetCollectionInterval(interval)
	tu.AssertOk(t, err, "failed to set collection interval")
	tu.Assert(t, f.statsPolicyRPCServer.collectionInterval.Load().(string) == interval,
		fmt.Sprintf("invalid collection interval %v",
			f.statsPolicyRPCServer.collectionInterval.Load()))
}

func TestWatchStatsPolicyError(t *testing.T) {

	policyDb := memdb.NewMemdb()
	f, err := NewRPCServer(listenURL, policyDb, "100s")
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	statsClient := tpmProtos.NewStatsPolicyApiClient(grpc.ClientConn)
	evWatch, err := statsClient.WatchStatsPolicy(context.Background(), &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch stats policy")
	f.Stop()
	policy, err := evWatch.Recv()
	tu.Assert(t, err != nil, fmt.Sprintf("failed to stop rpc server, %+v", policy))
}

func TestWatchStatsError(t *testing.T) {
	sp := map[string]*apiProtos.FwlogPolicy{
		"fwlog-1": {
			TypeMeta:   api.TypeMeta{Kind: "StatsPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-1"},
		},
		"fwlog-2": {
			TypeMeta:   api.TypeMeta{Kind: "StatsPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-2"},
		},
	}
	policyDb := memdb.NewMemdb()

	for _, p := range sp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add stats object %+v", p))
	}
	f, err := NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	tu.Assert(t, defaultCollectInterval == f.collectionInterval.Load().(string),
		fmt.Sprintf("interval [%v] didn't match in policy, {%s} ", f.collectionInterval.Load(),
			defaultCollectInterval))

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	statsClient := tpmProtos.NewStatsPolicyApiClient(grpc.ClientConn)
	evWatch, err := statsClient.WatchStatsPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch stats policy")
	_, err = evWatch.Recv()
	tu.Assert(t, err != nil, "failed to test invalid policy")

}

func TestWatchStatsPolicy(t *testing.T) {

	sp := map[string]*apiProtos.StatsPolicy{
		"stats-1": {
			TypeMeta:   api.TypeMeta{Kind: "StatsPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "stats-1"},
		},
		"stats-2": {
			TypeMeta:   api.TypeMeta{Kind: "StatsPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "stats-2"},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range sp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add stats object %+v", p))
	}
	f, err := NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	tu.Assert(t, defaultCollectInterval == f.collectionInterval.Load().(string),
		fmt.Sprintf("interval [%v] didn't match in policy, {%s} ", f.collectionInterval.Load(),
			defaultCollectInterval))

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	statsClient := tpmProtos.NewStatsPolicyApiClient(grpc.ClientConn)
	evWatch, err := statsClient.WatchStatsPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch stats policy")
	for i := 0; i < len(sp); i++ {
		ev, err := evWatch.Recv()
		tu.AssertOk(t, err, "failed to receive stats policy")
		tu.Assert(t, ev.EventType == api.EventType_CreateEvent, fmt.Sprintf("invalid stats event type %+v", ev))
		evPolicy := ev.Policy
		cfgPolicy := sp[evPolicy.GetName()]
		tu.Assert(t, evPolicy.GetName() == cfgPolicy.GetName(),
			fmt.Sprintf("policy [%v] didn't match in policy, {%+v} ", evPolicy, sp))
		tu.Assert(t, evPolicy.TypeMeta == cfgPolicy.TypeMeta,
			fmt.Sprintf("policy [%v] didn't match in policy, {%+v} ", evPolicy, sp))
		tu.Assert(t, evPolicy.Spec.Interval == f.collectionInterval.Load().(string),
			fmt.Sprintf("interval [%v] didn't match in policy, [%+v] ", evPolicy, f))
	}

	// update object
	policyDb.UpdateObject(sp["stats-1"])
	updObj, err := evWatch.Recv()
	tu.AssertOk(t, err, "failed to receive stats policy update")
	tu.Assert(t, updObj.EventType == api.EventType_UpdateEvent, fmt.Sprintf("got event: %+v, expected: %v",
		updObj, api.EventType_UpdateEvent))

	// client cancel
	cancel()
}

func TestWatchFwlogPolicy(t *testing.T) {
	fp := map[string]*apiProtos.FwlogPolicy{
		"fwlog-1": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-1"},
			Spec: apiProtos.FwlogSpec{
				Filter: []string{
					apiProtos.FwlogFilter_FWLOG_ACCEPT.String(),
					apiProtos.FwlogFilter_FWLOG_DENY.String(),
				},
				Exports: []*apiProtos.FwlogExport{
					{
						Targets: []api.ExportConfig{
							{
								Destination: "collector1.test.com",
								Transport:   "UDP/514",
							},
						},
						Format: apiProtos.MonitoringExportFormat_SYSLOG_BSD.String(),
						Filter: []string{apiProtos.FwlogFilter_FWLOG_ALL.String()},
						SyslogConfig: &api.SyslogExportConfig{
							Prefix:           "prefix1",
							FacilityOverride: "test-override1",
						},
					},
				},
			},
		},
		"fwlog-2": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-2"},
			Spec: apiProtos.FwlogSpec{
				Filter: []string{
					apiProtos.FwlogFilter_FWLOG_ACCEPT.String(),
					apiProtos.FwlogFilter_FWLOG_DENY.String(),
				},
				Exports: []*apiProtos.FwlogExport{
					{
						Targets: []api.ExportConfig{
							{
								Destination: "collector2.test.com",
								Transport:   "TCP/514",
							},
						},
						Format: apiProtos.MonitoringExportFormat_SYSLOG_BSD.String(),
						Filter: []string{apiProtos.FwlogFilter_FWLOG_ALL.String()},
						SyslogConfig: &api.SyslogExportConfig{
							Prefix:           "prefix2",
							FacilityOverride: "test-override2",
						},
					},
				},
			},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range fp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}

	f, err := NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	fwlogClient := tpmProtos.NewFwlogPolicyApiClient(grpc.ClientConn)
	evWatch, err := fwlogClient.WatchFwlogPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch fwlog policy")
	for i := 0; i < len(fp); i++ {
		policy, err := evWatch.Recv()
		tu.AssertOk(t, err, "failed to receive fwlog policy")
		tu.Assert(t, policy.EventType == api.EventType_CreateEvent, fmt.Sprintf("invalid event type %+v", policy))
		evPolicy := policy.Policy
		cfgPolicy := fp[evPolicy.Name]

		tu.Assert(t, evPolicy.TypeMeta == cfgPolicy.TypeMeta,
			fmt.Sprintf(" type meta [%v] didn't match in policy, {%+v} ", evPolicy, fp))
		tu.Assert(t, evPolicy.ObjectMeta.Name == cfgPolicy.ObjectMeta.Name,
			fmt.Sprintf("obj meta [%v] didn't match in policy, {%+v} ", evPolicy, fp))
		tu.Assert(t, reflect.DeepEqual(evPolicy.Spec.Filter, cfgPolicy.Spec.Filter),
			fmt.Sprintf("policy filter [%v] didn't match in policy, {%+v} ", evPolicy, fp))
		tu.Assert(t, len(evPolicy.Spec.Exports) == len(cfgPolicy.Spec.GetExports()),
			fmt.Sprintf("policy count didn't match in policy,{%+v}  {%+v} ", evPolicy, fp))

		tu.Assert(t, reflect.DeepEqual(evPolicy.Spec.Exports, cfgPolicy.Spec.Exports),
			fmt.Sprintf("policy exports [%v] didn't match in policy, {%+v} ", evPolicy, fp))
	}

	// update object
	policyDb.UpdateObject(fp["fwlog-2"])
	updObj, err := evWatch.Recv()
	tu.AssertOk(t, err, "failed to receive fwlog policy update")
	tu.Assert(t, updObj.EventType == api.EventType_UpdateEvent, fmt.Sprintf("got event: %+v, expected: %v",
		updObj, api.EventType_UpdateEvent))

	// client cancel
	cancel()
}

func TestFwlogError(t *testing.T) {
	sp := map[string]*apiProtos.StatsPolicy{
		"fwlog-1": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "stats-1"},
		},
		"fwlog-2": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "stats-2"},
		},
	}
	policyDb := memdb.NewMemdb()

	for _, p := range sp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}
	f, err := NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	tu.Assert(t, defaultCollectInterval == f.collectionInterval.Load().(string),
		fmt.Sprintf("interval [%v] didn't match in policy, {%s} ", f.collectionInterval.Load(),
			defaultCollectInterval))

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	client := tpmProtos.NewFwlogPolicyApiClient(grpc.ClientConn)
	evWatch, err := client.WatchFwlogPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch fwlog policy")
	_, err = evWatch.Recv()
	tu.Assert(t, err != nil, "failed to test invalid policy")
}

func TestWatchFlowExportPolicy(t *testing.T) {

	flow := map[string]*apiProtos.FlowExportPolicy{
		"flow-1": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "flow-1"},
			Spec: apiProtos.FlowExportSpec{
				Targets: []apiProtos.FlowExportTarget{
					{
						Interval: "60s",
						Format:   "syslog_bsd",
						Exports: []api.ExportConfig{
							{
								Destination: "collector1.test.com",
								Transport:   "TCP/514",
							},
						},
					},
				},
			},
		},
		"flow-2": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "flow-2"},
			Spec: apiProtos.FlowExportSpec{
				Targets: []apiProtos.FlowExportTarget{
					{
						Interval: "100s",
						Format:   "syslog_bsd",
						Exports: []api.ExportConfig{
							{
								Destination: "collector2.test.com",
								Transport:   "UDP/514",
							},
						},
					},
				},
			},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range flow {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add flow export object %+v", p))
	}

	f, err := NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	flowClient := tpmProtos.NewFlowExportPolicyApiClient(grpc.ClientConn)
	evWatch, err := flowClient.WatchFlowExportPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch flow export policy")

	for i := 0; i < len(flow); i++ {
		obj, err := evWatch.Recv()
		tu.AssertOk(t, err, "failed to receive flow export policy")
		tu.Assert(t, obj.EventType == api.EventType_CreateEvent, fmt.Sprintf("invalid event type in object %+v", obj))
		evPolicy := obj.Policy
		cfgPolicy := flow[evPolicy.GetName()]
		tu.Assert(t, evPolicy.TypeMeta == cfgPolicy.TypeMeta,
			fmt.Sprintf("policy [%v] didn't match in policy, {%+v} ", evPolicy, cfgPolicy))
		tu.Assert(t, evPolicy.ObjectMeta.Name == cfgPolicy.ObjectMeta.Name,
			fmt.Sprintf("policy [%v] didn't match in policy, {%+v} ", evPolicy, cfgPolicy))
	}

	// update object
	flowObj := flow["flow-2"]

	policyDb.UpdateObject(flowObj)
	updObj, err := evWatch.Recv()
	tu.AssertOk(t, err, "failed to receive fwlog policy update")
	tu.Assert(t, updObj.EventType == api.EventType_UpdateEvent, fmt.Sprintf("got event: %+v, expected: %v",
		updObj, api.EventType_UpdateEvent))

	// client cancel
	cancel()
}

func TestFlowExportError(t *testing.T) {
	sp := map[string]*apiProtos.StatsPolicy{
		"fwlog-1": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-1"},
		},
		"fwlog-2": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-2"},
		},
	}
	policyDb := memdb.NewMemdb()

	for _, p := range sp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}
	f, err := NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	tu.Assert(t, defaultCollectInterval == f.collectionInterval.Load().(string),
		fmt.Sprintf("interval [%v] didn't match in policy, {%s} ", f.collectionInterval.Load(),
			defaultCollectInterval))

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	client := tpmProtos.NewFlowExportPolicyApiClient(grpc.ClientConn)
	evWatch, err := client.WatchFlowExportPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch export policy")
	_, err = evWatch.Recv()
	tu.Assert(t, err != nil, "failed to test invalid policy")
}
