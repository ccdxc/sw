package cluster_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit"
)

var _ = Describe("auth tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})
	It("tags:type=basic;datapath=false;duration=short same token works on all nodes", func() {
		// get token by logging in to leader
		Expect(ts.model.Action().VeniceNodeLoggedInCtx(ts.model.VeniceNodes().Leader())).Should(Succeed())
		ts.model.ForEachVeniceNode(func(vnc *iotakit.VeniceNodeCollection) error {
			Eventually(func() error {
				return ts.model.Action().VeniceNodeGetCluster(vnc)
			}).Should(Succeed())
			return nil
		})
	})
})
