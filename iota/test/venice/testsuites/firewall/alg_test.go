// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("firewall ALG tests", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))

		// delete test policy if its left over. we can ignore the error here
		ts.model.NetworkSecurityPolicy("test-policy").Delete()
		ts.model.DefaultNetworkSecurityPolicy().Delete()

		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())
	})
	Context("tags:type=basic;datapath=true;duration=short  ALG tests", func() {
		It("tags:sanity=true Should be able to FTP get with FTP ALG policy", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling FTP ALG test on naples sim till traffic issue is debugged")
			}

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)

			// configure policy without ALG
			spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "tcp/21", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify FTP get fails
			Eventually(func() error {
				return ts.model.FTPGetFails(workloadPairs)
			}).Should(Succeed())

			// configure policy with ALG
			Expect(spc.Delete()).Should(Succeed())
			spc = ts.model.NewNetworkSecurityPolicy("test-policy").AddAlgRulesForWorkloadPairs(workloadPairs, "ftp-alg", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify FTP get succeeds now
			Eventually(func() error {
				return ts.model.FTPGet(workloadPairs)
			}).Should(Succeed())

			// verify FTP get fails in reverse direction
			Eventually(func() error {
				return ts.model.FTPGetFails(workloadPairs.ReversePairs())
			}).Should(Succeed())

		})

		It("Should be able to update FTP ALG policy", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling FTP ALG test on naples sim till traffic issue is debugged")
			}

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)

			// configure policy without ALG
			spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "tcp/21", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify FTP get fails
			Eventually(func() error {
				return ts.model.FTPGetFails(workloadPairs)
			}).Should(Succeed())

			// modify policy with ALG
			spc.DeleteAllRules().AddAlgRulesForWorkloadPairs(workloadPairs, "ftp-alg", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify FTP get succeeds now
			Eventually(func() error {
				return ts.model.FTPGet(workloadPairs)
			}).Should(Succeed())

			// verify FTP get fails in reverse direction
			Eventually(func() error {
				return ts.model.FTPGetFails(workloadPairs.ReversePairs())
			}).Should(Succeed())
		})
		It("Should be able to verify ICMP ALG", func() {
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			// verify ping fails
			Eventually(func() error {
				return ts.model.PingFails(workloadPairs)
			}).Should(Succeed())

			// configure policy with ICMP ALG
			spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddAlgRulesForWorkloadPairs(workloadPairs, "icmp-echo-resp", "PERMIT")
			spc = spc.AddAlgRulesForWorkloadPairs(workloadPairs.ReversePairs(), "icmp-echo-req", "PERMIT")

			Expect(spc.Commit()).Should(Succeed())

			// verify ping is successful
			Eventually(func() error {
				return ts.model.PingPairs(workloadPairs)
			}).Should(Succeed())

			/* FIXME: disable this part till datapath issue is debugged
			// verify pig fails in reverse direction
			Eventually(func() error {
				return ts.model.PingFails(workloadPairs.ReversePairs())
			}).Should(Succeed())
			*/
		})
	})
})
