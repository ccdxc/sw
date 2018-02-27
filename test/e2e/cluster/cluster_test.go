package cluster

import (
	"context"
	"fmt"
	"strings"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/globals"

	cmdclient "github.com/pensando/sw/api/generated/cmd/grpc/client"
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

	Context("Naples Node Object validation ", func() {
		var (
			nodes []*cmd.Node
			err   error
		)
		BeforeEach(func() {
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			cmdClient := cmdclient.NewRestCrudClientCmdV1(apiGwAddr)
			nodeIf := cmdClient.Node()
			nodes, err = nodeIf.List(context.Background(), &api.ListWatchOptions{})
		})
		By(fmt.Sprintf("Got Nodes: %#v", nodes))
		It("Node fields should be ok", func() {
			Expect(err).ShouldNot(HaveOccurred())
			numNaplesNodes := 0
			for _, node := range nodes {
				for _, role := range node.Spec.Roles {
					if role == cmd.NodeSpec_WORKLOAD.String() {
						numNaplesNodes++
						Expect(node.Status.Phase).Should(Equal(cmd.NodeStatus_JOINED.String()))
					}
				}
			}
			Expect(numNaplesNodes).Should(Equal(ts.tu.NumNaplesNodes))
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
			cmdClient := cmdclient.NewRestCrudClientCmdV1(apiGwAddr)
			clusterIf := cmdClient.Cluster()
			obj = api.ObjectMeta{Name: "testCluster"}
			cl, err = clusterIf.Get(context.Background(), &obj)
		})
		It("Cluster fields should be ok", func() {
			Expect(err).ShouldNot(HaveOccurred())
			Expect(cl.Kind).Should(Equal("Cluster"))
			Expect(cl.UUID).ShouldNot(BeEmpty())
		})
	})
})
