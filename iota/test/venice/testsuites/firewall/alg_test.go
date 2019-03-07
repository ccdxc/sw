// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("firewall ALG tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.SGPolicy("default-policy").Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()

		// delete test policy if its left over. we can ignore the error here
		ts.model.SGPolicy("test-policy").Delete()
		ts.model.SGPolicy("default-policy").Delete()

		// recreate default allow policy
		Expect(ts.model.NewSGPolicy("default-policy").AddRule("any", "any", "", "PERMIT").Commit()).ShouldNot(HaveOccurred())
	})
	Context("ALG tests", func() {
		It("Should be able to FTP get with FTP ALG policy", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling FTP ALG test on naples sim till traffic issue is debugged")
			}

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)

			// configure policy without ALG
			spc := ts.model.NewSGPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "tcp/21", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify FTP get fails
			Eventually(func() error {
				return ts.model.Action().FTPGetFails(workloadPairs)
			}).Should(Succeed())

			// configure policy with ALG
			Expect(spc.Delete()).Should(Succeed())
			spc = ts.model.NewSGPolicy("test-policy").AddAlgRulesForWorkloadPairs(workloadPairs, "ftp-alg", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify FTP get succeeds now
			Eventually(func() error {
				return ts.model.Action().FTPGet(workloadPairs)
			}).Should(Succeed())

			// verify FTP get fails in reverse direction
			Eventually(func() error {
				return ts.model.Action().FTPGetFails(workloadPairs.ReversePairs())
			}).Should(Succeed())
		})

		It("Should be able to update FTP ALG policy", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling FTP ALG test on naples sim till traffic issue is debugged")
			}

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)

			// configure policy without ALG
			spc := ts.model.NewSGPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "tcp/21", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify FTP get fails
			Eventually(func() error {
				return ts.model.Action().FTPGetFails(workloadPairs)
			}).Should(Succeed())

			// modify policy with ALG
			spc.DeleteAllRules().AddAlgRulesForWorkloadPairs(workloadPairs, "ftp-alg", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify FTP get succeeds now
			Eventually(func() error {
				return ts.model.Action().FTPGet(workloadPairs)
			}).Should(Succeed())

			// verify FTP get fails in reverse direction
			Eventually(func() error {
				return ts.model.Action().FTPGetFails(workloadPairs.ReversePairs())
			}).Should(Succeed())
		})
		It("Should be able to verify ICMP ALG", func() {
			Skip("Disabling ICMP ALG test till traffic issue is debugged")
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			// verify ping fails
			Eventually(func() error {
				return ts.model.Action().PingFails(workloadPairs)
			}).Should(Succeed())

			// configure policy with ICMP ALG
			spc := ts.model.NewSGPolicy("test-policy").AddAlgRulesForWorkloadPairs(workloadPairs, "icmp-echo-resp", "PERMIT")
			spc = spc.AddAlgRulesForWorkloadPairs(workloadPairs.ReversePairs(), "icmp-echo-req", "PERMIT")

			Expect(spc.Commit()).Should(Succeed())

			// verify ping is successful
			Eventually(func() error {
				return ts.model.Action().PingPairs(workloadPairs)
			}).Should(Succeed())

			/* FIXME: disable this part till datapath issue is debugged
			// verify pig fails in reverse direction
			Eventually(func() error {
				return ts.model.Action().PingFails(workloadPairs.ReversePairs())
			}).Should(Succeed())
			*/
		})
	})
})
