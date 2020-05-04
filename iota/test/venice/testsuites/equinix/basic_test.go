package equinix_test

import (
	"context"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("Equinix Bring up", func() {

	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})

	Context("Basic tests", func() {

		It("Make sure Ping works", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Skipping datapath tests on sim nodes")
			}
			workloads := ts.model.WorkloadPairs().WithinNetwork()
			Expect(len(workloads.Pairs) != 0).Should(BeTrue())
			Eventually(func() error {
				return ts.model.PingPairs(workloads)
			}).Should(Succeed())
		})

		It("Make sure TCP works", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Skipping datapath tests on sim nodes")
			}
			workloads := ts.model.WorkloadPairs().WithinNetwork()
			Expect(len(workloads.Pairs) != 0).Should(BeTrue())
			Expect(ts.model.TCPSession(workloads, 8000)).Should(Succeed())
		})

		It("Delete & Add Config", func() {
			Expect(ts.model.CleanupAllConfig()).Should(Succeed())
			Expect(ts.model.TeardownWorkloads(ts.model.Workloads()))
			err := ts.model.SetupDefaultConfig(context.Background(), ts.scaleData, ts.scaleData)
			Expect(err).ShouldNot(HaveOccurred())
		})
	})
})
