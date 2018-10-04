package agent

import (
	"context"

	log "github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

// Service implements agent service APIs
type Service struct{}

// NewAgentService returns an instance of Agent stub service
func NewAgentService() *Service {
	log.Info("IOTA Agent Started")
	var agentServer Service
	return &agentServer
}

// AddNode brings up the node with the personality
func (*Service) AddNode(ctx context.Context, req *iota.IotaNode) (*iota.IotaNode, error) {
	resp := &iota.IotaNode{}
	return resp, nil
}

// DeleteNode remove the personaltiy set
func (*Service) DeleteNode(ctx context.Context, req *iota.Node) (*iota.Node, error) {
	resp := &iota.Node{}
	return resp, nil
}

// AddWorkload brings up a workload type on a given node
func (*Service) AddWorkload(ctx context.Context, req *iota.Workload) (*iota.Workload, error) {
	resp := &iota.Workload{}
	return resp, nil
}

// DeleteWorkload deletes a given workload
func (*Service) DeleteWorkload(ctx context.Context, req *iota.Workload) (*iota.Workload, error) {
	resp := &iota.Workload{}
	return resp, nil

}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (*Service) Trigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	resp := &iota.TriggerMsg{}
	return resp, nil
}

// CheckHealth returns the cluster health
func (*Service) CheckHealth(ctx context.Context, req *iota.NodeHealth) (*iota.NodeHealth, error) {
	resp := &iota.NodeHealth{}
	return resp, nil
}
