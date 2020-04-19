// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package equinix_test

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/log"
)

func GetTechSupportRequest(name string, nodeNames []string) monitoring.TechSupportRequest {
	return monitoring.TechSupportRequest{
		TypeMeta: api.TypeMeta{
			Kind: string(monitoring.KindTechSupportRequest),
		},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: monitoring.TechSupportRequestSpec{
			Verbosity: 1,
			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Names: nodeNames,
			},
		},
	}
}

var _ = Describe("TechSupport", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	SetDefaultConsistentlyDuration(3 * time.Second)
	SetDefaultConsistentlyPollingInterval(1 * time.Second)

	Context("Naples TechSupport cases", func() {
		It("Request Techsupport for 1DSC", func() {
			//Skip("Disabling test for sanity")
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim until FRU mac is used for Nodes")
			}

			var nodeNames []string

			techsupportName := "techsupport-test-1"
			naples := ts.model.Naples().Names()
			nodeNames = append(nodeNames, naples[0])
			log.Infof("Requesting Techsupport for %v", nodeNames)
			techsupport := GetTechSupportRequest(techsupportName, nodeNames)

			err := ts.model.PerformTechsupport(&techsupport)
			Expect(err).ShouldNot(HaveOccurred())

			// verify if techsupport request got acccepted
			Eventually(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}, 30*time.Second, 1*time.Second).Should(Succeed())

			// verify if techsupport request got completed successfully
			Eventually(func() error {
				return ts.model.VerifyTechsupport(techsupportName)
			}, 2*time.Minute, 30*time.Second).Should(Succeed())

			// TODO: Add File validations

			// teardown
			// delete the techsupport
			err = ts.model.DeleteTechsupport(techsupportName)
			Expect(err).ShouldNot(HaveOccurred())

			// verify if techsupport get fails
			Consistently(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}).ShouldNot(Succeed())
		})

		It("Request Techsupport for All nodes", func() {
			//Skip("Disabling test for sanity")
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim until FRU mac is used for Nodes")
			}

			var nodeNames []string

			techsupportName := "techsupport-test-2"

			// get all naples nodes
			nodeNames = ts.model.Naples().Names()
			// TODO: Including Venice nodes crashes TSM
			// // get all venice nodes
			// nodes := ts.model.VeniceNodes()
			// for _, ip := range nodes.GenVeniceIPs() {
			// 	nodeNames = append(nodeNames, ip)
			// }

			log.Infof("Requesting Techsupport for %v", nodeNames)
			techsupport := GetTechSupportRequest(techsupportName, nodeNames)

			err := ts.model.PerformTechsupport(&techsupport)
			Expect(err).ShouldNot(HaveOccurred())

			// verify if techsupport request got acccepted
			Eventually(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}, 30*time.Second, 1*time.Second).Should(Succeed())

			// verify if techsupport request got completed successfully
			Eventually(func() error {
				return ts.model.VerifyTechsupport(techsupportName)
			}, 5*time.Minute, 30*time.Second).Should(Succeed())

			// TODO: Add File validations

			// teardown
			// delete the techsupport
			err = ts.model.DeleteTechsupport(techsupportName)
			Expect(err).ShouldNot(HaveOccurred())

			// verify if techsupport get fails post deletion
			Consistently(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}).ShouldNot(Succeed())
		})
	})

	Context("Negative TechSupport cases", func() {
		It("Request Techsupport for 0 Nodes", func() {
			//Skip("Disabling test for sanity")

			var nodeNames []string

			techsupportName := "techsupport-neg-1"
			log.Infof("Requesting Techsupport for %v", nodeNames)
			techsupport := GetTechSupportRequest(techsupportName, nodeNames)

			err := ts.model.PerformTechsupport(&techsupport)
			Expect(err).Should(HaveOccurred())

			// verify if techsupport request did not get acccepted
			Consistently(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}).ShouldNot(Succeed())
		})

		It("Get non-requested Techsupport", func() {
			techsupportName := "techsupport-neg-2"

			// verify if techsupport request fails
			Consistently(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}, 3*time.Second, 1*time.Second).ShouldNot(Succeed())
		})

		It("Delete non-requested Techsupport", func() {
			//Skip("Disabling test for sanity")
			techsupportName := "techsupport-neg-3"

			// verify if delete techsupport request fails
			err := ts.model.DeleteTechsupport(techsupportName)
			Expect(err).Should(HaveOccurred())
			Consistently(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}).ShouldNot(Succeed())
		})

		It("Request Techsupport for invalid node", func() {
			//Skip("Disabling test for sanity")
			var nodeNames []string

			techsupportName := "techsupport-neg-4"
			nodeNames = append(nodeNames, "e1ba.aced.a111")
			log.Infof("Requesting Techsupport for %v", nodeNames)
			techsupport := GetTechSupportRequest(techsupportName, nodeNames)

			err := ts.model.PerformTechsupport(&techsupport)
			Expect(err).ShouldNot(HaveOccurred())

			// verify if techsupport request got acccepted
			Eventually(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}, 30*time.Second, 1*time.Second).Should(Succeed())

			// verify if techsupport request got completed successfully
			Consistently(func() error {
				return ts.model.VerifyTechsupport(techsupportName)
			}, 2*time.Minute, 30*time.Second).ShouldNot(Succeed())

			// TODO: Add timeout validation

			// teardown
			// delete the techsupport
			err = ts.model.DeleteTechsupport(techsupportName)
			Expect(err).ShouldNot(HaveOccurred())

			// verify if techsupport get fails
			Consistently(func() error {
				return ts.model.VerifyTechsupportStatus(techsupportName)
			}).ShouldNot(Succeed())
		})
	})

})
