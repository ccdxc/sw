package testbed

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// Trigger triggers a workload
func (n *TestNode) Trigger(index int) error {
	resp, err := n.AgentClient.Trigger(context.Background(), n.TriggerInfo[index])
	log.Infof("TOPO SVC | DEBUG | Trigger Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Running trigger on node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Adding workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
		return fmt.Errorf("adding workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
	}

	n.TriggerResp[index] = resp
	return nil
}
