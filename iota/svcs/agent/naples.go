package agent

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"runtime"
	"strconv"
	"strings"
	"time"

	"github.com/pkg/errors"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Cmd "github.com/pensando/sw/iota/svcs/agent/command"
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Workload "github.com/pensando/sw/iota/svcs/agent/workload"
	Common "github.com/pensando/sw/iota/svcs/common"
)

const (
	naplesVMBringUpScript   = "naples_vm_bringup.py"
	naplesCfgDir            = "/naples/nic/conf"
	hntapCfgFile            = "hntap-cfg.json"
	naplesDataDir           = Common.DstIotaAgentDir + "/naples"
	arpTimeout              = 3000 //3 seconds
	arpAgeTimeout           = 3000 //3000 seconds
	sshPort                 = 22
	naplesSimHostIntfPrefix = "lif"
	naplesBsdHosIntfPrefix  = "ionic"
	naplesPciDevicePrefix   = "Device"
	mellanoxPciDevicePrefix = "Ethernet"
	broadcomPciDevicePrefix = "Ethernet"
	bareMetalWorkloadName   = "bareMetalWorkload"
	intelPciDevicePrefix    = "Ethernet"
)

var (
	naplesProcessess = [...]string{"hal", "netagent", "nic_infra_hntap", "cap_model"}
	hntapCfgTempFile = "/tmp/hntap-cfg.json"
	hntapProcessName = "nic_infra_hntap"
	apiSuccess       = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK, ErrorMsg: "Api success"}
	naplesHwUUIDFile = "/sysconfig/config0/sysuuid"
)

var workloadTypeMap = map[iota.WorkloadType]string{
	iota.WorkloadType_WORKLOAD_TYPE_CONTAINER:  Workload.WorkloadTypeContainer,
	iota.WorkloadType_WORKLOAD_TYPE_VM:         Workload.WorkloadTypeVM,
	iota.WorkloadType_WORKLOAD_TYPE_BARE_METAL: Workload.WorkloadTypeBareMetal,
}

type dataNode struct {
	iotaNode
	entityMap     map[string]iotaWorkload
	hostEntityKey string
}

type naplesHwNode struct {
	dataNode
	naplesEntityKey string
}

type naplesSimNode struct {
	dataNode
	container       *Utils.Container
	passThroughMode bool
}

type naplesQemuNode struct {
	naplesSimNode
}

type mellanoxNode struct {
	dataNode
}

type broadcomNode struct {
	dataNode
}

type intelNode struct {
	dataNode
}

func (dnode *dataNode) GetMsg() *iota.Node {
	return dnode.nodeMsg
}

func (dnode *dataNode) configureWorkloadInHntap(in *iota.Workload) error {
	return nil
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
		"--data-intfs", strings.Join(dataIntfs, ","), "--name", name}

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

	cmd = append(cmd, "--nmd-hostname")
	cmd = append(cmd, name)

	cmd = append(cmd, "--hntap-mode")
	if passThroughMode {
		cmd = append(cmd, "passthrough")
	} else {
		cmd = append(cmd, "tunnel")
	}

	if retCode, stdout, err := Utils.Run(cmd, nodeAddTimeout, false, false, env); err != nil || retCode != 0 {
		naples.logger.Println(stdout)
		msg := "Naples bring script up failed."
		naples.logger.Error(msg)
		return errors.Wrap(err, msg)
	}

	naples.logger.Println("Naples bring script up succesfull.")

	naples.name = name

	var err error
	if naples.container, err = Utils.GetContainer(name, "", name, ""); err != nil {
		return errors.Wrap(err, "Naples sim not running!")
	}

	naples.logger.Println("Naples bring up successfull")

	if err := naples.container.WaitForHealthy(naplesHealthyTimeout); err != nil {
		naples.container = nil
		return errors.Wrap(err, "Naples healthy timeout exceeded")
	}

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

func (dnode *dataNode) init(in *iota.Node) {
	dnode.entityMap = make(map[string]iotaWorkload)
	dnode.nodeMsg = in
}

func (naples *naplesSimNode) addNodeEntities(in *iota.Node) error {
	for _, entityEntry := range in.GetEntities() {
		var wload Workload.Workload
		if entityEntry.GetType() == iota.EntityType_ENTITY_TYPE_NAPLES {
			wload = Workload.NewWorkload(Workload.WorkloadTypeContainer, entityEntry.GetName(), naples.name, naples.logger)
		} else if entityEntry.GetType() == iota.EntityType_ENTITY_TYPE_HOST {
			wload = Workload.NewWorkload(Workload.WorkloadTypeBareMetal, entityEntry.GetName(), naples.name, naples.logger)
		}

		wDir := Common.DstIotaEntitiesDir + "/" + entityEntry.GetName()
		wload.SetBaseDir(wDir)
		if err := wload.BringUp(in.GetName(), ""); err != nil {
			naples.logger.Errorf("Naples sim entity type add failed")
			return err
		}
		if wload != nil {
			naples.entityMap[entityEntry.GetName()] = iotaWorkload{workload: wload, name: entityEntry.GetName()}
		}
	}
	return nil
}

func (naples *naplesSimNode) init(in *iota.Node, withQemu bool) (resp *iota.Node, err error) {

	naples.iotaNode.name = in.GetName()
	naples.dataNode.init(in)
	naples.logger.Println("Bring up request received for : " + in.GetName())

	nodeuuid := ""
	if in.GetNaplesConfig().GetControlIntf() != "" {
		var err error
		nodeuuid, err = Utils.GetIntfMacAddress(in.GetNaplesConfig().ControlIntf)
		if err != nil {
			msg := fmt.Sprintf("Control interface %s not found", in.GetNaplesConfig().ControlIntf)
			naples.logger.Errorf(msg)
			return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST,
				ErrorMsg: msg}}, err
		}
	}

	if err := naples.setArpTimeouts(); err != nil {
		msg := fmt.Sprintf("Sysctl set failed : %s : %s", in.GetName(), err.Error())
		naples.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
	}

	if err := naples.bringUpNaples(in.GetName(),
		in.GetImage(), in.GetNaplesConfig().GetControlIntf(),
		in.GetNaplesConfig().GetControlIp(),
		in.GetNaplesConfig().GetDataIntfs(), nil, nil,
		in.GetNaplesConfig().GetVeniceIps(), withQemu, true); err != nil {
		resp := "Naples bring up failed : " + err.Error()
		naples.logger.Error(resp)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: resp}}, err
	}
	naples.logger.Println("Naples bring up succesfull")

	if in.GetNaplesConfig() == nil {
		in.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{}}
	}
	in.GetNaplesConfig().HostIntfs = Utils.GetIntfsMatchingPrefix(naplesSimHostIntfPrefix)

	naples.logger.Println("Naples sim host interfaces : ", in.GetNaplesConfig().HostIntfs)

	/* Finally add entity type */
	if err := naples.addNodeEntities(in); err != nil {
		msg := fmt.Sprintf("Adding node entities failed : %s", err.Error())
		naples.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
	}

	return &iota.Node{NodeStatus: apiSuccess, NodeUuid: nodeuuid,
		Name: in.GetName(), IpAddress: in.GetIpAddress(), Type: in.GetType(),
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

	cpCmd := []string{"docker", "cp", naples.name + ":" + hntapFile, hntapCfgTempFile}

	if retCode, stdout, err := Utils.Run(cpCmd, 0, false, false, nil); err != nil || retCode != 0 {
		naples.logger.Error(stdout)
		return errors.Wrapf(err, "Hntap copy failed")
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

	cpCmd = []string{"docker", "cp", hntapCfgTempFile, naples.name + ":" + hntapFile}
	if retCode, stdout, err := Utils.Run(cpCmd, 0, false, false, nil); err != nil || retCode != 0 {
		naples.logger.Println(stdout)
		naples.logger.Println("Hntap copy failed")
		return err
	}

	noitfyHntapCmd := []string{"pkill", "-SIGUSR1", hntapProcessName}
	if naples.container != nil {

		cmdHandle, _ := naples.container.SetUpCommand(noitfyHntapCmd, "", false, false)
		cmdResp, err := naples.container.RunCommand(cmdHandle, 0)

		if err != nil || cmdResp.RetCode != 0 {
			naples.logger.Println("Error in sending signal to Hntap")
		}
	}

	naples.logger.Println("Update hntap configuration for the workload")

	return nil
}

func (dnode *dataNode) configureWorkload(wload Workload.Workload, in *iota.Workload) (*iota.Workload, error) {

	var intf string

	if attachedIntf, err := wload.AddInterface(in.GetInterface(), in.GetMacAddress(), in.GetIpPrefix(), int(in.GetEncapVlan())); err != nil {
		msg := fmt.Sprintf("Error in Interface attachment %s : %s", in.GetWorkloadName(), err.Error())
		dnode.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return resp, err
	} else {
		intf = attachedIntf
	}

	/* For SRIOV case, move the parent interface inside the workload so that it is not shared */
	if in.GetInterfaceType() == iota.InterfaceType_INTERFACE_TYPE_SRIOV {
		wload.MoveInterface(in.GetInterface())
	}

	resp := in
	resp.WorkloadStatus = apiSuccess
	resp.Interface = intf
	return resp, nil
}

func (dnode *dataNode) setupWorkload(wload Workload.Workload, in *iota.Workload) (*iota.Workload, error) {
	/* Create working directory and set that as base dir */
	wDir := Common.DstIotaEntitiesDir + "/" + in.GetWorkloadName()
	wload.SetBaseDir(wDir)
	if err := wload.BringUp(in.GetWorkloadName(), in.GetWorkloadImage()); err != nil {
		msg := fmt.Sprintf("Error in workload image bring up : %s : %s", in.GetWorkloadName(), err.Error())
		dnode.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return resp, err
	}
	dnode.logger.Printf("Bring up workload : %s done", in.GetWorkloadName())

	return dnode.configureWorkload(wload, in)
}

// AddWorkload brings up a workload type on a given node
func (dnode *dataNode) AddWorkload(in *iota.Workload) (*iota.Workload, error) {

	if _, ok := iota.WorkloadType_name[(int32)(in.GetWorkloadType())]; !ok {
		msg := fmt.Sprintf("Workload Type %d not supported", in.GetWorkloadType())
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
		return resp, nil
	}

	wloadKey := in.GetWorkloadName()
	var iotaWload iotaWorkload
	dnode.logger.Printf("Adding workload : %s", in.GetWorkloadName())
	if iotaWload, ok := dnode.entityMap[wloadKey]; ok {
		msg := fmt.Sprintf("Trying to add workload %s, which already exists ", wloadKey)
		dnode.logger.Error(msg)
		return dnode.configureWorkload(iotaWload.workload, in)
	}

	wlType, ok := workloadTypeMap[in.GetWorkloadType()]
	if !ok {
		msg := fmt.Sprintf("Workload type %v not found", in.GetWorkloadType())
		dnode.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
		return resp, nil
	}

	iotaWload.workload = Workload.NewWorkload(wlType, in.GetWorkloadName(), dnode.name, dnode.logger)

	if iotaWload.workload == nil {
		msg := fmt.Sprintf("Trying to add workload of invalid type : %v", wlType)
		dnode.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
		return resp, nil
	}

	dnode.logger.Printf("Setting up workload : %s", in.GetWorkloadName())
	resp, err := dnode.setupWorkload(iotaWload.workload, in)

	if err != nil || resp.GetWorkloadStatus().GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
		iotaWload.workload.TearDown()
		return resp, nil
	}

	iotaWload.workloadMsg = in
	dnode.entityMap[wloadKey] = iotaWload
	dnode.logger.Printf("Added workload : %s (%s)", in.GetWorkloadName(), in.GetWorkloadType())
	return resp, nil
}

// AddWorkload brings up a workload type on a given node
func (naples *naplesSimNode) AddWorkload(in *iota.Workload) (*iota.Workload, error) {

	resp, err := naples.dataNode.AddWorkload(in)
	if err != nil || resp.GetWorkloadStatus().GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
		return resp, nil
	}

	wloadKey := in.GetWorkloadName()
	iotaWload, _ := naples.dataNode.entityMap[wloadKey]
	/* Notify Hntap of the workload */
	if err := naples.configureWorkloadInHntap(in); err != nil {
		msg := fmt.Sprintf("Error in configuring workload to hntap %s", err.Error())
		naples.logger.Error(msg)
		resp = &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		delete(naples.entityMap, in.GetWorkloadName())
		iotaWload.workload.TearDown()
		return resp, err

	}

	//Set vlan 0 as interface is already set before
	if err := iotaWload.workload.SendArpProbe(strings.Split(in.GetIpPrefix(), "/")[0], in.GetInterface(), 0); err != nil {
		msg := fmt.Sprintf("Error in sending arp probe %s", err.Error())
		naples.logger.Errorf(msg)
		resp = &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		delete(naples.entityMap, in.GetWorkloadName())
		iotaWload.workload.TearDown()
		return resp, err
	}

	return resp, nil
}

// DeleteWorkload deletes a given workload
func (dnode *dataNode) DeleteWorkload(in *iota.Workload) (*iota.Workload, error) {
	dnode.logger.Printf("Deleting workload : %s", in.GetWorkloadName())

	wloadKey := in.GetWorkloadName()

	if _, ok := dnode.entityMap[wloadKey]; !ok {
		msg := fmt.Sprintf("Trying to delete workload %s which does not exist", wloadKey)
		dnode.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
		return resp, nil
	}

	dnode.entityMap[wloadKey].workload.TearDown()
	delete(dnode.entityMap, wloadKey)
	dnode.logger.Printf("Deleted workload : %s", in.GetWorkloadName())
	resp := &iota.Workload{WorkloadStatus: apiSuccess}
	return resp, nil
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (dnode *dataNode) Trigger(in *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	dnode.logger.Println("Trigger message received.")

	validate := func() error {
		for _, cmd := range in.Commands {
			wloadKey := cmd.GetEntityName()
			if _, ok := dnode.entityMap[wloadKey]; !ok {
				msg := fmt.Sprintf("Workload %s does not exist on node %s", cmd.GetEntityName(), dnode.NodeName())
				return errors.New(msg)
			}
		}
		return nil
	}

	if err := validate(); err != nil {
		return &iota.TriggerMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: err.Error()}}, nil
	}

	for _, cmd := range in.Commands {
		var err error
		var cmdKey string
		var cmdResp *Cmd.CommandCtx
		wloadKey := cmd.GetEntityName()

		if in.TriggerOp == iota.TriggerOp_EXEC_CMDS {
			cmdResp, cmdKey, err = dnode.entityMap[wloadKey].workload.RunCommand(strings.Split(cmd.GetCommand(), " "),
				cmd.GetRunningDir(), cmd.GetForegroundTimeout(),
				cmd.GetMode() == iota.CommandMode_COMMAND_BACKGROUND, true)

		} else {
			cmdResp, err = dnode.entityMap[wloadKey].workload.StopCommand(cmd.Handle)
			cmdKey = cmd.Handle
		}

		cmd.ExitCode, cmd.Stdout, cmd.Stderr, cmd.Handle, cmd.TimedOut = cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr, cmdKey, cmdResp.TimedOut
		dnode.logger.Println("Command error :", err)
		dnode.logger.Println("Command exit code :", cmd.ExitCode)
		dnode.logger.Println("Command timed out :", cmd.TimedOut)
		dnode.logger.Println("Command handle  :", cmd.Handle)
		dnode.logger.Println("Command stdout :", cmd.Stdout)
		dnode.logger.Println("Command stderr:", cmd.Stderr)
	}

	dnode.logger.Println("Completed running trigger.")
	in.ApiResponse = apiSuccess
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
		msg := fmt.Sprintf("Naples  :%s is down or unhealthy", naples.name)
		naples.logger.Printf(msg)
		return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_NAPLES_DOWN}, nil
	}

	if err := naples.EntitiesHealthy(); err != nil {
		return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_APP_DOWN}, nil
	}

	if !naples.allNaplesProcessRunning() {
		naples.logger.Println("Not all processs running as expected")
		return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_NAPLES_DOWN}, nil
	}
	naples.logger.Println("Node healthy")
	return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_HEALTH_OK}, nil
}

func (naples *naplesSimNode) Destroy(in *iota.Node) (*iota.Node, error) {

	for _, wloadMsg := range naples.GetWorkloadMsgs() {
		naples.DeleteWorkload(wloadMsg)
	}

	naples.iotaNode.Destroy(in)

	if naples.container != nil {
		naples.logger.Println("Stopping naples container.")
		naples.container.Stop()
		naples.container = nil
	}

	naples.logger.Println("Destroying naples personality.")
	return &iota.Node{NodeStatus: apiSuccess}, nil
}

//NodeType return node type
func (naples *naplesSimNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NAPLES_SIM
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
	return &iota.Node{NodeStatus: apiSuccess}, nil
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

func (naples *naplesHwNode) addNodeEntities(in *iota.Node) error {
	for _, entityEntry := range in.GetEntities() {
		var wload Workload.Workload
		if entityEntry.GetType() == iota.EntityType_ENTITY_TYPE_NAPLES {
			/*It is like running in a vm as its accesible only by ssh */
			wload = Workload.NewWorkload(Workload.WorkloadTypeRemote, entityEntry.GetName(), naples.name, naples.logger)
			naples.naplesEntityKey = entityEntry.GetName()
		} else if entityEntry.GetType() == iota.EntityType_ENTITY_TYPE_HOST {
			wload = Workload.NewWorkload(Workload.WorkloadTypeBareMetal, entityEntry.GetName(), naples.name, naples.logger)
			naples.hostEntityKey = entityEntry.GetName()
		}

		wDir := Common.DstIotaEntitiesDir + "/" + entityEntry.GetName()
		wload.SetBaseDir(wDir)

		//in.GetNaplesConfig().
		naplesCfg := in.GetNaplesConfig()
		if err := wload.BringUp(naplesCfg.GetNaplesIpAddress(),
			strconv.Itoa(sshPort), naplesCfg.GetNaplesUsername(), naplesCfg.GetNaplesPassword()); err != nil {
			naples.logger.Errorf("Naples Hw entity type add failed %v", err.Error())
			return err
		}
		if wload != nil {
			naples.entityMap[entityEntry.GetName()] = iotaWorkload{workload: wload}
		}
	}
	return nil
}

func (naples *naplesHwNode) getHwUUID(in *iota.Node) (uuid string, err error) {

	naplesEntity, ok := naples.entityMap[naples.naplesEntityKey]
	if !ok {
		return "", errors.Errorf("Naples entity not added : %s", naples.naplesEntityKey)
	}

	cmd := []string{"cat", naplesHwUUIDFile}
	cmdResp, _, _ := naplesEntity.workload.RunCommand(cmd, "", 0, false, true)
	naples.logger.Printf("naples hw uuid out %s", cmdResp.Stdout)
	naples.logger.Printf("naples hw uuid err %s", cmdResp.Stderr)
	naples.logger.Printf("naples hw uuid exit code %d", cmdResp.ExitCode)

	if cmdResp.ExitCode != 0 {
		return "", errors.Errorf("Running cat of %s failed : %s", naplesHwUUIDFile, cmdResp.Stdout)
	}

	return strings.Trim(cmdResp.Stdout, "\r\n"), nil
}

//Init initalize node type
func (naples *naplesHwNode) Init(in *iota.Node) (*iota.Node, error) {

	if in.StartupScript != "" {
		_, stdout, err := Utils.Run(strings.Split(in.StartupScript, " "), 0, false, true, nil)
		if err != nil {
			msg := fmt.Sprintf("Start up script failed %v up err : %v", err, stdout)
			naples.logger.Error(msg)
			// Don't return error as start up would have been completed before.
			//return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
		}
	}

	naples.init(in)
	naples.iotaNode.name = in.GetName()
	if in.GetNaplesConfig() == nil {
		in.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{}}
	}

	in.GetNaplesConfig().HostIntfs, _ = Utils.GetIntfsMatchingDevicePrefix(naplesPciDevicePrefix)

	if len(in.GetNaplesConfig().HostIntfs) == 0 && runtime.GOOS == "freebsd" {
		in.GetNaplesConfig().HostIntfs = Utils.GetIntfsMatchingPrefix(naplesBsdHosIntfPrefix)
	}

	naples.logger.Printf("Naples host interfaces : %v", in.GetNaplesConfig().HostIntfs)
	for _, intf := range in.GetNaplesConfig().HostIntfs {
		cmd := []string{"ifconfig", intf, "up"}
		_, stdout, err := Utils.Run(cmd, 0, false, true, nil)
		if err != nil {
			msg := fmt.Sprintf("Failed to bring interface %s up err : %s", intf, stdout)
			naples.logger.Error(msg)
			return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
		}
	}

	/* Finally add entity type */
	if err := naples.addNodeEntities(in); err != nil {
		msg := fmt.Sprintf("Adding node entities failed : %s", err.Error())
		naples.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
	}

	nodeUUID, err := naples.getHwUUID(in)
	if err != nil {
		msg := fmt.Sprintf("Error in reading naples hw uuid : %s", err.Error())
		naples.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
	}

	return &iota.Node{NodeStatus: apiSuccess, NodeUuid: nodeUUID,
		Name: in.GetName(), IpAddress: in.GetIpAddress(), Type: in.GetType(),
		NodeInfo: &iota.Node_NaplesConfig{NaplesConfig: in.GetNaplesConfig()}}, nil
}

// AddWorkload brings up a workload type on a given node
func (naples *naplesHwNode) AddWorkload(in *iota.Workload) (*iota.Workload, error) {

	resp, err := naples.dataNode.AddWorkload(in)
	if err != nil || resp.GetWorkloadStatus().GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
		return resp, nil
	}

	wloadKey := in.GetWorkloadName()
	iotaWload, _ := naples.entityMap[wloadKey]
	//Set vlan 0 as interface is already set before
	if err := iotaWload.workload.SendArpProbe(strings.Split(in.GetIpPrefix(), "/")[0], in.GetInterface(),
		0); err != nil {
		msg := fmt.Sprintf("Error in sending arp probe : %s", err.Error())
		naples.logger.Error(msg)
		resp = &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		delete(naples.entityMap, in.GetWorkloadName())
		iotaWload.workload.TearDown()
		return resp, err
	}

	return resp, nil
}

//NodeType return node type
func (naplesHwNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NAPLES
}

//NodeType return node type
func (naples *naplesQemuNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NAPLES_SIM_WITH_QEMU
}

func (dnode *dataNode) addNodeEntities(in *iota.Node) error {
	for _, entityEntry := range in.GetEntities() {
		var wload Workload.Workload
		if entityEntry.GetType() == iota.EntityType_ENTITY_TYPE_HOST {
			wload = Workload.NewWorkload(Workload.WorkloadTypeBareMetal, entityEntry.GetName(), dnode.name, dnode.logger)
			dnode.hostEntityKey = entityEntry.GetName()
		}

		wDir := Common.DstIotaEntitiesDir + "/" + entityEntry.GetName()
		wload.SetBaseDir(wDir)
		//in.GetNaplesConfig().
		naplesCfg := in.GetNaplesConfig()
		if err := wload.BringUp(naplesCfg.GetNaplesIpAddress(),
			strconv.Itoa(sshPort), naplesCfg.GetNaplesUsername(), naplesCfg.GetNaplesPassword()); err != nil {
			dnode.logger.Errorf("DataNode Hw entity type add failed %v", err.Error())
			return err
		}
		if wload != nil {
			dnode.entityMap[entityEntry.GetName()] = iotaWorkload{workload: wload}
		}
	}
	time.Sleep(1 * time.Second)
	return nil
}

//Init initalize node type
func (mlx *mellanoxNode) Init(in *iota.Node) (resp *iota.Node, err error) {

	mlx.init(in)
	mlx.iotaNode.name = in.GetName()

	if in.GetMellanoxConfig() == nil {
		in.NodeInfo = &iota.Node_MellanoxConfig{MellanoxConfig: &iota.MellanoxConfig{}}
	}

	in.GetMellanoxConfig().HostIntfs, _ = Utils.GetIntfsMatchingDevicePrefix(mellanoxPciDevicePrefix)

	mlx.logger.Printf("Mellanox host interfaces : %v", in.GetMellanoxConfig().HostIntfs)
	for _, intf := range in.GetMellanoxConfig().HostIntfs {
		cmd := []string{"ifconfig", intf, "up"}
		_, stdout, err := Utils.Run(cmd, 0, false, true, nil)
		if err != nil {
			msg := fmt.Sprintf("Failed to bring interface %s up err : %s", intf, stdout)
			mlx.logger.Errorf(msg)
			return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
		}
	}

	/* Finally add entity type */
	if err := mlx.addNodeEntities(in); err != nil {
		mlx.logger.Error("Adding node entities failed")
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, err
	}

	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}, NodeUuid: "",
		Name: in.GetName(), IpAddress: in.GetIpAddress(), Type: in.GetType(),
		NodeInfo: &iota.Node_MellanoxConfig{MellanoxConfig: in.GetMellanoxConfig()}}, nil
}

//NodeType return node type
func (mellanoxNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_MELLANOX
}

//Init initalize node type
func (dnode *dataNode) Init(in *iota.Node) (resp *iota.Node, err error) {
	return nil, nil
}

// EntitiesHealthy checks workloads healthy
func (dnode *dataNode) EntitiesHealthy() error {
	for _, wl := range dnode.entityMap {
		if !wl.workload.IsHealthy() {
			dnode.logger.Printf("Workload :%s down", wl.workload.Name())
			return errors.Errorf("Workload %s down", wl.workload.Name())
		}
	}
	return nil
}

// CheckHealth returns the node health
func (dnode *dataNode) CheckHealth(in *iota.NodeHealth) (*iota.NodeHealth, error) {
	if err := dnode.EntitiesHealthy(); err != nil {
		return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_APP_DOWN}, nil
	}

	dnode.logger.Println("Node healthy")
	return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_HEALTH_OK}, nil
}

//NodeType return node type
func (dnode *dataNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_NONE
}

//Init initalize node type
func (brcm *broadcomNode) Init(in *iota.Node) (resp *iota.Node, err error) {

	brcm.init(in)
	brcm.iotaNode.name = in.GetName()

	if in.GetBroadcomConfig() == nil {
		in.NodeInfo = &iota.Node_BroadcomConfig{BroadcomConfig: &iota.BroadcomConfig{}}
	}

	in.GetBroadcomConfig().HostIntfs, _ = Utils.GetIntfsMatchingDevicePrefix(broadcomPciDevicePrefix)

	brcm.logger.Printf("Broadcom host interfaces : %v", in.GetBroadcomConfig().HostIntfs)
	for _, intf := range in.GetBroadcomConfig().HostIntfs {
		cmd := []string{"ifconfig", intf, "up"}
		_, stdout, err := Utils.Run(cmd, 0, false, true, nil)
		if err != nil {
			msg := fmt.Sprintf("Failed to bring interface %s up err : %s", intf, stdout)
			brcm.logger.Errorf(msg)
			return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
		}
	}

	/* Finally add entity type */
	if err := brcm.addNodeEntities(in); err != nil {
		brcm.logger.Error("Adding node entities failed")
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, err
	}

	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}, NodeUuid: "",
		Name: in.GetName(), IpAddress: in.GetIpAddress(), Type: in.GetType(),
		NodeInfo: &iota.Node_BroadcomConfig{BroadcomConfig: in.GetBroadcomConfig()}}, nil
}

//Init initalize node type
func (intel *intelNode) Init(in *iota.Node) (resp *iota.Node, err error) {

	intel.init(in)
	intel.iotaNode.name = in.GetName()

	if in.GetIntelConfig() == nil {
		in.NodeInfo = &iota.Node_IntelConfig{IntelConfig: &iota.IntelConfig{}}
	}

	in.GetIntelConfig().HostIntfs, _ = Utils.GetIntfsMatchingDevicePrefix(intelPciDevicePrefix)

	intel.logger.Printf("Intel host interfaces : %v", in.GetIntelConfig().HostIntfs)
	for _, intf := range in.GetIntelConfig().HostIntfs {
		cmd := []string{"ifconfig", intf, "up"}
		_, stdout, err := Utils.Run(cmd, 0, false, true, nil)
		if err != nil {
			msg := fmt.Sprintf("Failed to bring interface %s up err : %s", intf, stdout)
			intel.logger.Errorf(msg)
			return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
		}
	}

	/* Finally add entity type */
	if err := intel.addNodeEntities(in); err != nil {
		intel.logger.Error("Adding node entities failed")
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, err
	}

	return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}, NodeUuid: "",
		Name: in.GetName(), IpAddress: in.GetIpAddress(), Type: in.GetType(),
		NodeInfo: &iota.Node_IntelConfig{IntelConfig: in.GetIntelConfig()}}, nil
}

func (dnode *dataNode) GetWorkloadMsgs() []*iota.Workload {
	wloadMsgs := []*iota.Workload{}
	for _, wl := range dnode.entityMap {
		if wl.workloadMsg != nil {
			wloadMsgs = append(wloadMsgs, wl.workloadMsg)
		}
	}

	return wloadMsgs
}
