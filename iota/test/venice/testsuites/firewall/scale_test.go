// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("firewall scale tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
			_, err := ts.model.RunNaplesCommand(nc, "/nic/bin/halctl debug trace --level error")
			Expect(err).ShouldNot(HaveOccurred())
			return nil
		})
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})

	AfterEach(func() {
		ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
			_, err := ts.model.RunNaplesCommand(nc, "/nic/bin/halctl debug trace --level debug")
			Expect(err).ShouldNot(HaveOccurred())
			return nil
		})
		ts.tb.AfterTestCommon()
	})

	Context("tags:type=basic;datapath=true;duration=long Scale TCP connections tests", func() {
		It("Should establish bulk TCP session between allowed workloads in scale config", func() {
			log.Infof("scaleData = %v hasNaples = %v", ts.scaleData, ts.tb.HasNaplesSim())
			if !ts.scaleData || ts.tb.HasNaplesSim() {
				Skip("Skipping scale connection runs")
			}

			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Expect(ts.model.FuzIt(workloadPairs, 100, "tcp", "0")).ShouldNot(HaveOccurred())

		})

		It("Should not establish bulk TCP session between denied workloads in scale config", func() {
			log.Infof("scaleData = %v hasNaples = %v", ts.scaleData, ts.tb.HasNaplesSim())
			if !ts.scaleData || ts.tb.HasNaplesSim() {
				Skip("Skipping scale connection runs")
			}

			workloadPairs := ts.model.WorkloadPairs().Deny(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Expect(ts.model.FuzIt(workloadPairs, 100, "tcp", "0")).Should(HaveOccurred())

		})
	})
})
