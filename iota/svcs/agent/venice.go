package agent

import (
	"os"
	"strings"

	"github.com/pkg/errors"

	iota "github.com/pensando/sw/iota/protos/gogen"
	utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Common "github.com/pensando/sw/iota/svcs/common"
)

const (
	veniceStartScript = "INSTALL.sh"
)

type veniceNode struct {
	iotaNode
}

type venicePeerNode struct {
	hostname string
	ip       string
}

func (venice *veniceNode) bringUpVenice(image string, hostname string,
	ctrlIntf string, ctrlIP string, peers []venicePeerNode) error {

	if ctrlIntf != "" {
		utils.DisableDhcpOnInterface(ctrlIntf)
		venice.logger.Println("Configuring intf : " + ctrlIntf + " with " + ctrlIP)
		ifConfigCmd := []string{"ifconfig", ctrlIntf, ctrlIP, "up"}
		if _, stdout, err := utils.Run(ifConfigCmd, 0, false, true, nil); err != nil {
			errors.New("Setting control interface IP to venice node failed.." + stdout)
		}
	}

	curDir, _ := os.Getwd()
	defer os.Chdir(curDir)
	os.Chdir(Common.DstIotaAgentDir)
	venice.logger.Println("Untar image : " + image)
	untar := []string{"tar", "-xvzf", image}
	if _, stdout, err := utils.Run(untar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	setHostname := []string{"hostnamectl", "set-hostname", hostname}
	if _, stdout, err := utils.Run(setHostname, 0, false, false, nil); err != nil {
		venice.logger.Println("Setting hostname failed")
		return errors.Wrap(err, stdout)
	}

	venice.logger.Println("Running Install Script : " + veniceStartScript)
	install := []string{"./" + veniceStartScript, "--clean"}
	if _, stdout, err := utils.Run(install, 0, false, false, nil); err != nil {
		venice.logger.Println("Running Install Script failed : " + veniceStartScript)
		return errors.Wrap(err, stdout)
	}

	for _, peer := range peers {
		if peer.hostname != "" && peer.ip != "" {
			cmd := []string{"echo", strings.Split(peer.ip, "/")[0], peer.hostname, " | sudo tee -a /etc/hosts"}
			if _, stdout, err := utils.Run(cmd, 0, false, true, nil); err != nil {
				venice.logger.Println("Setting venice peer hostnames failed")
				return errors.Wrap(err, stdout)
			}
		}
	}
	return nil
}

//Init initalize node type
func (venice *veniceNode) Init(in *iota.Node) (*iota.Node, error) {
	venice.iotaNode.name = in.GetName()
	venice.logger.Println("Bring up request received for : " + in.GetName())

	veniceNodes := []venicePeerNode{}

	for _, node := range in.GetVeniceConfig().GetVenicePeers() {
		veniceNodes = append(veniceNodes, venicePeerNode{hostname: node.GetHostName(),
			ip: node.GetIpAddress()})
	}
	if err := venice.bringUpVenice(in.GetImage(), in.GetName(),
		in.GetVeniceConfig().GetControlIntf(), in.GetVeniceConfig().GetControlIp(),
		veniceNodes); err != nil {
		venice.logger.Println("Venice bring up failed.")
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, err

	}

	venice.logger.Println("Venice bring script up successful.")

	return &iota.Node{Name: in.Name, IpAddress: in.IpAddress, NodeUuid: "", Type: in.GetType(),
		NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}

// AddWorkload brings up a workload type on a given node
func (venice *veniceNode) AddWorkload(*iota.Workload) (*iota.Workload, error) {
	venice.logger.Println("Add workload on venice not supported.")
	return &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
}

// DeleteWorkload deletes a given workload
func (venice *veniceNode) DeleteWorkload(*iota.Workload) (*iota.Workload, error) {
	venice.logger.Println("Delete workload on venice not supported.")
	return &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (venice *veniceNode) Trigger(*iota.TriggerMsg) (*iota.TriggerMsg, error) {
	venice.logger.Println("Venice node does not support trigger.")
	return &iota.TriggerMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
}

// CheckHealth returns the node health
func (venice *veniceNode) CheckHealth(*iota.NodeHealth) (*iota.NodeHealth, error) {
	return nil, nil
}

//NodeType return node type
func (venice *veniceNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_VENICE
}

//GetMsg node msg
func (venice *veniceNode) GetMsg() *iota.Node {
	return venice.nodeMsg
}

//GetWorkloadMsgs get workloads
func (venice *veniceNode) GetWorkloadMsgs() []*iota.Workload {
	return nil
}
