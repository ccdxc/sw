// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("firewall scale tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})

	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Scale TCP connections tests", func() {
		It("Should establish bulk TCP session between all workload with default policy", func() {
			log.Infof("scaleData = %v hasNaples = %v", ts.scaleData, ts.tb.HasNaplesSim())
			if !ts.scaleData || ts.tb.HasNaplesSim() {
				Skip("Skipping scale connection runs")
			}

			// fuz 100 tcp connectons from each workload to other workload on dest port 8000
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			Expect(ts.model.Action().FuzIt(workloadPairs, 100, "tcp", "8000")).ShouldNot(HaveOccurred())

		})
	})
})
