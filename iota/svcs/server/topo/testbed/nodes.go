package testbed

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// AddNode adds a node to the topology
func (n *TestNode) AddNode() error {
	resp, err := n.AgentClient.AddNode(context.Background(), n.Node)
	log.Infof("TOPO SVC | DEBUG | AddNode Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Adding node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if resp.NodeStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Adding node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.NodeStatus.ApiStatus)
		return fmt.Errorf("adding node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.NodeStatus.ApiStatus)
	}
	return nil
}
