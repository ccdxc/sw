// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package smartnic_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit"
)

var _ = Describe("smartnic tests with venice cluster triggers", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})

	Context("Venice node reload tests", func() {
		It("Should be able reload venice nodes and cluster should come back to normal state", func() {
			Skip("Skipping till we debug Venice node reload issues")

			// reload each host
			ts.model.ForEachVeniceNode(func(vnc *iotakit.VeniceNodeCollection) error {
				Expect(ts.model.Action().ReloadVeniceNodes(vnc)).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				// verify ping between all workloads
				Eventually(func() error {
					return ts.model.Action().PingPairs(ts.model.WorkloadPairs().WithinNetwork().Any(4))
				}).Should(Succeed())

				return nil
			})
		})

		It("Should be able reload venice leader node and cluster should come back to normal state", func() {
			Skip("Skipping till we debug Venice node reload issues")

			// reload the leader node
			for i := 0; i < 3; i++ {
				Expect(ts.model.Action().ReloadVeniceNodes(ts.model.VeniceNodes().Leader())).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				// verify ping between all workloads
				Eventually(func() error {
					return ts.model.Action().PingPairs(ts.model.WorkloadPairs().WithinNetwork().Any(4))
				}).Should(Succeed())

			}
		})
	})

})
