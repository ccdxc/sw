// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package mirror_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("mirror tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})
	Context("Mirror tests", func() {
		It("Mirror packets to collector and check TCPDUMP", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			// add permit rules for workload pairs
                        collector := ts.model.Workloads().Any(2)
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().ExcludeWorkloads(collector).Any(1)
			msc := ts.model.NewMirrorSession("test-mirror").AddRulesForWorkloadPairs(workloadPairs, "icmp/0/0")
			msc.AddCollector(collector, "udp/4545", 0)
                        Expect(msc.Commit()).Should(Succeed())

			// TODO: verify we receive mirror packets at the collector
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 8000)
			}).Should(Succeed())

                        // Clear collectors
			msc.ClearCollectors()

                        // Update the collector
			msc.AddCollector(collector, "udp/4545", 1)
                        Expect(msc.Commit()).Should(Succeed())

			// TODO: verify we receive mirror packets at the updated collector
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 8000)
			}).Should(Succeed())

                        // Delete the Mirror session
                        Expect(msc.Delete()).Should(Succeed())

		})
	})
})
