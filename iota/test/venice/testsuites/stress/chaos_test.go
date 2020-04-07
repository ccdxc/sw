// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package stress_test

import (
	"context"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	//"github.com/pensando/sw/iota/test/venice/iotakit"
)

var _ = Describe("Scale Chaos tests", func() {

	const (
		linkDownTime = 60 * time.Second
		flapInterval = 2 * time.Minute
	)
	var (
		flapPortCancel context.CancelFunc
	)
	BeforeEach(func() {
	})

	AfterEach(func() {

	})

	AfterSuite(func() {
		//Cancel flap ports if running
		if flapPortCancel != nil {
			flapPortCancel()
			flapPortCancel = nil
		}
	})

	Context("Scale Chaos tests", func() {

		startPortFlap := func() {
			sc, err := ts.model.SwitchPorts().SelectByPercentage(75)
			Expect(err).ShouldNot(HaveOccurred())

			ctx, cancel := context.WithCancel(context.Background())
			flapPortCancel = cancel
			go ts.model.FlapDataSwitchPortsPeriodically(ctx, sc,
				linkDownTime, flapInterval, 0)

		}

		stopPortFlap := func() {
			flapPortCancel()
			flapPortCancel = nil

		}

		runTraffic := func() error {

			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			options := &objects.ConnectionOptions{
				Cps:               10,
				Duration:          "120s",
				NumConns:          100,
				Port:              "0",
				Proto:             "tcp",
				ReconnectAttempts: 25,
			}

			return ts.model.ConnectionWithOptions(workloadPairs, options)
		}

		configPushCheck := func() (time.Duration, error) {
			startTime := time.Now()
			iter := 1
			for ; iter <= 3000; iter++ {
				//Check every second
				time.Sleep(time.Second)
				complete, err := ts.model.IsConfigPushComplete()
				if complete && err == nil {
					return time.Since(startTime), nil
				}
			}
			return 0, fmt.Errorf("Config push incomplete")
		}

		doDeleteAdd := func(retErr chan error) {

			err := ts.model.CleanupAllConfig()
			if err != nil {
				retErr <- err
				return
			}

			_, err = configPushCheck()
			if err != nil {
				retErr <- err
				return
			}

			err = ts.model.SetupDefaultConfig(context.Background(), ts.scaleData, ts.scaleData)
			if err != nil {
				retErr <- err
				return
			}

			_, err = configPushCheck()
			if err != nil {
				retErr <- err
				return
			}

			retErr <- nil
		}

		rebootFakeHosts := func(err chan error) {
			err <- ts.model.ReloadFakeHosts(ts.model.Hosts())
		}
		It("Flap links , run traffic make sure cluster in good state", func() {
			if !ts.scaleData || ts.tb.HasNaplesSim() {
				Skip("Skipping scale connection runs")
			}

			startPortFlap()

			//Ignore connection issues as ports are flapping
			runTraffic()

			stopPortFlap()

			//Verify cluster in good state after that.
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

			//Links should be up now, make sure traffic in good state after.
			Expect(runTraffic()).ShouldNot(HaveOccurred())

			//Verify cluster in good state after that.
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Flap links , reboot Venice nodes, run traffic make sure cluster in good state", func() {
			if !ts.scaleData || ts.tb.HasNaplesSim() {
				Skip("Skipping scale connection runs")
			}

			startPortFlap()

			//Reload venice Nodes
			ts.model.ForEachVeniceNode(func(vnc *objects.VeniceNodeCollection) error {
				Expect(ts.model.ReloadVeniceNodes(vnc)).Should(Succeed())

				return nil
			})

			//Ignore connection issues as ports are flapping
			runTraffic()

			stopPortFlap()

			//Verify cluster in good state after that.
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

			//Links should be up now, make sure traffic in good state after.
			Expect(runTraffic()).ShouldNot(HaveOccurred())

			//Verify cluster in good state after that.
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Flap links , reboot Sim nodes, run traffic make sure cluster in good state", func() {
			if !ts.scaleData || ts.tb.HasNaplesSim() {
				Skip("Skipping scale connection runs")
			}

			startPortFlap()

			rebootDone := make(chan error)
			go rebootFakeHosts(rebootDone)

			//Ignore connection issues as ports are flapping
			runTraffic()

			rebootErr := <-rebootDone
			Expect(rebootErr).ShouldNot(HaveOccurred())

			stopPortFlap()

			//Verify cluster in good state after that.
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

			//Links should be up now, make sure traffic in good state after.
			Expect(runTraffic()).ShouldNot(HaveOccurred())

			//Verify cluster in good state after that.
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Flap links , reboot Sim nodes, run traffic and endpoint and policy updates make sure cluster in good state", func() {
			if !ts.scaleData || ts.tb.HasNaplesSim() {
				Skip("Skipping scale connection runs")
			}

			startPortFlap()

			rebootDone := make(chan error)
			delAddDone := make(chan error)
			go rebootFakeHosts(rebootDone)

			go doDeleteAdd(delAddDone)

			//Ignore connection issues as ports are flapping
			runTraffic()

			rebootErr := <-rebootDone
			Expect(rebootErr).ShouldNot(HaveOccurred())

			delAddErr := <-delAddDone
			Expect(delAddErr).ShouldNot(HaveOccurred())

			stopPortFlap()

			//Verify cluster in good state after that.
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

			//Links should be up now, make sure traffic in good state after.
			Expect(runTraffic()).ShouldNot(HaveOccurred())

			//Links should be up now, make sure traffic in good state after.
			//Verify cluster in good state after that.
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

		})
	})

})
