package basenet_test

import (
	"context"
	"fmt"

	"github.com/pensando/sw/iota/test/venice/iotakit/model"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("Basnet Sanity", func() {

	var dscInsertionProfile *objects.DscProfile
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
		dscInsertionProfile = objects.NewDscProfileInsertion(ts.model.ConfigClient(), "dscInsertion")
		Expect(dscInsertionProfile.Commit()).Should(Succeed())
	})
	AfterEach(func() {
		//Delete if insertion profile exists
		dscInsertionProfile.Delete()
		ts.tb.AfterTestCommon()
	})

	Context("Basenet Tests", func() {

		It("Reset naples and expected to join back cluster", func() {
			for i := 0; i < int(ts.stress); i++ {
				log.Infof("Resetting Naples iteration %v", i+1)
				Expect(ts.model.ResetNaplesNodes(ts.model.Hosts())).Should(Succeed())
				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.VerifyClusterStatus()
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
	})
})
