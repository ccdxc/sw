// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package mirror_test

import (
	"context"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("mirror tests", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})
	Context("Mirror tests", func() {
		It("tags:sanity=true tags:type=basic;datapath=true;duration=short Mirror packets to collector and check TCPDUMP", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			veniceCollector := ts.model.VeniceNodes().Leader()
			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("test-mirror").AddRulesForWorkloadPairs(workloadPairs, "icmp/0/0")
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 0)
			Expect(msc.Commit()).Should(Succeed())

			ctx, cancel := context.WithCancel(context.Background())
			tcpdumpDone := make(chan error)
			var output string
			go func() {
				var err error
				output, err = veniceCollector.CaptureGRETCPDump(ctx)
				tcpdumpDone <- err

			}()

			//Sleep for a while so that tcpdump starts
			time.Sleep(2 * time.Second)
			ts.model.Action().PingPairs(workloadPairs)
			cancel()
			<-tcpdumpDone

			Expect(strings.Contains(output, "GREv0, length")).Should(BeTrue())

			// Clear collectors
			msc.ClearCollectors()

			// Update the collector
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			ctx, cancel = context.WithCancel(context.Background())
			tcpdumpDone = make(chan error)
			go func() {
				var err error
				output, err = veniceCollector.CaptureGRETCPDump(ctx)
				tcpdumpDone <- err

			}()

			//Sleep for a while so that tcpdump starts
			time.Sleep(2 * time.Second)
			ts.model.Action().PingPairs(workloadPairs)
			cancel()
			<-tcpdumpDone

			Expect(strings.Contains(output, "GREv0, length")).Should(BeTrue())

			// Delete the Mirror session
			Expect(msc.Delete()).Should(Succeed())

		})
	})
})
