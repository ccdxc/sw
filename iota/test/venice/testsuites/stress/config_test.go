// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package stress_test

import (
	"context"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("Config Stress tests", func() {

	var (
	//fwLogCmdCtx  interface{}
	//eventsCmdCtx interface{}
	//err          error
	)
	BeforeEach(func() {
		/*
			// verify cluster is in good health
			ts.model.ForEachNaples(func(nc *iotakit.NaplesCollection) error {
				_, err := ts.model.RunNaplesCommand(nc, "/nic/bin/halctl debug trace --level error")
				Expect(err).ShouldNot(HaveOccurred())
				return nil
			})
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

			//Start fwlog
			ts.model.ForEachFakeNaples(func(nc *iotakit.NaplesCollection) error {
				fwLogCmdCtx, err = ts.model.RunFakeNaplesBackgroundCommand(nc,
					"LD_LIBRARY_PATH=/naples/nic/lib/:/naples/nic/lib64/ /naples/nic/bin/fwloggen -metrics -rate 25 -num  5000000")
				Expect(err).ShouldNot(HaveOccurred())
				Expect(fwLogCmdCtx != nil).Should(BeTrue())
				return nil
			})

			//do skip setup
			ts.tb.DoSkipSetup()

			//Start events and logs.
			ts.model.ForEachFakeNaples(func(nc *iotakit.NaplesCollection) error {
				eventsCmdCtx, err = ts.model.RunFakeNaplesBackgroundCommand(nc,
					"LD_LIBRARY_PATH=/naples/nic/lib/:/naples/nic/lib64/ /naples/nic/bin/gen_events -r 1 -t 200000 -s \"scale-testing\"")
				Expect(err).ShouldNot(HaveOccurred())
				Expect(eventsCmdCtx != nil).Should(BeTrue())
				return nil
			})
		*/
	})

	AfterEach(func() {
		/*ts.model.ForEachNaples(func(nc *iotakit.NaplesCollection) error {
			_, err := ts.model.RunNaplesCommand(nc, "/nic/bin/halctl debug trace --level debug")
			Expect(err).ShouldNot(HaveOccurred())
			return nil
		})

		_, err = ts.model.StopCommands(eventsCmdCtx)
		//Expect(err).ShouldNot(HaveOccurred())
		_, err = ts.model.StopCommands(fwLogCmdCtx)
		//Expect(err).ShouldNot(HaveOccurred())
		*/
	})

	Context("Delete and Add scale configs", func() {
		It("Do delete/add and run-traffic", func() {

			Skip("Skipping scale connection runs")
			log.Infof("scaleData = %v hasNaples = %v", ts.scaleData, ts.tb.HasNaplesSim())
			if !ts.scaleData || ts.tb.HasNaplesSim() {
				Skip("Skipping scale connection runs")
			}

			//ts.model.ResetConfigStats()
			//ts.model.ClearConfigPushStat()
			configPushCheck := func() (time.Duration, error) {
				startTime := time.Now()
				iter := 1
				for ; iter <= 2000; iter++ {
					//Check every second
					time.Sleep(time.Second)
					complete, err := ts.model.IsConfigPushComplete()
					if complete && err == nil {
						return time.Since(startTime), nil
					}
				}
				return 0, fmt.Errorf("Config push incomplete")
			}

			totalCfgPushTime := time.Duration(0)
			totalCfgCleanTime := time.Duration(0)
			totalIterations := 0
			defer func() {
				log.Infof("Average Config Push time : %v",
					time.Duration(int64(totalCfgPushTime)/int64(totalIterations)))
				log.Infof("Average Config Cleanup time : %v",
					time.Duration(int64(totalCfgCleanTime)/int64(totalIterations)))
			}()
			if ts.stress == 0 {
				ts.stress = 1
			}
			for i := uint64(0); i < ts.stress; i++ {
				totalIterations++
				ctx, cancel := context.WithTimeout(context.TODO(), 30*time.Minute)
				defer cancel()
				// fully clean up venice/iota config before starting the tests
				err := ts.model.CleanupAllConfig()
				Expect(err).ShouldNot(HaveOccurred())

				cleanUpTime, err := configPushCheck()
				Expect(err).ShouldNot(HaveOccurred())

				log.Infof("Config clean time : %v", cleanUpTime)
				// setup default config for the sysmodel
				totalCfgCleanTime += cleanUpTime
				err = ts.model.SetupDefaultConfig(ctx, ts.scaleData, ts.scaleData)
				Expect(err).ShouldNot(HaveOccurred())

				configPushTime, err := configPushCheck()
				Expect(err).ShouldNot(HaveOccurred())
				log.Infof("Config push time : %v", cleanUpTime)
				totalCfgPushTime += configPushTime
				// verify cluster, workload are in good health
				Eventually(func() error {
					return ts.model.VerifySystemHealth(false)
				}).Should(Succeed())

				workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
				Expect(ts.model.FuzIt(workloadPairs, 100, "tcp", "8000")).ShouldNot(HaveOccurred())
			}

			//ts.model.ReadConfigStats()
			//ts.model.PrintConfigPushStat()
		})

	})
})
