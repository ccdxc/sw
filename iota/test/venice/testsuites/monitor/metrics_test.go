package monitor_test

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("metrics test", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})

	Context("tags:type=basic;datapath=true;duration=short Verify flowdropmetrics ", func() {
		It("tags:sanity=true Flow drops should increament", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling flow drop stats on naples sim")
			}
			//spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "any", "PERMIT")
			//Expect(spc.Commit()).ShouldNot(HaveOccurred())

			//Verif policy was propagted correctly

			startTime := time.Now().UTC()
			// establish TCP session between workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			log.Infof("wait to get to the console")
			time.Sleep(time.Second * 30)
			Eventually(func() error {
				return ts.model.Action().DropIcmpFlowTTLSession(workloadPairs, "--icmp --icmptype 4 --count 1")
			}).Should(Succeed())
			time.Sleep(time.Second * 30)

			// check fwlog, enable when fwlogs are reported to Venice
			/*
				Eventually(func() error {
					return ts.model.Action().FindFwlogForWorkloadPairs("ICMP", "allow", startTime.String(), 0, workloadPairs.ReversePairs())
				}).Should(Succeed())
			*/

			Eventually(func() error {
				return ts.model.Action().QueryDropMetricsForWorkloadPairs(workloadPairs, startTime.String())
			}).Should(Succeed())

		})
	})
})
