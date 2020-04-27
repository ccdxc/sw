package tsdb

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"math/rand"
	"net/http"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	mr "github.com/pensando/sw/venice/utils/resolver/mock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb/mock"
)

const (
	testSendInterval  = 10 * time.Millisecond
	objID             = "obj-id"
	maxTestCount      = 3
	maxRetriesPerTest = 10
)

type testSuite struct {
	rpcServer    *rpckit.RPCServer
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
	CPUUsage      api.Gauge
	Bandwidth     api.PrecisionGauge
	PacketErrors  api.Counter
	Violations    api.Counter
	LinkUp        api.Bool
	WorkloadName  api.String
	RxPacketSize  api.Histogram
	TxPacketSize  api.Histogram
	RxBandwidth   api.Summary
	TxBandwidth   api.Summary
}

func Setup(startLocalServer bool) error {
	s, err := rpckit.NewRPCServer(globals.Collector, ":0", rpckit.WithLoggerEnabled(false))
	if err != nil {
		return fmt.Errorf("failed to start grpc server: %v", err)
	}
	metricServer := &mock.Collector{}
	metric.RegisterMetricApiServer(s.GrpcServer, metricServer)
	s.Start()
	nctx, cancel := context.WithCancel(context.Background())
	ts = &testSuite{
		rpcServer:    s,
		context:      nctx,
		cancelFunc:   cancel,
		metricServer: metricServer,
	}

	v := strings.Split(s.GetListenURL(), ":")
	mc := mr.New()
	mc.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Collector,
		},
		Service: globals.Collector,
		URL:     "localhost:" + v[len(v)-1],
	})

	options := &Opts{
		ClientName:       "tsdb-test",
		ResolverClient:   mc,
		Collector:        globals.Collector,
		SendInterval:     testSendInterval,
		DBName:           "objMetrics",
		LocalPort:        0,
		StartLocalServer: startLocalServer,
	}

	// override the collection interval for tests to be smaller
	minCollectionInterval = testSendInterval / 2
	Init(ts.context, options)
	return nil
}

func TestFwlogPointLimits(t *testing.T) {
	// stop collector
	ts.rpcServer.Stop()

	keyTags := map[string]string{objID: t.Name()}
	obj, err := NewObj(t.Name(), keyTags, nil, nil)
	AssertOk(t, err, "unable to create obj")
	defer obj.Delete()

	err = obj.Points([]*Point{
		{
			Tags:   map[string]string{"src": "10.1.1.1", "dest": "11.1.1.1", "port": "8080"},
			Fields: map[string]interface{}{"action": "rejected"},
		},
	}, time.Now())
	AssertOk(t, err, "unable to create point")

	TearDown()
	Cleanup()
	Setup(true)

	err = obj.Points([]*Point{
		{
			Tags:   map[string]string{"src": "10.1.1.1", "dest": "11.1.1.1", "port": "8080"},
			Fields: map[string]interface{}{"action": "rejected"},
		},
	}, time.Now())
	AssertOk(t, err, "unable to create point")

}

func TestVeniceObj(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		ep := &endpoint{}
		ep.TypeMeta.Kind = tName
		ep.ObjectMeta.Tenant = "test"
		ep.ObjectMeta.Name = "ucase1"

		// Use case 1 - collect various kinds of metrics
		obj, err := NewVeniceObj(ep, &ep.epm, nil)
		AssertOk(t, err, "unable to create obj")

		timeStamp := time.Now()
		ep.epm.OutgoingConns.Add(32)
		ep.epm.IncomingConns.Add(43)
		ep.epm.CPUUsage.Set(80.9)
		ep.epm.Bandwidth.Set(608.2, timeStamp)
		ep.epm.PacketErrors.Add(12)
		ep.epm.Violations.Inc()
		ep.epm.LinkUp.Set(true, timeStamp)
		ep.epm.WorkloadName.Set("test-workload", timeStamp)
		ep.epm.RxPacketSize.AddSample(154)
		ep.epm.TxPacketSize.AddSample(4096)
		ep.epm.RxBandwidth.AddSample(23.4)
		ep.epm.TxBandwidth.AddSample(9066.32)

		obj.Delete()

		// Use case 2 - another obj with same kind/tenant
		ep.ObjectMeta.Name = "ucase2"
		epm := &endpointMetric{}
		obj, err = NewVeniceObj(ep, epm, nil)
		AssertOk(t, err, "unable to create obj")

		epm.OutgoingConns.Add(7)
		epm.Violations.Inc()

		obj.Delete()

		// Use case 3 - use freeform apis on VeniceObj
		ep.ObjectMeta.Name = "ucase3"
		epm = &endpointMetric{}
		obj, err = NewVeniceObj(ep, epm, nil)
		AssertOk(t, err, "unable to create obj")

		epm.OutgoingConns.Add(3)
		epm.OutgoingConns.Inc()
		epm.OutgoingConns.Dec()
		epm.OutgoingConns.Inc()
		epm.OutgoingConns.Add(29)
		epm.OutgoingConns.Sub(20)
		epm.CPUUsage.Set(33.5)
		obj.Counter("OutgoingConns").Inc()
		epm.IncomingConns.Set(77)

		time.Sleep(3 * testSendInterval)
		obj.Delete()

		// verify three points
		tags := []map[string]string{
			{"Tenant": "test", "Namespace": "", "Kind": tName, "Name": "ucase1"},
			{"Tenant": "test", "Namespace": "", "Kind": tName, "Name": "ucase2"},
			{"Tenant": "test", "Namespace": "", "Kind": tName, "Name": "ucase3"},
		}
		fields := []map[string]interface{}{
			{
				"OutgoingConns": int64(32), "IncomingConns": int64(43),
				"CPUUsage": float64(80.9), "Bandwidth": float64(608.2),
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
				"CPUUsage": float64(33.5), "OutgoingConns": int64(14), "IncomingConns": int64(77),
			},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestObj(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, nil)
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		timeStamp := time.Now()
		obj.Counter("rxpkts").Inc()
		obj.Counter("txpkts").Add(1)
		obj.Counter("txbytes").Add(12)
		obj.Counter("txbytes").Inc()
		obj.PrecisionGauge("bandwidth").Set(76.1, timeStamp)
		obj.String("status").Set("upgrading", timeStamp)
		obj.Bool("powerOn").Set(true, timeStamp)
		obj.Histogram("latency").AddSample(15)
		obj.Histogram("latency").AddSample(3022)
		obj.Histogram("latency").AddSample(12)
		obj.Histogram("latency").AddSample(0)
		obj.Summary("rtt").AddSample(32)
		obj.Summary("rtt").AddSample(13)
		obj.Summary("rtt").AddSample(120)
		obj.Summary("rtt").AddSample(18)

		tags := []map[string]string{
			keyTags,
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

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestObjWithMetrics(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		epm := endpointMetric{}
		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, &epm, nil)
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		timeStamp := time.Now()
		epm.OutgoingConns.Add(32)
		epm.IncomingConns.Add(43)
		epm.Bandwidth.Set(608.2, timeStamp)
		epm.PacketErrors.Add(12)
		epm.Violations.Inc()
		epm.LinkUp.Set(true, timeStamp)
		epm.WorkloadName.Set("test-workload", timeStamp)
		epm.RxPacketSize.AddSample(154)
		epm.TxPacketSize.AddSample(4096)
		epm.RxBandwidth.AddSample(23.4)
		epm.TxBandwidth.AddSample(9066.32)

		tags := []map[string]string{
			keyTags,
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
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestAtomicAdds(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		ep := &endpoint{}
		ep.TypeMeta.Kind = tName
		ep.ObjectMeta.Tenant = "test"
		ep.ObjectMeta.Name = "tcase1"

		obj, err := NewVeniceObj(ep, &ep.epm, nil)
		AssertOk(t, err, "unable to create obj")

		obj.AtomicBegin(time.Now())
		ep.epm.OutgoingConns.Add(32)
		ep.epm.IncomingConns.Add(43)
		ep.epm.Bandwidth.Set(607.2, time.Time{})
		ep.epm.PacketErrors.Add(112)
		ep.epm.Violations.Inc()
		ep.epm.LinkUp.Set(true, time.Time{})
		ep.epm.WorkloadName.Set("test-workload", time.Time{})
		ep.epm.RxPacketSize.AddSample(154)
		ep.epm.TxPacketSize.AddSample(4096)
		ep.epm.RxBandwidth.AddSample(83.4)
		ep.epm.TxBandwidth.AddSample(9066.13)
		obj.AtomicEnd()

		tags := []map[string]string{
			{"Tenant": "test", "Namespace": "", "Kind": tName, "Name": "tcase1"},
		}
		fields := []map[string]interface{}{
			{
				"OutgoingConns": int64(32), "IncomingConns": int64(43),
				"Bandwidth":    float64(607.2),
				"PacketErrors": int64(112), "Violations": int64(1),
				"LinkUp": true, "WorkloadName": "test-workload",
				"RxPacketSize_256": int64(1), "TxPacketSize_16384": int64(1),
				"RxBandwidth_totalValue": float64(83.4), "RxBandwidth_totalCount": int64(1),
				"TxBandwidth_totalValue": float64(9066.13), "TxBandwidth_totalCount": int64(1),
			},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestMetricsWithPoints(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, nil)
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		obj.Counter("rxpkts").Inc()
		obj.PrecisionGauge("bandwidth").Set(76.1, time.Time{})
		obj.Points([]*Point{
			{
				Tags:   map[string]string{"src": "10.1.1.1", "dest": "11.1.1.1", "port": "8080"},
				Fields: map[string]interface{}{"action": "rejected"},
			},
		}, time.Time{})

		obj.Points([]*Point{
			{
				Tags:   map[string]string{"src": "10.1.1.1", "dest": "12.1.1.1", "port": "80"},
				Fields: map[string]interface{}{"action": "permitted"},
			},
		}, time.Time{})

		tags := []map[string]string{
			keyTags,
			{"src": "10.1.1.1", "dest": "11.1.1.1", "port": "8080"},
			{"src": "10.1.1.1", "dest": "12.1.1.1", "port": "80"},
		}
		fields := []map[string]interface{}{
			{"rxpkts": int64(1), "bandwidth": float64(76.1)},
			{"action": "rejected"},
			{"action": "permitted"},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestMaxPoints(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tName := t.Name()

	keyTags := map[string]string{objID: tName}
	obj, err := NewObj(tName, keyTags, nil, nil)
	AssertOk(t, err, "unable to create obj")
	defer obj.Delete()

	for i := 0; i < maxMetricsPoints+5; i++ {
		err := obj.Points([]*Point{
			{
				Tags:   map[string]string{"src": "10.1.1.1", "dest": "11.1.1.1", "port": "8080"},
				Fields: map[string]interface{}{"action": "rejected"},
			},
		}, time.Now())
		AssertOk(t, err, "unable to create point")
		Assert(t, len(obj.(*iObj).metricPoints) <= maxMetricsPoints, "exceeded number of points %d", len(obj.(*iObj).metricPoints))
	}

	err = obj.Points([]*Point{
		{
			Tags:   map[string]string{"src": "20.1.1.1", "dest": "11.1.1.1", "port": "8080"},
			Fields: map[string]interface{}{"action": "rejected"},
		},
	}, time.Now())
	AssertOk(t, err, "unable to create point")

	Assert(t, len(obj.(*iObj).metricPoints) <= maxMetricsPoints, "exceeded number of points %d", len(obj.(*iObj).metricPoints))
}

func TestAttributeChangeWithAggregation(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, nil)
		AssertOk(t, err, "unable to create obj")

		timeStamp := time.Now()
		obj.String("status").Set("upgrading", timeStamp)
		obj.Bool("powerOn").Set(true, timeStamp)

		timeStamp = time.Now()
		obj.String("status").Set("upgraded", timeStamp)
		obj.Bool("powerOn").Set(false, timeStamp)

		obj.Delete()
		time.Sleep(3 * testSendInterval)

		tags := []map[string]string{
			keyTags,
			keyTags,
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

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestHistogramCustomRangeVeniceObj(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		ep := &endpoint{}
		ep.TypeMeta.Kind = tName
		ep.ObjectMeta.Tenant = "test"
		ep.ObjectMeta.Name = "tcase1"

		epm := &endpointMetric{}

		obj, err := NewVeniceObj(ep, epm, nil)
		AssertOk(t, err, "unable to create obj")

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
			{"Tenant": "test", "Namespace": "", "Kind": tName, "Name": "tcase1"},
		}
		fields := []map[string]interface{}{
			{
				"RxPacketSize_1000":  int64(2),
				"RxPacketSize_10":    int64(1),
				"TxPacketSize_10000": int64(3),
			},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
		obj.Delete()
	}

}

func TestHistogramCustomRange(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, &ObjOpts{})
		AssertOk(t, err, "unable to create obj")

		ranges := []int64{10, 100, 1000, 10000}
		obj.Histogram("latency").SetRanges(ranges)

		obj.Histogram("latency").AddSample(15)
		obj.Histogram("latency").AddSample(3022)
		obj.Histogram("latency").AddSample(12)
		obj.Histogram("latency").AddSample(0)

		obj.Delete()
		time.Sleep(3 * testSendInterval)

		tags := []map[string]string{
			keyTags,
		}
		fields := []map[string]interface{}{
			{
				"latency_10":    int64(1),
				"latency_100":   int64(2),
				"latency_10000": int64(1),
			},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestMultipleSets(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, &ObjOpts{})
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		obj.PrecisionGauge("bandwidth").Set(8.1, time.Time{})
		obj.Counter("rxpkts").Inc()
		obj.PrecisionGauge("bandwidth").Set(0.56, time.Time{})
		obj.Counter("rxpkts").Inc()
		obj.PrecisionGauge("bandwidth").Set(2.8, time.Time{})
		obj.Counter("rxpkts").Inc()

		tags := []map[string]string{
			keyTags,
			keyTags,
			keyTags,
		}
		fields := []map[string]interface{}{
			{"bandwidth": float64(8.1), "rxpkts": int64(1)},
			{"bandwidth": float64(0.56), "rxpkts": int64(2)},
			{"bandwidth": float64(2.8), "rxpkts": int64(3)},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestPushObj(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, &ObjOpts{})
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		obj.Gauge("bandwidth").Set(9.11)
		obj.Counter("rxpkts").Inc()
		obj.Push()

		tags := []map[string]string{
			keyTags,
		}
		fields := []map[string]interface{}{
			{"bandwidth": float64(9.11), "rxpkts": int64(1)},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestMultipleSendIntervals(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, &ObjOpts{})
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		obj.PrecisionGauge("bandwidth").Set(88.1, time.Time{})
		obj.Counter("rxpkts").Inc()
		obj.Counter("rxpkts").Inc()
		obj.Counter("rxpkts").Inc()
		time.Sleep(5 * testSendInterval)
		obj.PrecisionGauge("bandwidth").Set(8.56, time.Time{})
		obj.Counter("rxpkts").Inc()
		obj.Counter("rxpkts").Inc()

		tags := []map[string]string{
			keyTags,
			keyTags,
		}
		fields := []map[string]interface{}{
			{"bandwidth": float64(88.1), "rxpkts": int64(3)},
			{"bandwidth": float64(8.56), "rxpkts": int64(5)},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestUserSuppliedTime(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, &ObjOpts{})
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		timeStamp := time.Unix(12345, 67890)
		obj.PrecisionGauge("bandwidth").Set(94.3, timeStamp)
		obj.Counter("rxpkts").Inc()
		obj.Counter("rxpkts").Inc()
		obj.Counter("rxpkts").Inc()

		tags := []map[string]string{
			keyTags,
		}
		fields := []map[string]interface{}{
			{"bandwidth": float64(94.3), "rxpkts": int64(3)},
		}

		if validateMetrics(t, tName, timeStamp, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestLocalObj(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tName := t.Name()

	keyTags := map[string]string{objID: tName}
	obj, err := NewObj(tName, keyTags, nil, &ObjOpts{Local: true})
	AssertOk(t, err, "unable to create obj")
	defer obj.Delete()

	timeStamp := time.Now()
	obj.Counter("rx_ep_create_msg").Inc()
	obj.Counter("rx_ep_create_msg").Inc()
	obj.Counter("rx_ep_create_msg").Inc()
	obj.Counter("peer_disconnects").Inc()
	obj.Counter("peer_rpc_failure").Inc()
	obj.PrecisionGauge("cpu_in_use").Set(34.4, timeStamp)
	obj.PrecisionGauge("mem_in_use").Set(102, timeStamp)
	obj.String("version").Set("v0.1", timeStamp)

	lms := []LocalMetric{}
	httpGet(t, GetLocalAddress(), &lms)
	Assert(t, len(lms) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["rx_ep_create_msg"] == "3", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["peer_disconnects"] == "1", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["peer_rpc_failure"] == "1", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["cpu_in_use"] == "34.4", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["mem_in_use"] == "102", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["version"] == "v0.1", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestLocalObjMultipleRecords(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tName := t.Name()

	keyTags := map[string]string{objID: tName}
	obj, err := NewObj(tName, keyTags, nil, &ObjOpts{Local: true})
	AssertOk(t, err, "unable to create obj")
	defer obj.Delete()

	obj.Counter("counter1").Inc()
	obj.Counter("counter1").Inc()
	obj.PrecisionGauge("cpu_in_use").Set(34.4, time.Time{})
	obj.PrecisionGauge("cpu_in_use").Set(44.3, time.Time{})

	lms := []LocalMetric{}

	httpGet(t, fmt.Sprintf("%s/%s", GetLocalAddress(), tName), &lms)
	Assert(t, len(lms) == 2, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["counter1"] == "2", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["cpu_in_use"] == "34.4", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[1].Attributes["cpu_in_use"] == "44.3", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestLocalOneObj(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tName := t.Name()

	keyTags := map[string]string{objID: tName}
	obj, err := NewObj(tName, keyTags, nil, &ObjOpts{Local: true})
	AssertOk(t, err, "unable to create obj")
	defer obj.Delete()

	obj.Counter("counter1").Inc()
	obj.Counter("counter1").Inc()
	obj.PrecisionGauge("cpu_in_use").Set(67.6, time.Time{})

	objName2 := tName + "_2"
	keyTags2 := map[string]string{objID: objName2}
	obj2, err := NewObj(objName2, keyTags2, nil, &ObjOpts{Local: true})
	AssertOk(t, err, "unable to create obj")
	defer obj2.Delete()

	obj2.Counter("counter4").Inc()
	obj2.Counter("counter5").Inc()
	obj2.PrecisionGauge("guage2").Set(23, time.Time{})

	lms := []LocalMetric{}
	httpGet(t, fmt.Sprintf("%s/%s", GetLocalAddress(), tName), &lms)
	Assert(t, len(lms) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, len(lms[0].Attributes) == 2, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["counter1"] == "2", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["cpu_in_use"] == "67.6", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestLocalObjAttribute(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tName := t.Name()

	keyTags := map[string]string{objID: tName}
	obj, err := NewObj(tName, keyTags, nil, &ObjOpts{Local: true})
	AssertOk(t, err, "unable to create obj")
	defer obj.Delete()

	obj.Counter("counter1").Inc()
	obj.Counter("counter1").Inc()
	obj.PrecisionGauge("cpu_in_use").Set(34.4, time.Time{})

	lms := []LocalMetric{}
	httpGet(t, fmt.Sprintf("%s/%s/counter1", GetLocalAddress(), tName), &lms)
	Assert(t, len(lms) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, len(lms[0].Attributes) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["counter1"] == "2", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestReadNonLocalObj(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tName := t.Name()

	keyTags := map[string]string{objID: tName}
	obj, err := NewObj(tName, keyTags, nil, nil)
	AssertOk(t, err, "unable to create obj")
	defer obj.Delete()

	timeStamp := time.Now()
	obj.Counter("rxpkts").Set(33)
	obj.Counter("txbytes").Add(1500)
	obj.String("linkstatus").Set("up", timeStamp)
	obj.String("linkstatus").Set("down", timeStamp)
	obj.String("linkstatus").Set("up", timeStamp)
	obj.Counter("txbytes").Set(2200)

	lms := []LocalMetric{}
	httpGet(t, fmt.Sprintf("%s/%s/txbytes", GetLocalAddress(), tName), &lms)
	Assert(t, len(lms) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, len(lms[0].Attributes) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["txbytes"] == "2200", fmt.Sprintf("invalid lms attributes %+v", lms))

	lms = []LocalMetric{}
	httpGet(t, fmt.Sprintf("%s/%s", GetLocalAddress(), tName), &lms)
	Assert(t, len(lms) == 1, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, len(lms[0].Attributes) == 3, fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["rxpkts"] == "33", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["linkstatus"] == "up", fmt.Sprintf("invalid lms attributes %+v", lms))
	Assert(t, lms[0].Attributes["txbytes"] == "2200", fmt.Sprintf("invalid lms attributes %+v", lms))
}

func TestObjRecreate(t *testing.T) {
	tName := t.Name()
	keyTags := map[string]string{}
	t1, err := NewObj(tName, keyTags, nil, &ObjOpts{})
	AssertOk(t, err, "unable to create obj")
	defer t1.Delete()

	// Passing in the default key should give us the same obj
	keyTags2 := map[string]string{"name": tName}
	t2, err2 := NewObj(tName, keyTags2, nil, &ObjOpts{})
	AssertOk(t, err2, "unable to re-obtain obj with same keys")
	Assert(t, t1 == t2, "did not get the same object back")
}

func TestInitOptsReset(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, &ObjOpts{})
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		for i := 0; i < 3; i++ {
			obj.PrecisionGauge("RxTstamp").Set(float64(i*2), time.Time{})
			time.Sleep(testSendInterval)
		}

		if validateSendInterval(t, testSendInterval) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("%s: unable to verify send timer", t.Name())
	}

	// set the send interval
	Init(ts.context, &Opts{SendInterval: time.Second})

	tid = 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("_bigger_timeout-%d", tid)
		keyTags := map[string]string{objID + "_bigger_timeout": tName}
		obj, err := NewObj(tName, keyTags, nil, &ObjOpts{})
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		for i := 0; i < 3; i++ {
			obj.PrecisionGauge("RxTstamp").Set(float64(i*2), time.Time{})
			time.Sleep(time.Second)
		}

		if validateSendInterval(t, time.Second) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("%s: unable to verify big timer", t.Name())
	}

	// reset send interval back
	Init(ts.context, &Opts{SendInterval: testSendInterval})
	time.Sleep(time.Second)
}

func TestOptionObjPrecision(t *testing.T) {
	tid := 0
	for ; tid < maxTestCount; tid++ {
		ts.metricServer.ClearMetrics()
		tName := t.Name() + fmt.Sprintf("-%d", tid)

		keyTags := map[string]string{objID: tName}
		obj, err := NewObj(tName, keyTags, nil, &ObjOpts{})
		AssertOk(t, err, "unable to create obj")
		defer obj.Delete()

		timeStamp := time.Now()
		obj.PrecisionGauge("cpu_usage").Set(67.6, timeStamp)
		obj.PrecisionGauge("disk_usage").Set(31.4, timeStamp)
		time.Sleep(2 * time.Millisecond)
		obj.PrecisionGauge("memory_usage").Set(4.5, time.Time{})
		time.Sleep(3 * testSendInterval)

		tags := []map[string]string{
			keyTags,
			keyTags,
		}
		fields := []map[string]interface{}{
			{"cpu_usage": float64(67.6), "disk_usage": float64(31.4)},
			{"memory_usage": float64(4.5)},
		}

		if validateMetrics(t, tName, time.Time{}, tags, fields) {
			break
		}
	}
	if tid == maxTestCount {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}
}

func TestCollectionInterval(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tName := t.Name()

	keyTags := map[string]string{objID: tName}
	obj, err := NewObj(tName, keyTags, nil, &ObjOpts{CollectionInterval: minCollectionInterval})
	AssertOk(t, err, "unable to create obj")

	obj.Counter("rxpkts").Set(208)

	// wait for collections to kick in
	time.Sleep(3 * testSendInterval)

	// expect at least 2 points because collection interval is
	// half of the send interval
	tags := []map[string]string{
		keyTags,
		keyTags,
	}
	fields := []map[string]interface{}{
		{"rxpkts": int64(208)},
		{"rxpkts": int64(208)},
	}

	if !ts.metricServer.Validate(tName, time.Time{}, tags, fields, true) {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}

	// delete object otherwise we collect them for next test loop
	obj.Delete()
}

func TestCollectionIntervalBeyondSendInterval(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tName := t.Name()

	keyTags := map[string]string{objID: tName}
	obj, err := NewObj(tName, keyTags, nil, &ObjOpts{CollectionInterval: minCollectionInterval})
	AssertOk(t, err, "unable to create obj")

	obj.Counter("txpkts").Set(208)

	// wait enough for collections to kick in
	time.Sleep(3 * testSendInterval)

	// clear all collected metrics (esp occuring due to first time)
	ts.metricServer.ClearMetrics()

	// sleep a few more send intervals
	time.Sleep(3 * testSendInterval)

	// expect at least 2 points because collection interval is
	// half of the send interval
	tags := []map[string]string{
		keyTags,
		keyTags,
	}
	fields := []map[string]interface{}{
		{"txpkts": int64(208)},
		{"txpkts": int64(208)},
	}

	if !ts.metricServer.Validate(tName, time.Time{}, tags, fields, true) {
		t.Fatalf("metrics mismatch: name '%s'", t.Name())
	}

	// delete object otherwise we collect them for next test loop
	obj.Delete()
}

func TestVeniceObjPerf(t *testing.T) {
	ts.metricServer.ClearMetrics()
	tmpMaxPoints := maxMetricsPoints
	maxMetricsPoints = 50000
	defer func() {
		maxMetricsPoints = tmpMaxPoints
	}()

	time.Sleep(3 * testSendInterval)
	rand.Seed(102)

	ep := &endpoint{}
	tName := t.Name()
	ep.TypeMeta.Kind = tName
	ep.ObjectMeta.Tenant = "test"
	ep.ObjectMeta.Name = "ucase1"
	epm := &endpointMetric{}

	// increase precision to capture accurate count of exported metrics
	obj, err := NewVeniceObj(ep, epm, &ObjOpts{})
	AssertOk(t, err, "unable to create obj")
	epm.RxPacketSize.SetRanges([]int64{10, 100, 1000, 10000})
	epm.TxPacketSize.SetRanges([]int64{10, 100, 1000, 10000, 100000})
	intSamples := []int64{9, 99, 999, 9999}

	before := time.Now()
	nIters := 10000
	for i := 0; i < nIters; i++ {
		timeStamp := time.Now()
		epm.OutgoingConns.Inc()
		epm.LinkUp.Set(true, timeStamp)
		epm.WorkloadName.Set(fmt.Sprintf("test-%d", i), timeStamp)
		epm.RxPacketSize.AddSample(intSamples[i%4])
		epm.CPUUsage.Set(float64(i * 2))
		epm.Bandwidth.Set(float64(i*2), timeStamp)
		epm.RxBandwidth.AddSample(rand.Float64())
	}
	duration := time.Now().Sub(before) / time.Millisecond
	Assert(t, duration < 1000, fmt.Sprintf("took more than 500ms (%v) for 10k updates", duration))
	t.Logf("perf: 10k operations took %dms", duration)

	time.Sleep(10 * testSendInterval)
	numMetrics := nIters
	intsPerRec := 1    // OutgoingConns - counter
	intsPerRec++       // IncomingConns - counter
	floatsPerRec := 1  // CPUUsage - gauge
	floatsPerRec++     // Bandwidth - precision gauge
	intsPerRec++       // PacketErrors - counter
	intsPerRec++       // Violations - counter
	boolsPerRec := 1   // LinkUp - bool
	stringsPerRec := 1 // WorkloadName - string
	intsPerRec += 5    // RxPacketSize - histogram (4 ranges)
	intsPerRec += 6    // TxPacketSize - histogram (4 ranges)
	intsPerRec++       // RxBandwidth - summary (count)
	floatsPerRec++     // RxBandwidth - summary (total)
	intsPerRec++       // TxBandwidth - summary (count)
	floatsPerRec++     // TxBandwidth - summary (total)

	tags := map[string]string{"Tenant": "test", "Kind": tName, "Name": "ucase1"}
	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.ValidateCount("", tags, numMetrics, intsPerRec, floatsPerRec, boolsPerRec, stringsPerRec), nil
	}, "bundle didn't contain some metrics", "200ms", "2s")

	obj.Delete()

	ts.metricServer.ClearMetrics()
}

func TearDown() {
	ts.cancelFunc()
	ts.rpcServer.Stop()
}

func httpGet(t *testing.T, url string, toIf interface{}) {
	resp, err := http.Get(url)
	AssertOk(t, err, fmt.Sprintf("unable to fetch url %s", url))
	defer resp.Body.Close()
	body, _ := ioutil.ReadAll(resp.Body)
	err = json.Unmarshal(body, toIf)
	AssertOk(t, err, "unable to unmarshal http response")
}

func validateMetrics(t *testing.T, tName string, timeStamp time.Time, tags []map[string]string, fields []map[string]interface{}) bool {
	retryID := 0
	for ; retryID < maxRetriesPerTest; retryID++ {
		if ts.metricServer.Validate(tName, timeStamp, tags, fields, false) {
			break
		}
		time.Sleep(testSendInterval)
	}
	return retryID != maxRetriesPerTest
}

func validateSendInterval(t *testing.T, sendInterval time.Duration) bool {
	retryID := 0
	for ; retryID < maxRetriesPerTest; retryID++ {
		if ts.metricServer.ValidateSendInterval(sendInterval) {
			break
		}
		time.Sleep(sendInterval)
	}
	return retryID != maxRetriesPerTest
}

func TestMain(m *testing.M) {
	if err := Setup(false); err != nil {
		fmt.Println(err.Error())
		os.Exit(1)
	}
	ret := m.Run()
	TearDown()
	os.Exit(ret)
}
