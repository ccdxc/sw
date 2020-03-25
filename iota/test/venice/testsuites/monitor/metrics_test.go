package monitor_test

import (
	"encoding/json"
	"fmt"
	"time"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/metrics/genfields"
	"github.com/pensando/sw/metrics/types"
	cq "github.com/pensando/sw/venice/citadel/broker/continuous_query"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("metrics test", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})

	Context("tags:type=basic;datapath=true;duration=short Verify flowdropmetrics ", func() {
		It("tags:sanity=true Flow drops should increament", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling flow drop stats on naples sim")
			}
			//spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "any", "PERMIT")
			//Expect(spc.Commit()).ShouldNot(HaveOccurred())

			//Verif policy was propagted correctly

			startTime := time.Now().UTC().Format(time.RFC3339)
			// establish TCP session between workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			log.Infof("wait to get to the console")
			time.Sleep(time.Second * 30)
			Eventually(func() error {
				return ts.model.DropIcmpFlowTTLSession(workloadPairs, "--icmp --icmptype 4 --count 1")
			}).Should(Succeed())
			time.Sleep(time.Second * 30)

			// check fwlog, enable when fwlogs are reported to Venice
			/*
				Eventually(func() error {
					return ts.model.FindFwlogForWorkloadPairs("ICMP", "allow", startTime.String(), 0, workloadPairs.ReversePairs())
				}).Should(Succeed())
			*/

			Eventually(func() error {
				return ts.model.QueryDropMetricsForWorkloadPairs(workloadPairs, startTime)
			}).Should(Succeed())

		})
	})

	Context("tags:type=basic;datapath=true;duration=short Verify basic metrics ", func() {
		It("tags:sanity=true Check metrics fields", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling flow drop stats on naples sim")
			}
			tms := time.Now().UTC().Add(time.Second * -30).Format(time.RFC3339)

			for _, k := range types.DscMetricsList {
				if k == "IPv4FlowDropMetrics" {
					continue
				}

				Eventually(func() error {
					return ts.model.ForEachNaples(func(n *objects.NaplesCollection) error {
						fields := genfields.GetFieldNamesFromKind(k)
						for _, name := range n.Names() {
							By(fmt.Sprintf("checking %v in naples %v", k, name))

							resp, err := ts.model.QueryMetricsByReporter(k, name, tms)
							if err != nil {
								fmt.Printf("query failed %v \n", err)
								return err
							}

							if len(resp.Results) == 0 || len(resp.Results[0].Series) == 0 {
								res, err := json.Marshal(resp)
								fmt.Printf("query ts %v returned(%v) %+v \n", tms, err, string(res))
								return fmt.Errorf("no results")
							}

							for _, r := range resp.Results[0].Series {

								// get index
								cIndex := map[string]int{}
								for i, c := range r.Columns {
									cIndex[c] = i
								}

								for _, f := range fields {
									if _, ok := cIndex[f]; !ok {
										fmt.Printf("failed to find %v \n", f)
										return fmt.Errorf("failed to find %v", f)
									}
									fmt.Printf("\tcheck %v \u2714 \n", f)
								}
							}
						}
						return nil
					})
				}).Should(Succeed())
			}
		})

		It("tags:sanity=true Check CQ metrics fields", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling flow drop stats on naples sim")
			}

			tms := time.Now().Add(time.Hour * -2).Format(time.RFC3339)

			for s := range cq.RetentionPolicyMap {
				if s != "5minutes" {
					continue
				}
				for _, m := range types.DscMetricsList {
					// drop metrics are reported only on drop
					if m == "IPv4FlowDropMetrics" || m == "DropMetrics" || m == "EgressDropMetrics" {
						continue
					}
					cq := m + "_" + s
					Eventually(func() error {
						return ts.model.ForEachNaples(func(n *objects.NaplesCollection) error {
							fields := genfields.GetFieldNamesFromKind(m)
							for _, name := range n.Names() {
								By(fmt.Sprintf("checking %v in naples %v", cq, name))

								resp, err := ts.model.QueryMetricsByReporter(cq, name, tms)
								if err != nil {
									fmt.Printf("query failed %v \n", err)
									return err
								}

								if len(resp.Results) == 0 || len(resp.Results[0].Series) == 0 {
									res, err := json.Marshal(resp)
									fmt.Printf("query ts %v returned(%v) %+v \n", tms, err, string(res))
									return fmt.Errorf("no results")
								}

								for _, r := range resp.Results[0].Series {

									// get index
									cIndex := map[string]int{}
									for i, c := range r.Columns {
										cIndex[c] = i
									}

									for _, f := range fields {
										if _, ok := cIndex[f]; !ok {
											fmt.Printf("failed to find %v \n", f)
											return fmt.Errorf("failed to find %v", f)
										}
										fmt.Printf("\tcheck %v \u2714 \n", f)
									}
								}
							}
							return nil
						})
					}).Should(Succeed())
				}
			}
		})
	})
})
