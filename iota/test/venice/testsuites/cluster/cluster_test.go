// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cluster_test

import (
	//"context"
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

			naples := ts.model.Naples()
			leader := ts.model.VeniceNodes().Leader()
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(leader, naples)
			Expect(err).ShouldNot(HaveOccurred())

			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(600 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

			//Connect Back and make sure cluster is good
			ts.model.Action().ConnectVeniceNodesToCluster(leader, naples)
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Venice Non-Leader shutdown should not affect the cluster", func() {

			naples := ts.model.Naples()

			node := ts.model.VeniceNodes().NonLeaders().Any(1)
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(node, naples)
			Expect(err).ShouldNot(HaveOccurred())

			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(600 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

			//Connect Back and make sure cluster is good
			ts.model.Action().ConnectVeniceNodesToCluster(node, naples)
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Venice all Leader and non-leader shutdown bring non-leader back to establish quorum", func() {
			naples := ts.model.Naples()
			leader := ts.model.VeniceNodes().Leader()
			//Pick one non-leader
			nodes := ts.model.VeniceNodes().NonLeaders().Any(1)
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes, naples)
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DisconnectVeniceNodesFromCluster(leader, naples)
			Expect(err).ShouldNot(HaveOccurred())

			//Create some Chaos!
			time.Sleep(600 * time.Second)

			//Now lets make sure other nodes communicate and bring up
			ts.model.Action().ConnectVeniceNodesToCluster(nodes, naples)

			// Sleep for 60 seconds to make sure we detect partitioning.
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

			//Connect Back and make sure cluster is good
			ts.model.Action().ConnectVeniceNodesToCluster(leader, naples)
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Venice all node shutdown bring back all to establish quorum", func() {
			naples := ts.model.Naples()
			nodes := ts.model.VeniceNodes()
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes, naples)
			Expect(err).ShouldNot(HaveOccurred())

			//Create some Chaos!
			time.Sleep(600 * time.Second)

			//Now bring back all of them
			ts.model.Action().ConnectVeniceNodesToCluster(nodes, naples)

			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Venice Leader Shutdown, policy push and make sure traffic is good", func() {
			naples := ts.model.Naples()
			leader := ts.model.VeniceNodes().Leader()
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(leader, naples)
			Expect(err).ShouldNot(HaveOccurred())

			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(600 * time.Second)
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

			//Connect Back and make sure cluster is good
			ts.model.Action().ConnectVeniceNodesToCluster(leader, naples)
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())

		})

		It("Venice Shutdown NPM node, policy push and make sure traffic is good", func() {
			naples := ts.model.Naples()
			npmNode, err := ts.model.VeniceNodes().GetVeniceNodeWithService("pen-npm")
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DisconnectVeniceNodesFromCluster(npmNode, naples)
			Expect(err).ShouldNot(HaveOccurred())

			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(600 * time.Second)
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

			//Connect Back and make sure cluster is good
			ts.model.Action().ConnectVeniceNodesToCluster(npmNode, naples)
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Venice Shutdown APIServer node, policy push and make sure traffic is good", func() {
			naples := ts.model.Naples()
			apiServerNode, err := ts.model.VeniceNodes().GetVeniceNodeWithService("pen-apiserver")
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.Action().DisconnectVeniceNodesFromCluster(apiServerNode, naples)
			Expect(err).ShouldNot(HaveOccurred())

			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(600 * time.Second)
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

			//Connect Back and make sure cluster is good
			ts.model.Action().ConnectVeniceNodesToCluster(apiServerNode, naples)
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Venice Shutdown 2 nodes, make sure traffic is good", func() {
			naples := ts.model.Naples()
			nodes := ts.model.VeniceNodes().Any(2)
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes, naples)
			Expect(err).ShouldNot(HaveOccurred())

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 0)
			}).Should(Succeed())

			ts.model.Action().ConnectVeniceNodesToCluster(nodes, naples)
			time.Sleep(600 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
			}).Should(Succeed())
		})

		It("Venice Shutdown 2 nodes, restart naples and make sure traffic good", func() {
			naples := ts.model.Naples()
			nodes := ts.model.VeniceNodes().Any(2)
			err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes, naples)
			Expect(err).ShouldNot(HaveOccurred())

			// ping all workload pairs in same subnet
			workloadPairs := ts.model.WorkloadPairs().Permit(ts.model.DefaultNetworkSecurityPolicy(), "tcp")
			Eventually(func() error {
				return ts.model.Action().TCPSession(workloadPairs, 0)
			}).Should(Succeed())

			ts.model.Action().ConnectVeniceNodesToCluster(nodes, naples)
			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(600 * time.Second)
			Eventually(func() error {
				return ts.model.Action().VerifyClusterStatus()
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

		/*It("Venice removed and added back to cluster as a cleanode", func() {
			Expect(ts.model.ForEachVeniceNode(func(vnc *iotakit.VeniceNodeCollection) error {
				Expect(ts.model.Action().RemoveVenice(vnc)).Should(Succeed())
				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				//Lets run config add/del to make sure cluster is good
				err := ts.model.CleanupAllConfig()
				Expect(err).ShouldNot(HaveOccurred())

				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				err = ts.model.SetupDefaultConfig(context.Background(), ts.scaleData, ts.scaleData)
				Expect(err).ShouldNot(HaveOccurred())

				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				Expect(ts.model.Action().AddVenice(vnc)).Should(Succeed())
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				//Lets run config add/del to make sure cluster is good
				err = ts.model.CleanupAllConfig()
				Expect(err).ShouldNot(HaveOccurred())

				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

				err = ts.model.SetupDefaultConfig(context.Background(), ts.scaleData, ts.scaleData)
				Expect(err).ShouldNot(HaveOccurred())
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())
				return err
			})).ShouldNot(HaveOccurred())
		}) */

		/*
			//Network Partitioning test cases
			//Only the one where Venice nodes are disconnected from the cluster, but naples reacbable
			//  If Venice nodes are connected but naples node disconnected, they are not valid today as
			// there are some singleton services which may not be reachable.
			It("tags:sanity=true Venice Leader disconnected from cluster but reachable from naples", func() {

				leader := ts.model.VeniceNodes().Leader()
				err := ts.model.Action().DisconnectVeniceNodesFromCluster(leader, nil)
				Expect(err).ShouldNot(HaveOccurred())

				defer ts.model.Action().ConnectVeniceNodesToCluster(leader, nil)
				// Sleep for 60 seconds to make sure we detect partitioning.
				time.Sleep(120 * time.Second)
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

			})

			It("Venice Non-Leader disconnected from cluster reachable from naples", func() {

				node := ts.model.VeniceNodes().NonLeaders().Any(1)
				err := ts.model.Action().DisconnectVeniceNodesFromCluster(node, nil)
				Expect(err).ShouldNot(HaveOccurred())

				defer ts.model.Action().ConnectVeniceNodesToCluster(node, nil)
				// Sleep for 60 seconds to make sure we detect partitioning.
				time.Sleep(120 * time.Second)
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

			})

			It("Venice all Leader and non-leader disconnected naples bring non-leader", func() {
				leader := ts.model.VeniceNodes().Leader()
				//Pick one non-leader
				nodes := ts.model.VeniceNodes().NonLeaders().Any(1)
				err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes, nil)
				Expect(err).ShouldNot(HaveOccurred())
				err = ts.model.Action().DisconnectVeniceNodesFromCluster(leader, nil)
				Expect(err).ShouldNot(HaveOccurred())

				//Eventually bring back the leader
				defer ts.model.Action().ConnectVeniceNodesToCluster(leader, nil)

				//Create some Chaos!
				time.Sleep(120 * time.Second)

				//Now lets make sure other nodes communicate and bring up
				ts.model.Action().ConnectVeniceNodesToCluster(nodes, nil)

				// Sleep for 60 seconds to make sure we detect partitioning.
				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())
			})

			It("Venice all node disconnected from each other but reachable via naples", func() {
				nodes := ts.model.VeniceNodes()
				err := ts.model.Action().DisconnectVeniceNodesFromCluster(nodes, nil)
				Expect(err).ShouldNot(HaveOccurred())

				//Create some Chaos!
				time.Sleep(120 * time.Second)

				//Now bring back all of them
				ts.model.Action().ConnectVeniceNodesToCluster(nodes, nil)

				Eventually(func() error {
					return ts.model.Action().VerifyClusterStatus()
				}).Should(Succeed())

			})

			It("Venice Leader disconnected (naples reachable), policy push and make sure traffic is good", func() {
				leader := ts.model.VeniceNodes().Leader()
				err := ts.model.Action().DisconnectVeniceNodesFromCluster(leader, nil)
				Expect(err).ShouldNot(HaveOccurred())

				defer ts.model.Action().ConnectVeniceNodesToCluster(leader, nil)

				// Sleep for 60 seconds to make sure we detect partitioning.
				time.Sleep(120 * time.Second)
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

			It("Venice disconnected NPM node(reachable via naples), policy push and make sure traffic is good", func() {
				npmNode, err := ts.model.VeniceNodes().GetVeniceNodeWithService("pen-npm")
				Expect(err).ShouldNot(HaveOccurred())
				err = ts.model.Action().DisconnectVeniceNodesFromCluster(npmNode, nil)
				Expect(err).ShouldNot(HaveOccurred())

				defer ts.model.Action().ConnectVeniceNodesToCluster(npmNode, nil)
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

			}) */

	})

})
