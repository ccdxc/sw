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
	cmd "github.com/pensando/sw/api/generated/cmd"
	cmdclient "github.com/pensando/sw/api/generated/cmd/grpc/client"
	"github.com/pensando/sw/globals"
)

var _ = Describe("node tests", func() {
	Context("When a non-quorum node is added", func() {
		var (
			nodeIf   cmd.NodeInterface
			nonQnode string // the node being added and remove from cluster
			err      error
		)
		BeforeEach(func() {
			if e2eTest.NumQuorumNodes == e2eTest.NumVeniceNodes {
				Skip("All venice nodes are quorum nodes. Skipping non-quorum node addition tests")
			}
			numAvailNodes := rand.Intn(e2eTest.NumVeniceNodes - e2eTest.NumQuorumNodes)
			nonQnode = "node" + strconv.Itoa(e2eTest.NumQuorumNodes+1+numAvailNodes)

			apiGwAddr := e2eTest.ClusterVIP + ":" + globals.APIGwRESTPort
			cmdClient := cmdclient.NewRestCrudClientCmdV1(apiGwAddr)
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
					Roles: []cmd.NodeRole{
						cmd.NodeRole_CONTROLLER_NODE,
					},
				},
			}
			node, err = nodeIf.Create(context.Background(), node)
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
				out := vipCommandOutput("kubectl get pods  -l name=pen-ntp -o json")
				json.Unmarshal([]byte(out), &kubeOut)
				for _, i := range kubeOut.Items {
					if i.Spec.NodeName == nonQnode {
						return true
					}
				}
				return false
			}, 35, 1).Should(BeTrue(), "pen-ntp container should be scheduled by kubernetes on node %s", nonQnode)

			Eventually(func() string {
				return commandOutput(e2eTest.nameToIPMap[nonQnode], "/usr/bin/docker ps -q -f 'label=io.kubernetes.container.name=pen-ntp'")
			}, 35, 1).ShouldNot(BeEmpty(), "pen-ntp docker container should be running on %s", nonQnode)

		})

		AfterEach(func() {
			obj := api.ObjectMeta{Name: nonQnode}
			n, err2 := nodeIf.Delete(context.Background(), &obj)
			Expect(err2).ShouldNot(HaveOccurred())
			By(fmt.Sprintf("Disjoin %+v from cluster", n.Name))

		})
	})
})
