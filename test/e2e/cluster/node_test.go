package cluster

import (
	"context"
	"encoding/json"
	"fmt"
	"math/rand"
	"strconv"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	cmdclient "github.com/pensando/sw/api/generated/cluster/grpc/client"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("node tests", func() {
	Context("When a non-quorum node is added", func() {
		var (
			nodeIf   cmd.ClusterV1NodeInterface
			nonQnode string // the node being added and remove from cluster
			err      error
		)
		BeforeEach(func() {
			if ts.tu.NumQuorumNodes == ts.tu.NumVeniceNodes {
				Skip("All venice nodes are quorum nodes. Skipping non-quorum node addition tests")
			}
			numAvailNodes := rand.Intn(ts.tu.NumVeniceNodes - ts.tu.NumQuorumNodes)
			nonQnode = "node" + strconv.Itoa(ts.tu.NumQuorumNodes+1+numAvailNodes)

			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			cmdClient := cmdclient.NewRestCrudClientClusterV1(apiGwAddr)
			nodeIf = cmdClient.Node()
			node := &cmd.Node{
				TypeMeta: api.TypeMeta{
					Kind:       "Node",
					APIVersion: "v1",
				},
				ObjectMeta: api.ObjectMeta{
					Name:      nonQnode,
					Namespace: "tenant1",
				},
				Spec: cmd.NodeSpec{
					Roles: []string{cmd.NodeSpec_CONTROLLER.String()},
				},
			}

			node, err = nodeIf.Create(ts.tu.NewLoggedInContext(context.Background()), node)
			Expect(err).ShouldNot(HaveOccurred())
			By(fmt.Sprintf("Added %+v to cluster", node.Name))
		})

		It("pen-ntp should be running on new node", func() {
			var kubeOut struct {
				Items []struct {
					Spec struct {
						NodeName string
					}
				}
			}

			Eventually(func() bool {
				out := ts.tu.LocalCommandOutput("kubectl get pods  -l name=pen-ntp -o json")
				json.Unmarshal([]byte(out), &kubeOut)
				for _, i := range kubeOut.Items {
					if i.Spec.NodeName == nonQnode {
						return true
					}
				}
				return false
			}, 95, 1).Should(BeTrue(), "pen-ntp container should be scheduled by kubernetes on node %s", nonQnode)

			Eventually(func() string {
				return ts.tu.CommandOutput(ts.tu.NameToIPMap[nonQnode], "/usr/bin/docker ps -q -f 'label=io.kubernetes.container.name=pen-ntp'")
			}, 95, 1).ShouldNot(BeEmpty(), "pen-ntp docker container should be running on %s", nonQnode)

		})

		AfterEach(func() {
			obj := api.ObjectMeta{Name: nonQnode}
			n, err2 := nodeIf.Delete(ts.tu.NewLoggedInContext(context.Background()), &obj)
			Expect(err2).ShouldNot(HaveOccurred())
			By(fmt.Sprintf("Disjoin %+v from cluster", n.Name))
		})
	})
})
