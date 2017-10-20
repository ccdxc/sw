package cluster

import (
	"context"
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
			out := strings.Split(ts.tu.VIPCommandOutput("kubectl get pods --no-headers"), "\n")
			for _, line := range out {
				Expect(line).Should(ContainSubstring("Running"), "pod should be in Running state")
			}
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
