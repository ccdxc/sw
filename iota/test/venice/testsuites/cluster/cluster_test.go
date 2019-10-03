// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cluster_test

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit"
)

var _ = Describe("venice cluster tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("tags:type=basic;datapath=true;duration=long Basic cluster tests", func() {
		It("tags:sanity=true Venice Leader shutdown should not affect the cluster", func() {

			leader := ts.model.VeniceNodes().Leader()
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(leader)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(leader, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			defer ts.model.Action().ConnectVeniceNodesToCluster(leader)
			defer ts.model.Action().AllowVeniceNodesFromNaples(leader, ts.model.Naples())
			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Venice Non-Leader shutdown should not affect the cluster", func() {

			node := ts.model.VeniceNodes().NonLeaders().Any(1)
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(node)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(node, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			defer ts.model.Action().ConnectVeniceNodesToCluster(node)
			defer ts.model.Action().AllowVeniceNodesFromNaples(node, ts.model.Naples())
			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Venice all Leader and non-leader shutdown bring non-leader back to establish quorum", func() {
			leader := ts.model.VeniceNodes().Leader()
			//Pick one non-leader
			nodes := ts.model.VeniceNodes().NonLeaders().Any(1)
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DisconnectVeniceNodesFromCluster(leader)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(nodes, ts.model.Naples())
			err = ts.model.Action().DenyVeniceNodesFromNaples(leader, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			//Eventually bring back the leader
			defer ts.model.Action().ConnectVeniceNodesToCluster(leader)
			defer ts.model.Action().AllowVeniceNodesFromNaples(leader, ts.model.Naples())

			//Create some Chaos!
			time.Sleep(60 * time.Second)

			//Now lets make sure other nodes communicate and bring up
			ts.model.Action().ConnectVeniceNodesToCluster(nodes)
			ts.model.Action().AllowVeniceNodesFromNaples(nodes, ts.model.Naples())

			// Sleep for 60 seconds to make sure we detect partitioning.
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Venice all node shutdown bring back all to establish quorum", func() {
			nodes := ts.model.VeniceNodes()
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(nodes, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			//Create some Chaos!
			time.Sleep(60 * time.Second)

			//Now bring back all of them
			ts.model.Action().ConnectVeniceNodesToCluster(nodes)
			ts.model.Action().AllowVeniceNodesFromNaples(nodes, ts.model.Naples())

			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Venice Leader Shutdown, policy push and make sure traffic is good", func() {
			leader := ts.model.VeniceNodes().Leader()
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(leader)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(leader, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			defer ts.model.Action().ConnectVeniceNodesToCluster(leader)
			defer ts.model.Action().AllowVeniceNodesFromNaples(leader, ts.model.Naples())
			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

			//update is add and delete of the policy
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
			time.Sleep(30 * time.Second)
			Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.Action().VerifyPolicyStatus(ts.model.DefaultNetworkSecurityPolicy())
			}).Should(Succeed())

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 0)
			}).Should(Succeed())

		})

		It("Venice Shutdown NPM node, policy push and make sure traffic is good", func() {
			npmNode, err := ts.model.VeniceNodes().GetVeniceNodeWithService("pen-npm")
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DisconnectVeniceNodesFromCluster(npmNode)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(npmNode, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			defer ts.model.Action().ConnectVeniceNodesToCluster(npmNode)
			defer ts.model.Action().AllowVeniceNodesFromNaples(npmNode, ts.model.Naples())
			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

			//update is add and delete of the policy
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
			time.Sleep(30 * time.Second)
			Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.Action().VerifyPolicyStatus(ts.model.DefaultNetworkSecurityPolicy())
			}).Should(Succeed())

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 0)
			}).Should(Succeed())

		})

		It("Venice Shutdown APIServer node, policy push and make sure traffic is good", func() {
			apiServerNode, err := ts.model.VeniceNodes().GetVeniceNodeWithService("pen-apiserver")
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DisconnectVeniceNodesFromCluster(apiServerNode)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(apiServerNode, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			defer ts.model.Action().ConnectVeniceNodesToCluster(apiServerNode)
			defer ts.model.Action().AllowVeniceNodesFromNaples(apiServerNode, ts.model.Naples())
			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

			//update is add and delete of the policy
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
			time.Sleep(30 * time.Second)
			Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.Action().VerifyPolicyStatus(ts.model.DefaultNetworkSecurityPolicy())
			}).Should(Succeed())

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 0)
			}).Should(Succeed())

		})

		It("Venice Shutdown 2 nodes, make sure traffic is good", func() {
			nodes := ts.model.VeniceNodes().Any(2)
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(nodes, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			defer func() {
				ts.model.Action().ConnectVeniceNodesToCluster(nodes)
				ts.model.Action().AllowVeniceNodesFromNaples(nodes, ts.model.Naples())
				// Sleep for 60 seconds to make sure we detect partitioning.
				time.Sleep(60 * time.Second)
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())
			}()

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 0)
			}).Should(Succeed())

		})

		It("Venice Shutdown 2 nodes, restart naples and make sure traffic good", func() {
			nodes := ts.model.VeniceNodes().Any(2)
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DenyVeniceNodesFromNaples(nodes, ts.model.Naples())
			Expect(err).ShouldNot(HaveOccurred())

			defer func() {
				ts.model.Action().ConnectVeniceNodesToCluster(nodes)
				ts.model.Action().AllowVeniceNodesFromNaples(nodes, ts.model.Naples())
				// Sleep for 60 seconds to make sure we detect partitioning.
				time.Sleep(60 * time.Second)
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())
			}()

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 0)
			}).Should(Succeed())

		})

		It("Should be able reload venice nodes and cluster should come back to normal state", func() {
			// reload each host
			ts.model.ForEachVeniceNode(func(vnc *iotakit.VeniceNodeCollection) error {
				Expect(ts.model.Action().ReloadVeniceNodes(vnc)).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				return nil
			})
		})

		It("Should be able reload venice leader node and cluster should come back to normal state", func() {
			// reload the leader node
			for i := 0; i < 3; i++ {
				Expect(ts.model.Action().ReloadVeniceNodes(ts.model.VeniceNodes().Leader())).Should(Succeed())

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())
			}
		})
	})

})
