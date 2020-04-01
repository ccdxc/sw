package cloud

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
)

// helper to check if the quorum is healthy and size is as expected
func checkQuorumHealth(clusterIf cmd.ClusterV1ClusterInterface, clusterObjMeta *api.ObjectMeta, quorumSize int) error {
	cl, err := clusterIf.Get(ts.tu.MustGetLoggedInContext(context.Background()), clusterObjMeta)
	if err != nil {
		return fmt.Errorf("Unable to get cluster object")
	}
	if len(cl.Status.QuorumStatus.Members) != quorumSize {
		return fmt.Errorf("Unexpected quorum size. Have: %d, want: %d. Status: %+v", len(cl.Status.QuorumStatus.Members), quorumSize, cl.Status.QuorumStatus)
	}
	for _, member := range cl.Status.QuorumStatus.Members {
		for _, cond := range member.Conditions {
			if cond.Type == cmd.ClusterCondition_HEALTHY.String() {
				if cond.Status != cmd.ConditionStatus_TRUE.String() || cond.LastTransitionTime == nil {
					return fmt.Errorf("Unexpected quorum condition for member %s: %+v", member.Name, cond)
				}
			}
		}
	}
	return nil
}
