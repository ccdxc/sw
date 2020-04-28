// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	"errors"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

var _ = Describe("firewall tests", func() {
	var startTime time.Time
	BeforeEach(func() {

		startTime = time.Now().UTC()
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})

	AfterEach(func() {

		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
		// delete test policy if its left over. we can ignore the error here
		ts.model.NetworkSecurityPolicy("test-policy").Delete()
		ts.model.DefaultNetworkSecurityPolicy().Delete()

		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())
		ts.model.AfterTestCommon()
	})

	Context("Basic firewall tests", func() {
		It("Should establish TCP session between all workload with default policy", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Eventually(func() error {
				return ts.model.TCPSession(workloadPairs, 0)
			}).Should(Succeed())
		})

		It("Should not establish TCP session between any workload with deny policy", func() {
			// change the default policy to deny all
			workloadPairs := ts.model.WorkloadPairs().Deny(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			// randomly pick one workload and verify ping fails between them
			Eventually(func() error {
				//Pass 0 to derice ports from workload pairs
				return ts.model.TCPSessionFails(workloadPairs, 0)
			}).Should(Succeed())

		})

		It("Should establish TCP session between all workload with default policy for longer duration", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				_, err := ts.model.RunNaplesCommand(nc, "/nic/bin/halctl debug trace --level error")
				Expect(err).ShouldNot(HaveOccurred())
				return nil
			})
			defer ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				_, err := ts.model.RunNaplesCommand(nc, "/nic/bin/halctl debug trace --level debug")
				Expect(err).ShouldNot(HaveOccurred())
				return nil
			})
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			err := make(chan error)

			conns := 1
			cps := 10
			duration := "30s"
			timeout := time.Duration(120) * time.Second
			if ts.scaleData {
				conns = 100
				duration = "300s"
				timeout = time.Duration(900) * time.Second

			}
			go func() {
				options := &objects.ConnectionOptions{
					Cps:               cps,
					Duration:          duration,
					NumConns:          conns,
					Port:              "0",
					Proto:             "tcp",
					ReconnectAttempts: 5,
				}
				err <- ts.model.ConnectionWithOptions(workloadPairs, options)
			}()

			select {
			case <-err:
			case <-time.After(timeout):
				err <- errors.New("Test timed out")
			}

			Expect(err != nil).To(Equal(true))
		})
	})
})
