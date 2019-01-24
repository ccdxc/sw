package testbed

import (
	"context"
	"fmt"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// AddWorkloads adds a workload on the node
func (n *TestNode) AddWorkloads(w *iota.WorkloadMsg) error {
	log.Infof("TOPO SVC | DEBUG | STATE | WORKLOAD: %v", w)
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Minute)
	defer cancel()
	resp, err := n.AgentClient.AddWorkloads(ctx, w)
	log.Infof("TOPO SVC | DEBUG | AddWorkload Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Adding workload on node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Adding workloads on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
		return fmt.Errorf("adding workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
	}

	n.WorkloadResp = resp
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
