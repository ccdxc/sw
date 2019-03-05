// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("firewall tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})

	AfterEach(func() {
		ts.tb.AfterTestCommon()

		// delete test policy if its left over. we can ignore the error here
		ts.model.SGPolicy("test-policy").Delete()
		ts.model.SGPolicy("default-policy").Delete()

		// recreate default allow policy
		Expect(ts.model.NewSGPolicy("default-policy").AddRule("any", "any", "", "PERMIT").Commit()).ShouldNot(HaveOccurred())
	})

	Context("Basic firewall tests", func() {
		It("Should establish TCP session between all workload with default policy", func() {

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 8000)
			}).Should(Succeed())
		})

		It("Should not establish TCP session between any workload with deny policy", func() {
			// change the default policy to deny all
			err := ts.model.SGPolicy("default-policy").Rules().Update("action = DENY").Commit()
			Expect(err).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.Action().VerifyPolicyStatus(ts.model.SGPolicy("default-policy"))
			}).Should(Succeed())

			// randomly pick one workload and verify ping fails between them
			workloadPair := ts.model.WorkloadPairs().WithinNetwork()
			Eventually(func() error {
				return ts.model.Action().TCPSessionFails(workloadPair, 8000)
			}).Should(Succeed())

			// change the default policy back to allow all
			err = ts.model.SGPolicy("default-policy").Rules().Update("action = PERMIT").Commit()
			Expect(err).ShouldNot(HaveOccurred())

			// ping any workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 8000)
			}).Should(Succeed())
		})
	})
})
