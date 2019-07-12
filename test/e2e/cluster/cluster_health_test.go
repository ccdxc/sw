package cluster

import (
	"context"
	"fmt"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
)

// Contains all the cluster health related tests

var _ = Describe("cluster health tests", func() {
	var (
		obj       api.ObjectMeta
		clusterIf cmd.ClusterV1ClusterInterface
	)
	BeforeEach(func() {
		validateCluster()

		apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
		apiClient, err := apiclient.NewRestAPIClient(apiGwAddr)
		Expect(err).ShouldNot(HaveOccurred())
		clusterIf = apiClient.ClusterV1().Cluster()
		ctx := ts.tu.NewLoggedInContext(context.Background())
		obj = api.ObjectMeta{Name: "testCluster"}
		_, err = clusterIf.Get(ctx, &obj)
		Expect(err).ShouldNot(HaveOccurred())
	})

	It(fmt.Sprintf("Test cluster health by killing %s (venice service)", globals.EvtsMgr), func() {
		Skip("skipping this test until we get PR #13927 fixed and working")

		// repeatedly kill evtsmgr on one of the nodes and check cluster health
		nodeIP := ts.tu.VeniceNodeIPs[0]
		Eventually(func() bool {
			out := ts.tu.CommandOutput(nodeIP,
				fmt.Sprintf("docker ps | grep %s | grep -v pause | awk '{print $1}'", globals.EvtsMgr))
			if !utils.IsEmpty(out) {
				By(fmt.Sprintf("Killing %s on %v", globals.EvtsMgr, nodeIP))
				ts.tu.CommandOutput(nodeIP, fmt.Sprintf("docker kill %s", out))
			}
			return checkClusterHealth(clusterIf, &obj, cmd.ConditionStatus_FALSE.String(),
				fmt.Sprintf("Service %s failed to run desired number of instances", globals.EvtsMgr))
		}, 150, 2).Should(BeTrue(),
			fmt.Sprintf("Cluster status is expected to be un-healthy as %s is not running on %s",
				globals.EvtsMgr, nodeIP))

		// pod should run the container again and things should be back to normal
		Eventually(func() bool {
			return checkClusterHealth(clusterIf, &obj, cmd.ConditionStatus_TRUE.String(), "")
		}, 150, 2).Should(BeTrue(),
			fmt.Sprintf("Cluster status is expected to be healthy as %s should be started back on node %s",
				globals.EvtsMgr, nodeIP))
	})

	It("Test cluster health by pausing/resuming CMD", func() {
		if ts.tu.NumQuorumNodes < 2 {
			Skip(fmt.Sprintf("Skipping: %d quorum nodes found, need >= 2", ts.tu.NumQuorumNodes))
		}

		// pause cmd on one of the nodes and check cluster health
		nodeIP := ts.tu.VeniceNodeIPs[0]
		By(fmt.Sprintf("Pausing %s on %v", globals.Cmd, nodeIP))
		ts.tu.CommandOutput(nodeIP, fmt.Sprintf("docker pause %s", globals.Cmd))
		Eventually(func() bool {
			return checkClusterHealth(clusterIf, &obj, cmd.ConditionStatus_FALSE.String(),
				fmt.Sprintf("node %s is not healthy", ts.tu.IPToNameMap[nodeIP]))
		}, 200, 2).Should(BeTrue(),
			fmt.Sprintf("Cluster status is expected to be un-healthy as %s is not running on %s", globals.Cmd, nodeIP))

		// resume cmd and check cluster status
		By(fmt.Sprintf("Unpausing %s on %v", globals.Cmd, nodeIP))
		ts.tu.CommandOutput(nodeIP, fmt.Sprintf("docker unpause %s", globals.Cmd))
		Eventually(func() bool {
			return checkClusterHealth(clusterIf, &obj, cmd.ConditionStatus_TRUE.String(), "")
		}, 200, 2).Should(BeTrue(),
			fmt.Sprintf("Cluster status is expected to be healthy as %s is resumed back on node %s",
				globals.Cmd, nodeIP))
	})
})

// helper to check if the cluster health meets the given expected condition
func checkClusterHealth(clusterIf cmd.ClusterV1ClusterInterface, clusterObjMeta *api.ObjectMeta,
	expectedHealthCondition, expectedReason string) bool {
	cl, err := clusterIf.Get(ts.tu.NewLoggedInContext(context.Background()), clusterObjMeta)
	if err != nil {
		return false
	}
	for _, cond := range cl.Status.Conditions {
		if cond.Type == cmd.ClusterCondition_HEALTHY.String() {
			return cond.Status == expectedHealthCondition && cond.Reason == expectedReason
		}
	}
	return false
}
