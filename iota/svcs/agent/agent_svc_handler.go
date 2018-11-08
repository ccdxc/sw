package agent

import (
	"context"
	"io"
	"os"

	log "github.com/sirupsen/logrus"

	iota "github.com/pensando/sw/iota/protos/gogen"

	"github.com/pensando/sw/iota/svcs/common"
)

// Service implements agent service APIs
type Service struct {
	node   IotaNode
	logger *log.Logger
}

// NewAgentService returns an instance of Agent stub service
func NewAgentService() *Service {
	log.Info("IOTA Agent Started")
	var agentServer Service
	agentServer.init()

	return &agentServer
}

func (agent *Service) init() {

	agent.logger = log.New()
	file, err := os.OpenFile(common.DstIotaAgentDir+"/"+"agent.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
	if err != nil {
		log.Fatalln("Failed to open log file", "file.txt", ":", err)
	}
	agent.logger.Out = io.MultiWriter(file, os.Stdout)

	if _, err := os.Stat(common.DstIotaEntitiesDir); err != nil || !os.IsNotExist(err) {
		agent.logger.Println("Creating directory for entities...")
		os.Mkdir(common.DstIotaEntitiesDir, 0755)
		//os.Chmod(common.DstIotaEntitiesDir, 0766)
	}
	agent.logger.Println("Agent initialized...")
}

// AddNode brings up the node with the personality
func (agent *Service) AddNode(ctx context.Context, in *iota.Node) (*iota.Node, error) {

	agent.logger.Printf("Received add node :%v", in)
	/* Check if the node running an instance */
	if agent.node != nil {
		agent.logger.Errorf("Node already has personality type : %s", agent.node.NodeType())
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	if agent.node = newIotaNode(in.Type); agent.node == nil {
		agent.logger.Errorf("Personality type not supported %d", in.Type)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil

	}

	agent.node.SetLogger(agent.logger)
	resp, err := agent.node.Init(in)

	if err != nil {
		/* Init file and no point pretentding to have a personality */
		agent.logger.Errorf("Personality Init failed for type %d", in.Type)
		agent.node = nil
	}

	return resp, nil
}

// DeleteNode remove the personaltiy set
func (agent *Service) DeleteNode(ctx context.Context, in *iota.Node) (*iota.Node, error) {

	/* Check if the node running an instance */
	agent.logger.Printf("Received delete node :%v", in)
	if agent.node == nil {
		agent.logger.Errorf("Delete Node received with no personality set : %d", in.Type)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	agent.logger.Println("Deleting node personality : %s", agent.node.NodeType())
	resp, err := agent.node.Destroy(in)

	/* Unset ethe personality */
	agent.node = nil
	return resp, err

}

// AddWorkload brings up a workload type on a given node
func (agent *Service) AddWorkload(ctx context.Context, in *iota.Workload) (*iota.Workload, error) {

	/* Check if the node running an instance to add a workload */
	agent.logger.Printf("Received Add workload : %v", in)
	if agent.node == nil || agent.node.NodeName() != in.GetNodeName() {
		agent.logger.Println("Invalid workload add request received")
		return &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	return agent.node.AddWorkload(in)
}

// DeleteWorkload deletes a given workload
func (agent *Service) DeleteWorkload(ctx context.Context, in *iota.Workload) (*iota.Workload, error) {
	agent.logger.Printf("Received delete workload : %v", in)
	/* Check if the node running an instance to add a workload */
	if agent.node == nil || agent.node.NodeName() != in.GetNodeName() {
		agent.logger.Println("Invalid workload delete request received")
		return &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	return agent.node.DeleteWorkload(in)
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (agent *Service) Trigger(ctx context.Context, in *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	agent.logger.Printf("Trigger messasge received : %v", in)
	/* Check if the node running an instance to add a workload */
	if agent.node == nil {
		agent.logger.Println("Invalid trigger message received on unintialized node")
		return &iota.TriggerMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	return agent.node.Trigger(in)
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
	iota.PersonalityType_PERSONALITY_NAPLES:               newNaples,
	iota.PersonalityType_PERSONALITY_NAPLES_SIM:           newNaplesSim,
	iota.PersonalityType_PERSONALITY_VENICE:               newVenice,
	iota.PersonalityType_PERSONALITY_NAPLES_SIM_WITH_QEMU: newNaplesQemu,
	iota.PersonalityType_PERSONALITY_MELLANOX:             newMellanox,
}

func newNaples() IotaNode {
	return &naplesHwNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples"}}}
}

func newMellanox() IotaNode {
	return &mellanoxNode{dataNode: dataNode{iotaNode: iotaNode{name: "mellanox"}}}
}

func newNaplesSim() IotaNode {
	return &naplesSimNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples-sim"}}}
}

func newVenice() IotaNode {
	return &veniceNode{iotaNode: iotaNode{name: "venice"}}
}

func newNaplesQemu() IotaNode {
	return &naplesQemuNode{naplesSimNode: naplesSimNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples-qemu"}}}}
}

func newIotaNode(nodeType iota.PersonalityType) IotaNode {
	if _, ok := iotaNodes[nodeType]; ok {
		return iotaNodes[nodeType]()
	}

	return nil
}
