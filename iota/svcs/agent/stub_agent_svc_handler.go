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

// DeleteNode remove the personaltiy set
func (*StubService) DeleteNode(ctx context.Context, req *iota.Node) (*iota.Node, error) {
	resp := &iota.Node{}
	return resp, nil
}

// AddWorkload brings up a workload type on a given node
func (*StubService) AddWorkload(ctx context.Context, req *iota.Workload) (*iota.Workload, error) {
	resp := &iota.Workload{}
	return resp, nil
}

// DeleteWorkload deletes a given workload
func (*StubService) DeleteWorkload(ctx context.Context, req *iota.Workload) (*iota.Workload, error) {
	resp := &iota.Workload{}
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
