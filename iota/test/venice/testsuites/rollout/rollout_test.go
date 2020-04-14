package rollout_test

import (
	"errors"
	"time"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("rollout tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
			_, err := ts.model.RunNaplesCommand(nc, "touch /data/upgrade_to_same_firmware_allowed")
			Expect(err).ShouldNot(HaveOccurred())
			return nil
		})
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
			ts.model.RunNaplesCommand(nc, "rm /data/upgrade_to_same_firmware_allowed")
			return nil
		})

	})

	Context("Iota Rollout tests", func() {

		It("Perform Rollout", func() {
			var workloadPairs *objects.WorkloadPairCollection
			rollout, err := ts.model.GetRolloutObject(ts.scaleData)
			Expect(err).ShouldNot(HaveOccurred())

			if *simOnlyFlag == false {
				workloadPairs = ts.model.WorkloadPairs().WithinNetwork().Any(40)
				log.Infof(" Length workloadPairs %v", len(workloadPairs.ListIPAddr()))
				Expect(len(workloadPairs.ListIPAddr()) != 0).Should(BeTrue())
			}

			err = ts.model.PerformRollout(rollout, ts.scaleData, "upgrade-bundle")
			Expect(err).ShouldNot(HaveOccurred())
			rerr := make(chan bool)
			if *simOnlyFlag == false {
				go func() {
					options := &objects.ConnectionOptions{
						Duration:          "180s",
						Port:              "8000",
						Proto:             "tcp",
						ReconnectAttempts: 100,
					}
					_ = ts.model.ConnectionWithOptions(workloadPairs, options)
					log.Infof("TCP SESSION TEST COMPLETE")
					rerr <- true
					return
				}()
			}

			// verify rollout is successful
			Eventually(func() error {
				return ts.model.VerifyRolloutStatus(rollout.Name)
			}).Should(Succeed())
			log.Infof("Rollout Completed. Waiting for Fuz tests to complete..")

			if *simOnlyFlag == false {
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
			}
			return

		})
	})
})
