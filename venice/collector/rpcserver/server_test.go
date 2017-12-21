package rpcserver

import (
	"fmt"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	tec "github.com/pensando/sw/venice/collector"
	"github.com/pensando/sw/venice/collector/mockdb"
	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/collector/statssim"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

const (
	colURL = "localhost:10777"
	testDB = "t-e-s-t-D-B"
)

type suite struct {
	dbA       *mockdb.MockTSDB
	dbB       *mockdb.MockTSDB
	c         *tec.Collector
	srv       *CollRPCSrv
	rpcClient *rpckit.RPCClient
	mc        metric.MetricApiClient
	sim       *statssim.StatsSim
}

func (ts *suite) Setup(t *testing.T) {
	// setup a collector with two backends
	ts.dbA = &mockdb.MockTSDB{}
	dbServerA, err := ts.dbA.Setup()
	tu.AssertOk(t, err, "failed to setup mockdb")
	ts.dbB = &mockdb.MockTSDB{}
	dbServerB, err := ts.dbB.Setup()
	tu.AssertOk(t, err, "failed to setup mockdb")

	ts.c = tec.NewCollector(context.Background()).WithPeriod(100 * time.Millisecond)
	err = ts.c.AddBackEnd("http://" + *dbServerA)
	if err != nil {
		t.Fatalf(err.Error())
	}
	err = ts.c.AddBackEnd("http://" + *dbServerB)
	if err != nil {
		t.Fatalf(err.Error())
	}

	// setup an rpc server
	srv, err := NewCollRPCSrv(colURL, ts.c)
	if err != nil {
		t.Fatalf(err.Error())
	}
	ts.srv = srv

	// setup rpc client
	rpcClient, err := rpckit.NewRPCClient("collClient", colURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	ts.rpcClient = rpcClient
	ts.mc = metric.NewMetricApiClient(rpcClient.ClientConn)
	ts.sim = statssim.NewStatsSim()
	ts.sim.Init()
}

func (ts *suite) Teardown() {
	ts.rpcClient.Close()
	ts.srv.Stop()
	ts.dbA.Teardown()
	ts.dbB.Teardown()
}

func (ts *suite) getMetricBundle(len int, name, fType string) *metric.MetricBundle {
	mb := &metric.MetricBundle{}
	for ix := 0; ix < len; ix++ {
		tags, fields := ts.sim.GetFlowStats()
		f := xformFields(fields, fType)
		tt, _ := types.TimestampProto(time.Now())
		mp := &metric.MetricPoint{
			Name:   name,
			Tags:   tags,
			Fields: f,
			When: &api.Timestamp{
				Timestamp: *tt,
			},
		}

		mb.Metrics = append(mb.Metrics, mp)
	}

	return mb
}

func xformFields(f map[string]interface{}, fType string) map[string]*metric.Field {
	res := make(map[string]*metric.Field)
	for k, vv := range f {
		v := vv.(int)
		switch fType {
		case "Counter":
			res[k] = &metric.Field{
				F: &metric.Field_Counter{
					Counter: uint64(v),
				},
			}
		case "Gauge":
			res[k] = &metric.Field{
				F: &metric.Field_Gauge{
					Gauge: float64(v),
				},
			}
		case "String":
			res[k] = &metric.Field{
				F: &metric.Field_String_{
					String_: fmt.Sprintf("%d", v),
				},
			}
		case "Bool":
			var b bool
			if v%2 == 0 {
				b = true
			}
			res[k] = &metric.Field{
				F: &metric.Field_Bool{
					Bool: b,
				},
			}
		}
	}

	return res
}

func (ts *suite) verifyBasic(t *testing.T, count, exp int, fType string) {
	bundle := ts.getMetricBundle(count, "PktCount", fType)
	bundle.DbName = testDB
	bundle.Reporter = "UT"
	ts.mc.WriteMetrics(context.Background(), bundle)
	msg := fmt.Sprintf("Expected %d write(s), got ", exp)
	tu.Assert(t, ts.dbA.Writes == uint64(exp), msg, ts.dbA.Writes)
	tu.Assert(t, ts.dbB.Writes == uint64(exp), msg, ts.dbB.Writes)
}

func TestServer(t *testing.T) {
	tb := &suite{}
	tb.Setup(t)
	defer tb.Teardown()

	// Perform simple writes
	tb.verifyBasic(t, 1, 1, "Gauge")
	tb.verifyBasic(t, 1, 2, "Counter")
	tb.verifyBasic(t, 2, 3, "String")
	tb.verifyBasic(t, 2, 4, "Bool")
	// verify points written so far
	tu.Assert(t, tb.dbB.PointsWritten == 6, "Expected 6 points, got ", tb.dbB.PointsWritten)
	tu.Assert(t, tb.dbA.PointsWritten == 6, "Expected 6 points, got ", tb.dbA.PointsWritten)
	tu.Assert(t, tb.srv.badReqs == 0, "Expected 0 badReqs, got ", tb.srv.badReqs)
	tu.Assert(t, tb.srv.badPoints == 0, "Expected 0 badPoints, got ", tb.srv.badPoints)

	// No DB specified
	bundle := tb.getMetricBundle(1, "PktCount", "Counter")
	bundle.Reporter = "UT"
	tb.mc.WriteMetrics(context.Background(), bundle)
	tu.Assert(t, tb.srv.badReqs == 1, "Expected 1 badReqs, got ", tb.srv.badReqs)
	tu.Assert(t, tb.dbB.PointsWritten == 6, "Expected 6 points, got ", tb.dbB.PointsWritten)
	tu.Assert(t, tb.dbA.PointsWritten == 6, "Expected 6 points, got ", tb.dbA.PointsWritten)

	// Bad points
	bundle = tb.getMetricBundle(3, "PktCount", "Counter")
	// empty fields of the middle point
	bundle.Metrics[1].Fields = make(map[string]*metric.Field)
	bundle.Reporter = "UT"
	bundle.DbName = testDB
	tb.mc.WriteMetrics(context.Background(), bundle)
	tu.Assert(t, tb.dbB.PointsWritten == 8, "Expected 8 points, got ", tb.dbB.PointsWritten)
	tu.Assert(t, tb.dbA.PointsWritten == 8, "Expected 8 points, got ", tb.dbA.PointsWritten)
	tu.Assert(t, tb.srv.badPoints == 1, "Expected 1 badPoints, got ", tb.srv.badPoints)

	bundle = tb.getMetricBundle(0, "PktCount", "Counter")
	bundle.Reporter = "UT"
	bundle.DbName = testDB
	tb.mc.WriteMetrics(context.Background(), bundle)
	tu.Assert(t, tb.srv.badReqs == 2, "Expected 2 badReqs, got ", tb.srv.badReqs)
}
