package testbed

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// Trigger triggers a workload
func (n *TestNode) Trigger(triggerReq *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	return n.TriggerWithContext(context.Background(), triggerReq)
}

// TriggerWithContext runs a trigger on workload
func (n *TestNode) TriggerWithContext(ctx context.Context,
	triggerReq *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	resp, err := n.AgentClient.Trigger(ctx, triggerReq)
	log.Infof("TOPO SVC | DEBUG | Trigger Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Running trigger on node %v failed. Err: %v", n.Node.Name, err)
		return nil, err
	}

	if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Running trigger on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
		return nil, fmt.Errorf("Running trigger on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
	}

	return resp, nil
}
