package cluster

import (
	"context"
	"fmt"
	"math/rand"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	client "github.com/influxdata/influxdb/client/v2"

	"github.com/pensando/sw/api"
	telemetry_query "github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/telemetryclient"
)

var actionEnumMapping = map[string]string{
	"ALLOW":  "SECURITY_RULE_ACTION_ALLOW",
	"DENY":   "SECURITY_RULE_ACTION_DENY",
	"REJECT": "SECURITY_RULE_ACTION_REJECT",
}

var directionEnumMapping = map[string]string{
	"FROM_HOST":   "FLOW_DIRECTION_FROM_HOST",
	"FROM_UPLINK": "FLOW_DIRECTION_FROM_UPLINK",
}

func testQueryingMetrics(kind string) {
	// Create telemetry client
	apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	Expect(err).Should(BeNil())

	Eventually(func() bool {
		nodeQuery := &telemetry_query.MetricsQueryList{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Queries: []*telemetry_query.MetricsQuerySpec{
				&telemetry_query.MetricsQuerySpec{
					TypeMeta: api.TypeMeta{
						Kind: kind,
					},
				},
			},
		}
		ctx := ts.tu.NewLoggedInContext(context.Background())
		res, err := tc.Metrics(ctx, nodeQuery)
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

func testQueryingFwlogs() {
	// Injecting logs
	// Create telemetry client
	apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	Expect(err).Should(BeNil())

	stamp := time.Now()
	startTime := &api.Timestamp{}
	startTime.Parse(stamp.Format(time.RFC3339Nano))
	fivemin, err := time.ParseDuration("5ms")
	Expect(err).Should(BeNil())

	logs := []*telemetry_query.Fwlog{}
	for i := 0; i < 10; i++ {
		timestamp := &api.Timestamp{}
		timestamp.Parse(stamp.Format(time.RFC3339Nano))

		log := &telemetry_query.Fwlog{
			Src:       "10.1.1.1",
			Dest:      fmt.Sprintf("11.1.1.%d", i),
			SrcPort:   uint32(8000 + i),
			DestPort:  uint32(9000 + i),
			Action:    "ALLOW",
			Direction: "FROM_HOST",
			RuleID:    "1234",
			Timestamp: timestamp,
		}
		logs = append(logs, log)
		stamp = stamp.Add(fivemin)
	}
	for i := 0; i < 10; i++ {
		timestamp := &api.Timestamp{}
		timestamp.Parse(stamp.Format(time.RFC3339Nano))

		log := &telemetry_query.Fwlog{
			Src:       fmt.Sprintf("10.1.1.%d", i),
			Dest:      "11.1.1.1",
			SrcPort:   uint32(8000 + i),
			DestPort:  uint32(9000 + i),
			Action:    "ALLOW",
			Direction: "FROM_HOST",
			RuleID:    "1234",
			Timestamp: timestamp,
		}
		logs = append(logs, log)
		stamp = stamp.Add(fivemin)
	}
	err = writeFwlogs(logs)
	Expect(err).Should(BeNil())

	// Query the logs
	query := &telemetry_query.FwlogsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.FwlogsQuerySpec{
			// Get all logs that were just created
			&telemetry_query.FwlogsQuerySpec{
				StartTime: startTime,
			},
			// Get if src = 10.1.1.1 or 10.1.1.2
			&telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.1", "10.1.1.2"},
				StartTime: startTime,
			},
			// Get if src = 10.1.1.1 or 10.1.1.0 AND source-port is 8000 AND action ALL
			&telemetry_query.FwlogsQuerySpec{
				SourceIPs:   []string{"10.1.1.1", "10.1.1.0"},
				SourcePorts: []uint32{8000},
				Actions:     []string{"ACTION_ALLOW"},
				StartTime:   startTime,
			},
			// no logs match
			&telemetry_query.FwlogsQuerySpec{
				SourceIPs:   []string{"10.1.1.1", "10.1.1.0"},
				SourcePorts: []uint32{8000},
				Actions:     []string{"ACTION_DENY"},
				StartTime:   startTime,
			},
		},
	}
	ctx := ts.tu.NewLoggedInContext(context.Background())
	resp, err := tc.Fwlogs(ctx, query)
	Expect(err).Should(BeNil())

	// verify results
	Expect(resp.Tenant).To(Equal(globals.DefaultTenant))
	Expect(len(resp.Results)).To(Equal(4))

	Expect(resp.Results[0].StatementID).To(Equal(int32(0)))
	Expect(len(resp.Results[0].Logs)).To(Equal(20))
	Expect(logs).Should(Equal(resp.Results[0].Logs))

	Expect(resp.Results[1].StatementID).To(Equal(int32(1)))
	Expect(len(resp.Results[1].Logs)).To(Equal(12))

	Expect(resp.Results[2].StatementID).To(Equal(int32(2)))
	Expect(len(resp.Results[2].Logs)).To(Equal(2))

	Expect(resp.Results[3].StatementID).To(Equal(int32(3)))
	Expect(len(resp.Results[3].Logs)).To(Equal(0))
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
	It("telemetry Fwlogs query ", func() {
		testQueryingFwlogs()
	})
})

// Writes the given Fwlogs into influx
func writeFwlogs(logs []*telemetry_query.Fwlog) error {
	bp, err := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  "default",
		Precision: "s",
	})
	if err != nil {
		return err
	}

	for _, log := range logs {
		ipSrc := log.Src
		ipDest := log.Dest
		sPort := fmt.Sprintf("%v", log.SrcPort)
		dPort := fmt.Sprintf("%v", log.DestPort)
		ipProt := log.Protocol
		action := actionEnumMapping[log.Action]
		dir := directionEnumMapping[log.Direction]
		ruleID := log.RuleID
		timestamp, err := log.Timestamp.Time()
		if err != nil {
			return err
		}

		tags := map[string]string{"src": ipSrc, "dest": ipDest, "src-port": sPort, "dest-port": dPort, "protocol": ipProt, "action": action, "direction": dir, "rule-id": ruleID}
		fields := map[string]interface{}{"flowAction": 1}

		pt, err := client.NewPoint("Fwlogs", tags, fields, timestamp)
		if err != nil {
			return err
		}
		bp.AddPoint(pt)
	}

	url := fmt.Sprintf("http://%s:%s", globals.Localhost, globals.CitadelHTTPPort)
	writePoints(url, bp)
	return nil
}

// writePoints writes points to citadel using inflxdb client
func writePoints(url string, bp client.BatchPoints) {
	rand.Seed(time.Now().Unix())
	node := ts.tu.QuorumNodes[rand.Intn(len(ts.tu.QuorumNodes))]
	nodeIP := ts.tu.NameToIPMap[node]
	points := bp.Points()
	pointsStr := []string{}
	for _, p := range points {
		pointsStr = append(pointsStr, p.String())
	}
	res := ts.tu.CommandOutput(nodeIP, fmt.Sprintf(`curl -XPOST "%s/write?db=%s" --data-binary '%s'`, url, bp.Database(), strings.Join(pointsStr, "\n")))
	log.Infof("writing points returned %s", res)
}
