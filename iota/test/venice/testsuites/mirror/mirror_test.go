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

		// delete test policy if its left over. we can ignore the error here
		//ts.model.SGPolicy("test-policy").Delete()
		//ts.model.DefaultSGPolicy().Delete()

		// recreate default allow policy
		//Expect(ts.model.DefaultSGPolicy().Restore()).ShouldNot(HaveOccurred())
	})
	Context("Mirror tests", func() {
		It("Mirror packets to collector and check TCPDUMP", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			// add permit rules for workload pairs
                        collector := ts.model.Workloads().Any(1)
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().ExcludeWorkloads(collector).Any(1)
			spc := ts.model.NewMirrorSession("test-mirror").AddRulesForWorkloadPairs(workloadPairs, "icmp/0/0")
			spc.AddCollector(collector, "udp/4545")
                        Expect(spc.Commit()).Should(Succeed())

			// verify TCP connection works between workload pairs
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 8000)
			}).Should(Succeed())

		})
	})
})
