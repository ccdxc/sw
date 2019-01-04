package testbed

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// AddWorkload adds a workload on the node
func (n *TestNode) AddWorkload(w *iota.Workload) error {
	log.Infof("TOPO SVC | DEBUG | STATE | WORKLOAD: %v", w)
	resp, err := n.AgentClient.AddWorkload(context.Background(), w)
	log.Infof("TOPO SVC | DEBUG | AddWorkload Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Adding workload on node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if resp.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Adding workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.WorkloadStatus.ApiStatus)
		return fmt.Errorf("adding workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.WorkloadStatus.ApiStatus)
	}

	n.WorkloadResp.Workloads = append(n.WorkloadResp.Workloads, resp)
	return nil
}

// DeleteWorkload delete a workload on the node
func (n *TestNode) DeleteWorkload(w *iota.Workload) error {
	log.Infof("TOPO SVC | DEBUG | STATE | WORKLOAD: %v", w)
	resp, err := n.AgentClient.DeleteWorkload(context.Background(), w)
	log.Infof("TOPO SVC | DEBUG | DeleteWorkload Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Deleting workload on node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if resp.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Deleting workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.WorkloadStatus.ApiStatus)
		return fmt.Errorf("Deleting workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.WorkloadStatus.ApiStatus)
	}

	n.WorkloadResp.Workloads = append(n.WorkloadResp.Workloads, resp)
	return nil
}
