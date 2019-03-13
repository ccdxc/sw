// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	"fmt"
	"math/rand"

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
		ts.tb.AfterTestCommon()

		// delete test policy if its left over. we can ignore the error here
		ts.model.SGPolicy("test-policy").Delete()
		ts.model.SGPolicy("default-policy").Delete()

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
			if ts.tb.HasNaplesSim() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			spc := ts.model.NewSGPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "tcp/8000", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.Action().VerifyPolicyStatus(spc)
			}).Should(Succeed())

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
		})

		It("Should allow UDP connections with specific permit rules", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling UDP test on naples sim till traffic issue is debugged")
			}
			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			spc := ts.model.NewSGPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "udp/8000", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.Action().VerifyPolicyStatus(spc)
			}).Should(Succeed())

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
		})

		It("Ping should work with specific permit rules", func() {
			Skip("Disabling ICMP test till we figure out ICMP proto/port issue in netagent")

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			spc := ts.model.NewSGPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "icmp", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.Action().VerifyPolicyStatus(spc)
			}).Should(Succeed())

			// verify ping is successful
			Eventually(func() error {
				return ts.model.Action().PingPairs(workloadPairs)
			}).Should(Succeed())

			// verify TCP connections fail
			Eventually(func() error {
				return ts.model.Action().TCPSessionFails(workloadPairs, 8000)
			}).Should(Succeed())
		})
		It("Should be able to update policy and verify it takes effect", func() {
			const maxRules = 5000
			const numIter = 10
			startPort := 2000
			boundaryPort := (startPort - 1) + maxRules

			// pick a workload pair
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(4)

			// run multiple iterations, each time updating the policy with different number of rules
			for iter := 0; iter < numIter; iter++ {
				spc := ts.model.NewSGPolicy("test-policy")
				for i := startPort; i <= boundaryPort; i++ {
					spc = spc.AddRulesForWorkloadPairs(workloadPairs, fmt.Sprintf("tcp/%d", i), "PERMIT")
				}
				Expect(spc.Commit()).Should(Succeed())

				// verify policy was propagated correctly
				Eventually(func() error {
					return ts.model.Action().VerifyPolicyStatus(spc)
				}).Should(Succeed())

				// verify TCP connection works on boundary port
				Eventually(func() error {
					return ts.model.Action().TCPSession(workloadPairs, boundaryPort)
				}).Should(Succeed())

				// verify connections above boundary port does not work
				Eventually(func() error {
					return ts.model.Action().TCPSessionFails(workloadPairs, boundaryPort+1)
				}).Should(Succeed())

				// change the boundary port
				boundaryPort = rand.Intn(maxRules) + startPort

				// increment task count for each iteration
				ts.tb.AddTaskResult("", nil)
			}
		})
	})
})
