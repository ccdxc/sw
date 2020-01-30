package rpcserver

import (
	"fmt"
	"os"
	"testing"
	"time"

	"context"
	"reflect"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	tpmProtos "github.com/pensando/sw/nic/agent/protos/tpmprotos"
	vLog "github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/rpckit"
	tu "github.com/pensando/sw/venice/utils/testutils"

	"github.com/pensando/sw/api"
)

const listenURL = "127.0.0.1:"

func TestMain(m *testing.M) {
	rpcLog = vLog.WithContext("pkg", "TEST-"+pkgName)
	// call flag.Parse() here if TestMain uses flags
	os.Exit(m.Run())
}

func TestNewRPCServer(t *testing.T) {
	_, err := NewRPCServer("", nil, nil)
	tu.Assert(t, err != nil, "didn't fail for invalid URL")

	f, err := NewRPCServer(listenURL, nil, nil)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

}

func TestListFwlogPolicy(t *testing.T) {
	fp := map[string]*monitoring.FwlogPolicy{
		"pol-1": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "pol-1"},
		},
		"pol-2": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "pol-2"},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range fp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add fwlog policy object %+v", p))
	}
	f, err := NewRPCServer(listenURL, policyDb, nil)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	fwlogClient := tpmProtos.NewFwlogPolicyApiV1Client(grpc.ClientConn)
	evList, err := fwlogClient.ListFwlogPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to list fwlog policy")
	tu.Assert(t, len(evList.EventList) == len(fp), fmt.Sprintf("got %d, expected %d", len(evList.EventList), len(fp)))
}

func TestWatchFwlogPolicy(t *testing.T) {
	fp := map[string]*monitoring.FwlogPolicy{
		"fwlog-1": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-1"},
			Spec: monitoring.FwlogPolicySpec{
				Targets: []monitoring.ExportConfig{
					{
						Destination: "collector1.test.com",
						Transport:   "UDP/514",
					},
				},
				Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
				Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
				Config: &monitoring.SyslogExportConfig{
					Prefix:           "prefix1",
					FacilityOverride: "test-override1",
				},
			},
		},
		"fwlog-2": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-2"},
			Spec: monitoring.FwlogPolicySpec{
				Targets: []monitoring.ExportConfig{
					{
						Destination: "collector2.test.com",
						Transport:   "TCP/514",
					},
				},
				Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
				Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
				Config: &monitoring.SyslogExportConfig{
					Prefix:           "prefix2",
					FacilityOverride: "test-override2",
				},
			},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range fp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}

	f, err := NewRPCServer(listenURL, policyDb, nil)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	fwlogClient := tpmProtos.NewFwlogPolicyApiV1Client(grpc.ClientConn)
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
		tu.Assert(t, reflect.DeepEqual(evPolicy.Spec.Format, cfgPolicy.Spec.Format),
			fmt.Sprintf("policy format [%v] didn't match in policy, {%+v} ", evPolicy, fp))
		tu.Assert(t, reflect.DeepEqual(evPolicy.Spec.Config, cfgPolicy.Spec.Config),
			fmt.Sprintf("policy config [%v] didn't match in policy, {%+v} ", evPolicy, fp))

		tu.Assert(t, reflect.DeepEqual(evPolicy.Spec.Targets, cfgPolicy.Spec.Targets),
			fmt.Sprintf("policy exports [%v] didn't match in policy, {%+v} ", evPolicy, fp))
	}

	// Check debug
	clients := f.Debug()
	tu.Assert(t, len(clients["FwlogPolicy"]) > 0, "debug for fwlog policy was empty")
	for _, v := range clients["FwlogPolicy"] {
		entryTime, err := time.Parse(time.RFC3339, v)
		tu.AssertOk(t, err, "Failed to parse time")
		elapsedTime := time.Since(entryTime).Seconds()
		tu.Assert(t, elapsedTime < 5 && elapsedTime >= 0, "reported timestamp was in the expected time range")
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
	sp := map[string]*monitoring.FlowExportPolicy{
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
	f, err := NewRPCServer(listenURL, policyDb, nil)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	client := tpmProtos.NewFwlogPolicyApiV1Client(grpc.ClientConn)
	evWatch, err := client.WatchFwlogPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch fwlog policy")
	_, err = evWatch.Recv()
	tu.Assert(t, err != nil, "failed to test invalid policy")
}

func TestListFlowExportPolicy(t *testing.T) {
	fp := map[string]*monitoring.FlowExportPolicy{
		"pol-1": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "pol-1"},
			Spec: monitoring.FlowExportPolicySpec{
				TemplateInterval: "10s",
			},
		},
		"pol-2": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "pol-2"},
			Spec: monitoring.FlowExportPolicySpec{
				TemplateInterval: "20s",
			},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range fp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add fwlog policy object %+v", p))
	}
	f, err := NewRPCServer(listenURL, policyDb, nil)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	fwlogClient := netproto.NewFlowExportPolicyApiV1Client(grpc.ClientConn)
	evList, err := fwlogClient.ListFlowExportPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to list policy")
	tu.Assert(t, len(evList.FlowExportPolicyEvents) == len(fp), fmt.Sprintf("got %d, expected %d", len(evList.FlowExportPolicyEvents), len(fp)))

	for _, p := range evList.FlowExportPolicyEvents {
		fp, ok := fp[p.Policy.Name]
		tu.Assert(t, ok == true, "failed to find policy")
		tu.Assert(t, fp.Spec.TemplateInterval == p.Policy.Spec.TemplateInterval, "failed to match policy %v", p.Policy.Name)

	}
}

func TestWatchFlowExportPolicy(t *testing.T) {

	flow := map[string]*monitoring.FlowExportPolicy{
		"flow-1": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "flow-1"},
			Spec: monitoring.FlowExportPolicySpec{
				Interval: "60s",
				Format:   "syslog_bsd",
				Exports: []monitoring.ExportConfig{
					{
						Destination: "collector1.test.com",
						Transport:   "TCP/514",
					},
				},
			},
		},
		"flow-2": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "flow-2"},
			Spec: monitoring.FlowExportPolicySpec{
				Interval: "100s",
				Format:   "syslog_bsd",
				Exports: []monitoring.ExportConfig{
					{
						Destination: "collector2.test.com",
						Transport:   "UDP/514",
					},
				},
				MatchRules: []*monitoring.MatchRule{
					{
						Src: &monitoring.MatchSelector{
							IPAddresses: []string{"10.10.10.1"},
						},
						Dst: &monitoring.MatchSelector{
							IPAddresses: []string{"10.10.10.2"},
						},
						AppProtoSel: &monitoring.AppProtoSelector{
							ProtoPorts: []string{"tcp/80"},
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

	f, err := NewRPCServer(listenURL, policyDb, nil)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	flowClient := netproto.NewFlowExportPolicyApiV1Client(grpc.ClientConn)
	evWatch, err := flowClient.WatchFlowExportPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
	tu.AssertOk(t, err, "failed to watch flow export policy")

	for i := 0; i < len(flow); i++ {
		obj, err := evWatch.Recv()
		tu.AssertOk(t, err, "failed to receive flow export policy")
		tu.Assert(t, obj.FlowExportPolicyEvents[0].EventType == api.EventType_CreateEvent, fmt.Sprintf("invalid event type in object %+v", obj))
		evPolicy := obj.FlowExportPolicyEvents[0].Policy
		cfgPolicy := flow[evPolicy.GetName()]
		tu.Assert(t, evPolicy.TypeMeta == cfgPolicy.TypeMeta,
			fmt.Sprintf("policy [%v] didn't match in policy, {%+v} ", evPolicy, cfgPolicy))
		tu.Assert(t, evPolicy.ObjectMeta.Name == cfgPolicy.ObjectMeta.Name,
			fmt.Sprintf("policy [%v] didn't match in policy, {%+v} ", evPolicy, cfgPolicy))
	}

	// Check debug
	clients := f.Debug()
	tu.Assert(t, len(clients["FlowExportPolicy"]) > 0, "debug for FlowExportPolicy was empty")
	for _, v := range clients["FlowExportPolicy"] {
		entryTime, err := time.Parse(time.RFC3339, v)
		tu.AssertOk(t, err, "Failed to parse time")
		elapsedTime := time.Since(entryTime).Seconds()
		tu.Assert(t, elapsedTime < 5 && elapsedTime >= 0, "reported timestamp was in the expected time range")
	}

	// update object
	flowObj := flow["flow-2"]

	policyDb.UpdateObject(flowObj)
	updObj, err := evWatch.Recv()
	tu.AssertOk(t, err, "failed to receive flow export policy update")
	tu.Assert(t, updObj.FlowExportPolicyEvents[0].EventType == api.EventType_UpdateEvent, fmt.Sprintf("got event: %+v, expected: %v",
		updObj, api.EventType_UpdateEvent))

	// client cancel
	cancel()
}

func TestRpcError(t *testing.T) {
	sp := map[string]*netproto.Tenant{
		"fwlog-1": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-1"},
		},
		"fwlog-2": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-1"},
		},
	}
	policyDb := memdb.NewMemdb()

	for _, p := range sp {
		err := policyDb.AddObject(p)
		tu.AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}
	f, err := NewRPCServer(listenURL, policyDb, nil)
	tu.AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	u := f.GetListenURL()
	tu.Assert(t, u != "", "invalid server URL", u)

	grpc, err := rpckit.NewRPCClient("test-client", f.server.GetListenURL(), rpckit.WithLoggerEnabled(true))
	tu.AssertOk(t, err, "failed to create rpc client")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	if true {
		client := netproto.NewFlowExportPolicyApiV1Client(grpc.ClientConn)
		evWatch, err := client.WatchFlowExportPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
		tu.AssertOk(t, err, "failed to watch export policy")
		_, err = evWatch.Recv()
		tu.Assert(t, err != nil, "failed to test invalid policy")
		evWatch.CloseSend()
	}

	if true {
		client := tpmProtos.NewFwlogPolicyApiV1Client(grpc.ClientConn)
		evWatch, err := client.WatchFwlogPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
		tu.AssertOk(t, err, "failed to watch fwlog policy")
		_, err = evWatch.Recv()
		tu.Assert(t, err != nil, "failed to test invalid policy")
		evWatch.CloseSend()

		for _, p := range sp {
			err := policyDb.DeleteObject(p)
			tu.AssertOk(t, err, "failed to delete policy")
		}

		go func() {
			time.Sleep(time.Second)
			for _, p := range sp {
				policyDb.AddObject(p)
			}
		}()

		client = tpmProtos.NewFwlogPolicyApiV1Client(grpc.ClientConn)
		evWatch, err = client.WatchFwlogPolicy(ctx, &api.ObjectMeta{Name: "client-1"})
		tu.AssertOk(t, err, "failed to watch fwlog policy")
		_, err = evWatch.Recv()
		tu.Assert(t, err != nil, "failed to test invalid policy")
	}

}

func TestConvertFlowExportPolicySpec(t *testing.T) {
	mspec := &monitoring.FlowExportPolicySpec{
		Interval: "10s",
		MatchRules: []*monitoring.MatchRule{
			{
				Dst: &monitoring.MatchSelector{
					IPAddresses: []string{"5.5.5.5", "6.6.6.6"},
				},
				Src: &monitoring.MatchSelector{
					IPAddresses: []string{"4.4.4.4", "3.3.3.3"},
				},
				AppProtoSel: &monitoring.AppProtoSelector{
					ProtoPorts: []string{"tcp/4444", "udp/3333"},
				},
			},
		},
		Exports: []monitoring.ExportConfig{
			{
				Destination: "1.2.3.4",
				Transport:   "tcp/5555",
			},
		},
	}

	tspec := convertFlowExportPolicySpec(mspec)

	tu.Assert(t, len(tspec.Exports) == len(mspec.Exports), "got %+v, expected %+v", len(tspec.Exports),
		len(mspec.Exports))

	for i, e := range mspec.Exports {
		tu.Assert(t, e.Destination == tspec.Exports[i].Destination, "got %+v, expected %+v", tspec.Exports[i].Destination,
			e.Destination)

		tu.Assert(t, e.Transport == fmt.Sprintf("%v/%v", tspec.Exports[i].Transport.Protocol, tspec.Exports[i].Transport.Port),
			"got %+v, expected %+v", tspec.Exports[i].Transport, e.Transport)
	}

	tu.Assert(t, len(tspec.MatchRules) == len(mspec.MatchRules), "invalid match-rule got %d, expected %d",
		len(tspec.MatchRules), len(mspec.MatchRules))
	for i, mr := range mspec.MatchRules {
		tr := tspec.MatchRules[i]

		tu.Assert(t, reflect.DeepEqual(mr.Src.IPAddresses, tr.Src.Addresses), "invalid src, got %v, expected %v", tr.Src, mr.Src)
		tu.Assert(t, reflect.DeepEqual(mr.Dst.IPAddresses, tr.Dst.Addresses), "invalid dst, got %v, expected %+v",
			tr.Dst.Addresses, mr.Dst.IPAddresses)
		tu.Assert(t, len(mr.AppProtoSel.ProtoPorts) == len(tr.Dst.ProtoPorts), "invalid proto ports got %v, expected %v",
			len(tr.Dst.ProtoPorts), len(mr.AppProtoSel.ProtoPorts))
		for j, p := range mr.AppProtoSel.ProtoPorts {
			tu.Assert(t, p == fmt.Sprintf("%v/%v", tr.Dst.ProtoPorts[j].Protocol, tr.Dst.ProtoPorts[j].Port),
				"got %v, expected %v", tr.Dst.ProtoPorts[j], p)
		}
	}
}
