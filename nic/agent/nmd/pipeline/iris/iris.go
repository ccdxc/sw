package iris

import (
	"net"

	cmd "github.com/pensando/sw/api/generated/cluster"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/nic/delphi/gosdk"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	sysmgrProto "github.com/pensando/sw/nic/sysmgr/proto/sysmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// Pipeline ...
type Pipeline struct {
	Type         state.Kind
	DelSrv       *DelSrv
	Nmd          *state.NMD
	SysmgrClient interface{}
}

type service struct {
	name         string
	sysmgrClient *sysmgr.Client
}

var srv = &service{
	name: "nmd",
}

func (s *service) OnMountComplete() {
	log.Printf("OnMountComplete() done for %s\n", s.name)
	s.sysmgrClient.InitDone()
}

func (s *service) Name() string {
	return s.name
}

// DelSrv struct helps to convert NMD into a Delphi Service
type DelSrv struct {
	DelphiClient clientAPI.Client
	Agent        state.Agent
}

// NewDelSrv creates a new NMD delphi service
func NewDelSrv() *DelSrv {
	return &DelSrv{}
}

// OnMountComplete is the function which is called by Delphi when the mounting of Service objects is completed.
func (d *DelSrv) OnMountComplete() {
	log.Infof("OnMountComplete() called for %s", d.Name())
	if err := d.Agent.Nmd.UpdateNaplesConfig(d.Agent.Nmd.GetNaplesConfig()); err != nil {
		log.Errorf("Failed to update naples during onMountComplete. Err: %v", err)
	}
	d.Agent.Nmd.UpdateCurrentManagementMode()
	d.Agent.Nmd.CreateIPClient()
	if err := d.Agent.Nmd.Upgmgr.IsUpgClientRegistered(); err != nil {
		log.Errorf("No upgrade manager")
		return
	}
	log.Infof("OnMountComplete() checking if upgrade is in progress")
	if !d.Agent.Nmd.Upgmgr.IsUpgradeInProgress() {
		log.Infof("OnMountComplete() upgrade is not in progress")
		d.Agent.Nmd.IssueInProgressOp()
	}
	log.Infof("OnMountComplete() done for %s", d.Name())
}

// Name returns the name of the delphi service.
func (d *DelSrv) Name() string {
	return "NMD delphi client"
}

// InitDelphi ...
func (p *Pipeline) InitDelphi() interface{} {
	var delphiClient clientAPI.Client
	//var uc api.UpgMgrAPI
	var dServ *DelSrv
	dServ = NewDelSrv()
	delphiClient, err := gosdk.NewClient(dServ)
	if err != nil {
		log.Fatalf("Error creating delphi client . Err: %v", err)
	}
	dServ.DelphiClient = delphiClient
	p.DelSrv = dServ
	return nil
}

// GetDelphiClient ...
func (p *Pipeline) GetDelphiClient() clientAPI.Client {
	return p.DelSrv.DelphiClient
}

// MountDSCObjects ...
func (p *Pipeline) MountDSCObjects() interface{} {
	// mount objects
	log.Infof("Mounting naples status rw")
	delphiProto.DistributedServiceCardStatusMount(p.DelSrv.DelphiClient, delphi.MountMode_ReadWriteMode)
	return nil
}

// InitSysmgr ...
func (p *Pipeline) InitSysmgr() {
	srv.sysmgrClient = sysmgr.NewClient(p.DelSrv.DelphiClient, srv.name)
}

// MountSysmgrObjects ...
func (p *Pipeline) MountSysmgrObjects() interface{} {
	log.Infof("Mounting SysmgrSystemStatus")
	sysmgrProto.SysmgrSystemStatusMount(p.DelSrv.DelphiClient, delphi.MountMode_ReadMode)
	return nil
}

// GetSysmgrSystemStatus ...
func (p *Pipeline) GetSysmgrSystemStatus() (string, string) {
	status := cmd.ConditionStatus_TRUE.String()
	reason := ""
	sysmgrSysStatus := sysmgrProto.GetSysmgrSystemStatus(p.GetDelphiClient())
	if sysmgrSysStatus != nil && sysmgrSysStatus.State == sysmgrProto.SystemState_Fault {
		status = cmd.ConditionStatus_FALSE.String()
		reason = sysmgrSysStatus.Reason
	}
	return status, reason
}

// RunDelphiClient ...
func (p *Pipeline) RunDelphiClient(agent state.Agent) interface{} {
	p.DelSrv.Agent = agent
	go p.GetDelphiClient().Run()
	return nil
}

// SetNmd ...
func (p *Pipeline) SetNmd(nmd interface{}) {
	if val, ok := nmd.(*state.NMD); ok {
		p.Nmd = val
		return
	}
	log.Infof("type validation failed for nmd")
}

// GetPipelineType ...
func (p *Pipeline) GetPipelineType() string {
	return p.Type.String()
}

// WriteDelphiObjects ...
func (p *Pipeline) WriteDelphiObjects() (err error) {
	n := p.Nmd
	var mgmtIP string

	var transitionPhase delphiProto.DistributedServiceCardStatus_Transition
	naplesConfig := p.DelSrv.Agent.Nmd.GetNaplesConfig()

	switch naplesConfig.Status.TransitionPhase {
	case delphiProto.DistributedServiceCardStatus_DHCP_SENT.String():
		transitionPhase = delphiProto.DistributedServiceCardStatus_DHCP_SENT
	case delphiProto.DistributedServiceCardStatus_DHCP_DONE.String():
		transitionPhase = delphiProto.DistributedServiceCardStatus_DHCP_DONE
	case delphiProto.DistributedServiceCardStatus_DHCP_TIMEDOUT.String():
		transitionPhase = delphiProto.DistributedServiceCardStatus_DHCP_TIMEDOUT
	case delphiProto.DistributedServiceCardStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String():
		transitionPhase = delphiProto.DistributedServiceCardStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES
	case delphiProto.DistributedServiceCardStatus_VENICE_REGISTRATION_SENT.String():
		transitionPhase = delphiProto.DistributedServiceCardStatus_VENICE_REGISTRATION_SENT
	case delphiProto.DistributedServiceCardStatus_VENICE_REGISTRATION_DONE.String():
		transitionPhase = delphiProto.DistributedServiceCardStatus_VENICE_REGISTRATION_DONE
	case delphiProto.DistributedServiceCardStatus_VENICE_UNREACHABLE.String():
		transitionPhase = delphiProto.DistributedServiceCardStatus_VENICE_UNREACHABLE
	case delphiProto.DistributedServiceCardStatus_REBOOT_PENDING.String():
		transitionPhase = delphiProto.DistributedServiceCardStatus_REBOOT_PENDING
	default:
		transitionPhase = 0
	}

	// For static case write only the IP in mgmt IP and not the subnet
	if ip, _, err := net.ParseCIDR(naplesConfig.Status.IPConfig.IPAddress); err == nil {
		mgmtIP = ip.String()
	} else {
		mgmtIP = naplesConfig.Status.IPConfig.IPAddress
	}

	// Set up appropriate mode
	var naplesMode delphiProto.DistributedServiceCardStatus_Mode

	switch naplesConfig.Spec.NetworkMode {
	case nmd.NetworkMode_INBAND.String():
		naplesMode = delphiProto.DistributedServiceCardStatus_NETWORK_MANAGED_INBAND
	case nmd.NetworkMode_OOB.String():
		naplesMode = delphiProto.DistributedServiceCardStatus_NETWORK_MANAGED_OOB
	default:
		naplesMode = delphiProto.DistributedServiceCardStatus_HOST_MANAGED
		naplesStatus := delphiProto.DistributedServiceCardStatus{
			DistributedServiceCardMode: naplesMode,
			ID:                         naplesConfig.Spec.ID,
			DSCName:                    naplesConfig.Status.DSCName,
			Fru: &delphiProto.DistributedServiceCardFru{
				ManufacturingDate: naplesConfig.Status.Fru.ManufacturingDate,
				Manufacturer:      naplesConfig.Status.Fru.Manufacturer,
				ProductName:       naplesConfig.Status.Fru.ProductName,
				SerialNum:         naplesConfig.Status.Fru.SerialNum,
				PartNum:           naplesConfig.Status.Fru.PartNum,
				BoardId:           naplesConfig.Status.Fru.BoardId,
				EngChangeLevel:    naplesConfig.Status.Fru.EngChangeLevel,
				NumMacAddr:        naplesConfig.Status.Fru.NumMacAddr,
				MacStr:            naplesConfig.Status.Fru.MacStr,
			},
		}
		if err := n.DelphiClient.SetObject(&naplesStatus); err != nil {
			log.Errorf("Error writing the naples status object in host mode. Err: %v", err)
			return err
		}
		return nil
	}

	naplesStatus := delphiProto.DistributedServiceCardStatus{
		Controllers:                naplesConfig.Status.Controllers,
		DistributedServiceCardMode: naplesMode,
		TransitionPhase:            transitionPhase,
		MgmtIP:                     mgmtIP,
		ID:                         naplesConfig.Spec.ID,
		DSCName:                    naplesConfig.Status.DSCName,
		Fru: &delphiProto.DistributedServiceCardFru{
			ManufacturingDate: naplesConfig.Status.Fru.ManufacturingDate,
			Manufacturer:      naplesConfig.Status.Fru.Manufacturer,
			ProductName:       naplesConfig.Status.Fru.ProductName,
			SerialNum:         naplesConfig.Status.Fru.SerialNum,
			PartNum:           naplesConfig.Status.Fru.PartNum,
			BoardId:           naplesConfig.Status.Fru.BoardId,
			EngChangeLevel:    naplesConfig.Status.Fru.EngChangeLevel,
			NumMacAddr:        naplesConfig.Status.Fru.NumMacAddr,
			MacStr:            naplesConfig.Status.Fru.MacStr,
		},
	}

	if err := n.DelphiClient.SetObject(&naplesStatus); err != nil {
		log.Errorf("Error writing the naples status object in network mode. Err: %v", err)
		return err
	}

	return nil
}

// NewPipeline returns implementation of iris
func NewPipeline() (*Pipeline, error) {
	iris := &Pipeline{
		Type: globals.NaplesPipelineIris,
	}
	return iris, nil
}
