package ntsdb

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"math/rand"
	"net/http"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/ntsdb/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testSendInterval = 10 * time.Millisecond
)

type testSuite struct {
	rpcServer    *rpckit.RPCServer
	rpcClient    *rpckit.RPCClient
	context      context.Context
	cancelFunc   context.CancelFunc
	metricServer *mock.Collector
}

var ts *testSuite

type endpoint struct {
	api.TypeMeta
	api.ObjectMeta
	epm endpointMetric
}

type endpointMetric struct {
	OutgoingConns api.Counter
	IncomingConns api.Counter
	Bandwidth     api.Gauge
	PacketErrors  api.Counter
	Violations    api.Counter
	LinkUp        api.Bool
	WorkloadName  api.String
	RxPacketSize  api.Histogram
	TxPacketSize  api.Histogram
	RxBandwidth   api.Summary
	TxBandwidth   api.Summary
}

func TestSetup(t *testing.T) {
	s, err := rpckit.NewRPCServer("fake-collector", ":0", rpckit.WithLoggerEnabled(false))
	if err != nil {
		t.Fatalf("failed to start grpc server: %v", err)
	}
	metricServer := &mock.Collector{}
	metric.RegisterMetricApiServer(s.GrpcServer, metricServer)
	s.Start()
	rpcClient, err := rpckit.NewRPCClient("collector-client", s.GetListenURL(), rpckit.WithLoggerEnabled(false))
	if err != nil {
		t.Fatalf("fail to dial: %v", err)
	}
	nctx, cancel := context.WithCancel(context.Background())
	ts = &testSuite{
		rpcServer:    s,
		rpcClient:    rpcClient,
		context:      nctx,
		cancelFunc:   cancel,
		metricServer: metricServer,
	}

	options := &Opts{
		ClientName:   t.Name(),
		Collector:    ts.rpcServer.GetListenURL(),
		SendInterval: testSendInterval,
		DBName:       "objMetrics",
		LocalPort:    rand.Int()%52000 + 8000,
	}

	Init(ts.context, options)
}

func TestOMetricsAPI(t *testing.T) {
	ts.metricServer.ClearMetrics()

	ep := &endpoint{}
	ep.TypeMeta.Kind = t.Name()
	ep.ObjectMeta.Tenant = "test"
	ep.ObjectMeta.Name = "ucase1"

	// Use case 1 - collect various kinds of metrics
	table, err := NewOTable(ep, &ep.epm, &TableOpts{})
	AssertOk(t, err, "unable to create table")

	ep.epm.OutgoingConns.Add(32)
	ep.epm.IncomingConns.Add(43)
	ep.epm.Bandwidth.Set(608.2, time.Time{})
	ep.epm.PacketErrors.Add(12)
	ep.epm.Violations.Inc()
	ep.epm.LinkUp.Set(true, time.Time{})
	ep.epm.WorkloadName.Set("test-workload", time.Time{})
	ep.epm.RxPacketSize.AddSample(154)
	ep.epm.TxPacketSize.AddSample(4096)
	ep.epm.RxBandwidth.AddSample(23.4)
	ep.epm.TxBandwidth.AddSample(9066.32)

	table.Delete()

	// Use case 2 - another table with same kind/tenant
	ep.ObjectMeta.Name = "ucase2"
	epm := &endpointMetric{}
	table, err = NewOTable(ep, epm, &TableOpts{})
	AssertOk(t, err, "unable to create table")

	epm.OutgoingConns.Add(7)
	epm.Violations.Inc()

	table.Delete()

	// Use case 3 - use freeform apis on Otable
	ep.ObjectMeta.Name = "ucase3"
	epm = &endpointMetric{}
	table, err = NewOTable(ep, epm, &TableOpts{})
	AssertOk(t, err, "unable to create table")

	epm.OutgoingConns.Add(3)
	epm.OutgoingConns.Inc()
	table.Counter("OutgoingConns").Inc()

	time.Sleep(3 * testSendInterval)
	table.Delete()

	// verify three points
	tags := []map[string]string{
		{"Tenant": "test", "Kind": t.Name(), "Name": "ucase1"},
		{"Tenant": "test", "Kind": t.Name(), "Name": "ucase2"},
		{"Tenant": "test", "Kind": t.Name(), "Name": "ucase3"},
	}
	fields := []map[string]interface{}{
		{
			"OutgoingConns": int64(32), "IncomingConns": int64(43),
			"Bandwidth":    float64(608.2),
			"PacketErrors": int64(12), "Violations": int64(1),
			"LinkUp": true, "WorkloadName": "test-workload",
			"RxPacketSize_256": int64(1), "TxPacketSize_16384": int64(1),
			"RxBandwidth_totalValue": float64(23.4), "RxBandwidth_totalCount": int64(1),
			"TxBandwidth_totalValue": float64(9066.32), "TxBandwidth_totalCount": int64(1),
		},
		{
			"OutgoingConns": int64(7), "Violations": int64(1),
		},
		{
			"OutgoingConns": int64(5),
		},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "100ms", "1s")
}

func TestMetricsAPI(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Counter("rxpkts").Inc()
	table.Counter("txpkts").Add(1)
	table.Counter("txbytes").Add(12)
	table.Counter("txbytes").Inc()
	table.Gauge("bandwidth").Set(76.1, time.Time{})
	table.String("status").Set("upgrading", time.Time{})
	table.Bool("powerOn").Set(true, time.Time{})
	table.Histogram("latency").AddSample(15)
	table.Histogram("latency").AddSample(3022)
	table.Histogram("latency").AddSample(12)
	table.Histogram("latency").AddSample(0)
	table.Summary("rtt").AddSample(32)
	table.Summary("rtt").AddSample(13)
	table.Summary("rtt").AddSample(120)
	table.Summary("rtt").AddSample(18)

	tags := []map[string]string{
		{"Name": t.Name()},
	}
	fields := []map[string]interface{}{
		{
			"rxpkts": int64(1), "txpkts": int64(1),
			"txbytes": int64(13), "status": "upgrading", "powerOn": true,
			"bandwidth": float64(76.1),
			"latency_4": int64(1), "latency_16": int64(2), "latency_4096": int64(1),
			"rtt_totalValue": float64(183), "rtt_totalCount": int64(4),
		},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "100ms", "1s")
}

func TestMetricsWithPoints(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Counter("rxpkts").Inc()
	table.Gauge("bandwidth").Set(76.1, time.Time{})
	table.Point(
		map[string]string{"src": "10.1.1.1", "dest": "11.1.1.1", "port": "8080"},
		map[string]interface{}{"action": "rejected"}, time.Time{})
	table.Point(
		map[string]string{"src": "10.1.1.1", "dest": "12.1.1.1", "port": "80"},
		map[string]interface{}{"action": "permitted"}, time.Time{})

	tags := []map[string]string{
		{"Name": t.Name()},
		{"src": "10.1.1.1", "dest": "11.1.1.1", "port": "8080"},
		{"src": "10.1.1.1", "dest": "12.1.1.1", "port": "80"},
	}
	fields := []map[string]interface{}{
		{"rxpkts": int64(1), "bandwidth": float64(76.1)},
		{"action": "rejected"},
		{"action": "permitted"},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "100ms", "1s")
}

func TestAttributeChangeWithAggregation(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")

	table.String("status").Set("upgrading", time.Time{})
	table.Bool("powerOn").Set(true, time.Time{})
	table.String("status").Set("upgraded", time.Time{})
	table.Bool("powerOn").Set(false, time.Time{})

	table.Delete()
	time.Sleep(3 * testSendInterval)

	tags := []map[string]string{
		{"Name": t.Name()},
		{"Name": t.Name()},
	}
	fields := []map[string]interface{}{
		{
			"status":  "upgrading",
			"powerOn": true,
		},
		{
			"status":  "upgraded",
			"powerOn": false,
		},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "100ms", "1s")
}

func TestHistogramCustomRangeOTable(t *testing.T) {
	ts.metricServer.ClearMetrics()

	ep := &endpoint{}
	ep.TypeMeta.Kind = t.Name()
	ep.ObjectMeta.Tenant = "test"
	ep.ObjectMeta.Name = "tcase1"

	epm := &endpointMetric{}

	table, err := NewOTable(ep, epm, &TableOpts{})
	AssertOk(t, err, "unable to create table")

	ranges := []int64{10, 100, 1000, 10000}
	epm.RxPacketSize.SetRanges(ranges)
	epm.TxPacketSize.SetRanges(ranges)

	epm.RxPacketSize.AddSample(154)
	epm.RxPacketSize.AddSample(192)
	epm.RxPacketSize.AddSample(5)

	epm.TxPacketSize.AddSample(4096)
	epm.TxPacketSize.AddSample(2048)
	epm.TxPacketSize.AddSample(6000)

	time.Sleep(3 * testSendInterval)

	tags := []map[string]string{
		{"Tenant": "test", "Kind": t.Name(), "Name": "tcase1"},
	}
	fields := []map[string]interface{}{
		{
			"RxPacketSize_1000":  int64(2),
			"RxPacketSize_10":    int64(1),
			"TxPacketSize_10000": int64(3),
		},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "100ms", "1s")

	table.Delete()
}

func TestHistogramCustomRange(t *testing.T) {
	ts.metricServer.ClearMetrics()

	ranges := []int64{10, 100, 1000, 10000}
	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")

	table.Histogram("latency").SetRanges(ranges)

	table.Histogram("latency").AddSample(15)
	table.Histogram("latency").AddSample(3022)
	table.Histogram("latency").AddSample(12)
	table.Histogram("latency").AddSample(0)

	table.Delete()
	time.Sleep(3 * testSendInterval)

	tags := []map[string]string{
		{"Name": t.Name()},
	}
	fields := []map[string]interface{}{
		{
			"latency_10":    int64(1),
			"latency_100":   int64(2),
			"latency_10000": int64(1),
		},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "100ms", "1s")
}

func TestMultipleSets(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Gauge("bandwidth").Set(8.1, time.Time{})
	table.Counter("rxpkts").Inc()
	table.Gauge("bandwidth").Set(0.56, time.Time{})
	table.Counter("rxpkts").Inc()
	table.Gauge("bandwidth").Set(2.8, time.Time{})
	table.Counter("rxpkts").Inc()

	tags := []map[string]string{
		{"Name": t.Name()},
		{"Name": t.Name()},
		{"Name": t.Name()},
	}
	fields := []map[string]interface{}{
		{"bandwidth": float64(8.1)},
		{"bandwidth": float64(0.56)},
		{"bandwidth": float64(2.8), "rxpkts": int64(3)},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "100ms", "1s")
}

func TestMultipleSendIntervals(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Gauge("bandwidth").Set(88.1, time.Time{})
	table.Counter("rxpkts").Inc()
	table.Counter("rxpkts").Inc()
	table.Counter("rxpkts").Inc()
	time.Sleep(3 * testSendInterval)
	table.Gauge("bandwidth").Set(8.56, time.Time{})
	table.Counter("rxpkts").Inc()
	table.Counter("rxpkts").Inc()

	tags := []map[string]string{
		{"Name": t.Name()},
		{"Name": t.Name()},
	}
	fields := []map[string]interface{}{
		{"bandwidth": float64(88.1), "rxpkts": int64(3)},
		{"bandwidth": float64(8.56), "rxpkts": int64(5)},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "200ms", "2s")
}

func TestUserSuppliedTime(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	timeStamp := time.Unix(12345, 67890)
	table.Gauge("bandwidth").Set(94.3, timeStamp)
	table.Counter("rxpkts").Inc()
	table.Counter("rxpkts").Inc()
	table.Counter("rxpkts").Inc()

	tags := []map[string]string{
		{"Name": t.Name()},
	}
	fields := []map[string]interface{}{
		{"bandwidth": float64(94.3), "rxpkts": int64(3)},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), timeStamp, tags, fields), nil
	}, "bundle didn't contain some metrics", "200ms", "2s")
}

func TestLocalTable(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{Local: true})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Counter("rx_ep_create_msg").Inc()
	table.Counter("rx_ep_create_msg").Inc()
	table.Counter("rx_ep_create_msg").Inc()
	table.Counter("peer_disconnects").Inc()
	table.Counter("peer_rpc_failure").Inc()
	table.Gauge("cpu_in_use").Set(34.4, time.Time{})
	table.Gauge("mem_in_use").Set(102, time.Time{})
	table.String("version").Set("v0.1", time.Time{})

	lms := []LocalMetric{}
	httpGet(t, fmt.Sprintf("http://localhost:%v", global.opts.LocalPort), &lms)
	Assert(t, len(lms) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["rx_ep_create_msg"] == "3", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["peer_disconnects"] == "1", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["peer_rpc_failure"] == "1", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["cpu_in_use"] == "34.4", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["mem_in_use"] == "102", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["version"] == "v0.1", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestLocalTableMultipleRecords(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{Local: true})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Counter("counter1").Inc()
	table.Counter("counter1").Inc()
	table.Gauge("cpu_in_use").Set(34.4, time.Time{})
	table.Gauge("cpu_in_use").Set(44.3, time.Time{})

	lms := []LocalMetric{}
	httpGet(t, fmt.Sprintf("http://localhost:%v", global.opts.LocalPort), &lms)
	Assert(t, len(lms) == 2, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["counter1"] == "2", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["cpu_in_use"] == "34.4", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[1].Attributes["cpu_in_use"] == "44.3", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestLocalOneTable(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{Local: true})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Counter("counter1").Inc()
	table.Counter("counter1").Inc()
	table.Gauge("cpu_in_use").Set(67.6, time.Time{})

	table2, err := NewTable(t.Name()+"_2", &TableOpts{Local: true})
	AssertOk(t, err, "unable to create table")
	defer table2.Delete()

	table2.Counter("counter4").Inc()
	table2.Counter("counter5").Inc()
	table2.Gauge("guage2").Set(23, time.Time{})

	lms := []LocalMetric{}
	httpGet(t, fmt.Sprintf("http://localhost:%v/%s", global.opts.LocalPort, t.Name()), &lms)
	Assert(t, len(lms) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, len(lms[0].Attributes) == 2, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["counter1"] == "2", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["cpu_in_use"] == "67.6", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestLocalTableAttribute(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{Local: true})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Counter("counter1").Inc()
	table.Counter("counter1").Inc()
	table.Gauge("cpu_in_use").Set(34.4, time.Time{})

	lms := []LocalMetric{}
	httpGet(t, fmt.Sprintf("http://localhost:%v/%s/counter1", global.opts.LocalPort, t.Name()), &lms)
	Assert(t, len(lms) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, len(lms[0].Attributes) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["counter1"] == "2", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestTableRecreateFailure(t *testing.T) {
	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	_, err = NewTable(t.Name(), &TableOpts{})
	Assert(t, err != nil, "successfully able to recreate table")
}

func TestInitOptsReset(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	for i := 0; i < 100000; i++ {
		table.Counter("counter1").Inc()
	}
	time.Sleep(testSendInterval)

	Assert(t, ts.metricServer.ValidateSendInterval(testSendInterval), fmt.Sprintf("unable to verify the sendTimer"))
	ts.metricServer.ClearMetrics()

	// set the send interval
	Init(ts.context, &Opts{SendInterval: time.Second})

	table, err = NewTable(t.Name()+"_bigger_timeout", &TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	for i := 0; i < 1000000; i++ {
		table.Counter("counter1").Inc()
	}
	time.Sleep(time.Second)

	Assert(t, ts.metricServer.ValidateSendInterval(time.Second), fmt.Sprintf("unable to verify the sendTimer"))

	// reset send interval back
	Init(ts.context, &Opts{SendInterval: testSendInterval})
	time.Sleep(time.Second)
}

func TestOptionTablePrecision(t *testing.T) {
	ts.metricServer.ClearMetrics()

	table, err := NewTable(t.Name(), &TableOpts{Precision: time.Millisecond})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	table.Gauge("cpu_usage").Set(67.6, time.Time{})
	table.Gauge("disk_usage").Set(31.4, time.Time{})
	time.Sleep(2 * time.Millisecond)
	table.Gauge("memory_usage").Set(4.5, time.Time{})
	time.Sleep(3 * testSendInterval)

	tags := []map[string]string{
		{"Name": t.Name()},
		{"Name": t.Name()},
	}
	fields := []map[string]interface{}{
		{"cpu_usage": float64(67.6), "disk_usage": float64(31.4)},
		{"memory_usage": float64(4.5)},
	}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.Validate(t.Name(), time.Time{}, tags, fields), nil
	}, "bundle didn't contain some metrics", "200ms", "2s")
}

func TestOTablePerf(t *testing.T) {
	ts.metricServer.ClearMetrics()

	time.Sleep(3 * testSendInterval)
	rand.Seed(102)

	ep := &endpoint{}
	ep.TypeMeta.Kind = t.Name()
	ep.ObjectMeta.Tenant = "test"
	ep.ObjectMeta.Name = "ucase1"
	epm := &endpointMetric{}

	// increase precision to capture accurate count of exported metrics
	table, err := NewOTable(ep, epm, &TableOpts{})
	AssertOk(t, err, "unable to create table")
	epm.RxPacketSize.SetRanges([]int64{10, 100, 1000, 10000})
	intSamples := []int64{9, 99, 999, 9999}

	before := time.Now()
	nIters := 10000
	for i := 0; i < nIters; i++ {
		epm.Bandwidth.Set(float64(i*2), time.Time{})
		epm.LinkUp.Set(true, time.Time{})
		epm.WorkloadName.Set(fmt.Sprintf("test-%d", i), time.Time{})
		epm.OutgoingConns.Inc()
		epm.RxPacketSize.AddSample(intSamples[i%4])
		epm.RxBandwidth.AddSample(rand.Float64())
	}
	duration := time.Now().Sub(before) / time.Millisecond
	Assert(t, duration < 100, fmt.Sprintf("took more than 100ms (%v) for 10k updates", duration))
	t.Logf("perf: 10k operations took %dms", duration)

	time.Sleep(10 * testSendInterval)

	totalFloat64s := nIters // Bandwidth
	totalBools := nIters    // LinkUp
	totalStrings := nIters  // WorkloadName
	totalVarInt64s := 1     // OutgoingConns
	totalVarInt64s += 4     // RxPacketSize (4 ranges)
	totalVarInt64s++        // RxBandwidth Count
	totalVarFloat64s := 1   // RxBandwidth Total
	tags := map[string]string{"Tenant": "test", "Kind": t.Name(), "Name": "ucase1"}
	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.ValidateCount("", tags, totalVarInt64s, totalFloat64s, totalVarFloat64s, totalBools, totalStrings), nil
	}, "bundle didn't contain some metrics", "200ms", "2s")

	table.Delete()

	ts.metricServer.ClearMetrics()
}

func TestSetupCleanup(t *testing.T) {
	ts.cancelFunc()
	ts.rpcServer.Stop()
	ts.rpcClient.Close()
}

func httpGet(t *testing.T, url string, toIf interface{}) {
	resp, err := http.Get(url)
	AssertOk(t, err, fmt.Sprintf("unable to fetch url %s", url))
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	err = json.Unmarshal(body, toIf)
	AssertOk(t, err, "unable to unmarshal http response")
}
