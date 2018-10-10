package testbed

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// AddWorkload adds a workload on the node
func (n *TestNode) AddWorkload() error {
	for _, w := range n.Workloads {
		resp, err := n.AgentClient.AddWorkload(context.Background(), w)
		if err != nil {
			log.Errorf("Adding workload on node %v failed. Err: %v", n.Node.Name, err)
			return err
		}

		if resp.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			log.Errorf("Adding workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.WorkloadStatus.ApiStatus)
			return fmt.Errorf("adding workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.WorkloadStatus.ApiStatus)
		}
	}

	return nil
}
