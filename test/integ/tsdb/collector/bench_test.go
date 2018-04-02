package collectorinteg

import (
	"context"
	"os"
	"testing"
	"time"

	"github.com/influxdata/influxdb/models"
	"google.golang.org/grpc"

	"github.com/pensando/sw/test/integ/tsdb/aggutils"
	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/collector/statssim"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	testMeas = "meas1"
)

// This benchmark uses a flow stats simulator
var (
	ss    *statssim.StatsSim
	saSrv aggutils.Server
	suite *Suite
	mc    metric.MetricApiClient
)

func getPoints(ss *statssim.StatsSim, count int) []models.Point {
	res := make([]models.Point, 0, count)

	for ix := 0; ix < count; ix++ {
		tags, fields := ss.GetFlowStats()
		mTags := models.NewTags(tags)
		p, _ := models.NewPoint(testMeas, mTags, fields, time.Now())
		res = append(res, p)
	}

	return res
}

func bmDirectWrite(b *testing.B, total, batchSize int) {
	numPoints := total * b.N
	points := getPoints(ss, numPoints)
	wCount := numPoints / batchSize
	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < wCount; i++ {
		err := saSrv.WritePoints("db0", "rp0", models.ConsistencyLevelAny, nil, points[i*batchSize:(i+1)*batchSize])
		if err != nil {
			b.Error(err)
		}
	}
}

func Benchmark_100K_1P_Wr(b *testing.B) {
	bmDirectWrite(b, 100000, 1)
}

func Benchmark_100K_10P_Wr(b *testing.B) {
	bmDirectWrite(b, 100000, 10)
}

func Benchmark_100K_100P_Wr(b *testing.B) {
	bmDirectWrite(b, 100000, 100)
}

func Benchmark_100K_1000P_Wr(b *testing.B) {
	bmDirectWrite(b, 100000, 1000)
}

func Benchmark_100K_10000P_Wr(b *testing.B) {
	bmDirectWrite(b, 100000, 10000)
}

func Benchmark_1M_1000P_Wr(b *testing.B) {
	bmDirectWrite(b, 1000000, 1000)
}

func Benchmark_1M_10000P_Wr(b *testing.B) {
	bmDirectWrite(b, 1000000, 10000)
}

func Benchmark_4M_10000P_Wr(b *testing.B) {
	bmDirectWrite(b, 4000000, 10000)
}

func Benchmark_4M_50000P_Wr(b *testing.B) {
	bmDirectWrite(b, 4000000, 50000)
}

func getMetricPoints(ss *statssim.StatsSim, count int) []*metric.MetricPoint {
	res := make([]*metric.MetricPoint, 0, count)

	for ix := 0; ix < count; ix++ {
		tags, fields := ss.GetFlowStats()
		mp := GetMetricPoint(testMeas, tags, fields, time.Now())
		res = append(res, mp)
	}

	return res

}

func bmCollectorWrite(b *testing.B, total, batchSize int) {
	numPoints := total * b.N
	points := getMetricPoints(ss, numPoints)
	wCount := numPoints / batchSize
	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < wCount; i++ {
		bundle := &metric.MetricBundle{
			DbName:   "db0",
			Reporter: "IT",
			Metrics:  points[i*batchSize : (i+1)*batchSize],
		}
		_, err := mc.WriteMetrics(context.Background(), bundle)
		if err != nil {
			b.Error(err)
		}
	}
}

func Benchmark_10K_1000P_Coll_Wr(b *testing.B) {
	bmCollectorWrite(b, 10000, 1000)
}

func Benchmark_100K_10000P_Coll_Wr(b *testing.B) {
	bmCollectorWrite(b, 100000, 10000)
}

func Benchmark_1M_10000P_Coll_Wr(b *testing.B) {
	bmCollectorWrite(b, 1000000, 10000)
}

func bmCollectorLineWrite(b *testing.B, total, batchSize int) {
	numPoints := total * b.N
	points := statssim.GetLinePoints(ss, testMeas, numPoints)
	wCount := numPoints / batchSize
	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < wCount; i++ {
		bundle := &metric.LineBundle{
			DbName: "db0",
			Lines:  points[i*batchSize : (i+1)*batchSize],
		}
		_, err := mc.WriteLines(context.Background(), bundle)
		if err != nil {
			b.Error(err)
		}
	}
}

func Benchmark_10K_1000P_Ln_Wr(b *testing.B) {
	bmCollectorLineWrite(b, 10000, 1000)
}

func Benchmark_100K_10000P_Ln_Wr(b *testing.B) {
	bmCollectorLineWrite(b, 100000, 10000)
}

func Benchmark_1M_10000P_Ln_Wr(b *testing.B) {
	bmCollectorLineWrite(b, 1000000, 10000)
}

func TestMain(m *testing.M) {
	ss = statssim.NewStatsSim()
	ss.Init()
	// create a standalone influx server
	saSrv = aggutils.OpenDefaultServer(aggutils.NewConfig())

	// create a collector suite
	suite = NewSuite(1, ":0", 1*time.Second)

	// setup client
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(suite.CollectorURL(), opts...)
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}

	mc = metric.NewMetricApiClient(conn)

	ret := m.Run()
	saSrv.Close()
	suite.TearDown()
	os.Exit(ret)
}
