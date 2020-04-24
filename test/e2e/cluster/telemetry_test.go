package cluster

import (
	"context"
	"fmt"
	"math/rand"
	"strings"
	"time"

	"github.com/influxdata/influxdb/client/v2"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/nic/agent/tmagent/state/fwgen/fwevent"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/telemetryclient"
)

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
				{
					TypeMeta: api.TypeMeta{
						Kind: kind,
					},
				},
			},
		}
		ctx := ts.tu.MustGetLoggedInContext(context.Background())
		res, err := tc.Metrics(ctx, nodeQuery)
		if err != nil {
			By(fmt.Sprintf("Query for %s returned err: %s", kind, err))
			return false
		}
		if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
			By(fmt.Sprintf("Query for %s returned empty data", kind))
			return false
		}
		series := res.Results[0].Series[0]
		Expect(len(series.Columns)).ShouldNot(BeZero(), "Query response had no column entries")
		Expect(len(series.Values)).ShouldNot(BeZero(), "Query response had no value entries in its series")

		colMap := make(map[string]int)
		expectedCols := []string{"CPUUsedPercent", "MemUsedPercent"}
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

	Eventually(func() bool {
		_, err := tc.Fwlogs(context.Background(), &telemetry_query.FwlogsQueryList{})
		if err != nil {
			By(fmt.Sprintf("Fwlog query returned err %v", err))
			return false
		}
		return true
	}, "1s", "180s")

	logs := []*telemetry_query.Fwlog{}
	for i := 0; i < 10; i++ {
		timestamp := &api.Timestamp{}
		timestamp.Parse(stamp.Format(time.RFC3339Nano))

		log := &telemetry_query.Fwlog{
			Src:       "10.1.1.1",
			Dest:      fmt.Sprintf("11.1.1.%d", i),
			SrcPort:   uint32(8000 + i),
			DestPort:  uint32(9000 + i),
			Action:    "allow",
			Direction: "from_host",
			RuleID:    "1234",
			Time:      timestamp,
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
			Action:    "allow",
			Direction: "from_host",
			RuleID:    "5678",
			Time:      timestamp,
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
			{
				StartTime: startTime,
			},
			// Get all logs that were just created but in reverse order
			{
				StartTime: startTime,
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
			},
			// Get if src = 10.1.1.1 or 10.1.1.2
			{
				SourceIPs: []string{"10.1.1.1", "10.1.1.2"},
				StartTime: startTime,
			},
			// Get if src = 10.1.1.1 or 10.1.1.0 AND source-port is 8000 AND action ALL
			{
				SourceIPs:   []string{"10.1.1.1", "10.1.1.0"},
				SourcePorts: []uint32{8000},
				Actions:     []string{"allow"},
				StartTime:   startTime,
			},
			// no logs match
			{
				SourceIPs:   []string{"10.1.1.1", "10.1.1.0"},
				SourcePorts: []uint32{8000},
				Actions:     []string{"deny"},
				StartTime:   startTime,
			},
		},
	}
	ctx := ts.tu.MustGetLoggedInContext(context.Background())
	numLogs := []int{20, 20, 12, 2, 0}

	verifyLogs := func() {
		Eventually(func() bool {
			resp, err := tc.Fwlogs(ctx, query)
			if err != nil {
				By(fmt.Sprintf("Fwlog query returned err %v", err))
				return false
			}
			// Even if citadel isn't ready, it should return 5 results
			Expect(len(resp.Results)).To(Equal(5))
			// Since each query may go to a different replica, we check all the result lengths
			for i, n := range numLogs {
				if len(resp.Results[i].Logs) != n {
					By(fmt.Sprintf("[%d] got %d records, expected %v, %+v", i, len(resp.Results[i].Logs), n, resp.Results))
					return false
				}
			}
			return true
		}, 180, 2).Should(BeTrue(), "Citadel failed to return expected amount of results")

		Eventually(func() bool {
			resp, err := tc.Fwlogs(ctx, query)
			if err != nil {
				By(fmt.Sprintf("Fwlog query returned err %v", err))
				return false
			}

			// verify results
			Expect(resp.Tenant).To(Equal(globals.DefaultTenant))
			Expect(len(resp.Results)).To(Equal(5))

			By(fmt.Sprintf("query[0] %+v", resp.Results[0].Logs))
			Expect(resp.Results[0].StatementID).To(Equal(int32(0)))

			if len(resp.Results[0].Logs) != 20 {
				By(fmt.Sprintf("mismatch in query results, expected 20, got %d", len(resp.Results[0].Logs)))
				return false
			}

			resLog := resp.Results[0].Logs
			reverse(resLog)
			Expect(logs).Should(Equal(resLog))

			By(fmt.Sprintf("query[1] %+v", resp.Results[1].Logs))
			Expect(resp.Results[1].StatementID).To(Equal(int32(1)))
			if len(resp.Results[1].Logs) != 20 {
				By(fmt.Sprintf("mismatch in query results, expected 20, got %d", len(resp.Results[1].Logs)))
				return false
			}
			Expect(logs).Should(Equal(resp.Results[1].Logs))

			By(fmt.Sprintf("query[2] %+v", resp.Results[2].Logs))
			Expect(resp.Results[2].StatementID).To(Equal(int32(2)))
			if len(resp.Results[2].Logs) != 12 {
				By(fmt.Sprintf("mismatch in query results, expected 12, got %d", len(resp.Results[2].Logs)))
				return false
			}

			By(fmt.Sprintf("query[3] %+v", resp.Results[3].Logs))
			Expect(resp.Results[3].StatementID).To(Equal(int32(3)))
			if len(resp.Results[3].Logs) != 2 {
				By(fmt.Sprintf("mismatch in query results, expected 2, got %d", len(resp.Results[3].Logs)))
				return false
			}

			By(fmt.Sprintf("query[4] %+v", resp.Results[4].Logs))
			Expect(resp.Results[4].StatementID).To(Equal(int32(4)))
			if len(resp.Results[4].Logs) != 0 {
				By(fmt.Sprintf("mismatch in query results, expected 0, got %d", len(resp.Results[3].Logs)))
				return false
			}

			return true
		}, 180, 2).Should(BeTrue(), "Citadel failed to return expected amount of results")
	}

	By("check fwlogs before the test")
	verifyLogs()

	// Restart Citadel - logs should be persisted
	By("Restarting citadel...")
	_, err = ts.tu.KillContainer(globals.Citadel)
	Expect(err).To(BeNil())

	nodesList, err := ts.tu.APIClient.ClusterV1().Node().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
	Expect(err).Should(BeNil())

	for _, node := range nodesList {
		Eventually(func() string {
			return ts.tu.GetContainerOnNode(ts.tu.NameToIPMap[node.GetName()], globals.Citadel)
		}, 120, 1).ShouldNot(BeEmpty(), globals.Citadel, " container not running on ", ts.tu.NameToIPMap[node.GetName()])
	}
	validateCluster() // reach consistent state and then look for logs

	verifyLogs()
}

var _ = Describe("telemetry tests", func() {
	BeforeEach(func() {
		validateCluster()
	})
	It("telemetry Node data", func() {
		testQueryingMetrics("Node")
	})
	It("telemetry SmartNIC data", func() {
		if ts.tu.NumNaplesHosts == 0 {
			Skip("No Naples node to report metrics")
		}
		testQueryingMetrics("DistributedServiceCard")
	})
	It("telemetry Fwlogs query", func() {
		Skip("Skip until it gets stabilized") // FIXME
		testQueryingFwlogs()
	})

	It("verify query functions", func() {
		kind := "Node"
		// Create telemetry client
		apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
		tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
		Expect(err).Should(BeNil())
		ctx := ts.tu.MustGetLoggedInContext(context.Background())

		for qf := range telemetry_query.TsdbFunctionType_value {
			qf = strings.ToLower(qf)
			Eventually(func() bool {
				fields := []string{}
				if qf == telemetry_query.TsdbFunctionType_MAX.String() ||
					qf == telemetry_query.TsdbFunctionType_TOP.String() ||
					qf == telemetry_query.TsdbFunctionType_BOTTOM.String() {
					fields = append(fields, "CPUUsedPercent")
				}
				nodeQuery := &telemetry_query.MetricsQueryList{
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
					Queries: []*telemetry_query.MetricsQuerySpec{
						{
							TypeMeta: api.TypeMeta{
								Kind: kind,
							},
							Function: qf,
							Fields:   fields,
						},
					},
				}

				By(fmt.Sprintf("query function:%v kind:%v", qf, kind))
				res, err := tc.Metrics(ctx, nodeQuery)
				if err != nil {
					By(fmt.Sprintf("query for %s returned err: %s", kind, err))
					return false
				}
				if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
					By(fmt.Sprintf("query for %v returned empty data", nodeQuery.Queries[0]))
					return false
				}
				if len(res.Results[0].Series[0].Values) == 0 {
					By(fmt.Sprintf("query returned empty data %v", res.Results[0].Series[0].Values))
					return false
				}

				// skip unsupported
				if qf == telemetry_query.TsdbFunctionType_NONE.String() ||
					qf == telemetry_query.TsdbFunctionType_DERIVATIVE.String() ||
					qf == telemetry_query.TsdbFunctionType_DIFFERENCE.String() {
					return true
				}

				// group by time
				By(fmt.Sprintf("query function:%v kind:%v group by time(3m)", qf, kind))
				nodeQuery.Queries[0].GroupbyTime = "3m"
				res, err = tc.Metrics(ctx, nodeQuery)
				if err != nil {
					By(fmt.Sprintf("query for %s returned err: %s", kind, err))
					return false
				}
				if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
					By(fmt.Sprintf("query for %v returned empty data", nodeQuery.Queries[0]))
					return false
				}
				if len(res.Results[0].Series[0].Values) == 0 {
					By(fmt.Sprintf("query returned empty data %v", res.Results[0].Series[0].Values))
					return false
				}

				return true
			}, 90, 10).Should(BeTrue(), "%s should have reported stats and been queryable", kind)
		}
	})

	Context("on restarting citadel service", func() {
		It("fwlog query should succeed", func() {
			Skip("skip fwlog test")
			nodesList, err := ts.tu.APIClient.ClusterV1().Node().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
			Expect(err).Should(BeNil())

			if len(nodesList) == 1 || len(ts.tu.NaplesNodeIPs) == 0 {
				Skip("skip citadel restart test on single node cluster")
			}

			for _, node := range nodesList {
				nodeName := node.GetName()
				Eventually(func() string {
					return ts.tu.GetContainerOnNode(ts.tu.NameToIPMap[nodeName], globals.Citadel)
				}, 120, 1).ShouldNot(BeEmpty(), globals.Citadel, " container not running on ", ts.tu.NameToIPMap[nodeName])
			}
			// validateCluster is called at BeforeEach()

			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			apiClient, err := apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())

			smartnics := map[string]int{}
			Eventually(func() bool {
				ctx := ts.tu.MustGetLoggedInContext(context.Background())
				snic, err := apiClient.ClusterV1().DistributedServiceCard().List(ctx, &api.ListWatchOptions{})
				if err != nil {
					By(fmt.Sprintf("failed to get smartnics %v", err))
					return false
				}

				if len(snic) != len(ts.tu.NaplesNodes) {
					By(fmt.Sprintf("invalid num. of smartnics, got %d, expected %d", len(snic), len(ts.tu.NaplesNodes)))
					return false
				}
				for i, s := range snic {
					smartnics[s.Name] = i
				}
				return true
			}, 120, 2).Should(BeTrue())

			stime := &api.Timestamp{}
			err = stime.Parse(time.Now().Format(time.RFC3339Nano))
			Expect(err).Should(BeNil())

			for _, naple := range ts.tu.NaplesNodes {
				By(fmt.Sprintf("trigger fwlog in NIC container %s", naple))
				st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naple, fwevent.Cmd(100, 65)))
				Expect(st == "null").Should(BeTrue())
			}
			time.Sleep(time.Second)

			By("verify fwlogs before the test")
			for smartnic := range smartnics {
				By(fmt.Sprintf("verify fwlog fron smartnic: %v", smartnic))
				Eventually(func() bool {
					apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
					tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
					Expect(err).Should(BeNil())

					// fwlog
					fwQuery := &telemetry_query.FwlogsQueryList{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Queries: []*telemetry_query.FwlogsQuerySpec{
							{
								ReporterIDs: []string{smartnic},
								StartTime:   stime,
							},
						},
					}

					ctx := ts.tu.MustGetLoggedInContext(context.Background())
					res, err := tc.Fwlogs(ctx, fwQuery)
					if err != nil {
						By(fmt.Sprintf("failed to get fwlog, %s", err))
						// print cluster
						GetCitadelState()
						return false
					}

					if len(res.Results) != 1 || len(res.Results[0].Logs) == 0 {
						By(fmt.Sprintf("didn't match query result, len(Results): %d, len(Results[0].Logs):%d",
							len(res.Results), len(res.Results[0].Logs)))
						return false
					}
					By(fmt.Sprintf("smartnic %s reported fwlog \u2714", smartnic))
					return true
				}, 180, 2).Should(BeTrue())
			}

			for _, vnode := range nodesList {
				By(fmt.Sprintf("restart venice node: %v", vnode.GetName()))
				Eventually(func() error {
					return ts.tu.KillContainerOnNodeByName(ts.tu.NameToIPMap[vnode.GetName()], globals.Citadel)
				}, 60, 2).Should(Succeed())

				for smartnic := range smartnics {
					By(fmt.Sprintf("verify fwlog fron smartnic: %v", smartnic))
					Eventually(func() bool {
						apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
						tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
						Expect(err).Should(BeNil())

						// fwlog
						fwQuery := &telemetry_query.FwlogsQueryList{
							Tenant:    globals.DefaultTenant,
							Namespace: globals.DefaultNamespace,
							Queries: []*telemetry_query.FwlogsQuerySpec{
								{
									ReporterIDs: []string{smartnic},
									StartTime:   stime,
								},
							},
						}

						ctx := ts.tu.MustGetLoggedInContext(context.Background())
						res, err := tc.Fwlogs(ctx, fwQuery)
						if err != nil {
							By(fmt.Sprintf("failed to get fwlog, %s", err))
							return false
						}

						if len(res.Results) != 1 || len(res.Results[0].Logs) == 0 {
							By(fmt.Sprintf("didn't match query result, len(Results): %d, len(Results[0].Logs):%d",
								len(res.Results), len(res.Results[0].Logs)))
							return false
						}

						By(fmt.Sprintf("smartnic %s reported %d fwlogs \u2714", smartnic, len(res.Results[0].Logs)))
						return true
					}, 180, 2).Should(BeTrue())

					Eventually(func() bool {
						apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
						tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
						Expect(err).Should(BeNil())

						// metrics
						mquery := &telemetry_query.MetricsQueryList{
							Tenant:    globals.DefaultTenant,
							Namespace: globals.DefaultNamespace,
							Queries: []*telemetry_query.MetricsQuerySpec{
								{
									TypeMeta: api.TypeMeta{
										Kind: "DistributedServiceCard",
									},
									StartTime: stime,
									Selector: &fields.Selector{
										Requirements: []*fields.Requirement{
											{
												Key:      "reporterID",
												Operator: fields.Operator_equals.String(),
												Values:   []string{smartnic},
											},
										},
									},
								},
							},
						}

						ctx := ts.tu.MustGetLoggedInContext(context.Background())
						mres, err := tc.Metrics(ctx, mquery)
						if err != nil {
							By(fmt.Sprintf("failed to get metrics, %s", err))
							return false
						}

						if len(mres.Results) != 1 || len(mres.Results[0].Series) == 0 {
							By(fmt.Sprintf("didn't match query result, len(Results): %d, len(Results[0].Series):%d",
								len(mres.Results), len(mres.Results[0].Series)))
							return false
						}

						By(fmt.Sprintf("smartnic %s reported metrics \u2714", smartnic))
						return true
					}, 120, 2).Should(BeTrue())
				}
			}
		})
	})
	AfterEach(func() {
		validateCluster()
	})

})

// Writes the given Fwlogs into influx
func writeFwlogs(logs []*telemetry_query.Fwlog) error {
	bp, err := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  "default",
		Precision: "ns",
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
		action := log.Action
		dir := log.Direction
		ruleID := log.RuleID
		timestamp, err := log.Time.Time()
		if err != nil {
			return err
		}

		tags := map[string]string{"source": ipSrc, "destination": ipDest, "destination-port": dPort, "protocol": ipProt}
		flds := map[string]interface{}{"source-port": sPort, "action": action, "direction": dir, "rule-id": ruleID}

		By(fmt.Sprintf("adding tags %+v", tags))
		pt, err := client.NewPoint("Fwlogs", tags, flds, timestamp)
		if err != nil {
			return err
		}
		bp.AddPoint(pt)
	}

	url := fmt.Sprintf("http://%s:%s", globals.Localhost, globals.CitadelHTTPPort)
	writePoints(url, bp)
	return nil
}

func GetCitadelState() {
	node := ts.tu.QuorumNodes[rand.Intn(len(ts.tu.QuorumNodes))]
	nodeIP := ts.tu.NameToIPMap[node]
	url := fmt.Sprintf("http://%s:%s", globals.Localhost, globals.CitadelHTTPPort)
	res := ts.tu.CommandOutput(nodeIP, fmt.Sprintf(`curl %s/info`, url))
	By(res)
}

// writePoints writes points to citadel using inflxdb client
func writePoints(url string, bp client.BatchPoints) {
	rand.Seed(time.Now().Unix())
	node := ts.tu.QuorumNodes[rand.Intn(len(ts.tu.QuorumNodes))]
	nodeIP := ts.tu.NameToIPMap[node]
	By(fmt.Sprintf("Selecting node %s to write points", node))
	points := bp.Points()
	pointsStr := []string{}
	for _, p := range points {
		pointsStr = append(pointsStr, p.String())
	}
	Eventually(func() bool {
		res := ts.tu.CommandOutput(nodeIP, fmt.Sprintf(`curl -s -o /dev/null -w "%%{http_code}" -XPOST "%s/write?db=%s" --data-binary '%s'`, url, bp.Database(), strings.Join(pointsStr, "\n")))
		By(fmt.Sprintf("writing points returned code %s", res))
		if strings.HasPrefix(res, "20") {
			return true
		}
		GetCitadelState()
		return false
	}, 10, 1).Should(BeTrue(), "Failed to write logs")
}

func reverse(logs []*telemetry_query.Fwlog) {
	for i := len(logs)/2 - 1; i >= 0; i-- {
		opp := len(logs) - 1 - i
		logs[i], logs[opp] = logs[opp], logs[i]
	}
}
