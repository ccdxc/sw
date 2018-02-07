package collectorinteg

import (
	"context"
	"testing"
	"time"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testPeriod = 50 * time.Millisecond
)

var (
	tagsA = map[string]string{
		"location": "New York",
		"floor":    "second",
	}

	tagsB = map[string]string{
		"security": "high",
		"value":    "low",
	}

	fieldsA = map[string]interface{}{
		"cpu":    float64(80.5),
		"mem":    int64(128000),
		"active": true,
		"inuse":  "yes",
	}

	fieldsB = map[string]interface{}{
		"cpu": float64(70.5),
		"mem": int64(138000),
	}
)

func TestCollector(t *testing.T) {
	cs := NewSuite(1, ":0", testPeriod)
	defer cs.TearDown()
	cs.CreateDB("dbA")

	// setup client
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(cs.CollectorURL(), opts...)
	if err != nil {
		t.Fatalf("fail to dial: %v", err)
	}

	client := metric.NewMetricApiClient(conn)

	// write a bundle
	mb := &metric.MetricBundle{
		DbName:   "dbA",
		Reporter: "IT",
	}

	ts1 := time.Now()
	mp1 := GetMetricPoint("measA", tagsA, fieldsA, ts1)
	mp2 := GetMetricPoint("measB", tagsB, fieldsB, time.Now())
	mb.Metrics = append(mb.Metrics, mp1)
	mb.Metrics = append(mb.Metrics, mp2)

	// verify the write on backend

	res, err := cs.Query(0, "dbA", "SELECT * FROM measA")
	Assert(t, err == nil, "Expected no error")
	Assert(t, len(res[0].Series) == 0, "Expected empty result")
	client.WriteMetrics(context.Background(), mb)
	res, err = cs.Query(0, "dbA", "SELECT * FROM measA")
	Assert(t, err == nil, "Expected no error")
	Assert(t, len(res[0].Series) == 1, "Expected 1 series")
	tt := NewTimeTable("measA")
	tt.AddRow(InfluxTS(ts1, time.Millisecond), tagsA, fieldsA)
	err = tt.MatchQueryRow(res[0].Series[0])
	if err != nil {
		t.Fatal(err)
	}

}
