package cluster

import (
	"context"
	"strings"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/globals"

	cmdclient "github.com/pensando/sw/api/generated/cmd/grpc/client"
)

var _ = Describe("cluster tests", func() {
	Context("Services should be running", func() {
		It("pen-base should be running on all nodes", func() {
			for _, ip := range e2eTest.veniceNodeIPs {
				out := commandOutput(ip, "docker ps -q -f Name=pen-base")
				Expect(out).ShouldNot(BeEmpty(), "pen-base container should be running on %s", ip)
			}
		})
		It("etcd should be running on all quorum nodes", func() {
			for _, qnode := range e2eTest.quorumnodes {
				ip := e2eTest.nameToIPMap[qnode]
				out := commandOutput(ip, "docker ps -q -f Name=pen-etcd")
				Expect(out).ShouldNot(BeEmpty(), "pen-etcd container should be running on %s(%s)",
					e2eTest.ipToNameMap[ip], ip)
			}
		})
		It("etcd should not be running on non-quorum nodes", func() {
			ips := nonClusterNodes()
			for nonQnode := range ips {
				out := commandOutput(nonQnode, "docker ps -q -f Name=pen-etcd")
				Expect(out).Should(BeEmpty(), "pen-etcd container should not be running on %s(%s)",
					e2eTest.ipToNameMap[nonQnode], nonQnode)
			}
		})
		It("kubernetes indicated all pods to be Running", func() {
			out := strings.Split(vipCommandOutput("kubectl get pods --no-headers"), "\n")
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
			apiGwAddr := e2eTest.ClusterVIP + ":" + globals.APIGwRESTPort
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
