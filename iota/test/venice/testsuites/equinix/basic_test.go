package techsupport_test

import (
	"context"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("Equinx Bring up", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Push config Tests", func() {

		It("Basic traffic tests", func() {
			workloads := ts.model.WorkloadPairs().WithinNetwork()
			Expect(len(workloads.Pairs) != 0).Should(BeTrue())
			Expect(ts.model.TCPSession(workloads, 8000)).Should(Succeed())
		})

		It("Delete & Add Config", func() {
			Expect(ts.model.CleanupAllConfig()).Should(Succeed())
			err := ts.model.SetupDefaultConfig(context.Background(), ts.scaleData, ts.scaleData)
			Expect(err).ShouldNot(HaveOccurred())
		})
	})
})
