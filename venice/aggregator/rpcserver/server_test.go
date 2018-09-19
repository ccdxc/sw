package rpcserver

import (
	"context"
	"io/ioutil"
	"os"
	"testing"
	"time"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/services/meta"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/metrics_query"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/test/integ/tsdb/aggutils"
	ci "github.com/pensando/sw/test/integ/tsdb/collector"
	"github.com/pensando/sw/venice/aggregator/gatherer"
	"github.com/pensando/sw/venice/citadel/collector/statssim"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testKind      = "Node"
	numPointsFlow = 15
	numPointsIf   = 4
)

type sSuite struct {
	metaDir string
	mc      *meta.Client
	table   *ci.TimeTable
	backend aggutils.Server
	sim     *statssim.StatsSim
}

var testCases = []struct {
	qs        *metrics_query.QuerySpec
	numSeries int
	numPoints int
	tsBeg     string
	tsEnd     string
}{
	{
		&metrics_query.QuerySpec{
			Object: &metrics_query.ObjectSelector{
				Kind: testKind,
			},
			Metrics: &metrics_query.MetricSpec{
				Tags: &labels.Selector{
					Requirements: []*labels.Requirement{
						{
							Key:      "CntType",
							Operator: "equals",
							Values: []string{
								"interface",
							},
						},
					},
				},
			},
		},
		1,
		numPointsIf,
		"",
		"",
	},
	{
		&metrics_query.QuerySpec{
			Object: &metrics_query.ObjectSelector{
				Kind: testKind,
			},
			Metrics: &metrics_query.MetricSpec{
				Tags: &labels.Selector{
					Requirements: []*labels.Requirement{
						{
							Key:      "CntType",
							Operator: "equals",
							Values: []string{
								"interface",
							},
						},
						{
							Key:      "App",
							Operator: "notEquals",
							Values: []string{
								"testApp1",
							},
						},
					},
				},
			},
		},
		1,
		numPointsIf,
		"",
		"",
	},
	{
		&metrics_query.QuerySpec{
			Object: &metrics_query.ObjectSelector{
				Kind: testKind,
			},
			Metrics: &metrics_query.MetricSpec{
				Tags: &labels.Selector{
					Requirements: []*labels.Requirement{
						{
							Key:      "CntType",
							Operator: "equals",
							Values: []string{
								"interface",
							},
						},
						{
							Key:      "App",
							Operator: "notEquals",
							Values: []string{
								"testApp1",
							},
						},
					},
				},
			},
		},
		1,
		numPointsIf,
		"now() - 1h",
		"now() + 5m",
	},
	{
		&metrics_query.QuerySpec{
			Object: &metrics_query.ObjectSelector{
				Kind: testKind,
			},
			Metrics: &metrics_query.MetricSpec{
				Tags: &labels.Selector{
					Requirements: []*labels.Requirement{
						{
							Key:      "CntType",
							Operator: "notEquals",
							Values: []string{
								"alien",
							},
						},
						{
							Key:      "App",
							Operator: "notEquals",
							Values: []string{
								"alien",
							},
						},
					},
				},
			},
			Pagination: &metrics_query.PaginationSpec{
				Offset: 5,
				Count:  10,
			},
		},
		0,
		0,
		"now() - 1h",
		"now() + 5m",
	},
	{
		&metrics_query.QuerySpec{
			Object: &metrics_query.ObjectSelector{
				Kind: testKind,
			},
			Metrics: &metrics_query.MetricSpec{
				Tags: &labels.Selector{
					Requirements: []*labels.Requirement{
						{
							Key:      "CntType",
							Operator: "notEquals",
							Values: []string{
								"alien",
							},
						},
						{
							Key:      "App",
							Operator: "notEquals",
							Values: []string{
								"alien",
							},
						},
					},
				},
			},
			Pagination: &metrics_query.PaginationSpec{
				Offset: 0,
				Count:  1,
			},
		},
		1,
		(numPointsFlow + numPointsIf),
		"now() - 1h",
		"now() + 5m",
	},
}

func setupTestSuite(t *testing.T) *sSuite {
	ss := &sSuite{
		table:   ci.NewTimeTable(testKind),
		backend: aggutils.OpenServer(aggutils.NewConfig()),
		sim:     statssim.NewStatsSim(),
	}
	ss.sim.Init()
	err := ss.backend.CreateDatabaseAndRetentionPolicy(objMetricsDB, aggutils.NewRetentionPolicySpec("rp0", 1, 0), true)
	if err != nil {
		t.Fatalf("Error %v creating DB on backend", err)
	}
	metaDir, err := ioutil.TempDir("", "meta")
	if err != nil {
		ss.backend.Close()
		t.Fatal(err)
	}
	ss.metaDir = metaDir

	ss.mc = meta.NewClient(&meta.Config{Dir: ss.metaDir, RetentionAutoCreate: true})
	if err := ss.mc.Open(); err != nil {
		ss.tearDown()
		t.Fatalf("mc.Open : %s", err)
	}
	//Create a DB
	_, err = ss.mc.CreateDatabase(objMetricsDB)
	if err != nil {
		ss.tearDown()
		t.Fatal(err.Error())
	}
	gatherer.AddDatabase(objMetricsDB)

	return ss

}

func (ss *sSuite) tearDown() {
	ss.backend.Close()
	os.RemoveAll(ss.metaDir) // clean up
}

func (ss *sSuite) addPoints(count int, objTags map[string]string) error {
	res := make([]models.Point, 0, count)

	for ix := 0; ix < count; ix++ {
		tags, fields := ss.sim.GetFlowStats()
		// add objTags
		for k, v := range objTags {
			tags[k] = v
		}
		mTags := models.NewTags(tags)
		stamp := time.Now()
		p, _ := models.NewPoint(testKind, mTags, fields, stamp)
		ss.table.AddRow(stamp.UTC().String(),
			tags, fields)
		res = append(res, p)
	}

	return ss.backend.WritePoints(objMetricsDB, "rp0", models.ConsistencyLevelAny, nil, res)
}

func TestRPCServer(t *testing.T) {
	ss := setupTestSuite(t)
	defer ss.tearDown()

	gatherer.SetBackends([]string{ss.backend.URL()})
	// start an RPC server
	as, err := NewAggRPCSrv(":0", ss.mc)
	AssertOk(t, err, "Failed to start AggRPCSrv - %v", err)

	// seed backend with some points
	extraTags1 := map[string]string{"Kind": testKind,
		"App":     "testApp1",
		"CntType": "Flow",
	}

	err = ss.addPoints(numPointsFlow, extraTags1)
	gatherer.SyncMeta() // force sync of meta data
	// query
	qs1 := &metrics_query.QuerySpec{
		Object: &metrics_query.ObjectSelector{
			Kind: testKind,
		},
		Metrics: &metrics_query.MetricSpec{},
	}
	qr, err := as.Query(context.Background(), qs1)
	AssertOk(t, err, "Querying server - %v", err)
	//t.Logf("qr: %+v", qr)
	for _, series := range qr.SeriesList {
		vv := make([][]interface{}, 0, len(series.Rows))
		for _, r := range series.Rows {
			v := make([]interface{}, len(r.Values))
			for ix, val := range r.Values {
				v[ix] = val
			}
			vv = append(vv, v)
		}
		modelRow := models.Row{
			Name:    testKind,
			Columns: series.Columns,
			Values:  vv,
		}
		err := ss.table.MatchQueryRow(modelRow)
		AssertOk(t, err, "Matching query - %v", err)
	}

	// specify a time range
	start := &api.Timestamp{}
	err = start.Parse("now() - 1h")
	AssertOk(t, err, "Parsing timestamp")
	qs2 := &metrics_query.QuerySpec{
		Object: &metrics_query.ObjectSelector{
			Kind: testKind,
		},
		Metrics: &metrics_query.MetricSpec{},
		Time: &metrics_query.TimeRange{
			Begin: start,
		},
	}

	qr, err = as.Query(context.Background(), qs2)
	AssertOk(t, err, "Querying server - %v", err)
	Assert(t, len(qr.SeriesList) == 1, "Expected 1 series, got %v", len(qr.SeriesList))
	Assert(t, len(qr.SeriesList[0].Rows) == numPointsFlow, "Expected %d spoints, got %v", numPointsFlow, len(qr.SeriesList[0].Rows))

	start = &api.Timestamp{}
	err = start.Parse("now() - 1h")
	AssertOk(t, err, "Parsing timestamp")
	end := &api.Timestamp{}
	err = end.Parse("now() - 30m")
	AssertOk(t, err, "Parsing timestamp")
	qs3 := &metrics_query.QuerySpec{
		Object: &metrics_query.ObjectSelector{
			Kind: testKind,
		},
		Metrics: &metrics_query.MetricSpec{},
		Time: &metrics_query.TimeRange{
			Begin: start,
			End:   end,
		},
	}

	verifyConditionalQuery(t, as, qs3, 0, 0)

	// add more points with different tags
	extraTags2 := map[string]string{"Kind": testKind,
		"App":     "testApp2",
		"CntType": "interface",
	}

	err = ss.addPoints(numPointsIf, extraTags2)
	gatherer.SyncMeta() // force sync of meta data

	getTimeSpec := func(bts, ets string) *metrics_query.TimeRange {
		b := &api.Timestamp{}
		e := &api.Timestamp{}

		if bts != "" {
			err = b.Parse(bts)
			AssertOk(t, err, "Bad testcase parsing begin ts - %v", err)
		}
		if ets != "" {
			err = e.Parse(ets)
			AssertOk(t, err, "Bad testcase parsing begin ts - %v", err)
		}

		return &metrics_query.TimeRange{
			Begin: b,
			End:   e,
		}
	}

	for _, tc := range testCases {
		if tc.tsBeg != "" || tc.tsEnd != "" {
			tc.qs.Time = getTimeSpec(tc.tsBeg, tc.tsEnd)
		}
		verifyConditionalQuery(t, as, tc.qs, tc.numSeries, tc.numPoints)
	}

	as.Stop()
}

func verifyConditionalQuery(t *testing.T, a *AggRPCSrv, qs *metrics_query.QuerySpec, numSeries, numPoints int) {
	qr, err := a.Query(context.Background(), qs)
	AssertOk(t, err, "Querying server - %v", err)
	Assert(t, len(qr.SeriesList) == numSeries, "Expected %d series, got %v", numSeries, len(qr.SeriesList))
	if numSeries == 1 {
		Assert(t, len(qr.SeriesList[0].Rows) == numPoints, "Expected %d points, got %v", numPoints, len(qr.SeriesList[0].Rows))
	}
}
