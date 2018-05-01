package client

import (
	"fmt"
	"math/rand"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/test/integ/tsdb/collector"
	"github.com/pensando/sw/venice/utils/ntsdb"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestTableAPI(t *testing.T) {
	dbName := t.Name() + "_db"
	measName := t.Name()

	// setup tsdb, collector
	ts := NewSuite(1, ":0", testPeriod, t.Name(), dbName)
	defer ts.TearDown()

	// ensure that we start with empty backend
	res, err := ts.cs.Query(0, dbName, fmt.Sprintf("SELECT * FROM %s", measName))
	Assert(t, err == nil, "Expected no error")
	Assert(t, len(res[0].Series) == 0, "Expected empty result")

	// push metrics
	table, err := ntsdb.NewTable(measName, &ntsdb.TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	ts1 := time.Now()
	table.Counter("rxpkts").Inc()
	table.Counter("txpkts").Add(33)

	// verify metrics in tsdb
	time.Sleep(50 * testSendInterval)
	tt := collectorinteg.NewTimeTable(measName)
	tags := map[string]string{
		"Name": t.Name(),
	}
	fields := map[string]interface{}{
		"rxpkts": int64(1),
		"txpkts": int64(33),
	}
	tt.AddRow(collectorinteg.InfluxTS(ts1, time.Millisecond), tags, fields)

	AssertEventually(t, func() (bool, interface{}) {
		return validate(ts, dbName, measName, tt)
	}, "mismatching metrics", "100ms", "2s")
}

func TestPointsPrecision(t *testing.T) {
	dbName := t.Name() + "_db"
	measName := t.Name()

	// setup tsdb, collector
	ts := NewSuite(1, ":0", testPeriod, t.Name(), dbName)
	defer ts.TearDown()

	// create tsdb table (change the precision between metrics)
	table, err := ntsdb.NewTable(t.Name(), &ntsdb.TableOpts{Precision: 3 * time.Millisecond})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()

	// push metrics
	ts1 := time.Now()
	table.Gauge("cpu_usage").Set(67.6, time.Time{})
	table.Gauge("disk_usage").Set(31.4, time.Time{})
	time.Sleep(4 * testSendInterval)
	ts2 := time.Now()
	table.Gauge("memory_usage").Set(4.5, time.Time{})
	time.Sleep(50 * testSendInterval)

	// create expected records
	tt := collectorinteg.NewTimeTable(measName)
	tags := map[string]string{
		"Name": t.Name(),
	}
	fields := map[string]interface{}{
		"cpu_usage":    float64(67.6),
		"disk_usage":   float64(31.4),
		"memory_usage": nil,
	}
	tt.AddRow(collectorinteg.InfluxTS(ts1, time.Millisecond), tags, fields)

	tags = map[string]string{
		"Name": t.Name(),
	}
	fields = map[string]interface{}{
		"cpu_usage":    nil,
		"disk_usage":   nil,
		"memory_usage": float64(4.5),
	}
	tt.AddRow(collectorinteg.InfluxTS(ts2, time.Millisecond), tags, fields)

	// validate
	AssertEventually(t, func() (bool, interface{}) {
		return validate(ts, dbName, measName, tt)
	}, "mismatching metrics", "100ms", "2s")

}

func TestRegression(t *testing.T) {
	dbName := t.Name() + "_db"
	measName := t.Name()

	// setup tsdb, collector
	ts := NewSuite(1, ":0", testPeriod, t.Name(), dbName)
	defer ts.TearDown()

	// push metrics
	ep := &endpoint{}
	ep.TypeMeta.Kind = t.Name()
	ep.ObjectMeta.Tenant = testTenant
	ep.ObjectMeta.Name = "ep1"
	epm := &endpointMetric{}

	table, err := ntsdb.NewOTable(ep, epm, &ntsdb.TableOpts{})
	AssertOk(t, err, "unable to create table")
	defer table.Delete()
	epm.RxPacketSize.SetRanges([]int64{10, 100, 1000, 10000})
	intSamples := []int64{9, 99, 999, 9999}

	nIters := 10000
	for i := 0; i < nIters; i++ {
		// minimum 2ms to allow separate points within measurement
		sleepTime := time.Duration(2+(rand.Int()%10)) * time.Millisecond
		time.Sleep(sleepTime)
		ts1 := time.Now()
		epm.Bandwidth.Set(float64(i*2), ts1)
		epm.LinkUp.Set(true, ts1)
		epm.WorkloadName.Set(fmt.Sprintf("test-%d", i), ts1)

		epm.OutgoingConns.Inc()
		epm.RxPacketSize.AddSample(intSamples[i%4])
		epm.RxBandwidth.AddSample(rand.Float64())
	}

	f := func() (bool, interface{}) {
		numSeries, err := getCount(ts, dbName, measName)
		AssertOk(t, err, "error getting count from db")
		if numSeries == nIters {
			return true, nil
		}
		fmt.Printf("Expected %d records, Got %d instead\n", nIters, numSeries)
		return false, nil
	}
	AssertEventually(t, f, "Records retrieved from the DB did not match the number of records written.", "1s", maxTimeOut)
}

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

func TestOTableAPI(t *testing.T) {
	dbName := t.Name() + "_db"
	measName := t.Name()

	// setup tsdb, collector
	ts := NewSuite(1, ":0", testPeriod, t.Name(), dbName)
	defer ts.TearDown()

	// ensure that we start with empty backend
	res, err := ts.cs.Query(0, dbName, fmt.Sprintf("SELECT * FROM %s", measName))
	Assert(t, err == nil, "Expected no error")
	Assert(t, len(res[0].Series) == 0, "Expected empty result")

	// push metrics
	ep := &endpoint{}
	ep.TypeMeta.Kind = t.Name()
	ep.ObjectMeta.Tenant = testTenant
	ep.ObjectMeta.Name = "ep1"
	table, err := ntsdb.NewOTable(ep, &ep.epm, &ntsdb.TableOpts{})
	AssertOk(t, err, "unable to create table")

	ts1 := time.Now()
	ep.epm.OutgoingConns.Add(32)
	ep.epm.IncomingConns.Add(43)
	ep.epm.Bandwidth.Set(608.2, ts1)
	ep.epm.PacketErrors.Add(12)
	ep.epm.Violations.Inc()
	ep.epm.LinkUp.Set(true, ts1)
	ep.epm.WorkloadName.Set("test-workload", ts1)
	ep.epm.RxPacketSize.AddSample(154)
	ep.epm.TxPacketSize.AddSample(4096)
	ep.epm.RxBandwidth.AddSample(23.4)
	ep.epm.TxBandwidth.AddSample(9066.32)
	table.Delete()

	// verify metrics in tsdb
	time.Sleep(50 * testSendInterval)
	tt := collectorinteg.NewTimeTable(measName)
	tags := map[string]string{
		"Tenant": testTenant,
		"Kind":   t.Name(),
		"Name":   "ep1",
	}
	fields := map[string]interface{}{
		"OutgoingConns":          int64(32),
		"IncomingConns":          int64(43),
		"Bandwidth":              float64(608.2),
		"PacketErrors":           int64(12),
		"Violations":             int64(1),
		"LinkUp":                 true,
		"WorkloadName":           "test-workload",
		"RxPacketSize_256":       int64(1),
		"TxPacketSize_16384":     int64(1),
		"RxBandwidth_totalValue": float64(23.4),
		"RxBandwidth_totalCount": int64(1),
		"TxBandwidth_totalValue": float64(9066.32),
		"TxBandwidth_totalCount": int64(1),
	}
	tt.AddRow(collectorinteg.InfluxTS(ts1, time.Millisecond), tags, fields)

	AssertEventually(t, func() (bool, interface{}) {
		return validate(ts, dbName, measName, tt)
	}, "mismatching metrics", "1s", "5s")
}
