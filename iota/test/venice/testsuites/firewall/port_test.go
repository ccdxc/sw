package firewall_test

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("Port tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("tags:type=basic;datapath=true;duration=short Basic port flap tests", func() {
		It("Link flap should not result in traffic failure", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("link flap cannot be run on NAPLES sim")
			}
			startTime := time.Now()
			startTime = startTime.Add(-3 * time.Minute) // TODO: remove this; there is ~2 to ~3 minute delay between naples sim and rund VM

			// get a random naples and flap the port
			nc := ts.model.Naples().Any(1)
			Expect(nc.Error()).ShouldNot(HaveOccurred())
			Expect(ts.model.PortFlap(nc)).Should(Succeed())
			time.Sleep(60 * time.Second) // wait for the event to reach venice

			// verify ping is successful across all workloads after the port flap
			Eventually(func() error {
				return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
			}).Should(Succeed())
		})
	})
})
