package equinix_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("Trigger Tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.model.AfterTestCommon()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})

	Context("Trigger Tests", func() {

		// Temporarily disable while root causing Sanity issues

		// It("Venice Reboot", func() {
		// 	Expect(ts.model.TriggerVeniceReboot(100)).Should(Succeed())
		// 	Eventually(func() error {
		// 		return ts.model.VerifyClusterStatus()
		// 	}).Should(Succeed())
		// })

		It("Delete Add config", func() {
			Expect(ts.model.TriggerDeleteAddConfig(100)).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Host Reboot", func() {
			Expect(ts.model.TriggerHostReboot(100)).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Link Flap", func() {
			if ts.tb.HasNaplesSim() {
				Skip("link flap trigger disabled on SIM")
			}
			Expect(ts.model.TriggerLinkFlap(100)).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())
		})

		// Temporarily disable while root causing Sanity issues

		// It("Venice Paritition", func() {
		// 	Expect(ts.model.TriggerVenicePartition(100)).Should(Succeed())
		// 	Eventually(func() error {
		// 		return ts.model.VerifyClusterStatus()
		// 	}).Should(Succeed())
		// })
	})
})
