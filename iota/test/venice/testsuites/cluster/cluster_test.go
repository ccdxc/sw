// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cluster_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit"
)

var _ = Describe("venice cluster tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})

	Context("Basic cluster tests", func() {
		It("Should be able reload venice nodes and cluster should come back to normal state", func() {
			// reload each host
			ts.model.ForEachVeniceNode(func(vnc *iotakit.VeniceNodeCollection) error {
				Expect(ts.model.Action().ReloadVeniceNodes(vnc)).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				return nil
			})
		})

		It("Should be able reload venice leader node and cluster should come back to normal state", func() {
			// reload the leader node
			for i := 0; i < 3; i++ {
				Expect(ts.model.Action().ReloadVeniceNodes(ts.model.VeniceNodes().Leader())).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())
			}
		})
	})

})
