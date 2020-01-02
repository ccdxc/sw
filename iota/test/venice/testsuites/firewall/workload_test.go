// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("workload tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Basic workload tests", func() {
		It("Should be able to bringup new workloads and connect between them", func() {
			workloads := ts.model.Hosts().BringUpNewWorkloads(ts.model.Networks().Any(1), 1)
			Expect(workloads.Error()).ShouldNot(HaveOccurred())
			// verify workload status is good
			Eventually(func() error {
				return ts.model.Action().VerifyWorkloadStatus(workloads)
			}).Should(Succeed())

			// check ping between new workloads
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloads.MeshPairs().Any(4), 8000)
			}).ShouldNot(HaveOccurred())

			//Teardown the new workloads
			err := workloads.Teardown()
			Expect(err == nil)
		})
	})
})
