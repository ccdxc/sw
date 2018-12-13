package cluster

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	cmdprotos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"

	cmdclient "github.com/pensando/sw/api/generated/cluster/grpc/client"
)

var _ = Describe("cluster tests", func() {

	Context("Failover test", func() {
		var (
			obj               api.ObjectMeta
			cl                *cmd.Cluster
			clusterIf         cmd.ClusterV1ClusterInterface
			err               error
			oldLeader         string
			oldLeaderIP       string
			serviceListBefore string
		)
		BeforeEach(func() {
			validateCluster()

			if ts.tu.NumQuorumNodes < 2 {
				Skip(fmt.Sprintf("Skipping failover test: %d quorum nodes found, need >= 2", ts.tu.NumQuorumNodes))
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

			serviceListBefore = getServices(oldLeader)

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
			time.Sleep(2 * time.Second)

			By(fmt.Sprintf("before: %v", serviceListBefore))
			Eventually(func() bool {
				serviceListAfter := getServices(oldLeader)
				By(fmt.Sprintf("after: %v", serviceListAfter))
				return serviceListAfter == serviceListBefore
			}, 20, 2).Should(BeTrue(), "Services should be same after leader change")

			validateCluster()
		})
	})
})

func getServices(node string) string {
	var srvList cmdprotos.ServiceList
	srvURL := "http://" + node + ":" + globals.CMDRESTPort + "/api/v1/services"
	By(fmt.Sprintf("Getting services from %v", node))
	resp, err := http.Get(srvURL)
	Expect(err).ShouldNot(HaveOccurred())
	data, err := ioutil.ReadAll(resp.Body)
	Expect(err).ShouldNot(HaveOccurred())
	err = json.Unmarshal(data, &srvList)
	Expect(err).ShouldNot(HaveOccurred())
	resp.Body.Close()
	return srvList.String()
}

func validateCluster() {
	By(fmt.Sprintf("pen-base should be running on all nodes"))
	for _, ip := range ts.tu.VeniceNodeIPs {
		Eventually(func() string {
			return ts.tu.CommandOutput(ip, "docker ps -q -f Name=pen-cmd")
		}, 10, 1).ShouldNot(BeEmpty(), "pen-cmd container should be running on %s", ip)
	}

	By(fmt.Sprintf("etcd should be running on all quorum nodes"))
	for _, qnode := range ts.tu.QuorumNodes {
		ip := ts.tu.NameToIPMap[qnode]
		Eventually(func() string {
			return ts.tu.CommandOutput(ip, "docker ps -q -f Name=pen-etcd")
		}, 10, 1).ShouldNot(BeEmpty(), "pen-etcd container should be running on %s(%s)", ts.tu.IPToNameMap[ip], ip)
	}

	By(fmt.Sprintf("etcd should not be running on non-quorum nodes"))
	ips := ts.tu.NonQuorumNodes()
	for nonQnode := range ips {
		Eventually(func() string {
			return ts.tu.CommandOutput(nonQnode, "docker ps -q -f Name=pen-etcd")
		}, 10, 1).Should(BeEmpty(), "pen-etcd container should not be running on %s(%s)", ts.tu.IPToNameMap[nonQnode], nonQnode)
	}

	By(fmt.Sprintf("kubernetes indicated all pods to be Running"))
	Eventually(func() string {
		out := strings.Split(ts.tu.LocalCommandOutput("kubectl get pods --no-headers"), "\n")
		for _, line := range out {
			if !strings.Contains(line, "Running") {
				return line
			}
		}
		return ""
	}, 95, 1).Should(BeEmpty(), "All pods should be in Running state")

	apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
	cmdClient := cmdclient.NewRestCrudClientClusterV1(apiGwAddr)
	clusterIf := cmdClient.Cluster()
	obj := api.ObjectMeta{Name: "testCluster"}
	cl, err := clusterIf.Get(ts.tu.NewLoggedInContext(context.Background()), &obj)

	By(fmt.Sprintf("Cluster fields should be ok"))
	Expect(err).ShouldNot(HaveOccurred())
	Expect(cl.Kind).Should(Equal("Cluster"))
	Expect(cl.UUID).ShouldNot(BeEmpty())

	By(fmt.Sprintf("Resolver data should be same on all quorum nodes"))
	serviceListNode1 := getServices(ts.tu.QuorumNodes[0])
	for _, n := range ts.tu.QuorumNodes[1:] {
		serviceList := getServices(n)
		Expect(serviceListNode1).To(Equal(serviceList))
	}

}
