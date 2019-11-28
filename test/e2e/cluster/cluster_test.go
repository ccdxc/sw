package cluster

import (
	"context"
	"fmt"
	"net"
	"reflect"
	"regexp"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/test/utils"
	cmdprotos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var _ = Describe("cluster tests", func() {

	Context("Failover test", func() {
		var (
			obj                  api.ObjectMeta
			cl                   *cmd.Cluster
			clusterIf            cmd.ClusterV1ClusterInterface
			smartNICIf           cmd.ClusterV1DistributedServiceCardInterface
			err                  error
			oldLeader, newLeader string
			oldLeaderIP          string
			serviceListBefore    cmdprotos.ServiceList
		)
		BeforeEach(func() {
			validateCluster()

			if ts.tu.NumQuorumNodes < 2 {
				Skip(fmt.Sprintf("Skipping failover test: %d quorum nodes found, need >= 2", ts.tu.NumQuorumNodes))
			}
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			apiClient, err := apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())
			clusterIf = apiClient.ClusterV1().Cluster()
			smartNICIf = apiClient.ClusterV1().DistributedServiceCard()
			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			obj = api.ObjectMeta{Name: "testCluster"}
			cl, err = clusterIf.Get(ctx, &obj)
			Expect(err).ShouldNot(HaveOccurred())

			validateNICHealth(ctx, smartNICIf, ts.tu.NumNaplesHosts, cmd.ConditionStatus_TRUE)
		})

		It("Pause the master", func() {
			Skip("Skip till we fix the sanity issues")
			Expect(err).ShouldNot(HaveOccurred())
			Expect(cl.Status.Leader).ShouldNot(Equal(""))
			oldLeader = cl.Status.Leader
			oldLeaderIP = ts.tu.NameToIPMap[oldLeader]

			serviceListBefore = getServices(ts, oldLeader)

			By(fmt.Sprintf("Pausing cmd on old leader %v", oldLeader))
			ts.tu.CommandOutput(oldLeaderIP, "docker pause pen-cmd")
			Eventually(func() bool {
				cl, err = clusterIf.Get(ts.tu.MustGetLoggedInContext(context.Background()), &obj)
				if err != nil {
					return false
				}
				if cl.Status.Leader == "" {
					return false
				}
				if cl.Status.Leader == oldLeader {
					return false
				}
				newLeader = cl.Status.Leader
				By(fmt.Sprintf("Found new leader %v", cl.Status.Leader))
				return true
			}, 30, 1).Should(BeTrue(), "Did not find a new leader")
		})

		AfterEach(func() {
			By(fmt.Sprintf("Resuming cmd on %v", oldLeader))
			ts.tu.CommandOutput(oldLeaderIP, "docker unpause pen-cmd")
			time.Sleep(2 * time.Second)

			By(fmt.Sprintf("before: %v", serviceListBefore.String()))
			for i, svc := range serviceListBefore.Items {
				if svc.Name == "pen-kube-apiserver" {
					for j, inst := range svc.Instances {
						if inst.Name == oldLeader {
							serviceListBefore.Items[i].Instances[j].Name = newLeader
							serviceListBefore.Items[i].Instances[j].Node = newLeader
							By(fmt.Sprintf("Replaced %v to %v in serviceListBefore", oldLeader, newLeader))
						}
					}
				}
				if svc.Name == globals.CmdNICUpdatesSvc {
					for j, inst := range svc.Instances {
						if inst.Node == oldLeader {
							serviceListBefore.Items[i].Instances[j].Node = newLeader
							serviceListBefore.Items[i].Instances[j].URL = fmt.Sprintf("%s:%s", newLeader, globals.CMDSmartNICUpdatesPort)
							By(fmt.Sprintf("Replaced %v to %v in serviceListBefore", oldLeader, newLeader))
						}
					}
				}
			}

			xformList := func(inp []*cmdprotos.Service) []*cmdprotos.Service {
				filteredItemList := []*cmdprotos.Service{nil}
				for _, svc := range inp {
					if svc.Name != "pen-filebeat" {
						filteredItemList = append(filteredItemList, svc)
					}
				}
				return filteredItemList
			}
			serviceListBefore.Items = xformList(serviceListBefore.Items)

			Eventually(func() bool {
				serviceListAfter := getServices(ts, newLeader)
				By(fmt.Sprintf("after: %v", serviceListAfter.String()))
				serviceListAfter.Items = xformList(serviceListAfter.Items)
				return reflect.DeepEqual(serviceListAfter, serviceListBefore)
			}, 20, 2).Should(BeTrue(), "Services except filebeat,pen-kubeapiserver and CMD NIC updates should be same after leader change")

			validateCluster()

			// Since we just switched over the cluster, the connections to old master node are dropped.
			// Hence create new client for use in further tests
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			ts.restSvc, err = apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())

			// Old leader CMD instance should have shut down leader-only services and
			// NAPLES should send health updates to the the new leader CMD instance.
			// Wait 1.5 * dead interval (15 s) to make sure NIC is marked ad unhealthy
			// if no fresh updates are received.
			time.Sleep(15 * time.Second)
			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			validateNICHealth(ctx, smartNICIf, ts.tu.NumNaplesHosts, cmd.ConditionStatus_TRUE)
		})
	})
})

func getServices(ts *TestSuite, node string) cmdprotos.ServiceList {
	By(fmt.Sprintf("Getting services from %v", node))
	rpcClient, err := rpckit.NewRPCClient("cluster_test", node+":"+globals.CMDGRPCAuthPort, rpckit.WithRemoteServerName(globals.Cmd), rpckit.WithTLSProvider(ts.tu.TLSProvider))
	Expect(err).ShouldNot(HaveOccurred())
	defer rpcClient.Close()
	client := cmdprotos.NewServiceAPIClient(rpcClient.ClientConn)
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	srvList, err := client.ListServices(ctx, &api.Empty{})
	Expect(err).ShouldNot(HaveOccurred())
	return *srvList
}

func validateCluster() {
	By(fmt.Sprintf("ts:%s Validating Cluster", time.Now().String()))
	By(fmt.Sprintf("pen-base should be running on all nodes"))
	for _, ip := range ts.tu.VeniceNodeIPs {
		Eventually(func() string {
			return ts.tu.GetContainerOnNode(ip, "pen-cmd")
		}, 10, 1).ShouldNot(BeEmpty(), "pen-cmd container should be running on %s", ip)
	}

	By(fmt.Sprintf("etcd should be running on all quorum nodes"))
	for _, qnode := range ts.tu.QuorumNodes {
		ip := ts.tu.NameToIPMap[qnode]
		Eventually(func() string {
			return ts.tu.GetContainerOnNode(ip, "pen-etcd")
		}, 10, 1).ShouldNot(BeEmpty(), "pen-etcd container should be running on %s(%s)", ts.tu.IPToNameMap[ip], ip)
	}

	By(fmt.Sprintf("etcd should not be running on non-quorum nodes"))
	ips := ts.tu.NonQuorumNodes()
	for nonQnode := range ips {
		Eventually(func() string {
			return ts.tu.GetContainerOnNode(nonQnode, "pen-etcd")
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
	}, 120, 1).Should(BeEmpty(), "All pods should be in Running state")

	Eventually(func() string {
		// TODO: remove workaround for following Kubernetes issues when move to a version with proper fixes:
		// https://github.com/kubernetes/kubernetes/issues/80968
		// https://github.com/kubernetes/kubernetes/issues/82346
		// refer PR https://github.com/pensando/sw/pull/15711
		// Ignore condition Ready == false (i.e. consider the pod good assuming all other conditions are True)
		return ts.tu.LocalCommandOutput(`kubectl get pods --all-namespaces -o json  | jq-linux64 -r '.items[] | select(.status.phase != "Running" or ([ .status.conditions[] | select(.type != "Ready" and .status == "False") ] | length ) > 0 ) | .metadata.namespace + "/" + .metadata.name'`)
	}, 95, 1).Should(BeEmpty(), "All pods should be in Ready state")

	apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
	apiClient, err := apiclient.NewRestAPIClient(apiGwAddr)
	Expect(err).ShouldNot(HaveOccurred())
	clusterIf := apiClient.ClusterV1().Cluster()
	obj := api.ObjectMeta{Name: "testCluster"}
	cl, err := clusterIf.Get(ts.tu.MustGetLoggedInContext(context.Background()), &obj)

	By(fmt.Sprintf("Cluster fields should be ok"))
	Expect(err).ShouldNot(HaveOccurred())
	Expect(cl.Kind).Should(Equal("Cluster"))
	Expect(cl.UUID).ShouldNot(BeEmpty())

	By(fmt.Sprintf("All quorum members should be healthy"))
	Eventually(func() error {
		return checkQuorumHealth(clusterIf, &obj, len(ts.tu.QuorumNodes))
	}, 60, 5).Should(BeNil(), "Quorum is not healthy")

	versionIf := apiClient.ClusterV1().Version()

	var nctx context.Context

	By(fmt.Sprintf("Version fields should be ok"))
	Eventually(func() bool {
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		nctx, err = ts.tu.NewLoggedInContext(ctx)
		if err != nil {
			cancel()
			By(fmt.Sprintf("ts:%s err:%v logging in", time.Now().String(), err))
			return false
		}
		vl, err := versionIf.Get(ts.tu.MustGetLoggedInContext(context.Background()), &obj)
		cancel()
		if err != nil {
			By(fmt.Sprintf("ts:%s err:%v getting version Info", time.Now().String(), err))
			return false
		}
		if vl.Kind != "Version" {
			By(fmt.Sprintf("ts:%s unknown Kind %s", time.Now().String(), vl.Kind))
			return false
		}
		if vl.APIVersion == "" {
			By(fmt.Sprintf("ts:%s APIVersion is empty", time.Now().String()))
			return false
		}
		return true
	}, 60, 5).Should(BeTrue(), "version object should have correct values")

	Eventually(func() string {
		s1 := getServices(ts, ts.tu.QuorumNodes[0])
		serviceListNode1 := s1.String()
		for index, n := range ts.tu.QuorumNodes {
			s := getServices(ts, n)
			serviceList := s.String()
			if serviceListNode1 != serviceList {
				m := make(map[string]cmdprotos.Service)

				for _, svc := range s1.Items {
					m[svc.Name] = *svc
				}
				for _, svc := range s.Items {
					if reflect.DeepEqual(m[svc.Name], *svc) {
						delete(m, svc.Name)
					} else {
						svcNode1 := m[svc.Name]
						return fmt.Sprintf("serviceList on %s and %s differ.\nOn %s, serviceList is %s \nOn %s, serviceList is %s\n",
							ts.tu.QuorumNodes[0], ts.tu.QuorumNodes[index], ts.tu.QuorumNodes[0], svcNode1.String(), ts.tu.QuorumNodes[index], (*svc).String())
					}
				}
				if len(m) != 0 {
					return fmt.Sprintf("services %v on %s are not present on %s", m, ts.tu.QuorumNodes[0], ts.tu.QuorumNodes[index])
				}
			}
		}
		return ""
	}, 15, 3).Should(BeEmpty(), "Resolver data should be same on all quorum nodes")

	By(fmt.Sprintf("ts: %s Cluster Health check", time.Now().String()))

	// cluster should be in healthy state
	Eventually(func() bool {
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		nctx, err = ts.tu.NewLoggedInContext(ctx)
		if err != nil {
			cancel()
			By(fmt.Sprintf("ts: %s err: %v logging in", time.Now().String(), err))
			return false
		}
		cl, err = clusterIf.Get(nctx, &obj)
		cancel()
		if err != nil {
			By(fmt.Sprintf("ts: %s err: %v getting cluster Info", time.Now().String(), err))
			return false
		}
		for _, cond := range cl.Status.Conditions {
			if cond.Type == cmd.ClusterCondition_HEALTHY.String() {
				if cond.Status != cmd.ConditionStatus_TRUE.String() {
					By(fmt.Sprintf("ts: %s cluster health status: %v, reason: %v", time.Now().String(), cond.Status, cond.Reason))

					// kubectl get pods on the failed services
					tmp := regexp.MustCompile(`[a-z]+\-[a-z]+`).FindAll([]byte(cond.Reason), -1)
					var svcsStr []string
					for _, svc := range tmp {
						svcsStr = append(svcsStr, string(svc))
					}
					if len(svcsStr) != 0 {
						getPodsCmd := fmt.Sprintf("kubectl get pods -o wide | grep '%s'", strings.Join(svcsStr, "\\|"))
						By(fmt.Sprintf("ts: %s %s:\n %s", time.Now().String(), getPodsCmd, ts.tu.LocalCommandOutput(getPodsCmd)))
					}
				}

				return cond.Status == cmd.ConditionStatus_TRUE.String()
			}
		}
		By(fmt.Sprintf("ts: %s cluster health status: %v", time.Now().String(), cl.Status.Conditions))
		By(fmt.Sprintf("ts: %s kubectl get pods -o wide: %s", time.Now().String(), ts.tu.LocalCommandOutput("kubectl get pods -o wide")))
		return false
	}, 60, 5).Should(BeTrue(), "cluster should be in healthy state")

	// Bootstrap port should be shut on all nodes that are part of the cluster and open on those that are not
	Eventually(func() bool {
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		nctx, err = ts.tu.NewLoggedInContext(ctx)
		if err != nil {
			cancel()
			By(fmt.Sprintf("ts: %s err: %v logging in", time.Now().String(), err))
			return false
		}
		allNodes, err := apiClient.ClusterV1().Node().List(nctx, &api.ListWatchOptions{})
		cancel()
		if err != nil {
			By(fmt.Sprintf("ts: %s err: %v getting node Info", time.Now().String(), err))
			return false
		}

		isJoinedNode := func(name string) bool {
			for _, n := range allNodes {
				if name == n.Name {
					return true
				}
			}
			return false
		}

		for _, ip := range ts.tu.VeniceNodeIPs {
			bsURL := net.JoinHostPort(ip, globals.CMDRESTPort)
			nodeName := ts.tu.IPToNameMap[ip]
			conn, err := net.Dial("tcp", bsURL)
			if isJoinedNode(nodeName) && !utils.IsConnRefusedError(err) {
				By(fmt.Sprintf("Node %v(%s) has joined the cluster but bootstrap port %s is not shut. Dial error: %+v",
					ip, nodeName, globals.CMDRESTPort, err))
				return false
			}
			if !isJoinedNode(nodeName) && err != nil {
				By(fmt.Sprintf("Node %v(%s) has not joined the cluster but was unable to dial bootstrap port %s. Err: %v",
					ip, nodeName, globals.CMDRESTPort, err))
				return false
			}
			if conn != nil {
				conn.Close()
			}
		}
		return true
	}, 60, 5).Should(BeTrue(), "Bootstrap ports should be shut on all cluster nodes")

	By(fmt.Sprintf("ts: %s Cluster is Healthy", time.Now().String()))
}
