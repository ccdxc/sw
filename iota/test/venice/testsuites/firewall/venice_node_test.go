// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

var _ = Describe("smartnic tests with venice cluster triggers", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
	})

	Context("Venice node reload tests", func() {
		It("Should be able to reload venice nodes and cluster should come back to normal state", func() {
			Skip("Skipping venice reload tests till we debug issues")
			// reload each host
			ts.model.ForEachVeniceNode(func(vnc *objects.VeniceNodeCollection) error {
				Expect(ts.model.ReloadVeniceNodes(vnc)).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.VerifyClusterStatus()
				}).Should(Succeed())

				// verify ping between all workloads
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork().Any(4))
				}).Should(Succeed())

				return nil
			})
		})

		It("Should be able to reload venice leader node and cluster should come back to normal state", func() {
			Skip("Skipping venice reload tests till we debug issues")
			// reload the leader node
			for i := 0; i < 3; i++ {
				Expect(ts.model.ReloadVeniceNodes(ts.model.VeniceNodes().Leader())).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.VerifyClusterStatus()
				}).Should(Succeed())

				// verify ping between all workloads
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork().Any(4))
				}).Should(Succeed())

			}
		})
		It("Should be able to reload all venice nodes and cluster should come back up", func() {
			Skip("Skipping venice reload tests till we debug issues")
			// run 3 iterations
			for i := 0; i < 3; i++ {
				// reload all venice nodes
				ts.model.ForEachVeniceNode(func(vnc *objects.VeniceNodeCollection) error {
					Expect(ts.model.ReloadVeniceNodes(vnc)).Should(Succeed())
					return nil
				})

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.VerifyClusterStatus()
				}).Should(Succeed())

				// verify ping between all workloads
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork().Any(4))
				}).Should(Succeed())
			}
		})
	})

})
