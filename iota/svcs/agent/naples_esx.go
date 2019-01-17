package agent

import (
	"context"
	"fmt"
	"strconv"
	"strings"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"

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
	if err := wload.BringUp(in.GetWorkloadName(), in.GetWorkloadImage(),
		naples.hostIP, naples.hostUsername, naples.hostPassword); err != nil {
		msg := fmt.Sprintf("Error in workload image bring up : %s : %s", in.GetWorkloadName(), err.Error())
		naples.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return resp, err
	}
	naples.logger.Printf("Bring up workload : %s done", in.GetWorkloadName())

	return naples.configureWorkload(wload, in)
}

// AddWorkload brings up a workload type on a given node
func (naples *esxHwNode) AddWorkload(in *iota.Workload) (*iota.Workload, error) {

	if _, ok := iota.WorkloadType_name[(int32)(in.GetWorkloadType())]; !ok {
		msg := fmt.Sprintf("Workload Type %d not supported", in.GetWorkloadType())
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
		return resp, nil
	}

	wloadKey := in.GetWorkloadName()
	var iotaWload iotaWorkload
	naples.logger.Printf("Adding workload : %s", in.GetWorkloadName())
	if iotaWload, ok := naples.entityMap[wloadKey]; ok {
		msg := fmt.Sprintf("Trying to add workload %s, which already exists ", wloadKey)
		naples.logger.Error(msg)
		return naples.configureWorkload(iotaWload.workload, in)
	}

	wlType, ok := workloadTypeMap[in.GetWorkloadType()]
	if !ok {
		msg := fmt.Sprintf("Workload type %v not found", in.GetWorkloadType())
		naples.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
		return resp, nil
	}

	iotaWload.workload = Workload.NewWorkload(wlType, in.GetWorkloadName(), naples.name, naples.logger)

	if iotaWload.workload == nil {
		msg := fmt.Sprintf("Trying to add workload of invalid type : %v", wlType)
		naples.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
		return resp, nil
	}

	naples.logger.Printf("Setting up workload : %s", in.GetWorkloadName())
	resp, err := naples.setupWorkload(iotaWload.workload, in)

	if err != nil || resp.GetWorkloadStatus().GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
		iotaWload.workload.TearDown()
		return resp, nil
	}

	if err := iotaWload.workload.SendArpProbe(strings.Split(in.GetIpPrefix(), "/")[0], Common.EsxDataVMInterface,
		0); err != nil {
		msg := fmt.Sprintf("Error in sending arp probe : %s", err.Error())
		naples.logger.Error(msg)
		resp = &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		delete(naples.entityMap, in.GetWorkloadName())
		iotaWload.workload.TearDown()
		return resp, err
	}

	iotaWload.workloadMsg = in
	naples.entityMap[wloadKey] = iotaWload
	naples.logger.Printf("Added workload : %s (%s)", in.GetWorkloadName(), in.GetWorkloadType())
	return resp, nil
}

func (naples *esxHwNode) getDataIntfs() ([]string, error) {

	hostEntity, ok := naples.entityMap[naples.esxHostEntityKey]
	if !ok {
		return nil, errors.Errorf("Host entity not added : %s", naples.esxHostEntityKey)
	}

	cmd, err := naples.getInterfaceFindCommand("esx", naples.nicType)
	if err != nil {
		return nil, err
	}
	fullCmd := []string{cmd}
	cmdResp, _, _ := hostEntity.workload.RunCommand(fullCmd, "", 0, false, true)
	naples.logger.Printf("naples data intf find out %s", cmdResp.Stdout)
	naples.logger.Printf("naples data intf find err %s", cmdResp.Stderr)
	naples.logger.Printf("naples data intf find  exit code %d", cmdResp.ExitCode)

	if cmdResp.ExitCode != 0 {
		return nil, errors.Errorf("Running command failed : %s", cmdResp.Stdout)
	}

	return strings.Split(cmdResp.Stdout, "\r\n"), nil
}

func (naples *esxHwNode) getNaplesMgmtIntf() (string, error) {

	var sshHandle *ssh.Client
	var err error
	sshConfig := &ssh.ClientConfig{
		User: naples.hostUsername,
		Auth: []ssh.AuthMethod{
			ssh.Password(naples.hostPassword),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
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
	vsspec := vmware.VswitchSpec{Name: vsname, Pnics: []string{naplesDataIntfs[0]}}
	return naples.host.AddVswitch(vsspec)
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

	cmd := []string{"ip", "addr", "add", Common.CtrlVMNaplesInterfaceIP, "dev", Common.EsxCtrlVMNaplesMgmtInterface}
	if retCode, stdout, err := Utils.Run(cmd, 0, false, true, nil); retCode != 0 {
		return errors.Wrap(err, stdout)
	}

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
			naples.entityMap[naples.esxHostEntityKey] = iotaWorkload{workload: wload}

			//Add Naples host too
			wload = Workload.NewWorkload(Workload.WorkloadTypeBareMetal, entityEntry.GetName(), naples.name, naples.logger)
			naples.hostEntityKey = entityEntry.GetName()
			if err := wload.BringUp(); err != nil {
				naples.logger.Errorf("Naples Hw entity type add failed %v", err.Error())
				return err
			}
			naples.entityMap[naples.hostEntityKey] = iotaWorkload{workload: wload}
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
			if err := naples.setUpNaplesMgmtNetwork(); err != nil {
				return err
			}
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
				naples.entityMap[entityEntry.GetName()] = iotaWorkload{workload: wload}
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

	err = naples.createNaplesDataSwitch()
	if err != nil {
		msg := "failed to create naples data switch"
		naples.logger.Error(msg)
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}, err
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
