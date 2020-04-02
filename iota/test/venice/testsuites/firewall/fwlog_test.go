// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("fwlog tests", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})

	AfterEach(func() {
		ts.tb.AfterTestCommon()
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))

		// delete test policy if its left over. we can ignore thes error here
		ts.model.NetworkSecurityPolicy("test-policy").Delete()
		ts.model.DefaultNetworkSecurityPolicy().Delete()

		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Commit()).ShouldNot(HaveOccurred())

		// verify policy was propagated correctly
		Eventually(func() error {
			return ts.model.VerifyPolicyStatus(ts.model.DefaultNetworkSecurityPolicy())
		}).Should(Succeed())
	})

	Context("tags:type=basic;datapath=true;duration=short verify fwlog on traffic ", func() {
		It("tags:sanity=true should log ICMP allow in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			policy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "icmp", "PERMIT")
			Expect(policy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.PingPairs(workloadPairs)
			}).Should(Succeed())

			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "ICMP", 0, "allow", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "ICMP", 0, "allow", workloadPairs)
			}).Should(Succeed())
		})

		It("tags:sanity=true should log TCP/8000 allow in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			policy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "tcp/0-65535", "PERMIT")
			Expect(policy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.TCPSession(workloadPairs, 8000)
			}).Should(Succeed())

			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "TCP", 8000, "allow", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "TCP", 8000, "allow", workloadPairs)
			}).Should(Succeed())
		})

		It("should log UDP/9000 allow in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			// allow policy
			policy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "udp/0-65535", "PERMIT")
			Expect(policy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			Eventually(func() error {
				return ts.model.UDPSession(workloadPairs, 9000)
			}).Should(Succeed())

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "UDP", 9000, "allow", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "UDP", 9000, "allow", workloadPairs)
			}).Should(Succeed())
		})

		It("should log ICMP deny in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			// deny policy
			denyPolicy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "icmp", "DENY")
			denyPolicy.AddRule("any", "any", "", "PERMIT")
			Expect(denyPolicy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			Eventually(func() error {
				return ts.model.PingFails(workloadPairs)
			}).Should(Succeed())

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "ICMP", 0, "deny", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "ICMP", 0, "deny", workloadPairs)
			}).Should(Succeed())
		})

		It("tags:sanity=true should log TCP/8100 deny in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			// deny policy
			debyPolicy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "tcp/0-65535", "DENY")
			debyPolicy.AddRule("any", "any", "", "PERMIT")
			Expect(debyPolicy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			Eventually(func() error {
				return ts.model.TCPSessionFails(workloadPairs, 8100)
			}).Should(Succeed())

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "TCP", 8100, "deny", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "TCP", 8100, "deny", workloadPairs)
			}).Should(Succeed())
		})

		It("should log UDP/9100 deny in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			// deny policy
			debyPolicy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "udp/0-65535", "DENY")
			debyPolicy.AddRule("any", "any", "", "PERMIT")
			Expect(debyPolicy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			Eventually(func() error {
				return ts.model.UDPSessionFails(workloadPairs, 9100)
			}).Should(Succeed())

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "UDP", 9100, "deny", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "UDP", 9100, "deny", workloadPairs)
			}).Should(Succeed())
		})

		It("should log ICMP reject in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			// reject policy
			denyPolicy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "icmp", "REJECT")
			denyPolicy.AddRule("any", "any", "", "PERMIT")
			Expect(denyPolicy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			Eventually(func() error {
				return ts.model.PingFails(workloadPairs)
			}).Should(Succeed())

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "ICMP", 0, "reject", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "ICMP", 0, "reject", workloadPairs)
			}).Should(Succeed())
		})

		It("should log TCP/8200 reject in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			// deny policy
			denyPolicy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "tcp/0-65535", "REJECT")
			denyPolicy.AddRule("any", "any", "", "PERMIT")
			Expect(denyPolicy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			Eventually(func() error {
				return ts.model.TCPSessionFails(workloadPairs, 8200)
			}).Should(Succeed())

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "TCP", 8200, "reject", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "TCP", 8200, "reject", workloadPairs)
			}).Should(Succeed())
		})

		It("should log UDP/9200 reject in fwlog", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			// reject policy
			rejectPolicy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "udp/0-65535", "REJECT")
			rejectPolicy.AddRule("any", "any", "", "PERMIT")
			Expect(rejectPolicy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			Eventually(func() error {
				return ts.model.UDPSessionFails(workloadPairs, 9200)
			}).Should(Succeed())

			// check fwlog
			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromObjStore("default", "UDP", 9200, "reject", workloadPairs)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.FindFwlogForWorkloadPairsFromElastic("default", "UDP", 9200, "reject", workloadPairs)
			}).Should(Succeed())
		})
	})
})
