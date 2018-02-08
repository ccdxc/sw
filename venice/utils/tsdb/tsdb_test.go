package tsdb

import (
	"fmt"
	"reflect"
	"testing"
	"time"

	context "golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// fake transmitter mimics a transmitter residing alongside client code
var fc fakeTransmitter

type fakeTransmitter struct {
	dbName         string
	receivedPoints []Point
}

func (c *fakeTransmitter) Init(opts Options) error {
	return nil
}

func (c *fakeTransmitter) Update(opts Options) error {
	return nil
}

func (c *fakeTransmitter) SendPoints(dbName string, points []Point) error {
	c.dbName = dbName
	c.receivedPoints = points

	return nil
}

func printPoints(header string, points []Point) {
	log.Infof("%s", header)
	for _, pt := range points {
		for k, v := range pt.Tags {
			log.Infof("tag = %T=%v, %T=%v", k, k, v, v)
		}
		for k, v := range pt.Fields {
			log.Infof("field = %T=%v, %T=%v", k, k, v, v)
		}
	}
}

func (c *fakeTransmitter) validate(dbName string, points []Point) bool {
	if dbName != c.dbName {
		log.Errorf("unexpected dbName, got '%s' expected '%s'", dbName, c.dbName)
		return false
	}

	// null out the time before comparing
	for idx := range c.receivedPoints {
		c.receivedPoints[idx].Time = time.Time{}
	}
	if reflect.DeepEqual(points, c.receivedPoints) {
		return true
	}

	printPoints("Expected", points)
	printPoints("Got", c.receivedPoints)
	return false
}

// initialization test
func TestInit(t *testing.T) {
	if _, err := NewTable("table1", Config{}); err == nil {
		t.Fatalf("error creating a table without global init")
	}

	if err := Init(&fc, Options{}); err != nil {
		t.Fatalf("error initializing fake transmitter")
	}
	Close()
	if err := Init(&DummyTransmitter{}, Options{}); err != nil {
		t.Fatalf("error initializing Dummy transmitter")
	}
	Close()

}

// table creation
func TestNewTableCreate(t *testing.T) {
	Init(&fc, Options{})
	defer Close()
	tbl, err := NewTable("table1", Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}

	tbl.Delete()
}

// table creation with invalid params
func TestNewTableCreateInvalidConfig(t *testing.T) {
	Init(&fc, Options{})
	defer Close()

	if _, err := NewTable("", Config{}); err == nil {
		t.Fatalf("no error creating table with invalid config")
	}
}

func TestTableErrors(t *testing.T) {
	Init(&fc, Options{})
	defer Close()

	if tbl, err := NewTable("junk", Config{}); err != nil {
		t.Fatalf("error creating table")
	} else {
		defer tbl.Delete()
	}

	if _, err := NewTable("junk", Config{}); err == nil {
		t.Fatalf("successfully created table with same name")
	}
}

// test adding points to the table
func TestAddPoints(t *testing.T) {
	Init(&fc, Options{})
	defer Close()

	tbl, err := NewTable("table2", Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}
	defer tbl.Delete()

	pts := []Point{{Tags: map[string]string{"ifname": "veth1", "node": "naples23", "workload": "vm223"},
		Fields: map[string]interface{}{"rxBytes": 3443, "txBytes": 5009684, "rxPackets": 5, "txPackets": 7474}}}

	err = tbl.AddPoints("veth1-ifstats", pts)
	if err != nil {
		t.Fatalf("error adding points to the  table")
	}

	// verify vm was removed
	AssertEventually(t, func() (bool, interface{}) {
		return fc.validate("table2", pts), nil
	}, "add points not received on transmitter if", "1ms", "1s")
}

type epMetric struct {
	rxBytes   Counter
	rxPackets Counter
	txBytes   Increment
	txPackets Increment
	cpuUtil   Gauge
	up        Flag
	user      String
}
type endpoint struct {
	api.TypeMeta
	api.ObjectMeta
	Metric epMetric
}

func TestObjPoints(t *testing.T) {
	Init(&fc, Options{})
	defer Close()

	epStat := &endpoint{
		TypeMeta:   api.TypeMeta{Kind: "endpoint"},
		ObjectMeta: api.ObjectMeta{Tenant: "foo", Namespace: "bar", Name: "vm87-veth39"}}
	tblObj, err := NewTableObj(&epStat, Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}

	// first reading
	epStat.Metric = epMetric{rxBytes: 7743, txBytes: 6644, rxPackets: 76, txPackets: 988, cpuUtil: 34.1, up: true, user: "admin"}
	if err := tblObj.AddObjPoint(&epStat); err != nil {
		t.Fatalf("error adding points to the table: %s", err)
	}

	// validate first reading
	time.Sleep(time.Millisecond)
	pts := []Point{{Tags: map[string]string{"Tenant": "foo", "Namespace": "bar", "Name": "vm87-veth39"},
		Fields: map[string]interface{}{"rxBytes": int64(7743), "txBytes": int64(6644), "rxPackets": int64(76),
			"txPackets": int64(988), "cpuUtil": 34.1, "up": true, "user": "admin"}}}
	if !fc.validate("endpoint", pts) {
		t.Fatalf("error receiving all points: got %+v, expected %+v", fc.receivedPoints, pts)
	}

	// second reading
	epStat.Metric = epMetric{rxBytes: 80003, txBytes: 1125546, rxPackets: 1232, txPackets: 88559, cpuUtil: 23.1, up: false, user: "admin"}
	if err := tblObj.AddObjPoint(&epStat); err != nil {
		t.Fatalf("error adding points to the table: %s", err)
	}

	// validate second reading
	time.Sleep(time.Millisecond)
	tblObj.Delete()

	// TBD: validate batching
	pts = []Point{{Tags: map[string]string{"Tenant": "foo", "Namespace": "bar", "Name": "vm87-veth39"},
		Fields: map[string]interface{}{"rxBytes": int64(80003), "txBytes": int64(1125546), "rxPackets": int64(1232),
			"txPackets": int64(88559), "cpuUtil": 23.1, "up": false, "user": "admin"}}}
	if !fc.validate("endpoint", pts) {
		t.Fatalf("error receiving all points: got %+v, expected %+v", fc.receivedPoints, pts)
	}
}

func TestObjPointsErrors(t *testing.T) {
	Init(&fc, Options{})
	defer Close()

	type invalid struct {
		char byte
	}

	if _, err := NewTableObj(&endpoint{}, Config{}); err == nil {
		t.Fatalf("not errored with invalid kind in the object")
	}

	if _, err := NewTableObj(&endpoint{TypeMeta: api.TypeMeta{Kind: "endpoint"}}, Config{}); err == nil {
		t.Fatalf("not errored with invalid keys in the object")
	}

	epStat := &endpoint{
		TypeMeta:   api.TypeMeta{Kind: "endpoint"},
		ObjectMeta: api.ObjectMeta{Tenant: "foo", Namespace: "bar", Name: "vm87-veth39"}}
	tblObj, err := NewTableObj(&epStat, Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}
	if err := tblObj.AddObjPoint(&invalid{char: 'a'}); err == nil {
		t.Fatalf("not errored on invalid metrics object")
	}

	tblObj.Delete()
}

type dummyMetricServer struct {
	mb *metric.MetricBundle
}

func (d *dummyMetricServer) validate(measurementName string, points []Point) bool {
	if d.mb == nil {
		return false
	}

	if defaultDBName != d.mb.DbName {
		log.Errorf("unexpected dbName, got '%s' expected '%s'", d.mb.DbName, defaultDBName)
		return false
	}

	for idx := range d.mb.Metrics {
		if d.mb.Metrics[idx].When == nil {
			log.Errorf("received time in Metric is nil")
			return false
		}
		if d.mb.Metrics[idx].When.Seconds == 0 && d.mb.Metrics[idx].When.Nanos == 0 {
			log.Errorf("received time in Metric is zero")
			return false
		}
		if d.mb.Metrics[idx].Name == "" {
			log.Errorf("received name in Metric is nil")
			return false
		}
		if len(d.mb.Metrics[idx].Fields) == 0 {
			log.Errorf("No fields received in Metric")
			return false

		}
		receivedMetricBundle := d.mb.Metrics[idx]
		receivedMetricBundle.When = nil

		f := xformFields(&points[idx])
		mp := &metric.MetricPoint{
			Name:   measurementName,
			Tags:   points[idx].Tags,
			Fields: f,
		}

		if !reflect.DeepEqual(receivedMetricBundle, mp) {
			fmt.Printf("Expected %#v Got %#v", mp, d.mb.Metrics[idx])
			return false
		}
	}
	return true
}

func (d *dummyMetricServer) WriteMetrics(ctx context.Context, mb *metric.MetricBundle) (*api.Empty, error) {
	d.mb = mb
	return &api.Empty{}, nil
}

type testSuite struct {
	rpcServer    *rpckit.RPCServer
	rpcClient    *rpckit.RPCClient
	context      context.Context
	cancelFunc   context.CancelFunc
	metricServer *dummyMetricServer
}

func (ts *testSuite) Cleanup() {
	ts.cancelFunc()
	ts.rpcServer.Stop()
	ts.rpcClient.Close()
}

func setupServer(t *testing.T) *testSuite {

	s, err := rpckit.NewRPCServer("Collector", ":0", rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Fatalf("failed to start grpc server: %v", err)
	}
	metricServer := &dummyMetricServer{}
	metric.RegisterMetricApiServer(s.GrpcServer, metricServer)
	s.Start()
	rpcClient, err := rpckit.NewRPCClient("collClient", s.GetListenURL(), rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	nctx, cancel := context.WithCancel(context.Background())
	ts := testSuite{
		rpcServer:    s,
		rpcClient:    rpcClient,
		context:      nctx,
		cancelFunc:   cancel,
		metricServer: metricServer,
	}
	return &ts
}

func TestBatchCollector(t *testing.T) {
	ts := setupServer(t)
	defer ts.Cleanup()

	options := Options{
		ClientName:   t.Name(),
		Collector:    ts.rpcServer.GetListenURL(),
		SendInterval: 10 * time.Millisecond,
	}
	Init(NewBatchTransmitter(ts.context), options)
	defer Close()

	epStat := &endpoint{
		TypeMeta:   api.TypeMeta{Kind: "endpoint"},
		ObjectMeta: api.ObjectMeta{Tenant: "foo", Namespace: "bar", Name: "vm87-veth39"}}
	tblObj, err := NewTableObj(&epStat, Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}

	// first reading
	epStat.Metric = epMetric{rxBytes: 7743, txBytes: 6644, rxPackets: 76, txPackets: 988, cpuUtil: 34.1, up: true, user: "admin"}
	if err := tblObj.AddObjPoint(&epStat); err != nil {
		t.Fatalf("error adding points to the table: %s", err)
	}

	pts := []Point{{Tags: map[string]string{"Tenant": "foo", "Namespace": "bar", "Name": "vm87-veth39"},
		Fields: map[string]interface{}{"rxBytes": int64(7743), "txBytes": int64(6644), "rxPackets": int64(76),
			"txPackets": int64(988), "cpuUtil": 34.1, "up": true, "user": "admin"}}}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.validate("endpoint", pts), nil
	}, "add points not received on collector", "100ms", "1s")

	LogField("latencyStats",
		api.ObjectMeta{Tenant: "foo", Namespace: "bar", Name: "nicagent-fakename"},
		"networkLatency", int64(100))

	pts = []Point{{Tags: map[string]string{"Tenant": "foo", "Namespace": "bar", "Name": "nicagent-fakename"},
		Fields: map[string]interface{}{"networkLatency": int64(100)}}}

	AssertEventually(t, func() (bool, interface{}) {
		return ts.metricServer.validate("latencyStats", pts), nil
	}, "add points not received on collector", "100ms", "1s")

}
