package basenet_test

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("events tests", func() {
	var startTime time.Time

	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.model.AfterTestCommon()
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})

	Context("Basic events tests", func() {
		It("nevtsproxy should be running all the naples nodes", func() {
			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				out, err := ts.model.RunNaplesCommand(nc, "ps aux | grep [n]evtsproxy")
				Expect(err).ShouldNot(HaveOccurred())
				Expect(len(out) == 1).Should(BeTrue())
				Expect(out[0]).ShouldNot(BeEmpty())
				return nil
			})
		})

		It("check for basic bootup event", func() {
			Skip("Skipping as this is failing")
			if ts.tb.HasNaplesSim() {
				Skip("link flap cannot be run on NAPLES sim")
			}

			npc := ts.model.Naples()

			// ensures the link events are triggered and available in venice
			ec := ts.model.SystemBootEvents(npc)
			log.Debugf("%v", ec)

			Expect(ec.Error()).ShouldNot(HaveOccurred())
			Expect(ec.Len(len(npc.Nodes))).Should(BeTrue())

		})

		It("Link flap should trigger an event from hal/linkmgr", func() {
			Skip("link flap cannot be run on NAPLES sim")
			log.Infof("Sim: %v Hw %v", ts.tb.HasNaplesSim(), ts.tb.HasNaplesHW())
			if !ts.tb.HasNaplesHW() {
				//Skip("link flap cannot be run on NAPLES sim")
			}
			//Naples time in UTC
			startTime := time.Now().UTC()
			startTime = startTime.Add(-3 * time.Minute) // TODO: remove this; there is ~2 to ~3 minute delay between naples sim and rund VM

			// get a random naples and flap the port
			npc := ts.model.Naples()
			nc := npc.Any(1)
			Expect(nc.Error()).ShouldNot(HaveOccurred())
			Expect(ts.model.PortFlap(nc)).Should(Succeed())
			time.Sleep(60 * time.Second) // wait for the event to reach venice

			// ensures the link events are triggered and available in venice
			ec := ts.model.LinkUpEventsSince(startTime, npc)
			Expect(ec.Error()).ShouldNot(HaveOccurred())
			Expect(ec.LenGreaterThanEqualTo(1)).Should(BeTrue())

			ec = ts.model.LinkDownEventsSince(startTime, npc)
			Expect(ec.Error()).ShouldNot(HaveOccurred())
			Expect(ec.LenGreaterThanEqualTo(1)).Should(BeTrue())

			// verify ping is successful across all workloads after the port flap
			Eventually(func() error {
				return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
			}).Should(Succeed())
		})
	})
})
