package rollout_test

import (
	"errors"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit"
)

var _ = Describe("rollout tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())

		ts.model.ForEachNaples(func(nc *iotakit.NaplesCollection) error {
			_, err := ts.model.Action().RunNaplesCommand(nc, "touch /update/upgrade_to_same_firmware_allowed")
			Expect(err).ShouldNot(HaveOccurred())
			return nil
		})
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		ts.model.ForEachNaples(func(nc *iotakit.NaplesCollection) error {
			ts.model.Action().RunNaplesCommand(nc, "rm /update/upgrade_to_same_firmware_allowed")
			return nil
		})

	})

	Context("Iota Rollout tests", func() {

		It("Perform Rollout", func() {

			rollout, err := ts.model.GetRolloutObject()
			Expect(err).ShouldNot(HaveOccurred())

			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultSGPolicy(), "any")
			Expect(len(workloadPairs.ListIPAddr()) != 0).Should(BeTrue())

			err = ts.model.Action().PerformRollout(rollout)
			Expect(err).ShouldNot(HaveOccurred())

			rerr := make(chan error)

			go func() {
				rerr <- ts.model.Action().TCPSessionWithOptions(workloadPairs, 8000, "240s", 100)
			}()

			// verify rollout is successful
			Eventually(func() error {
				return ts.model.Action().VerifyRolloutStatus(rollout.Name)
			}).Should(Succeed())

			//Readd workloads
			ts.model.Action().WorkloadsSayHelloToDataPath()

			select {
			case err = <-rerr:
			case <-time.After(time.Duration(600) * time.Second):
				err = errors.New("Test timed out")
			}

			Expect(err == nil).To(Equal(true))
		})
	})
})
