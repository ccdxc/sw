package techsupport_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
)

var _ = Describe("techsupport tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Iota Techsupport tests", func() {

		It("Perform Techsupport", func() {
			techsupport := &monitoring.TechSupportRequest{
				TypeMeta: api.TypeMeta{
					Kind: "TechSupportRequest",
				},
				ObjectMeta: api.ObjectMeta{
					Name:"techsupport-test",
				},
				Spec: monitoring.TechSupportRequestSpec{
				},
			}
			err := ts.model.Action().PerformTechsupport(techsupport)
			Expect(err).ShouldNot(HaveOccurred())

			// verify techsupport is successful
			Eventually(func() error {
				return ts.model.Action().VerifyTechsupportStatus(techsupport.Name)
			}).Should(Succeed())
			return

		})
	})
})
