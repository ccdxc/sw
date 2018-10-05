package agent

import (
	"context"
	"os"

	log "github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

// Service implements agent service APIs
type Service struct {
	node IotaNode
}

// NewAgentService returns an instance of Agent stub service
func NewAgentService() *Service {
	log.Info("IOTA Agent Started")
	os.Mkdir(agentDir, 0777)
	var agentServer Service
	return &agentServer
}

// AddNode brings up the node with the personality
func (agent *Service) AddNode(ctx context.Context, in *iota.Node) (*iota.Node, error) {

	log.Errorf("Receivied ADD Node Msg: %v", in)
	/* Check if the node running an instance */
	if agent.node != nil {
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	if agent.node = newIotaNode(in.Type); agent.node == nil {
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil

	}

	resp, err := agent.node.Init(in)

	if err != nil {
		/* Init file and no point pretentding to have a personality */
		agent.node = nil
	}

	return resp, nil
}

// DeleteNode remove the personaltiy set
func (agent *Service) DeleteNode(ctx context.Context, in *iota.Node) (*iota.Node, error) {

	/* Check if the node running an instance */
	if agent.node == nil {
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	resp, err := agent.node.Destroy(in)

	/* Unset ethe personality */
	agent.node = nil
	return resp, err

}

// AddWorkload brings up a workload type on a given node
func (agent *Service) AddWorkload(ctx context.Context, in *iota.Workload) (*iota.Workload, error) {

	/* Check if the node running an instance to add a workload */
	if agent.node == nil || agent.node.NodeName() != in.GetNodeName() {
		return &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	return agent.node.AddWorkload(in)
}

// DeleteWorkload deletes a given workload
func (agent *Service) DeleteWorkload(ctx context.Context, in *iota.Workload) (*iota.Workload, error) {
	/* Check if the node running an instance to add a workload */
	if agent.node == nil || agent.node.NodeName() != in.GetNodeName() {
		return &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	return agent.node.DeleteWorkload(in)
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (*Service) Trigger(context.Context, *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	return nil, nil

}

// CheckHealth returns the cluster health
func (agent *Service) CheckHealth(ctx context.Context, in *iota.NodeHealth) (*iota.NodeHealth, error) {
	/* Check if the node running an instance to add a workload */
	if agent.node == nil || agent.node.NodeName() != in.GetNodeName() {
		return &iota.NodeHealth{HealthCode: iota.NodeHealth_NODE_DOWN}, nil
	}

	return agent.node.CheckHealth(in)
}

func newService() *Service {
	return &Service{}
}

var iotaNodes = map[iota.PersonalityType]func() IotaNode{
	iota.PersonalityType_PERSONALITY_NAPLES:           newNaples,
	iota.PersonalityType_PERSONALITY_VENICE:           newVenice,
	iota.PersonalityType_PERSONALITY_NAPLES_WITH_QEMU: newNaplesQemu,
}

func newNaples() IotaNode {
	return &naplesNode{iotaNode: iotaNode{name: "naples"}}
}

func newVenice() IotaNode {
	return &veniceNode{iotaNode: iotaNode{name: "venice"}}
}

func newNaplesQemu() IotaNode {
	return &naplesQemuNode{naplesNode: naplesNode{iotaNode: iotaNode{name: "naples_qemu"}}}
}

func newIotaNode(nodeType iota.PersonalityType) IotaNode {
	if _, ok := iotaNodes[nodeType]; ok {
		return iotaNodes[nodeType]()
	}

	return nil
}
