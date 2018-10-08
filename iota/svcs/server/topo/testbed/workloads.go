package testbed

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// AddWorkload adds a workload on the node
func (n *TestNode) AddWorkload() error {
	resp, err := n.AgentClient.AddWorkload(context.Background(), n.Workload)
	if err != nil {
		log.Errorf("Adding node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if resp.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Adding node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.WorkloadStatus.ApiStatus)
		return fmt.Errorf("adding node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.WorkloadStatus.ApiStatus)
	}
	return nil
}
