package agent

import (
	"context"
	"fmt"
	"os/exec"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"
	"golang.org/x/sync/errgroup"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Cmd "github.com/pensando/sw/iota/svcs/agent/command"
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Workload "github.com/pensando/sw/iota/svcs/agent/workload"
	Common "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/vmware"
)

type esxHwNode struct {
	naplesHwNode
	hostIP           string
	hostUsername     string
	hostPassword     string
	esxHostEntityKey string
	nicType          string
	host             *vmware.Host
	imagesMap        map[string]string
}

type esxNaplesHwNode struct {
	esxHwNode
}

type esxThirdPartyHwNode struct {
	esxHwNode
}

func (naples *esxHwNode) setupWorkload(wload Workload.Workload, in *iota.Workload) (*iota.Workload, error) {
	/* Create working directory and set that as base dir */
	naples.logger.Println("Doing setup of esx workload")
	wDir := Common.DstIotaEntitiesDir + "/" + in.GetWorkloadName()
	wload.SetBaseDir(wDir)

	naples.logger.Println("Doing bring up of esx workload")
	imageDir, _ := naples.imagesMap[in.GetWorkloadImage()]
	if err := wload.BringUp(in.GetWorkloadName(), imageDir,
		naples.hostIP, naples.hostUsername, naples.hostPassword,
		strconv.Itoa(int(in.GetCpus())), strconv.Itoa(int(in.GetMemory()))); err != nil {
		msg := fmt.Sprintf("Error in workload image bring up : %s : %s", in.GetWorkloadName(), err.Error())
		naples.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return resp, err
	}
	naples.logger.Printf("Bring up workload : %s done", in.GetWorkloadName())

	return naples.configureWorkload(wload, in)
}

func (naples *esxHwNode) downloadDataVMImage(image string) (string, error) {

	dataVMDir := Common.DataVMImageDirectory + "/" + image
	dstImage := dataVMDir + "/" + image + ".ova"
	mkdir := []string{"mkdir", "-p", dataVMDir}
	if stdout, err := exec.Command(mkdir[0], mkdir[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	buildIt := []string{"/usr/local/bin/buildit", "-t", Common.BuildItURL, "image", "pull", "-o", dstImage, image}
	if stdout, err := exec.Command(buildIt[0], buildIt[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	naples.logger.Info("Download complete for VM image")
	tarCmd := []string{"tar", "-xvf", dstImage, "-C", dataVMDir}
	if stdout, err := exec.Command(tarCmd[0], tarCmd[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	return dataVMDir, nil
}

// AddWorkload brings up a workload type on a given node
func (naples *esxHwNode) AddWorkloads(in *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {

	addWorkload := func(in *iota.Workload) *iota.Workload {

		if _, ok := iota.WorkloadType_name[(int32)(in.GetWorkloadType())]; !ok {
			msg := fmt.Sprintf("Workload Type %d not supported", in.GetWorkloadType())
			resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
			return resp
		}

		wloadKey := in.GetWorkloadName()
		var iotaWload iotaWorkload
		naples.logger.Printf("Adding workload : %s", in.GetWorkloadName())
		if item, ok := naples.entityMap.Load(wloadKey); ok {
			msg := fmt.Sprintf("Trying to add workload %s, which already exists ", wloadKey)
			naples.logger.Error(msg)
			resp, _ := naples.configureWorkload(item.(iotaWorkload).workload, in)
			return resp
		}

		wlType, ok := workloadTypeMap[in.GetWorkloadType()]
		if !ok {
			msg := fmt.Sprintf("Workload type %v not found", in.GetWorkloadType())
			naples.logger.Error(msg)
			resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
			return resp
		}

		iotaWload.workload = Workload.NewWorkload(wlType, in.GetWorkloadName(), naples.name, naples.logger)

		if iotaWload.workload == nil {
			msg := fmt.Sprintf("Trying to add workload of invalid type : %v", wlType)
			naples.logger.Error(msg)
			resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
			return resp
		}

		naples.logger.Printf("Setting up workload : %s", in.GetWorkloadName())
		resp, err := naples.setupWorkload(iotaWload.workload, in)

		if err != nil || resp.GetWorkloadStatus().GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
			iotaWload.workload.TearDown()
			return resp
		}

		if err := iotaWload.workload.SendArpProbe(strings.Split(in.GetIpPrefix(), "/")[0], Common.EsxDataVMInterface,
			0); err != nil {
			msg := fmt.Sprintf("Error in sending arp probe : %s", err.Error())
			naples.logger.Error(msg)
			resp = &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
			naples.entityMap.Delete(in.GetWorkloadName())
			iotaWload.workload.TearDown()
			return resp
		}

		iotaWload.workloadMsg = in
		resp.MgmtIp = iotaWload.workload.MgmtIP()
		naples.entityMap.Store(wloadKey, iotaWload)
		naples.logger.Printf("Added workload : %s (%s)", in.GetWorkloadName(), in.GetWorkloadType())
		return resp
	}

	//First download all VM images which we don't know about
	for _, wload := range in.Workloads {

		if _, ok := naples.imagesMap[wload.GetWorkloadImage()]; !ok {
			if dataVMDir, err := naples.downloadDataVMImage(wload.GetWorkloadImage()); err != nil {
				in.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: err.Error()}
				return in, errors.New(in.ApiResponse.ErrorMsg)
			} else {
				naples.imagesMap[wload.GetWorkloadImage()] = dataVMDir
			}
		}
	}

	pool, _ := errgroup.WithContext(context.Background())
	maxParallelThreads := 5
	currThreads := 0
	scheduleWloads := []*iota.Workload{}
	wloadIndex := []int{}

	for index, wload := range in.Workloads {
		currThreads++
		scheduleWloads = append(scheduleWloads, wload)
		wloadIndex = append(wloadIndex, index)
		if currThreads == maxParallelThreads-1 || index+1 == len(in.Workloads) {
			for thread, wl := range scheduleWloads {
				wload := wl
				index := wloadIndex[thread]
				pool.Go(func() error {
					resp := addWorkload(wload)
					in.Workloads[index] = resp
					return nil
				})
			}
			pool.Wait()
			scheduleWloads = []*iota.Workload{}
			wloadIndex = []int{}
			currThreads = 0
		}
	}

	pool.Wait()
	for _, wload := range in.Workloads {
		if wload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			in.ApiResponse = wload.WorkloadStatus
			return in, errors.New(wload.WorkloadStatus.ErrorMsg)
		}
	}
	in.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}

	return in, nil
}

func (naples *esxHwNode) getDataIntfs() ([]string, error) {

	hostEntity, ok := naples.entityMap.Load(naples.esxHostEntityKey)
	if !ok {
		return nil, errors.Errorf("Host entity not added : %s", naples.esxHostEntityKey)
	}

	cmd, err := naples.getInterfaceFindCommand("esx", naples.nicType)
	if err != nil {
		return nil, err
	}
	fullCmd := []string{cmd}
	cmdResp, _, _ := hostEntity.(iotaWorkload).workload.RunCommand(fullCmd, "", 0, false, true)
	naples.logger.Printf("naples data intf find out %s", cmdResp.Stdout)
	naples.logger.Printf("naples data intf find err %s", cmdResp.Stderr)
	naples.logger.Printf("naples data intf find  exit code %d", cmdResp.ExitCode)

	if cmdResp.ExitCode != 0 {
		return nil, errors.Errorf("Running command failed : %s", cmdResp.Stdout)
	}

	intfs := []string{}
	for _, str := range strings.Split(cmdResp.Stdout, "\r\n") {
		if str != "" {
			intfs = append(intfs, str)
		}
	}
	return intfs, nil
}

func (naples *esxHwNode) getNaplesMgmtIntf() (string, error) {

	var sshHandle *ssh.Client
	var err error
	sshConfig := &ssh.ClientConfig{
		User: naples.hostUsername,
		Auth: []ssh.AuthMethod{
			ssh.Password(naples.hostPassword),
			ssh.KeyboardInteractive(func(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
				answers = make([]string, len(questions))
				for n := range questions {
					answers[n] = naples.hostPassword
				}

				return answers, nil
			}),
		}, HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	sshHandle, err = ssh.Dial("tcp", naples.hostIP+":"+strconv.Itoa(sshPort), sshConfig)
	if err != nil {
		return "", err
	}

	cmd := []string{"esxcfg-nics", "-l", "|", "grep 'Pensando Ethernet Management'", "|", "cut -f1 -d ' '"}
	cmdResp, _ := Cmd.RunSSHCommand(sshHandle, strings.Join(cmd, " "), 0, false, false, naples.logger)

	naples.logger.Printf("naples mgmt intf find out %s", cmdResp.Ctx.Stdout)
	naples.logger.Printf("naples mgmt intf find err %s", cmdResp.Ctx.Stderr)
	naples.logger.Printf("naples mgmt intf find  exit code %d", cmdResp.Ctx.ExitCode)

	if cmdResp.Ctx.ExitCode != 0 {
		return "", errors.Errorf("Running command failed : %s", cmdResp.Ctx.Stdout)
	}

	return strings.TrimSpace(strings.Split(cmdResp.Ctx.Stdout, "\n")[0]), nil
}

func (naples *esxHwNode) createNaplesMgmtSwitch() error {

	naplesMgmtIntf, err := naples.getNaplesMgmtIntf()
	if err != nil || naplesMgmtIntf == "" {
		return errors.New("No naples management interfaces discovered")
	}

	naples.logger.Printf("Found naples management interface : %v", naplesMgmtIntf)

	vsname := Common.EsxIotaNaplesMgmtSwitch
	vsspec := vmware.VswitchSpec{Name: vsname, Pnics: []string{naplesMgmtIntf}}

	if err = naples.host.AddVswitch(vsspec); err != nil {
		return errors.Wrap(err, "Failed to create naples mgmt switch")
	}

	nws := []vmware.NWSpec{{Name: Common.EsxNaplesMgmtNetwork, Vlan: 0}}

	_, err = naples.host.AddNetworks(nws, vsspec)

	return err
}

func (naples *esxHwNode) createNaplesDataSwitch() error {

	naplesDataIntfs, err := naples.getDataIntfs()
	if err != nil || len(naplesDataIntfs) == 0 {
		return errors.New("No naples data interfaces discovered")
	}

	naples.logger.Printf("Naples data interfaces are %v", naplesDataIntfs)
	vsname := Common.EsxIotaDataSwitch
	vsspec := vmware.VswitchSpec{Name: vsname, Pnics: []string{}}
	for _, intf := range naplesDataIntfs {
		naples.logger.Printf("Adding Naples data interface %v", intf)
		vsspec.Pnics = append(vsspec.Pnics, intf)
		//Add just one interface for now
		break
	}
	return naples.host.AddVswitch(vsspec)
}

func (naples *esxHwNode) setUpNaplesMgmtIP() error {

	cmd := []string{"ip", "link", "set", "dev", Common.EsxCtrlVMNaplesMgmtInterface, "up"}
	if retCode, stdout, err := Utils.Run(cmd, 0, false, true, nil); retCode != 0 {
		return errors.Wrap(err, stdout)
	}

	cmd = []string{"ip", "addr", "add", Common.CtrlVMNaplesInterfaceIP, "dev", Common.EsxCtrlVMNaplesMgmtInterface}
	if retCode, stdout, err := Utils.Run(cmd, 0, false, true, nil); retCode != 0 {
		return errors.Wrap(err, stdout)
	}

	naples.logger.Println("Setting complete for naples Mgmt IP")
	return nil
}

func (naples *esxHwNode) setUpNaplesMgmtNetwork() error {

	if err := naples.createNaplesMgmtSwitch(); err != nil {
		return errors.Wrap(err, "Failed to create naples mgmt switch")
	}

	ctrlVM, err := naples.host.NewVM(Common.EsxControlVMName)
	if err != nil {
		return errors.Wrap(err, "Failed to find control VM")
	}

	err = ctrlVM.ReconfigureNetwork(Common.EsxDefaultNetwork, Common.EsxNaplesMgmtNetwork)
	if err != nil {
		return errors.Wrap(err, "Failed to reconfigure Ctrl VM to Naples Mgmt network")
	}

	time.Sleep(2 * time.Second)

	Utils.DisableDhcpOnInterface(Common.EsxCtrlVMNaplesMgmtInterface)

	naples.logger.Println("Setting up of naples management switch complete")
	return nil
}

func (naples *esxHwNode) addHostEntity(in *iota.Node) error {
	for _, entityEntry := range in.GetEntities() {
		var wload Workload.Workload
		if entityEntry.GetType() == iota.EntityType_ENTITY_TYPE_HOST {
			wload = Workload.NewWorkload(Workload.WorkloadTypeRemote, entityEntry.GetName(), naples.name, naples.logger)
			naples.esxHostEntityKey = entityEntry.GetName() + "_ESX"
			if err := wload.BringUp(naples.hostIP,
				strconv.Itoa(sshPort), naples.hostUsername, naples.hostPassword); err != nil {
				naples.logger.Errorf("Naples ESX Hw entity type add failed %v", err.Error())
				return err
			}
			naples.entityMap.Store(naples.esxHostEntityKey, iotaWorkload{workload: wload})

			//Add Naples host too
			wload = Workload.NewWorkload(Workload.WorkloadTypeBareMetal, entityEntry.GetName(), naples.name, naples.logger)
			naples.hostEntityKey = entityEntry.GetName()
			if err := wload.BringUp(); err != nil {
				naples.logger.Errorf("Naples Hw entity type add failed %v", err.Error())
				return err
			}
			naples.entityMap.Store(naples.hostEntityKey, iotaWorkload{workload: wload})
			wDir := Common.DstIotaEntitiesDir + "/" + entityEntry.GetName()
			wload.SetBaseDir(wDir)
		}
	}

	return nil
}

func (naples *esxHwNode) addNaplesEntity(in *iota.Node) error {
	for _, entityEntry := range in.GetEntities() {
		var wload Workload.Workload
		if entityEntry.GetType() == iota.EntityType_ENTITY_TYPE_NAPLES {
			if !in.Reload {
				if err := naples.setUpNaplesMgmtNetwork(); err != nil {
					return err
				}
			}
			naples.setUpNaplesMgmtIP()
			/*It is like running in a vm as its accesible only by ssh */
			wload = Workload.NewWorkload(Workload.WorkloadTypeRemote, entityEntry.GetName(), naples.name, naples.logger)
			naples.naplesEntityKey = entityEntry.GetName()
			naplesCfg := in.GetNaplesConfig()
			if err := wload.BringUp(naplesCfg.GetNaplesIpAddress(),
				strconv.Itoa(sshPort), naplesCfg.GetNaplesUsername(), naplesCfg.GetNaplesPassword()); err != nil {
				naples.logger.Errorf("Naples Hw entity type add failed %v", err.Error())
				return err
			}
			wDir := Common.DstIotaEntitiesDir + "/" + entityEntry.GetName()
			wload.SetBaseDir(wDir)

			if wload != nil {
				naples.entityMap.Store(entityEntry.GetName(), iotaWorkload{workload: wload})
			}
		}
	}
	return nil
}

func (naples *esxHwNode) addNodeEntities(in *iota.Node) error {

	/* First add host entitiy , as it requires to query for Naples interfaces*/
	if err := naples.addHostEntity(in); err != nil {
		return err
	}

	return naples.addNaplesEntity(in)
}

func (naples *esxHwNode) setHostIntfs(in *iota.Node) error {
	return nil
}

func (naples *esxNaplesHwNode) setHostIntfs(in *iota.Node) error {
	naplesDataIntfs, err := naples.getDataIntfs()
	if err != nil || len(naplesDataIntfs) == 0 {
		msg := "No naples data interfaces discovered"
		naples.logger.Error(msg)
		return errors.New(msg)
	}
	in.GetNaplesConfig().HostIntfs = naplesDataIntfs
	return nil
}

func (naples *esxThirdPartyHwNode) setHostIntfs(in *iota.Node) error {
	dataIntfs, err := naples.getDataIntfs()
	if err != nil || len(dataIntfs) == 0 {
		msg := "No data interfaces discovered"
		naples.logger.Error(msg)
		return errors.New(msg)
	}
	in.GetThirdPartyNicConfig().HostIntfs = dataIntfs
	return nil
}

//Init initalize node type
func (naples *esxNaplesHwNode) Init(in *iota.Node) (*iota.Node, error) {
	naples.nicType = in.GetNaplesConfig().GetNicType()
	resp, err := naples.esxHwNode.Init(in)
	if err != nil {
		return resp, err
	}

	newResp := &iota.Node{NodeStatus: apiSuccess, NodeUuid: resp.GetNodeUuid(),
		Name: resp.GetName(), IpAddress: resp.GetIpAddress(), Type: resp.GetType(),
		NodeInfo: &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{}}}

	if err = naples.setHostIntfs(newResp); err != nil {
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: err.Error()}}, err

	}
	return newResp, nil

}

//Init initalize node type
func (thirdParty *esxThirdPartyHwNode) Init(in *iota.Node) (*iota.Node, error) {
	thirdParty.nicType = in.GetThirdPartyNicConfig().GetNicType()
	resp, err := thirdParty.esxHwNode.Init(in)
	if err != nil {
		return resp, err
	}

	newResp := &iota.Node{NodeStatus: apiSuccess, NodeUuid: resp.GetNodeUuid(),
		Name: resp.GetName(), IpAddress: resp.GetIpAddress(), Type: resp.GetType(),
		NodeInfo: &iota.Node_ThirdPartyNicConfig{ThirdPartyNicConfig: &iota.ThirdPartyNicConfig{}}}

	if err = thirdParty.setHostIntfs(newResp); err != nil {
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: err.Error()}}, err

	}
	return newResp, nil
}

//Init initalize node type
func (naples *esxHwNode) Init(in *iota.Node) (*iota.Node, error) {

	naples.init(in)
	naples.iotaNode.name = in.GetName()
	if in.GetNaplesConfig() == nil {
		in.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{}}
	}

	naples.hostIP = in.GetEsxConfig().GetIpAddress()
	naples.hostUsername = in.GetEsxConfig().GetUsername()
	naples.hostPassword = in.GetEsxConfig().GetPassword()
	naples.imagesMap = make(map[string]string)

	host, err := vmware.NewHost(context.Background(), naples.hostIP, naples.hostUsername, naples.hostPassword)
	if err != nil {
		msg := "Cannot connect to ESX Host"
		naples.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
	}

	naples.host = host

	if err = naples.addNodeEntities(in); err != nil {
		msg := fmt.Sprintf("Adding node entities failed : %s", err.Error())
		naples.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
	}

	if !in.Reload {
		err = naples.createNaplesDataSwitch()
		if err != nil {
			msg := "failed to create naples data switch"
			naples.logger.Error(msg)
			return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
		}

	}

	nodeUUID, err := naples.getHwUUID(in)
	if err != nil {
		msg := fmt.Sprintf("Error in reading naples hw uuid : %s", err.Error())
		naples.logger.Error(msg)
		//For now ignore the error
		//return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
	}

	resp := &iota.Node{NodeStatus: apiSuccess, NodeUuid: nodeUUID,
		Name: in.GetName(), IpAddress: in.GetIpAddress(), Type: in.GetType(),
		NodeInfo: &iota.Node_NaplesConfig{}}

	return resp, nil
}

type triggerWrap struct {
	triggerMsg *iota.TriggerMsg
	cmdIndex   []int
	wloadName  string
}

// Trigger invokes the workload's trigger.
func (node *esxHwNode) Trigger(in *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	if _, err := node.dataNode.triggerValidate(in); err != nil {
		return &iota.TriggerMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: err.Error()}}, nil
	}

	runTrigger := func(client iota.IotaAgentApiClient, tw *triggerWrap) (*triggerWrap, error) {
		var err error
		node.logger.Printf("Sending trigger to : %s ", tw.wloadName)
		tw.triggerMsg, err = client.Trigger(context.Background(), tw.triggerMsg)
		node.logger.Printf("Completed trigger from  : %s ", tw.wloadName)
		return tw, err
	}
	if in.GetTriggerMode() == iota.TriggerMode_TRIGGER_NODE_PARALLEL {
		triggerMap := new(sync.Map)
		var twrap *triggerWrap

		for cmdIndex, cmd := range in.Commands {
			if val, ok := triggerMap.Load(cmd.EntityName); !ok {
				triggerMsg := &iota.TriggerMsg{Commands: []*iota.Command{},
					TriggerOp:   in.GetTriggerOp(),
					TriggerMode: in.GetTriggerMode()}
				twrap = &triggerWrap{triggerMsg: triggerMsg, cmdIndex: []int{}, wloadName: cmd.EntityName}
				triggerMap.Store(cmd.EntityName, twrap)
			} else {
				twrap = val.(*triggerWrap)
			}

			twrap.cmdIndex = append(twrap.cmdIndex, cmdIndex)
			twrap.triggerMsg.Commands = append(twrap.triggerMsg.Commands, cmd)
		}
		pool, _ := errgroup.WithContext(context.Background())
		triggerMap.Range(func(key interface{}, item interface{}) bool {
			twrap := item.(*triggerWrap)
			wload, _ := node.entityMap.Load(key.(string))
			iotaWload := wload.(iotaWorkload)
			wloadAgent := iotaWload.workload.GetWorkloadAgent()
			if wloadAgent != nil {
				pool.Go(func() error {
					runTrigger(wloadAgent.(iota.IotaAgentApiClient), twrap)
					return nil
				})
			} else {
				pool.Go(func() error {
					twrap.triggerMsg, _ = node.dataNode.Trigger(twrap.triggerMsg)
					return nil
				})
			}
			return true
		})
		pool.Wait()
		triggerMap.Range(func(key interface{}, item interface{}) bool {
			twrap := item.(*triggerWrap)
			for index, cmd := range twrap.triggerMsg.GetCommands() {
				realIndex := twrap.cmdIndex[index]
				in.Commands[realIndex] = cmd
			}

			return true
		})

	} else {
		for index, cmd := range in.Commands {
			wload, _ := node.entityMap.Load(cmd.EntityName)
			triggerMsg := &iota.TriggerMsg{Commands: []*iota.Command{cmd},
				TriggerOp:   in.GetTriggerOp(),
				TriggerMode: in.GetTriggerMode()}
			twrap := &triggerWrap{triggerMsg: triggerMsg, cmdIndex: []int{}, wloadName: cmd.EntityName}
			iotaWload := wload.(iotaWorkload)
			wloadAgent := iotaWload.workload.GetWorkloadAgent()
			if wloadAgent != nil {
				runTrigger(wloadAgent.(iota.IotaAgentApiClient), twrap)
			} else {
				twrap.triggerMsg, _ = node.dataNode.Trigger(twrap.triggerMsg)
			}
			in.Commands[index] = twrap.triggerMsg.GetCommands()[0]
		}
	}

	node.logger.Println("Completed running trigger.")
	in.ApiResponse = apiSuccess
	return in, nil

}
