package rollout_test

import (
	"encoding/json"
	"errors"
	"fmt"
	"time"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/metrics/genfields"
	"github.com/pensando/sw/metrics/types"
	cq "github.com/pensando/sw/venice/citadel/broker/continuous_query"
	cmdtypes "github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/telemetryclient"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("rollout tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
			_, err := ts.model.RunNaplesCommand(nc, "touch /data/upgrade_to_same_firmware_allowed")
			Expect(err).ShouldNot(HaveOccurred())
			return nil
		})
	})
	AfterEach(func() {
		ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
			ts.model.RunNaplesCommand(nc, "rm /data/upgrade_to_same_firmware_allowed")
			return nil
		})
	})

	Context("Iota Rollout tests", func() {

		It("Perform Rollout", func() {
			var workloadPairs *objects.WorkloadPairCollection
			rollout, err := ts.model.GetRolloutObject(ts.scaleData)
			Expect(err).ShouldNot(HaveOccurred())

			if *simOnlyFlag == false {
				workloadPairs = ts.model.WorkloadPairs().WithinNetwork().Any(40)
				log.Infof(" Length workloadPairs %v", len(workloadPairs.ListIPAddr()))
				Expect(len(workloadPairs.ListIPAddr()) != 0).Should(BeTrue())
			}

			err = ts.model.PerformRollout(rollout, ts.scaleData, "upgrade-bundle")
			Expect(err).ShouldNot(HaveOccurred())
			rerr := make(chan bool)
			if *simOnlyFlag == false {
				go func() {
					options := &objects.ConnectionOptions{
						Duration:          "180s",
						Port:              "8000",
						Proto:             "tcp",
						ReconnectAttempts: 100,
					}
					_ = ts.model.ConnectionWithOptions(workloadPairs, options)
					log.Infof("TCP SESSION TEST COMPLETE")
					rerr <- true
					return
				}()
			}

			// verify rollout is successful
			Eventually(func() error {
				return ts.model.VerifyRolloutStatus(rollout.Name)
			}).Should(Succeed())
			log.Infof("Rollout Completed. Waiting for Fuz tests to complete..")

			if *simOnlyFlag == false {
				errWaitingForFuz := func() error {
					select {
					case <-rerr:
						log.Infof("Verified DataPlane using fuz connections")
						return nil
					case <-time.After(time.Duration(900) * time.Second):
						log.Infof("Timeout while waiting for fuz api to return")
						return errors.New("Timeout while waiting for fuz api to return")
					}
				}
				Expect(errWaitingForFuz()).ShouldNot(HaveOccurred())
			}
			return

		})

		It("Check metrics fields after rollout", checkMetricsFields)

		It("Check CQ metrics fields after rollout", checkCQMetricsFields)

		It("Check cluster metrics after rollout", checkClusterMetrics)
	})
})

func checkMetricsFields() {
	if !ts.tb.HasNaplesHW() {
		Skip("No naples hw detected, skip checking metrics")
	}
	tms := time.Now().UTC().Add(time.Second * -30).Format(time.RFC3339)

	// get node collection and init telemetry client
	vnc := ts.model.VeniceNodes()
	err := vnc.InitTelemetryClient()
	Expect(err).Should(BeNil())

	for _, k := range types.DscMetricsList {
		if k == "IPv4FlowDropMetrics" {
			continue
		}

		Eventually(func() error {
			return ts.model.ForEachNaples(func(n *objects.NaplesCollection) error {
				fields := genfields.GetFieldNamesFromKind(k)
				for _, name := range n.Names() {
					By(fmt.Sprintf("checking %v in naples %v", k, name))

					resp, err := vnc.QueryMetricsByReporter(k, name, tms)
					if err != nil {
						fmt.Printf("query failed %v \n", err)
						return err
					}

					err = validateResp(resp, fields, tms)
					if err != nil {
						return err
					}
				}
				return nil
			})
		}).Should(Succeed())
	}
}

func checkClusterMetrics() {
	if !ts.tb.HasNaplesHW() {
		Skip("No naples hw detected, skip checking metrics")
	}
	tms := time.Now().UTC().Add(time.Second * -60).Format(time.RFC3339)
	cqtms := time.Now().UTC().Add(time.Minute * -10).Format(time.RFC3339)

	// get node collection and init telemetry client
	vnc := ts.model.VeniceNodes()
	err := vnc.InitTelemetryClient()
	Expect(err).Should(BeNil())

	kind := "Cluster"
	clusterZeroMap := cmdtypes.GetSmartNICMetricsZeroMap()
	fields := []string{}
	for f := range clusterZeroMap {
		fields = append(fields, f)
	}
	Eventually(func() error {
		By(fmt.Sprintf("checking %v\n", kind))

		// query cluster metrics
		resp, err := vnc.QueryMetricsFields(kind, tms)
		if err != nil {
			fmt.Printf("query failed %v \n", err)
			return err
		}

		// make sure cluster metrics exists
		err = validateResp(resp, fields, tms)
		if err != nil {
			return err
		}

		// query cluster CQ metrics
		for s := range cq.RetentionPolicyMap {
			if s != "5minutes" {
				continue
			}
			cq := kind + "_" + s
			By(fmt.Sprintf("checking %v\n", cq))
			resp, err := vnc.QueryMetricsFields(cq, cqtms)
			if err != nil {
				fmt.Printf("query failed %v \n", err)
				return err
			}

			// make sure cluster metrics exists
			err = validateResp(resp, fields, cqtms)
			if err != nil {
				return err
			}
		}

		return nil
	}, time.Duration(10)*time.Minute, time.Duration(30)*time.Second).Should(Succeed())
}

func checkCQMetricsFields() {
	if !ts.tb.HasNaplesHW() {
		Skip("No naples hw detected, skip checking metrics")
	}
	tms := time.Now().Add(time.Minute * -10).Format(time.RFC3339)

	// get node collection and init telemetry client
	vnc := ts.model.VeniceNodes()
	err := vnc.InitTelemetryClient()
	Expect(err).Should(BeNil())

	for s := range cq.RetentionPolicyMap {
		if s != "5minutes" {
			continue
		}
		for _, m := range types.DscMetricsList {
			// drop metrics are reported only on drop
			if m == "IPv4FlowDropMetrics" {
				continue
			}
			cq := m + "_" + s
			Eventually(func() error {
				return ts.model.ForEachNaples(func(n *objects.NaplesCollection) error {
					fields := genfields.GetFieldNamesFromKind(m)
					for _, name := range n.Names() {
						By(fmt.Sprintf("checking %v in naples %v", cq, name))

						resp, err := vnc.QueryMetricsByReporter(cq, name, tms)
						if err != nil {
							fmt.Printf("query failed %v \n", err)
							return err
						}
						err = validateResp(resp, fields, tms)
						if err != nil {
							return err
						}
					}
					return nil
				})
			}, time.Duration(10)*time.Minute, time.Duration(30)*time.Second).Should(Succeed())
		}
	}
}

func validateResp(resp *telemetryclient.MetricsQueryResponse, fields []string, tms string) error {
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

	return nil
}
