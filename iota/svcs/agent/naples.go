package agent

import (
	"os"
	"strings"

	"github.com/pkg/errors"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Common "github.com/pensando/sw/iota/svcs/common"
)

const (
	naplesSimName         = "naples-sim"
	naplesVMBringUpScript = "naples_vm_bringup.py"
)

var (
	naplesProcessess = [...]string{"hal", "netagent", "nic_infra_hntap", "cap_model"}
)

type naplesNode struct {
	iotaNode
	simName    string
	container  *Utils.Container
	worloadMap map[string]workload
}

type naplesQemuNode struct {
	naplesNode
}

func (naples *naplesNode) bringUpNaples(name string, image string, ctrlIntf string,
	ctrlIP string, dataIntfs []string, dataIPs []string, naplesIPs []string,
	veniceIPs []string, withQemu bool, passThroughMode bool) error {
	if err := os.Chdir(Common.DstIotaAgentDir); err != nil {
		return err
	}
	dir, _ := os.Getwd()
	naples.log("Untar image : " + dir + "/" + image)
	untar := []string{"tar", "-xvzf", image}
	if _, stdout, err := Utils.Run(untar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}
	naples.log("Untar successfull")
	env := []string{"NAPLES_HOME=" + Common.DstIotaAgentDir}
	cmd := []string{"sudo", "-E", "python", naplesVMBringUpScript,
		"--data-intfs", strings.Join(dataIntfs, ",")}

	if ctrlIntf != "" {
		cmd = append(cmd, "--control-intf")
		cmd = append(cmd, ctrlIntf)
		cmd = append(cmd, "--control-ip")
		cmd = append(cmd, ctrlIP)
	}

	if len(dataIPs) != 0 {
		cmd = append(cmd, "--data-ips")
		cmd = append(cmd, strings.Join(dataIPs, ","))
	}

	if len(naplesIPs) != 0 {
		cmd = append(cmd, "--naples-ips")
		cmd = append(cmd, strings.Join(naplesIPs, ","))
	}

	if len(veniceIPs) != 0 {
		cmd = append(cmd, "--venice-ips")
		cmd = append(cmd, strings.Join(veniceIPs, ","))
	}

	if withQemu {
		cmd = append(cmd, "--qemu")
	}

	cmd = append(cmd, "--hntap-mode")
	if passThroughMode {
		cmd = append(cmd, "passthrough")
	} else {
		cmd = append(cmd, "tunnel")
	}

	if retCode, stdout, err := Utils.Run(cmd, nodeAddTimeout, false, false, env); err != nil || retCode != 0 {
		naples.log(stdout)
		naples.log("Naples bring script up failed.")
		return err
	}

	naples.log("Naples bring script up succesfull.")

	var err error
	if naples.container, err = Utils.GetContainer(naplesSimName, "", naplesSimName); err != nil {
		return errors.Wrap(err, "Naples sim not running!")
	}

	naples.simName = name
	return nil
}

func (naples *naplesNode) init(in *iota.Node, withQemu bool) (resp *iota.Node, err error) {

	naples.iotaNode.name = in.GetNodeName()
	naples.worloadMap = make(map[string]workload)
	naples.log("Bring up request received for : " + in.GetNodeName())
	if err := naples.bringUpNaples(in.GetNodeName(),
		in.GetImage(), in.GetNaplesConfig().GetControlIntf(),
		in.GetNaplesConfig().GetControlIp(),
		in.GetNaplesConfig().GetDataIntfs(), nil, nil,
		in.GetNaplesConfig().GetVeniceIps(), withQemu, true); err != nil {
		resp := "Naples bring up failed : " + err.Error()
		naples.log(resp)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, err
	}
	naples.log("Naples bring up succesfull")

	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}

//Init initalize node type
func (naples *naplesNode) Init(in *iota.Node) (resp *iota.Node, err error) {
	return naples.init(in, false)
}

// AddWorkload brings up a workload type on a given node
func (naples *naplesNode) AddWorkload(in *iota.Workload) (*iota.Workload, error) {

	naples.logger.Printf("Adding workload : %s", in.GetWorkloadName())
	if _, ok := naples.worloadMap[in.GetWorkloadName()]; ok {
		naples.log("Trying to add workload which already exists")
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}
		return resp, nil
	}

	wload := newWorkload()
	naples.logger.Printf("Added workload : %s", in.GetWorkloadName())
	if err := wload.BringUp(in.GetWorkloadName(), workloadImage); err != nil {
		naples.logger.Errorf("Error in workload image bring up : %s : %s", in.GetWorkloadName(), err.Error())
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}
		return resp, nil
	}
	naples.logger.Printf("Bring up workload : %s done", in.GetWorkloadName())
	naples.worloadMap[in.GetWorkloadName()] = wload

	if err := wload.AttachInterface(in.GetInterface(), in.GetMacAddress(), in.GetIpAddress(), int(in.GetEncapVlan())); err != nil {
		naples.logger.Errorf("Error in Interface attachment %s : %s", in.GetWorkloadName(), err.Error())
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}
		return resp, nil
	}
	naples.logger.Printf("Attachinng interface to workload : %s done", in.GetWorkloadName())
	resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}
	return resp, nil
}

// DeleteWorkload deletes a given workload
func (naples *naplesNode) DeleteWorkload(in *iota.Workload) (*iota.Workload, error) {
	naples.logger.Printf("Deleteing workload : %s", in.GetWorkloadName())
	if _, ok := naples.worloadMap[in.GetWorkloadName()]; !ok {
		naples.log("Trying to delete workload which does not exist")
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}
		return resp, nil
	}

	naples.worloadMap[in.GetWorkloadName()].TearDown()
	delete(naples.worloadMap, in.GetWorkloadName())
	naples.logger.Printf("Deleted workload : %s", in.GetWorkloadName())
	resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}
	return resp, nil
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (naples *naplesNode) Trigger(*iota.TriggerMsg) (*iota.TriggerMsg, error) {
	return nil, nil
}

func (naples *naplesNode) allNaplesProcessRunning() bool {
	for _, process := range naplesProcessess {
		if !naples.container.CheckProcessRunning(process) {
			naples.logger.Printf("Process : %s is not running", process)
			return false
		}
		naples.logger.Printf("Process : %s is running", process)
	}

	return true
}

// CheckHealth returns the node health
func (naples *naplesNode) CheckHealth(in *iota.NodeHealth) (*iota.NodeHealth, error) {
	naples.logger.Println("Checking node health")
	if naples.container == nil || !naples.container.IsHealthy() {
		naples.logger.Printf("Naples  :%s is down or unhealthy", naples.name)
		return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_NAPLES_DOWN}, nil
	}

	for wlName, wl := range naples.worloadMap {
		if !wl.IsWorkloadHealthy() {
			naples.logger.Printf("Workload :%s down", wlName)
			return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_APP_DOWN}, nil
		}
	}

	if !naples.allNaplesProcessRunning() {
		naples.logger.Println("Not all processs running as expected")
		return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_NAPLES_DOWN}, nil
	}
	naples.logger.Println("Node healthy")
	return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_HEALTH_OK}, nil
}

func (naples *naplesNode) Destroy(in *iota.Node) (*iota.Node, error) {
	naples.iotaNode.Destroy(in)

	if naples.container != nil {
		naples.logger.Println("Stopping naples container.")
		naples.container.Stop()
		naples.container = nil
	}

	naples.logger.Println("Destroying naples personality.")
	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}

//NodeType return node type
func (naples *naplesNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NAPLES
}

//Init initalize node type
func (naples *naplesQemuNode) Init(in *iota.Node) (resp *iota.Node, err error) {
	return naples.init(in, true)
}

func (naples *naplesQemuNode) Destroy(in *iota.Node) (*iota.Node, error) {
	naples.naplesNode.Destroy(in)
	killCmd := []string{"pkill", "-9", "-f", "qemu-run"}
	Utils.Run(killCmd, 0, false, false, nil)
	killCmd = []string{"pkill", "-9", "-f", "qemu-system"}
	Utils.Run(killCmd, 0, false, false, nil)
	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}

// AddWorkload brings up a workload type on a given node
func (naples *naplesQemuNode) AddWorkload(*iota.Workload) (*iota.Workload, error) {
	return nil, nil
}

// DeleteWorkload deletes a given workload
func (naples *naplesQemuNode) DeleteWorkload(*iota.Workload) (*iota.Workload, error) {
	return nil, nil
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (naples *naplesQemuNode) Trigger(*iota.TriggerMsg) (*iota.TriggerMsg, error) {
	return nil, nil
}

// CheckHealth returns the node health
func (naples *naplesQemuNode) CheckHealth(*iota.NodeHealth) (*iota.NodeHealth, error) {
	return nil, nil
}

//NodeType return node type
func (naples *naplesQemuNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NAPLES_WITH_QEMU
}
