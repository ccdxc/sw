package equinix_test

import (
	"encoding/json"
	"fmt"
	"time"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/metrics/apulu/genfields"
	"github.com/pensando/sw/metrics/types"
	cq "github.com/pensando/sw/venice/citadel/broker/continuous_query"
	"github.com/pensando/sw/venice/utils/telemetryclient"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("metrics test", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
		startTime = time.Now().UTC()
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})

	Context("tags:type=basic;datapath=true;duration=short Verify basic metrics ", func() {
		It("tags:sanity=true Check metrics fields", checkMetricsFields)

		It("tags:sanity=true Check CQ metrics fields", checkCQMetricsFields)

		It("tags:sanity=true Reloading venice nodes", func() {
			// get node collection and init telemetry client
			vnc := ts.model.VeniceNodes()
			err := vnc.InitTelemetryClient()
			Expect(err).Should(BeNil())

			// reload each host
			ts.model.ForEachVeniceNode(func(vnc *objects.VeniceNodeCollection) error {
				Expect(ts.model.ReloadVeniceNodes(vnc)).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.VerifyClusterStatus()
				}).Should(Succeed())

				return nil
			})
		})

		It("tags:sanity=true Check metrics fields after reloading nodes", checkMetricsFields)

		It("tags:sanity=true Check CQ metrics fields after reloading nodes", checkCQMetricsFields)

	})
})

func checkMetricsFields() {
	if !ts.tb.HasNaplesHW() {
		Skip("No naples hw detected, skip checking metrics")
	}
	tms := time.Now().UTC().Add(time.Second * -60).Format(time.RFC3339)

	// get node collection and init telemetry client
	vnc := ts.model.VeniceNodes()
	err := vnc.InitTelemetryClient()
	Expect(err).Should(BeNil())

	for _, k := range types.CloudDscMetricsList {
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

func checkCQMetricsFields() {
	if !ts.tb.HasNaplesHW() {
		Skip("No naples hw detected, skip checking metrics")
	}
	tms := time.Now().Add(time.Hour * -2).Format(time.RFC3339)

	// get node collection and init telemetry client
	vnc := ts.model.VeniceNodes()
	err := vnc.InitTelemetryClient()
	Expect(err).Should(BeNil())

	for s := range cq.RetentionPolicyMap {
		if s != "5minutes" {
			continue
		}
		for _, m := range types.CloudDscMetricsList {
			// drop metrics are reported only on drop
			if m == "IPv4FlowDropMetrics" || m == "DropMetrics" {
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
			}).Should(Succeed())
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
