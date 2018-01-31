package aggregatorinteg

import (
	"testing"
	"time"

	"github.com/go-test/deep"
	ic "github.com/influxdata/influxdb/client/v2"
	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/test/integ/tsdb/aggutils"
	"github.com/pensando/sw/venice/aggregator/gatherer"
	"github.com/pensando/sw/venice/collector/statssim"
)

const testMeas = "testStats"

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

func TestAggBasic(t *testing.T) {
	// make a 5 node cluster
	bec := aggutils.NewBECluster(5)
	defer bec.TearDown()

	agg, err := StartAgg(bec.URLs(), ":0")
	if err != nil {
		t.Fatal(err)
	}
	defer agg.Close()

	gatherer.AddDatabase("db0")

	// create a standalone influx server for reference
	ref := aggutils.OpenDefaultServer(aggutils.NewConfig())

	// write some points to backends
	ss := statssim.NewStatsSim()
	ss.Init()

	points := getPoints(ss, 5000)
	err = bec.WritePoints(points)

	if err != nil {
		t.Fatal(err)
	}

	// write to ref influx
	err = ref.WritePoints("db0", "rp0", models.ConsistencyLevelAny, nil, points)
	if err != nil {
		t.Fatal(err)
	}

	gatherer.SyncMeta() // force sync of meta data
	// query aggregator
	// Create a new HTTPClient
	aggC, err := ic.NewHTTPClient(ic.HTTPConfig{
		Addr: agg.Addr(),
	})

	if err != nil {
		t.Fatal(err)
	}

	// ref client
	refC, err := ic.NewHTTPClient(ic.HTTPConfig{
		Addr: ref.URL(),
	})

	respGetter := func(c ic.Client, cmd string) []ic.Result {
		q := ic.Query{
			Command:  cmd,
			Database: "db0",
		}
		response, err := c.Query(q)
		if err != nil {
			t.Fatal(err)
		}
		if response.Error() != nil {
			t.Error(response.Error())
		}
		return response.Results

	}

	// send query to agg and reference, compare results
	verifier := func(a, r ic.Client, cmd string) bool {

		aggResp := respGetter(a, cmd)
		refResp := respGetter(r, cmd)
		if d := deep.Equal(aggResp, refResp); d != nil {
			t.Errorf("Results do not match")
			t.Error(d)
			return false
		}

		return true
	}

	for _, cmd := range []string{
		`SELECT * FROM testStats`,
		`SELECT "Rcv Bytes" FROM testStats`,
		`SELECT max("Rcv Bytes") FROM testStats`,
		`SELECT max("Rcv Bytes"), "srcIP" FROM testStats`,
		`SELECT max("Tx Bytes"), * FROM testStats`,
		`SELECT max(*) FROM testStats`,
		`SELECT max(/Rcv/) FROM testStats`,
		`SELECT min("Rcv Bytes") FROM testStats`,
		`SELECT min("Rcv Bytes"), "srcIP" FROM testStats`,
		`SELECT min("Tx Bytes"), * FROM testStats`,
		`SELECT min(*) FROM testStats`,
		`SELECT min(/Rcv/) FROM testStats`,
		`SELECT first("Rcv Bytes") FROM testStats`,
		`SELECT first("Rcv Bytes"), "srcIP" FROM testStats`,
		`SELECT first("Tx Bytes"), * FROM testStats`,
		`SELECT first(*) FROM testStats`,
		`SELECT first(/Rcv/) FROM testStats`,
		`SELECT last("Rcv Bytes") FROM testStats`,
		`SELECT last("Rcv Bytes"), "srcIP" FROM testStats`,
		`SELECT last("Tx Bytes"), * FROM testStats`,
		`SELECT last(*) FROM testStats`,
		`SELECT last(/Rcv/) FROM testStats`,
		`SELECT count("Rcv Bytes") FROM testStats`,
		`SELECT count(distinct("Rcv Bytes")) FROM testStats`,
		`SELECT count(distinct "Rcv Bytes") FROM testStats`,
		`SELECT count(*) FROM testStats`,
		`SELECT count(/Rcv/) FROM testStats`,
		`SELECT mean("Rcv Bytes") FROM testStats`,
		`SELECT mean(*) FROM testStats`,
		`SELECT mean(/Rcv/) FROM testStats`,
		`SELECT min("Rcv Bytes"), max("Rcv Bytes") FROM testStats`,
		`SELECT min(*), max(*) FROM testStats`,
		`SELECT min(/Rcv/), max(/Rcv/) FROM testStats`,
		`SELECT first("Rcv Bytes"), last("Rcv Bytes") FROM testStats`,
		`SELECT first(*), last(*) FROM testStats`,
		`SELECT first(/Rcv/), last(/Rcv/) FROM testStats`,
		`SELECT count("Rcv Bytes") FROM testStats WHERE time >= now() - 1h GROUP BY time(10s)`,
		`SELECT distinct "Rcv Bytes" FROM testStats`,
		`SELECT distinct("Rcv Bytes") FROM testStats`,
		`SELECT "Rcv Bytes" / total FROM testStats`,
		//                `SELECT min("Rcv Bytes") / total FROM testStats`,
		//                `SELECT max("Rcv Bytes") / total FROM testStats`,
		`SELECT top("Rcv Bytes", 1) FROM testStats`,
		`SELECT top("Rcv Bytes", srcIP, 1) FROM testStats`,
		`SELECT top("Rcv Bytes", 1), srcIP FROM testStats`,
		`SELECT min(top) FROM (SELECT top("Rcv Bytes", srcIP, 1) FROM testStats) GROUP BY region`,
		`SELECT bottom("Rcv Bytes", 1) FROM testStats`,
		`SELECT bottom("Rcv Bytes", srcIP, 1) FROM testStats`,
		`SELECT bottom("Rcv Bytes", 1), srcIP FROM testStats`,
		`SELECT max(bottom) FROM (SELECT bottom("Rcv Bytes", srcIP, 1) FROM testStats) GROUP BY region`,
		`SELECT percentile("Rcv Bytes", 75) FROM testStats`,
		`SELECT percentile("Rcv Bytes", 75.0) FROM testStats`,
		// Cannot verify sample because they are random by def
		//`SELECT sample("Rcv Bytes", 2) FROM testStats`,
		//`SELECT sample(*, 2) FROM testStats`,
		//`SELECT sample(/val/, 2) FROM testStats`,
		`SELECT elapsed("Rcv Bytes") FROM testStats`,
		`SELECT elapsed("Rcv Bytes", 10s) FROM testStats`,
		`SELECT integral("Rcv Bytes") FROM testStats`,
		`SELECT integral("Rcv Bytes", 10s) FROM testStats`,
		`SELECT max("Rcv Bytes") FROM testStats WHERE time >= now() - 1m GROUP BY time(10s, 5s)`,
		//		`SELECT max("Rcv Bytes") FROM testStats WHERE time >= now() - 1m GROUP BY time(10s, '2000-01-01T00:00:05Z')`,
		//`SELECT max("Rcv Bytes") FROM testStats WHERE time >= now() - 1m GROUP BY time(10s, now())`,
		`SELECT max(mean) FROM (SELECT mean("Rcv Bytes") FROM testStats GROUP BY srcIP)`,
		`SELECT max(derivative) FROM (SELECT derivative(mean("Rcv Bytes")) FROM testStats) WHERE time >= now() - 1m GROUP BY time(10s)`,
		`SELECT max("Rcv Bytes") FROM (SELECT "Rcv Bytes" + total FROM testStats) WHERE time >= now() - 1m GROUP BY time(10s)`,
		`SELECT "Rcv Bytes" FROM testStats WHERE time >= '2000-01-01T00:00:00Z' AND time <= '2000-01-01T01:00:00Z'`,
		`SELECT "Rcv Bytes" FROM (SELECT "Rcv Bytes" FROM testStats) ORDER BY time DESC`,
		`SELECT distinct "Rcv Bytes" FROM testStats`,
		`SELECT "time", "Rcv Bytes" FROM testStats`,
		`SELECT "time", * FROM testStats`,
	} {
		if verifier(aggC, refC, cmd) {
			t.Logf("###[ Verify %s : PASS]###", cmd)
		} else {
			t.Logf("###[ Verify %s : FAIL]###", cmd)
		}
	}
}
