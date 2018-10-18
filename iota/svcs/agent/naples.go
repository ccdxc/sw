package agent

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strconv"
	"strings"

	"github.com/pkg/errors"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Common "github.com/pensando/sw/iota/svcs/common"
)

const (
	naplesSimName           = "naples-sim"
	naplesVMBringUpScript   = "naples_vm_bringup.py"
	naplesCfgDir            = "/naples/nic/conf"
	hntapCfgFile            = "hntap-cfg.json"
	naplesDataDir           = Common.DstIotaAgentDir + "/naples"
	arpTimeout              = 3000 //3 seconds
	arpAgeTimeout           = 3000 //3000 seconds
	naplesSimHostIntfPrefix = "lif"
	naplesPciDevicePrefix   = "Device"
)

var (
	naplesProcessess = [...]string{"hal", "netagent", "nic_infra_hntap", "cap_model"}
	hntapCfgTempFile = "/tmp/hntap-cfg.json"
	hntapProcessName = "nic_infra_hntap"
)

type naplesNode struct {
	iotaNode
	name string
}

type naplesSimNode struct {
	iotaNode
	simName         string
	container       *Utils.Container
	worloadMap      map[string]workload
	passThroughMode bool
}

type naplesQemuNode struct {
	naplesSimNode
}

func (naples *naplesSimNode) bringUpNaples(name string, image string, ctrlIntf string,
	ctrlIP string, dataIntfs []string, dataIPs []string, naplesIPs []string,
	veniceIPs []string, withQemu bool, passThroughMode bool) error {

	curDir, _ := os.Getwd()
	defer os.Chdir(curDir)
	if err := os.Chdir(Common.DstIotaAgentDir); err != nil {
		return err
	}
	dir, _ := os.Getwd()
	naples.logger.Println("Untar image : " + dir + "/" + image)
	untar := []string{"tar", "-xvzf", image}
	if _, stdout, err := Utils.Run(untar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}
	naples.logger.Println("Untar successfull")
	env := []string{"NAPLES_HOME=" + Common.DstIotaAgentDir, "NAPLES_DATA_DIR=" + naplesDataDir}
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
		naples.logger.Println(stdout)
		naples.logger.Println("Naples bring script up failed.")
		return err
	}

	naples.logger.Println("Naples bring script up succesfull.")

	var err error
	if naples.container, err = Utils.GetContainer(naplesSimName, "", naplesSimName, ""); err != nil {
		return errors.Wrap(err, "Naples sim not running!")
	}

	naples.logger.Println("Naples bring up successfull")

	if err := naples.container.WaitForHealthy(naplesHealthyTimeout); err != nil {
		naples.container = nil
		return errors.Wrap(err, "Naples healthy timeout exceeded")
	}

	naples.simName = name
	naples.passThroughMode = passThroughMode
	return nil
}

func (naples *naplesSimNode) setArpTimeouts() error {
	cmd := []string{"sysctl", "-w", "net.ipv4.neigh.default.retrans_time_ms=" + strconv.Itoa(arpTimeout)}
	if retCode, _, _ := Utils.Run(cmd, 0, false, false, nil); retCode != 0 {
		return errors.New("ARP retrans timeout set failed")
	}

	cmd = []string{"sysctl", "-w", "net.ipv4.neigh.default.gc_stale_time=" + strconv.Itoa(arpAgeTimeout)}
	if retCode, _, _ := Utils.Run(cmd, 0, false, false, nil); retCode != 0 {
		return errors.New("ARP entry age timeout set failed")
	}

	return nil
}

func (naples *naplesSimNode) init(in *iota.Node, withQemu bool) (resp *iota.Node, err error) {

	naples.iotaNode.name = in.GetName()
	naples.worloadMap = make(map[string]workload)
	naples.logger.Println("Bring up request received for : " + in.GetName())

	nodeuuid := ""
	if in.GetNaplesConfig().GetControlIntf() != "" {
		var err error
		nodeuuid, err = Utils.GetIntfMacAddress(in.GetNaplesConfig().ControlIntf)
		if err != nil {
			naples.logger.Errorf("Control interface %s not found",
				in.GetNaplesConfig().ControlIntf)
			return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
		}
	}

	if err := naples.setArpTimeouts(); err != nil {
		naples.logger.Error("Sysctl set failed : " + in.GetName())
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, nil
	}

	if err := naples.bringUpNaples(in.GetName(),
		in.GetImage(), in.GetNaplesConfig().GetControlIntf(),
		in.GetNaplesConfig().GetControlIp(),
		in.GetNaplesConfig().GetDataIntfs(), nil, nil,
		in.GetNaplesConfig().GetVeniceIps(), withQemu, true); err != nil {
		resp := "Naples bring up failed : " + err.Error()
		naples.logger.Println(resp)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, err
	}
	naples.logger.Println("Naples bring up succesfull")

	in.GetNaplesConfig().HostIntfs = Utils.GetIntfsMatchingPrefix(naplesSimHostIntfPrefix)

	naples.logger.Println("Naples sim host interfaces : ", in.GetNaplesConfig().HostIntfs)

	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}, NodeUuid: nodeuuid,
		NodeInfo: &iota.Node_NaplesConfig{NaplesConfig: in.GetNaplesConfig()}}, nil
}

//Init initalize node type
func (naples *naplesSimNode) Init(in *iota.Node) (resp *iota.Node, err error) {
	return naples.init(in, false)
}

func (naples *naplesSimNode) configureWorkloadInHntap(in *iota.Workload) error {

	if !naples.passThroughMode {
		/* no configuraiton if not in pass through mode */
		return nil
	}
	hntapFile := naplesCfgDir + "/" + hntapCfgFile

	cpCmd := []string{"docker", "cp", naplesSimName + ":" + hntapFile, hntapCfgTempFile}

	if retCode, stdout, err := Utils.Run(cpCmd, 0, false, false, nil); err != nil || retCode != 0 {
		naples.logger.Println(stdout)
		naples.logger.Println("Hntap copy failed")
		return err
	}

	fmt.Println(hntapCfgTempFile)

	dir, _ := os.Getwd()
	fmt.Println(dir)
	file, e := ioutil.ReadFile(hntapCfgTempFile)
	if e != nil {
		naples.logger.Error("Error in reading hntap file")
		return errors.Wrap(e, "Error opening hntap cfg file")
	}

	var hntapData map[string]interface{}
	var err error

	err = json.Unmarshal(file, &hntapData)
	if err != nil {
		naples.logger.Errorf("Error in unmarshaling hntap cfg file")
		return errors.Wrap(err, "Error in unmarshalling hntap cfg file")
	}

	if _, ok := hntapData["switch"]; !ok {
		naples.logger.Errorf("No switch section in hntap")
		return errors.New("Switch section not found in hntap cfg file")
	}

	switchData := hntapData["switch"].(map[string]interface{})
	if _, ok := switchData["passthrough-mode"]; !ok {
		naples.logger.Errorf("No passthrough section in hntap")
		return errors.New("Passthrough mode section not found")
	}

	passThroughModeData := switchData["passthrough-mode"].(map[string]interface{})
	if _, ok := passThroughModeData["allowed-macs"]; !ok {
		naples.logger.Errorf("No allowed-macs section in hntap")
		return errors.New("Allowed Macs section not found")
	}

	allowedMacs := passThroughModeData["allowed-macs"].(map[string]interface{})

	macData := make(map[string]uint32)
	macData["port"] = in.GetPinnedPort()
	macData["vlan"] = in.GetUplinkVlan()
	allowedMacs[in.GetMacAddress()] = macData

	hntapJSON, _ := json.MarshalIndent(hntapData, "", "\t")
	if err := ioutil.WriteFile(hntapCfgTempFile, hntapJSON, 0644); err != nil {
		naples.logger.Println("Error in hntap file write")
		return errors.New("Allowed Macs section not found")
	}

	cpCmd = []string{"docker", "cp", hntapCfgTempFile, naplesSimName + ":" + hntapFile}
	if retCode, stdout, err := Utils.Run(cpCmd, 0, false, false, nil); err != nil || retCode != 0 {
		naples.logger.Println(stdout)
		naples.logger.Println("Hntap copy failed")
		return err
	}

	noitfyHntapCmd := []string{"pkill", "-SIGUSR1", hntapProcessName}
	if naples.container != nil {
		if retCode, _, _, _ := naples.container.RunCommand(noitfyHntapCmd, 0, false, false); retCode != 0 {
			naples.logger.Println("Error in sending signal to Hntap")
		}
	}

	naples.logger.Println("Update hntap configuration for the workload")

	return nil
}

// AddWorkload brings up a workload type on a given node
func (naples *naplesSimNode) AddWorkload(in *iota.Workload) (*iota.Workload, error) {

	naples.logger.Printf("Adding workload : %s", in.GetWorkloadName())
	if _, ok := naples.worloadMap[in.GetWorkloadName()]; ok {
		naples.logger.Println("Trying to add workload which already exists")
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

	if in.GetEncapVlan() != 0 {
		if err := wload.AddInterface(in.GetInterface(), in.GetMacAddress(), in.GetIpAddress(), int(in.GetEncapVlan())); err != nil {
			naples.logger.Errorf("Error in Interface attachment %s : %s", in.GetWorkloadName(), err.Error())
			resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}
			wload.TearDown()
			return resp, nil
		}
	}

	/* For SRIOV case, move the parent interface inside the workload so that it is not shared */
	if in.GetInterfaceType() == iota.InterfaceType_INTERFACE_TYPE_SRIOV {
		wload.MoveInterface(in.GetInterface())
	}

	naples.logger.Printf("Attaching interface to workload : %s done", in.GetWorkloadName())

	if err := naples.configureWorkloadInHntap(in); err != nil {
		naples.logger.Errorf("Error in configuring workload to hntap", err.Error())
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}
		wload.TearDown()
		return resp, nil

	}

	if err := wload.SendArpProbe(strings.Split(in.GetIpAddress(), "/")[0], in.GetInterface(),
		int(in.GetEncapVlan())); err != nil {
		naples.logger.Errorf("Error in sending arp probe", err.Error())
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}
		wload.TearDown()
		return resp, nil
	}

	naples.worloadMap[in.GetWorkloadName()] = wload

	/* Notify Hntap of the workload */
	resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}
	return resp, nil
}

// DeleteWorkload deletes a given workload
func (naples *naplesSimNode) DeleteWorkload(in *iota.Workload) (*iota.Workload, error) {
	naples.logger.Printf("Deleteing workload : %s", in.GetWorkloadName())
	if _, ok := naples.worloadMap[in.GetWorkloadName()]; !ok {
		naples.logger.Println("Trying to delete workload which does not exist")
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
func (naples *naplesSimNode) Trigger(in *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	naples.logger.Println("Trigger message received.")

	validate := func() error {
		switch in.TriggerOp {
		case iota.TriggerOp_EXEC_CMDS:
		case iota.TriggerOp_TERMINATE_ALL_CMDS:
			naples.logger.Errorf("Terminate all commands trigger not supported yet")
			return errors.New("Not supported")
		}

		for _, cmd := range in.Commands {
			if _, ok := naples.worloadMap[cmd.GetWorkloadName()]; !ok {
				naples.logger.Errorf("Workload %s does not exist on node %s", cmd.GetWorkloadName(), naples.NodeName())
				return errors.New("Invalid request")
			}
		}
		return nil
	}

	if err := validate(); err != nil {
		return &iota.TriggerMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
	}

	for _, cmd := range in.Commands {
		var err error
		cmd.ExitCode, cmd.Stdout, cmd.Stderr, err = naples.worloadMap[cmd.GetWorkloadName()].RunCommand(strings.Split(cmd.GetCommand(), " "), 0, false, true)
		naples.logger.Println("Command error :", err)
		naples.logger.Println("Command exit code :", cmd.ExitCode)
		naples.logger.Println("Command stdout :", cmd.Stdout)
		naples.logger.Println("Command stderr:", cmd.Stderr)
	}

	naples.logger.Println("Completed running trigger.")
	in.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	return in, nil
}

func (naples *naplesSimNode) allNaplesProcessRunning() bool {
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
func (naples *naplesSimNode) CheckHealth(in *iota.NodeHealth) (*iota.NodeHealth, error) {
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

func (naples *naplesSimNode) Destroy(in *iota.Node) (*iota.Node, error) {
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
func (naples *naplesSimNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NAPLES
}

//Init initalize node type
func (naples *naplesQemuNode) Init(in *iota.Node) (resp *iota.Node, err error) {
	return naples.init(in, true)
}

func (naples *naplesQemuNode) Destroy(in *iota.Node) (*iota.Node, error) {
	naples.naplesSimNode.Destroy(in)
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
func (naples *naplesQemuNode) CheckHealth(in *iota.NodeHealth) (*iota.NodeHealth, error) {
	return naples.naplesSimNode.CheckHealth(in)
}

func (naples *naplesNode) getHostIntfs() ([]string, error) {
	hostIntfs := []string{}

	pciIntfMap := make(map[string]string)
	cmd := []string{"systool", "-c", "net"}

	_, stdout, err := Utils.Run(cmd, 0, false, false, nil)
	if err != nil {
		return nil, errors.Wrap(err, stdout)
	}

	lines := strings.Split(stdout, "\n")
	for idx, line := range lines {
		if strings.Contains(line, "Class Device") {
			if (idx+1) < len(lines) && strings.Contains(lines[idx+1], "Device") {
				pci := strings.Replace(lines[idx+1], " ", "", -1)
				pci = strings.Split(pci, "=")[1]
				intfName := strings.Replace(line, " ", "", -1)
				intfName = strings.Split(intfName, "=")[1]
				pciIntfMap[pci] = intfName
			}
		}
	}

	cmd = []string{"lspci", "|", "grep", "Ethernet"}
	_, stdout, err = Utils.Run(cmd, 0, false, true, nil)
	if err != nil {
		return nil, errors.Wrap(err, stdout)
	}

	for _, line := range strings.Split(stdout, "\n") {
		if strings.Contains(line, naplesPciDevicePrefix) {
			pci := strings.Split(line, " ")[0]
			for pciAddr, intf := range pciIntfMap {
				if strings.Contains(pciAddr, pci) {
					hostIntfs = append(hostIntfs, intf)
				}
			}

		}
	}

	return hostIntfs, nil
}

//Init initalize node type
func (naples *naplesNode) Init(in *iota.Node) (resp *iota.Node, err error) {

	in.GetNaplesConfig().HostIntfs, _ = naples.getHostIntfs()

	naples.logger.Printf("Naples host interfaces : ", in.GetNaplesConfig().HostIntfs)
	for _, intf := range in.GetNaplesConfig().HostIntfs {
		cmd := []string{"ifconfig", intf, "up"}
		_, stdout, err := Utils.Run(cmd, 0, false, true, nil)
		if err != nil {
			naples.logger.Errorf("Failed to bring interface %s up err : %s", intf, stdout)
		}
	}

	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}, NodeUuid: "",
		NodeInfo: &iota.Node_NaplesConfig{NaplesConfig: in.GetNaplesConfig()}}, nil
}

//NodeType return node type
func (naples *naplesQemuNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NAPLES_SIM_WITH_QEMU
}

// AddWorkload brings up a workload type on a given node
func (naples *naplesNode) AddWorkload(*iota.Workload) (*iota.Workload, error) {
	return nil, nil
}

// DeleteWorkload deletes a given workload
func (naples *naplesNode) DeleteWorkload(*iota.Workload) (*iota.Workload, error) {
	return nil, nil
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (naples *naplesNode) Trigger(*iota.TriggerMsg) (*iota.TriggerMsg, error) {
	return nil, nil
}

// CheckHealth returns the node health
func (naples *naplesNode) CheckHealth(in *iota.NodeHealth) (*iota.NodeHealth, error) {
	return nil, nil
}

//NodeType return node type
func (naples *naplesNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NAPLES
}
