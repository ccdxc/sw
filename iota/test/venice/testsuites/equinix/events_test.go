package equinix_test

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
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
		It("Link flap should trigger an event from hal/linkmgr", func() {
			// get a random naples and flap the port
			npc := ts.model.Naples()
			nc := npc.Any(1)
			Expect(nc.Error()).ShouldNot(HaveOccurred())
			Expect(ts.model.PortFlap(nc)).Should(Succeed())
			time.Sleep(60 * time.Second) // wait for the event to reach venice

			// ensures the link events are triggered and available in venice
			startTime := time.Now().UTC()
			startTime = startTime.Add(-3 * time.Minute)

			ec := ts.model.LinkDownEventsSince(startTime, npc)
			Expect(ec.Error()).ShouldNot(HaveOccurred())
			Expect(ec.LenGreaterThanEqualTo(1)).Should(BeTrue())

			ec = ts.model.LinkUpEventsSince(startTime, npc)
			Expect(ec.Error()).ShouldNot(HaveOccurred())
			Expect(ec.LenGreaterThanEqualTo(1)).Should(BeTrue())

			// verify ping is successful across all workloads after the port flap
			Eventually(func() error {
				return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
			}).Should(Succeed())
		})
		It("Events generation on sim", func() {
			// get a random naples
			npc := ts.model.Naples()
			nc := npc.Any(1)
			Expect(nc.Error()).ShouldNot(HaveOccurred())

			// generate 10 SYSTEM_COLDBOOT events
			Expect(ts.model.StartEventsGenOnNaples(nc, "10", "10")).Should(Succeed())
			time.Sleep(120 * time.Second) // wait for the event to reach venice

			// verify the events made it to Venice
			ec := ts.model.SystemBootEvents(npc)
			Expect(ec.Error()).ShouldNot(HaveOccurred())
			Expect(ec.LenGreaterThanEqualTo(1)).Should(BeTrue())
		})
	})
})
