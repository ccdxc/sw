package cluster

import (
	"context"
	"fmt"
	"strings"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"

	cmdclient "github.com/pensando/sw/api/generated/cluster/grpc/client"
)

var _ = Describe("cluster tests", func() {
	Context("Services should be running", func() {
		It("pen-base should be running on all nodes", func() {
			for _, ip := range ts.tu.VeniceNodeIPs {
				out := ts.tu.CommandOutput(ip, "docker ps -q -f Name=pen-cmd")
				Expect(out).ShouldNot(BeEmpty(), "pen-cmd container should be running on %s", ip)
			}
		})
		It("etcd should be running on all quorum nodes", func() {
			for _, qnode := range ts.tu.QuorumNodes {
				ip := ts.tu.NameToIPMap[qnode]
				out := ts.tu.CommandOutput(ip, "docker ps -q -f Name=pen-etcd")
				Expect(out).ShouldNot(BeEmpty(), "pen-etcd container should be running on %s(%s)",
					ts.tu.IPToNameMap[ip], ip)
			}
		})
		It("etcd should not be running on non-quorum nodes", func() {
			ips := ts.tu.NonQuorumNodes()
			for nonQnode := range ips {
				out := ts.tu.CommandOutput(nonQnode, "docker ps -q -f Name=pen-etcd")
				Expect(out).Should(BeEmpty(), "pen-etcd container should not be running on %s(%s)",
					ts.tu.IPToNameMap[nonQnode], nonQnode)
			}
		})
		It("kubernetes indicated all pods to be Running", func() {
			Eventually(func() string {
				out := strings.Split(ts.tu.LocalCommandOutput("kubectl get pods --no-headers"), "\n")
				for _, line := range out {
					if !strings.Contains(line, "Running") {
						return line
					}
				}
				return ""
			}, 95, 1).Should(BeEmpty(), "All pods should be in Running state")
		})
	})

	Context("Naples Host Object validation ", func() {
		var (
			hosts []*cmd.Host
			err   error
		)
		BeforeEach(func() {
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			cmdClient := cmdclient.NewRestCrudClientClusterV1(apiGwAddr)
			hostIf := cmdClient.Host()
			hosts, err = hostIf.List(ts.tu.NewLoggedInContext(context.Background()), &api.ListWatchOptions{})
		})
		It("Node fields should be ok", func() {
			Expect(err).ShouldNot(HaveOccurred())
			numNaplesHosts := len(hosts)
			Expect(numNaplesHosts).Should(Equal(ts.tu.NumNaplesHosts))
		})
	})

	Context("Object validation ", func() {
		var (
			obj api.ObjectMeta
			cl  *cmd.Cluster
			err error
		)
		BeforeEach(func() {
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			cmdClient := cmdclient.NewRestCrudClientClusterV1(apiGwAddr)
			clusterIf := cmdClient.Cluster()
			obj = api.ObjectMeta{Name: "testCluster"}
			cl, err = clusterIf.Get(ts.tu.NewLoggedInContext(context.Background()), &obj)
		})
		It("Cluster fields should be ok", func() {
			Expect(err).ShouldNot(HaveOccurred())
			Expect(cl.Kind).Should(Equal("Cluster"))
			Expect(cl.UUID).ShouldNot(BeEmpty())
		})
	})

	Context("Failover test", func() {
		var (
			obj         api.ObjectMeta
			cl          *cmd.Cluster
			clusterIf   cmd.ClusterV1ClusterInterface
			err         error
			oldLeader   string
			oldLeaderIP string
		)
		BeforeEach(func() {
			if ts.tu.NumQuorumNodes < 3 {
				Skip(fmt.Sprintf("Skipping failover test: %d quorum nodes found, need >= 3", ts.tu.NumQuorumNodes))
			}
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			cmdClient := cmdclient.NewRestCrudClientClusterV1(apiGwAddr)
			clusterIf = cmdClient.Cluster()
			obj = api.ObjectMeta{Name: "testCluster"}
			cl, err = clusterIf.Get(ts.tu.NewLoggedInContext(context.Background()), &obj)
		})
		It("Pause the master", func() {
			Expect(err).ShouldNot(HaveOccurred())
			Expect(cl.Status.Leader).ShouldNot(Equal(""))
			oldLeader = cl.Status.Leader
			oldLeaderIP = ts.tu.NameToIPMap[oldLeader]
			By(fmt.Sprintf("Pausing cmd on old leader %v", oldLeader))
			ts.tu.CommandOutput(oldLeaderIP, "docker pause pen-cmd")
			Eventually(func() bool {
				cl, err = clusterIf.Get(ts.tu.NewLoggedInContext(context.Background()), &obj)
				if err != nil {
					return false
				}
				if cl.Status.Leader == "" {
					return false
				}
				if cl.Status.Leader == oldLeader {
					return false
				}
				By(fmt.Sprintf("Found new leader %v", cl.Status.Leader))
				return true
			}, 30, 1).Should(BeTrue(), "Did not find a new leader")
		})
		AfterEach(func() {
			By(fmt.Sprintf("Resuming cmd on %v", oldLeader))
			ts.tu.CommandOutput(oldLeaderIP, "docker unpause pen-cmd")
		})
	})
})
