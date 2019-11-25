package rollout_test

import (
	"errors"
	"github.com/pensando/sw/venice/utils/log"
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

			rollout, err := ts.model.GetRolloutObject(ts.scaleData)
			Expect(err).ShouldNot(HaveOccurred())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(40)
			log.Infof(" Length workloadPairs %v", len(workloadPairs.ListIPAddr()))
			Expect(len(workloadPairs.ListIPAddr()) != 0).Should(BeTrue())

			err = ts.model.Action().PerformRollout(rollout, ts.scaleData)
			Expect(err).ShouldNot(HaveOccurred())
			rerr := make(chan bool)
			go func() {
				_ = ts.model.Action().TCPSessionWithOptions(workloadPairs, 8000, "180s", 100)
				log.Infof("TCP SESSION TEST COMPLETE")
				rerr <- true
				return
			}()

			// verify rollout is successful
			Eventually(func() error {
				return ts.model.Action().VerifyRolloutStatus(rollout.Name)
			}).Should(Succeed())
			log.Infof("Rollout Completed. Waiting for Fuz tests to complete..")
			errWaitingForFuz := func() error {
				select {
				case <-rerr:
					log.Infof("Verified DataPlane using fuz connections")
					return nil
				case <-time.After(time.Duration(900) * time.Second):
					log.Infof("Timeout while waiting for fuz api to return")
					return errors.New("Timeout while waiting for fuz api to return")
				}
			}
			Expect(errWaitingForFuz()).ShouldNot(HaveOccurred())
			return

		})
	})
})
