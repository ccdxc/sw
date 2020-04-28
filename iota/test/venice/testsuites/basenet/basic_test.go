package basenet_test

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/iota/test/venice/iotakit/model"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("Basnet Sanity", func() {

	dataPathEnabled := true
	var dscInsertionProfile *objects.DscProfile
	var dscFlowawareProfile *objects.DscProfile
	var dscEnforcedProfile *objects.DscProfile
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
		dscInsertionProfile = objects.NewDscProfileInsertion(ts.model.ConfigClient(), "dscInsertion")
		Expect(dscInsertionProfile.Commit()).Should(Succeed())
		dscFlowawareProfile = objects.NewDscProfileFlowAware(ts.model.ConfigClient(), "dscFlowaware")
		Expect(dscFlowawareProfile.Commit()).Should(Succeed())
		dscEnforcedProfile = objects.NewDscProfileEnforced(ts.model.ConfigClient(), "dscEnforced")
		Expect(dscEnforcedProfile.Commit()).Should(Succeed())
		if *runFwLogs {
			log.Infof("user requested to start fwlogs")
			err := ts.model.StopFWLogGenOnNaples(ts.model.Naples())
			Expect(err).Should(Succeed())
			err = ts.model.StartFWLogGenOnNaples(ts.model.Naples(), "5", "50000")
			Expect(err).Should(Succeed())
		}
	})
	AfterEach(func() {
		if *runFwLogs {
			ts.model.VerifyFwlogErrors()
			err := ts.model.StopFWLogGenOnNaples(ts.model.Naples())
			Expect(err).Should(Succeed())
		}
		//Delete if insertion profile exists
		ts.model.CleanupAllConfig()
		dscInsertionProfile.Delete()
		dscFlowawareProfile.Delete()
		dscEnforcedProfile.Delete()
		ts.model.AfterTestCommon()
	})

	Context("Basenet Tests", func() {

		runDataPath := func(workloadPairs *objects.WorkloadPairCollection,
			veniceCollector *objects.VeniceNodeCollection) error {
			ctx, cancel := context.WithCancel(context.Background())
			tcpdumpDone := make(chan error)
			var output int
			go func() {
				var err error
				output, err = veniceCollector.GetGRETCPDumpCount(ctx)
				tcpdumpDone <- err

			}()

			//Sleep for a while so that tcpdump starts
			time.Sleep(2 * time.Second)
			ts.model.PingPairs(workloadPairs)
			cancel()
			err := <-tcpdumpDone

			if err != nil {
				return fmt.Errorf("Error running command.")
			}

			if output == 0 {
				return fmt.Errorf("Did not receive mirror packets")
			}

			return nil
		}

		verifyDatapathCollection := func(workloadPairs *objects.WorkloadPairCollection,
			veniceCollector *objects.VeniceNodeCollection) error {
			if !dataPathEnabled {
				return nil
			}
			return runDataPath(workloadPairs, veniceCollector)
		}

		verifyNoDatapathCollection := func(workloadPairs *objects.WorkloadPairCollection,
			veniceCollector *objects.VeniceNodeCollection) error {
			if !dataPathEnabled {
				return nil
			}
			err := runDataPath(workloadPairs, veniceCollector)
			if err != nil {
				return nil
			}
			return fmt.Errorf("Collector still found packets..")
		}

		It("Move Naples to flow aware mode and make sure cluster is normal and reset", func() {
			for i := 0; i < int(ts.stress); i++ {

				//Make sure flow aware mirroing works
				veniceCollector := ts.model.VeniceNodes().Leader()
				// add permit rules for workload pairs
				workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
				msc := ts.model.NewMirrorSession("test-mirror").AddRulesForWorkloadPairs(workloadPairs, "icmp")
				msc.AddVeniceCollector(veniceCollector, "udp/4545", 0)
				Expect(msc.Commit()).Should(Succeed())

				Eventually(func() error {
					return verifyNoDatapathCollection(workloadPairs, veniceCollector)
				}).Should(Succeed())

				Expect(ts.model.Naples().SetDscProfile(dscFlowawareProfile)).Should(Succeed())
				//Ping should succeed
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).Should(Succeed())

				Eventually(func() error {
					return verifyDatapathCollection(workloadPairs, veniceCollector)
				}).Should(Succeed())

				Expect(ts.model.Naples().Decommission()).Should(Succeed())
				Eventually(func() error {
					admit, _ := ts.model.Naples().IsAdmitted()
					if !admit {
						return nil
					}
					return fmt.Errorf("Naples still admitted")
				}).Should(Succeed())

				Expect(ts.model.Naples().ResetProfile()).Should(Succeed())
				Expect(ts.model.Naples().Admit()).Should(Succeed())
				//reload Nodes so that it can Join
				Eventually(func() error {
					return ts.model.ReloadHosts(ts.model.Hosts())
				}).Should(Succeed())

				Eventually(func() error {
					admit, err := ts.model.Naples().IsAdmitted()
					if err == nil && admit {
						return nil
					}
					return fmt.Errorf("Naples not admitted")
				}).Should(Succeed())

				Eventually(func() error {
					return verifyNoDatapathCollection(workloadPairs, veniceCollector)
				}).Should(Succeed())

				//Ping should work as expected
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).Should(Succeed())

			}
		})

		It("Move naples to insertion mode make sure traffic stops and restore it back with reset", func() {

			//totalNaples := len(ts.model.Naples().Nodes) + len(ts.model.Naples().FakeNodes)
			for i := 0; i < int(ts.stress); i++ {
				log.Infof("Basenet to Insertion mode iteration :  %v", i+1)
				Expect(ts.model.Naples().SetDscProfile(dscInsertionProfile)).Should(Succeed())

				//Ping should fail as expected
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).ShouldNot(Succeed())

				Expect(ts.model.Naples().Decommission()).Should(Succeed())
				Eventually(func() error {
					admit, _ := ts.model.Naples().IsAdmitted()
					if !admit {
						return nil
					}
					return fmt.Errorf("Naples still admitted")
				}).Should(Succeed())

				Expect(ts.model.Naples().ResetProfile()).Should(Succeed())
				Expect(ts.model.Naples().Admit()).Should(Succeed())
				//reload Nodes so that it can Join
				Eventually(func() error {
					return ts.model.ReloadHosts(ts.model.Hosts())
				}).Should(Succeed())

				Eventually(func() error {
					admit, err := ts.model.Naples().IsAdmitted()
					if err == nil && admit {
						return nil
					}
					return fmt.Errorf("Naples not admitted")
				}).Should(Succeed())

				//Ping should work as expected
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).Should(Succeed())

			}

		})

		It("Naples to insertion mode, Push new config and make sure traffic good and reset to basnet", func() {

			var err error
			for i := 0; i < int(ts.stress); i++ {

				Expect(ts.model.Naples().SetDscProfile(dscInsertionProfile)).Should(Succeed())

				//Ping should fail as expected
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).ShouldNot(Succeed())

				workloads := ts.model.Workloads()
				Expect(ts.model.TeardownWorkloads(workloads)).Should(Succeed())

				ts.model, err = model.ReinitSysModel(ts.model, common.DefaultModel)
				Expect(err).ShouldNot(HaveOccurred())

				err = ts.model.SetupDefaultConfig(context.Background(), false, false)
				Expect(err).ShouldNot(HaveOccurred())

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)

				err = ts.model.DefaultNetworkSecurityPolicy().Delete()
				Expect(err).ShouldNot(HaveOccurred())
				spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "icmp", "PERMIT")
				spc.AddRulesForWorkloadPairs(workloadPairs.ReversePairs(), "icmp", "PERMIT")
				Expect(spc.Commit()).Should(Succeed())

				//Ping should succed as expected
				Eventually(func() error {
					return ts.model.PingPairs(workloadPairs)
				}).Should(Succeed())

				Expect(ts.model.CleanupAllConfig()).Should(Succeed())
				//Ping should not succeed
				Eventually(func() error {
					return ts.model.PingPairs(workloadPairs)
				}).ShouldNot(Succeed())

				Expect(ts.model.Naples().Decommission()).Should(Succeed())
				Eventually(func() error {
					admit, _ := ts.model.Naples().IsAdmitted()
					if !admit {
						return nil
					}
					return fmt.Errorf("Naples still admitted")
				}).Should(Succeed())
				workloads = ts.model.Workloads()
				Expect(ts.model.TeardownWorkloads(workloads)).Should(Succeed())

				//Reload Nodes so that it can Join
				Expect(ts.model.Naples().ResetProfile()).Should(Succeed())
				Expect(ts.model.Naples().Admit()).Should(Succeed())
				Eventually(func() error {
					return ts.model.ReloadHosts(ts.model.Hosts())
				}).Should(Succeed())

				Eventually(func() error {
					admit, err := ts.model.Naples().IsAdmitted()
					if err == nil && admit {
						return nil
					}
					return fmt.Errorf("Naples not admitted")
				}).Should(Succeed())

				ts.model, err = model.ReinitSysModel(ts.model, common.BaseNetModel)
				Expect(err).ShouldNot(HaveOccurred())

				err = ts.model.SetupDefaultConfig(context.Background(), false, false)
				Expect(err).ShouldNot(HaveOccurred())

				workloadPairs = ts.model.WorkloadPairs().WithinNetwork().Any(2)

				//Ping should work as expected
				Eventually(func() error {
					return ts.model.PingPairs(workloadPairs)
				}).Should(Succeed())

			}

		})
		It("Naples to transparent enforced mode, Push new config and make sure traffic good and reset to basnet", func() {

			var err error
			for i := 0; i < int(ts.stress); i++ {

				Expect(ts.model.Naples().SetDscProfile(dscEnforcedProfile)).Should(Succeed())

				//Ping should fail as expected
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).ShouldNot(Succeed())

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)

				Expect(err).ShouldNot(HaveOccurred())
				spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "icmp", "PERMIT")
				spc.AddRulesForWorkloadPairs(workloadPairs.ReversePairs(), "icmp", "PERMIT")
				Expect(spc.Commit()).Should(Succeed())

				//Ping should succed as expected
				Eventually(func() error {
					return ts.model.PingPairs(workloadPairs)
				}).Should(Succeed())

				Expect(ts.model.CleanupAllConfig()).Should(Succeed())

				//Ping should not succeed
				Eventually(func() error {
					return ts.model.PingPairs(workloadPairs)
				}).ShouldNot(Succeed())

				Expect(ts.model.Naples().Decommission()).Should(Succeed())
				Eventually(func() error {
					admit, _ := ts.model.Naples().IsAdmitted()
					if !admit {
						return nil
					}
					return fmt.Errorf("Naples still admitted")
				}).Should(Succeed())

				Expect(ts.model.Naples().ResetProfile()).Should(Succeed())
				Expect(ts.model.Naples().Admit()).Should(Succeed())
				//reload Nodes so that it can Join
				Eventually(func() error {
					return ts.model.ReloadHosts(ts.model.Hosts())
				}).Should(Succeed())

				Eventually(func() error {
					admit, err := ts.model.Naples().IsAdmitted()
					if err == nil && admit {
						return nil
					}
					return fmt.Errorf("Naples not admitted")
				}).Should(Succeed())

				//Ping should work as expected
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).Should(Succeed())

			}

		})

		It("Move Naples to flow aware mode to insertion mode and make sure cluster is normal and reset", func() {
			var err error
			for i := 0; i < int(ts.stress); i++ {
				veniceCollector := ts.model.VeniceNodes().Leader()
				// add permit rules for workload pairs
				workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
				msc := ts.model.NewMirrorSession("test-mirror").AddRulesForWorkloadPairs(workloadPairs, "icmp")
				msc.AddVeniceCollector(veniceCollector, "udp/4545", 0)
				Expect(msc.Commit()).Should(Succeed())

				Eventually(func() error {
					return verifyNoDatapathCollection(workloadPairs, veniceCollector)
				}).Should(Succeed())

				Expect(ts.model.Naples().SetDscProfile(dscFlowawareProfile)).Should(Succeed())
				//Ping should succeed
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).Should(Succeed())

				Eventually(func() error {
					return verifyDatapathCollection(workloadPairs, veniceCollector)
				}).Should(Succeed())

				Expect(ts.model.Naples().SetDscProfile(dscInsertionProfile)).Should(Succeed())

				//Ping should fail as expected
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).ShouldNot(Succeed())

				workloads := ts.model.Workloads()
				Expect(ts.model.TeardownWorkloads(workloads)).Should(Succeed())

				ts.model, err = model.ReinitSysModel(ts.model, common.DefaultModel)
				Expect(err).ShouldNot(HaveOccurred())

				err := ts.model.SetupDefaultConfig(context.Background(), false, false)
				Expect(err).ShouldNot(HaveOccurred())

				workloadPairs = ts.model.WorkloadPairs().WithinNetwork().Any(4)

				err = ts.model.DefaultNetworkSecurityPolicy().Delete()
				Expect(err).ShouldNot(HaveOccurred())
				spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "icmp", "PERMIT")
				spc.AddRulesForWorkloadPairs(workloadPairs.ReversePairs(), "icmp", "PERMIT")
				Expect(spc.Commit()).Should(Succeed())

				//Ping should succed as expected
				Eventually(func() error {
					return ts.model.PingPairs(workloadPairs)
				}).Should(Succeed())

				Expect(ts.model.CleanupAllConfig()).Should(Succeed())
				//Ping should not succeed
				Eventually(func() error {
					return ts.model.PingPairs(workloadPairs)
				}).ShouldNot(Succeed())

				Expect(ts.model.Naples().Decommission()).Should(Succeed())
				Eventually(func() error {
					admit, _ := ts.model.Naples().IsAdmitted()
					if !admit {
						return nil
					}
					return fmt.Errorf("Naples still admitted")
				}).Should(Succeed())

				Expect(ts.model.Naples().ResetProfile()).Should(Succeed())
				Expect(ts.model.Naples().Admit()).Should(Succeed())

				workloads = ts.model.Workloads()
				Expect(ts.model.TeardownWorkloads(workloads)).Should(Succeed())

				ts.model, err = model.ReinitSysModel(ts.model, common.BaseNetModel)
				Expect(err).ShouldNot(HaveOccurred())

				err = ts.model.SetupDefaultConfig(context.Background(), false, false)
				Expect(err).ShouldNot(HaveOccurred())

				//reload Nodes so that it can Join
				Eventually(func() error {
					return ts.model.ReloadHosts(ts.model.Hosts())
				}).Should(Succeed())

				Eventually(func() error {
					admit, err := ts.model.Naples().IsAdmitted()
					if err == nil && admit {
						return nil
					}
					return fmt.Errorf("Naples not admitted")
				}).Should(Succeed())

				Eventually(func() error {
					return verifyNoDatapathCollection(workloadPairs, veniceCollector)
				}).Should(Succeed())

				//Ping should work as expected
				Eventually(func() error {
					return ts.model.PingPairs(ts.model.WorkloadPairs().WithinNetwork())
				}).Should(Succeed())

			}
		})

	})
})
