package cluster

import (
	"context"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	metrics_query "github.com/pensando/sw/api/generated/metrics_query"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/metricsclient"
)

func testQueryingMetrics(kind string) {
	// Create metric client
	apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
	mc, err := metricsclient.NewMetricsClient(apiGwAddr)
	Expect(err).Should(BeNil())

	Eventually(func() bool {
		nodeQuery := &metrics_query.QuerySpec{
			TypeMeta: api.TypeMeta{
				Kind: kind,
			},
			ObjectSelector: metrics_query.ObjectSelector{
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
			},
		}
		ctx := ts.tu.NewLoggedInContext(context.Background())
		res, err := mc.Query(ctx, nodeQuery)
		if err != nil {
			log.Infof("Query for %s returned err: %s", kind, err)
			return false
		}
		if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
			log.Infof("Query for %s returned empty data", kind)
			return false
		}
		series := res.Results[0].Series[0]
		Expect(len(series.Columns)).ShouldNot(BeZero(), "Query response had no column entries")
		Expect(len(series.Values)).ShouldNot(BeZero(), "Query response had no value entries in its series")

		colMap := make(map[string]int)
		expectedCols := []string{"CPUUsedPercent", "MemUsedPercent", "DiskUsedPercent"}
		for i, col := range series.Columns {
			colMap[col] = i
		}

		// Check that the columns are present and don't have zero or nil values
		for _, expCol := range expectedCols {
			i, inMap := colMap[expCol]
			Expect(inMap).Should(BeTrue(), "Query Response didn't have column %s", expCol)
			Expect(series.Values[0][i]).ShouldNot(BeNil(), "Value for column %s was nil", expCol)
			Expect(series.Values[0][i]).ShouldNot(BeZero(), "Value for column %s was zero")
		}

		return true
	}, 90, 10).Should(BeTrue(), "%s should have reported stats and been queryable", kind)
}

var _ = Describe("telemetry test", func() {
	It("telemetry Node data", func() {
		testQueryingMetrics("Node")
	})
	It("telemetry SmartNIC data", func() {
		if ts.tu.NumNaplesHosts == 0 {
			Skip("No Naples node to report metrics")
		}
		testQueryingMetrics("SmartNIC")
	})
})
