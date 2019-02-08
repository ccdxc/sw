// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("firewall whitelist tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.SGPolicy("default-policy").Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		// delete test policy if its left over. we can ignore the error here
		ts.model.SGPolicy("test-policy").Delete()

		// recreate default allow policy
		Expect(ts.model.NewSGPolicy("default-policy").AddRule("any", "any", "", "PERMIT").Commit()).ShouldNot(HaveOccurred())
	})

	Context("basic whitelist tests", func() {
		It("Should not ping between any workload without permit rules", func() {
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)
			Eventually(func() error {
				return ts.model.Action().PingFails(workloadPairs)
			}).Should(Succeed())
		})

		It("Should allow TCP connections with specific permit rules", func() {
			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)
			spc := ts.model.NewSGPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "tcp/8000", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify TCP connection works between workload pairs
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 8000)
			}).Should(Succeed())

			// verify ping fails
			Eventually(func() error {
				return ts.model.Action().PingFails(workloadPairs)
			}).Should(Succeed())

			// verify connections in reverse direction fail
			Eventually(func() error {
				return ts.model.Action().TCPSessionFails(workloadPairs.ReversePairs(), 8000)
			}).Should(Succeed())

			// finally, delete the policy
			Expect(ts.model.SGPolicy("test-policy").Delete()).ShouldNot(HaveOccurred())
		})

		It("Should allow UDP connections with specific permit rules", func() {
			Skip("Disabling UDP test till HAL crash is debugged")
			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)
			spc := ts.model.NewSGPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "udp/8000", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify TCP connection works between workload pairs
			Eventually(func() error {
				return ts.model.Action().UDPSession(workloadPairs, 8000)
			}).Should(Succeed())

			// verify ping fails
			Eventually(func() error {
				return ts.model.Action().PingFails(workloadPairs)
			}).Should(Succeed())

			// verify connections in reverse direction fail
			Eventually(func() error {
				return ts.model.Action().UDPSessionFails(workloadPairs.ReversePairs(), 8000)
			}).Should(Succeed())

			// finally, delete the policy
			Expect(ts.model.SGPolicy("test-policy").Delete()).ShouldNot(HaveOccurred())
		})

		It("Ping should work with specific permit rules", func() {
			Skip("Disabling ICMP test due to HAL session timeout issue")

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)
			spc := ts.model.NewSGPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "icmp", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify ping is successful
			Eventually(func() error {
				return ts.model.Action().PingPairs(workloadPairs)
			}).Should(Succeed())

			// verify TCP connections fail
			Eventually(func() error {
				return ts.model.Action().TCPSessionFails(workloadPairs, 8000)
			}).Should(Succeed())

			// finally, delete the policy
			Expect(ts.model.SGPolicy("test-policy").Delete()).ShouldNot(HaveOccurred())
		})
	})
})
