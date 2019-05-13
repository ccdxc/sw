package rollout_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("rollout tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Iota Rollout tests", func() {

		It("Perform Rollout", func() {

			rollout, err := ts.model.GetRolloutObject()
			Expect(err).ShouldNot(HaveOccurred())

			err = ts.model.Action().PerformRollout(rollout)
			Expect(err).ShouldNot(HaveOccurred())

			// verify rollout is successful
			Eventually(func() error {
				return ts.model.Action().VerifyRolloutStatus(rollout.Name)
			}).Should(Succeed())
		})
	})
})
