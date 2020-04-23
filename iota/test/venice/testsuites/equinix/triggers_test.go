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
		ts.tb.AfterTestCommon()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})

	Context("Trigger Tests", func() {

		It("Venice Reboot", func() {
			Expect(ts.model.TriggerVeniceReboot(100)).Should(Succeed())
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
			Expect(ts.model.TriggerLinkFlap(100)).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Venice Paritition", func() {
			Expect(ts.model.TriggerVenicePartition(100)).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Delete Add config", func() {
			Skip("Disabling test for sanity")
			Expect(ts.model.TriggerDeleteAddConfig(100)).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())
		})
	})
})
