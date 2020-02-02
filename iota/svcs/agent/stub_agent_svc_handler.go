package agent

// This handler is used for testing only. It should be nuked once we are IOTA server and IOTA Agent are ready for
// integ testing
import (
	"context"

	log "github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

// StubService implements stubbed out agent service APIs
type StubService struct{}

// NewAgentStubService returns an instance of Agent stub service
func NewAgentStubService() *StubService {
	log.Info("IOTA Agent Started in Stub Mode")
	var agentStubServer StubService
	return &agentStubServer
}

// AddNode brings up the node with the personality
func (*StubService) AddNode(ctx context.Context, req *iota.Node) (*iota.Node, error) {
	resp := &iota.Node{}
	return resp, nil
}

// SaveNode save node personality for reboot
func (*StubService) SaveNode(ctx context.Context, req *iota.Node) (*iota.Node, error) {
	resp := &iota.Node{}
	return resp, nil
}

// ReloadNode saves and loads node personality
func (*StubService) ReloadNode(ctx context.Context, req *iota.Node) (*iota.Node, error) {
	resp := &iota.Node{}
	return resp, nil
}

// DeleteNode remove the personaltiy set
func (*StubService) DeleteNode(ctx context.Context, req *iota.Node) (*iota.Node, error) {
	resp := &iota.Node{}
	return resp, nil
}

// AddWorkloads brings up a workload type on a given node
func (*StubService) AddWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	resp := &iota.WorkloadMsg{}
	return resp, nil
}

// DeleteWorkloads deletes a given workloads
func (*StubService) DeleteWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	resp := &iota.WorkloadMsg{}
	return resp, nil

}

// GetWorkloads  get a given workloads
func (*StubService) GetWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	resp := &iota.WorkloadMsg{}
	return resp, nil

}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (*StubService) Trigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	resp := &iota.TriggerMsg{}
	return resp, nil
}

// CheckHealth returns the cluster health
func (*StubService) CheckHealth(ctx context.Context, req *iota.NodeHealth) (*iota.NodeHealth, error) {
	resp := &iota.NodeHealth{}
	return resp, nil
}
