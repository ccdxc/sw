package agent

import (
	"context"
	"encoding/gob"
	"fmt"
	"io"
	"os"
    "os/exec"
    "time"

	"github.com/pkg/errors"
	log "github.com/sirupsen/logrus"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Cmd "github.com/pensando/sw/iota/svcs/agent/command"

	"github.com/pensando/sw/iota/svcs/agent/utils"
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

	cmdOutDir := common.DstIotaAgentDir + "/cmdOut"
	if _, err := os.Stat(cmdOutDir); err != nil || !os.IsNotExist(err) {
		agent.logger.Println("Creating directory for command outputs..")
		os.Mkdir(cmdOutDir, 0755)
	}
	Cmd.SetOutputDirectory(cmdOutDir)

	dbDir := common.DstIotaDBDir
	if _, err := os.Stat(dbDir); err != nil || !os.IsNotExist(err) {
		agent.logger.Println("Creating directory for agent db ")
		os.Mkdir(dbDir, 0755)
	}

	utils.SSHKeygen()
	agent.logger.Println("Agent initialized...")
}

// AddNode brings up the node with the personality
func (agent *Service) AddNode(ctx context.Context, in *iota.Node) (*iota.Node, error) {

	agent.logger.Printf("Received add node :%v", in)

	/* Check if the node running an instance */
	if agent.node != nil {
		msg := fmt.Sprintf("Node already has personality type : %s", agent.node.NodeType())
		agent.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	if agent.node = newIotaNode(in.Type, in.Os); agent.node == nil {
		msg := fmt.Sprintf("Personality type not supported %d", in.Type)
		agent.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	agent.node.SetLogger(agent.logger)
	resp, err := agent.node.Init(in)

	if err != nil {
		/* Init file and no point pretentding to have a personality */
		agent.logger.Errorf("Personality Init failed for type %d", in.Type)
		agent.node = nil
	}

	resp.Name = in.GetName()

	return resp, nil
}

func gobFile(name string) string {
	return name + ".gob"
}

func wloadsGobFile(name string) string {
	return name + "_workloads.gob"
}

// SaveNode save node personality for reboot
func (agent *Service) SaveNode(ctx context.Context, req *iota.Node) (*iota.Node, error) {
	agent.logger.Printf("Received save node :%v", req)

	/* Check if the node running an instance */
	if agent.node == nil {
		msg := fmt.Sprintf("No node personality assigned")
		agent.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	msg, _ := agent.node.GetMsg().Marshal()
	err := writeGob(common.DstIotaDBDir+"/"+gobFile(agent.node.NodeName()), msg)
	if err != nil {
		msg := fmt.Sprintf("Saving Node failed %v", err)
		agent.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	workloadMsgs := agent.node.GetWorkloadMsgs()

	if len(workloadMsgs) != 0 {
		wloadGob := wloadsGobFile(agent.node.NodeName())
		msgs := [][]byte{}
		for _, wloadMsg := range workloadMsgs {
			if wloadMsg == nil {
				continue
			}
			agent.logger.Printf(" Workload msg %v\n", wloadMsg)
			msg, _ := wloadMsg.Marshal()
			msgs = append(msgs, msg)
		}
		err := writeGob(common.DstIotaDBDir+"/"+wloadGob, msgs)
		if err != nil {
			msg := fmt.Sprintf("Saving Node failed %v", err)
			agent.logger.Error(msg)
			return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
		}

	}

    exec.Command("sync")
    time.Sleep(5 * time.Second)
	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}

func (agent *Service) reloadWorkloads(ctx context.Context, req *iota.Node) error {
	wloads := []*iota.Workload{}
	//Read workload messages
	var rawWloadmsg = [][]byte{}
	wloadfile := common.DstIotaDBDir + "/" + wloadsGobFile(req.GetName())
	if _, err := os.Stat(wloadfile); err == nil {
		err = readGob(wloadfile, &rawWloadmsg)
		if err != nil {
			msg := fmt.Sprintf("reload node of type : %s failed : %v", req.GetType(), err.Error())
			return errors.Wrap(err, msg)
		}
		for _, msg := range rawWloadmsg {
			var wload = new(iota.Workload)
			wload.Unmarshal(msg)
			wloads = append(wloads, wload)
		}
	}
	//Add workloads
	wloadMsg := &iota.WorkloadMsg{Workloads: wloads}
	if resp, err := agent.AddWorkloads(ctx, wloadMsg); err != nil || resp.GetApiResponse().GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
		return err
	}

	return nil
}

// ReloadNode saves and loads node personality
func (agent *Service) ReloadNode(ctx context.Context, req *iota.Node) (*iota.Node, error) {

	/* Check if the node running an instance */
	if agent.node != nil {
		msg := fmt.Sprintf("Node already has personality type : %s", agent.node.NodeType())
		agent.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	var node = new(iota.Node)
	rawmsg := []byte{}
	err := readGob(common.DstIotaDBDir+"/"+gobFile((req.GetName())), &rawmsg)
	if err != nil {
		msg := fmt.Sprintf("reload node of type : %s failed : %v", req.GetType(), err.Error())
		agent.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	node.Unmarshal(rawmsg)

	node.Reload = true
	if resp, err := agent.AddNode(ctx, node); err != nil || resp.GetNodeStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return resp, nil
	}

	if err := agent.reloadWorkloads(ctx, req); err != nil {
		msg := fmt.Sprintf("reload workloads  failed : %v", err.Error())
		agent.logger.Error(msg)
		agent.node.Destroy(node)
		agent.node = nil
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}

// DeleteNode remove the personaltiy set
func (agent *Service) DeleteNode(ctx context.Context, in *iota.Node) (*iota.Node, error) {

	/* Check if the node running an instance */
	agent.logger.Printf("Received delete node :%v", in)
	if agent.node == nil {
		msg := fmt.Sprintf("Delete Node received with no personality set : %d", in.Type)
		agent.logger.Errorf(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	agent.logger.Printf("Deleting node personality : %d", agent.node.NodeType())
	resp, _ := agent.node.Destroy(in)

	/* Unset ethe personality */
	agent.node = nil
	return resp, nil

}

// AddWorkloads brings up a workload type on a given node
func (agent *Service) AddWorkloads(ctx context.Context, in *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {

	/* Check if the node running an instance to add a workload */
	agent.logger.Printf("Received Add workloads : %v", in)
	if agent.node == nil {
		msg := fmt.Sprintf("Invalid workload add request received")
		agent.logger.Error(msg)
		return &iota.WorkloadMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	resp, err := agent.node.AddWorkloads(in)

	if err != nil {

	}

	return resp, nil
}

// GetWorkloads brings up a workload type on a given node
func (agent *Service) GetWorkloads(ctx context.Context, in *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {

	/* Check if the node running an instance to add a workload */
	agent.logger.Printf("Received Add workloads : %v", in)
	if agent.node == nil {
		msg := fmt.Sprintf("Invalid workload add request received")
		agent.logger.Error(msg)
		return &iota.WorkloadMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	in.Workloads = agent.node.GetWorkloadMsgs()

	return in, nil
}

// DeleteWorkloads deletes workload specified
func (agent *Service) DeleteWorkloads(ctx context.Context, in *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	agent.logger.Printf("Received delete workloads : %v", in)
	/* Check if the node running an instance to add a workload */
	if agent.node == nil {
		msg := fmt.Sprintf("Invalid workload delete request received")
		agent.logger.Error(msg)
		return &iota.WorkloadMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	resp, err := agent.node.DeleteWorkloads(in)
	if err != nil {

	}

	return resp, nil
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (agent *Service) Trigger(ctx context.Context, in *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	agent.logger.Printf("Trigger message received : %v", in)
	/* Check if the node running an instance to add a workload */
	if agent.node == nil {
		msg := fmt.Sprintf("Invalid trigger message received on unintialized node")
		return &iota.TriggerMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}, nil
	}

	resp, err := agent.node.Trigger(in)

	if err != nil {

	}

	return resp, nil
}

// CheckHealth returns the cluster health
func (agent *Service) CheckHealth(ctx context.Context, in *iota.NodeHealth) (*iota.NodeHealth, error) {
	/* Check if the node running an instance to add a workload */
	if agent.node == nil || agent.node.NodeName() != in.GetNodeName() {
		agent.logger.Printf("Check health failed %v %v\n", agent.node.NodeName(), in.GetNodeName())
		return &iota.NodeHealth{HealthCode: iota.NodeHealth_NODE_DOWN}, nil
	}

	resp, err := agent.node.CheckHealth(in)

	if err != nil {

	}

	return resp, nil
}

func newService() *Service {
	return &Service{}
}

var iotaNodes = map[iota.PersonalityType]func() IotaNode{
	iota.PersonalityType_PERSONALITY_NAPLES:               newNaples,
	iota.PersonalityType_PERSONALITY_NAPLES_BITW:          newNaplesBitw,
	iota.PersonalityType_PERSONALITY_NAPLES_BITW_PERF:     newNaplesBitwPerf,
	iota.PersonalityType_PERSONALITY_NAPLES_SIM:           newNaplesSim,
	iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM:     newNaplesMultiSim,
	iota.PersonalityType_PERSONALITY_VENICE:               newVenice,
	iota.PersonalityType_PERSONALITY_VENICE_BM:            newVeniceBM,
	iota.PersonalityType_PERSONALITY_NAPLES_SIM_WITH_QEMU: newNaplesQemu,
	iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC:      newThirdPartyNic,
	iota.PersonalityType_PERSONALITY_COMMAND_NODE:         newCommandNode,
}

func newNaples() IotaNode {
	return &naplesHwNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples"}}}
}

func newNaplesBitw() IotaNode {
	return &naplesBitwHwNode{naplesHwNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples"}}}}
}

func newNaplesBitwPerf() IotaNode {
	return &naplesBitwPerfHwNode{naplesHwNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples"}}}}
}

func newEsxNaples() IotaNode {
	return &esxNaplesHwNode{esxHwNode: esxHwNode{naplesHwNode: naplesHwNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples-esx"}}}}}
}

func newEsxNaplesDvs() IotaNode {
	return &esxNaplesDvsHwNode{esxNaplesHwNode: esxNaplesHwNode{esxHwNode: esxHwNode{naplesHwNode: naplesHwNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples-esx-dvs"}}}}}}

}

func newEsxThirdPartyNic() IotaNode {
	return &esxThirdPartyHwNode{esxHwNode: esxHwNode{naplesHwNode: naplesHwNode{dataNode: dataNode{iotaNode: iotaNode{name: "third-party-esx"}}}}}
}

func newEsxThirdPartyNicDvs() IotaNode {
	return &esxThirdPartyDvsHwNode{esxThirdPartyHwNode: esxThirdPartyHwNode{esxHwNode: esxHwNode{naplesHwNode: naplesHwNode{dataNode: dataNode{iotaNode: iotaNode{name: "third-party-esx-dvs"}}}}}}
}

func newThirdPartyNic() IotaNode {
	return &thirdPartyDataNode{dataNode: dataNode{iotaNode: iotaNode{name: "third-party"}}}
}

func newCommandNode() IotaNode {
	return &commandNode{iotaNode: iotaNode{name: "command-node"}}
}

func newNaplesMultiSim() IotaNode {
	return &naplesMultiSimNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples-multi-sim"}}}
}

func newNaplesSim() IotaNode {
	return &naplesSimNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples-sim"}}}
}

func newVenice() IotaNode {
	return &veniceNode{commandNode: commandNode{iotaNode: iotaNode{name: "venice"}}}
}

func newVeniceBM() IotaNode {
	return &veniceBMNode{commandNode: commandNode{iotaNode: iotaNode{name: "venice"}}}
}

func newNaplesQemu() IotaNode {
	return &naplesQemuNode{naplesSimNode: naplesSimNode{dataNode: dataNode{iotaNode: iotaNode{name: "naples-qemu"}}}}
}

func newIotaNode(nodeType iota.PersonalityType, os iota.TestBedNodeOs) IotaNode {

	//Hack for now as its just one type
	if nodeType == iota.PersonalityType_PERSONALITY_NAPLES && os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		return newEsxNaples()
	} else if nodeType == iota.PersonalityType_PERSONALITY_NAPLES_DVS && os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		return newEsxNaplesDvs()
	} else if nodeType == iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC && os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		return newEsxThirdPartyNic()
	} else if nodeType == iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC_DVS && os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		return newEsxThirdPartyNicDvs()
	}

	if _, ok := iotaNodes[nodeType]; ok {
		return iotaNodes[nodeType]()
	}

	return nil
}

func writeGob(filePath string, object interface{}) error {
	file, err := os.Create(filePath)
	if err == nil {
		encoder := gob.NewEncoder(file)
		encoder.Encode(object)
	}
	file.Sync()
	file.Close()
	return err
}

func readGob(filePath string, object interface{}) error {
	fmt.Println("Read file : ", filePath)
	file, err := os.Open(filePath)
	if err == nil {
		decoder := gob.NewDecoder(file)
		err = decoder.Decode(object)
	}
	file.Close()
	return err
}
