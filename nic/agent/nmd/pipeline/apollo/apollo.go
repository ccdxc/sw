package apollo

import (
	"encoding/json"
	"io/ioutil"
	"net"

	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// Pipeline state info for apollo Pipeline
type Pipeline struct {
	Type  state.Kind
	Agent state.Agent
}

// InitDelphi ...
func (p *Pipeline) InitDelphi() interface{} {
	return nil
}

// GetDelphiClient ...
func (p *Pipeline) GetDelphiClient() clientAPI.Client {
	return nil
}

// MountDelphiObjects ...
func (p *Pipeline) MountDelphiObjects() interface{} {
	return nil
}

// InitSysmgr ...
func (p *Pipeline) InitSysmgr() {
	return
}

// MountSysmgrObjects ...
func (p *Pipeline) MountSysmgrObjects() interface{} {
	return nil
}

// RunDelphiClient ...
func (p *Pipeline) RunDelphiClient(agent state.Agent) interface{} {
	return nil
}

// GetSysmgrSystemStatus ...
func (p *Pipeline) GetSysmgrSystemStatus() (string, string) {
	return "", ""
}

// SetNmd ...
func (p *Pipeline) SetNmd(nmd interface{}) {
	if val, ok := nmd.(*state.NMD); ok {
		ag := &state.Agent{
			Nmd: val,
		}
		p.Agent = *ag
	}
	log.Infof("type validation failed for nmd")
	return
}

// GetPipelineType ...
func (p *Pipeline) GetPipelineType() string {
	return p.Type.String()
}

// WriteDelphiObjects ...
func (p *Pipeline) WriteDelphiObjects() (err error) {
	var mgmtIP string

	var transitionPhase delphiProto.NaplesStatus_Transition
	naplesConfig := p.Agent.Nmd.GetNaplesConfig()

	switch naplesConfig.Status.TransitionPhase {
	case delphiProto.NaplesStatus_DHCP_SENT.String():
		transitionPhase = delphiProto.NaplesStatus_DHCP_SENT
	case delphiProto.NaplesStatus_DHCP_DONE.String():
		transitionPhase = delphiProto.NaplesStatus_DHCP_DONE
	case delphiProto.NaplesStatus_DHCP_TIMEDOUT.String():
		transitionPhase = delphiProto.NaplesStatus_DHCP_TIMEDOUT
	case delphiProto.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String():
		transitionPhase = delphiProto.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES
	case delphiProto.NaplesStatus_VENICE_REGISTRATION_SENT.String():
		transitionPhase = delphiProto.NaplesStatus_VENICE_REGISTRATION_SENT
	case delphiProto.NaplesStatus_VENICE_REGISTRATION_DONE.String():
		transitionPhase = delphiProto.NaplesStatus_VENICE_REGISTRATION_DONE
	case delphiProto.NaplesStatus_VENICE_UNREACHABLE.String():
		transitionPhase = delphiProto.NaplesStatus_VENICE_UNREACHABLE
	case delphiProto.NaplesStatus_REBOOT_PENDING.String():
		transitionPhase = delphiProto.NaplesStatus_REBOOT_PENDING
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
	var naplesMode delphiProto.NaplesStatus_Mode

	switch naplesConfig.Spec.NetworkMode {
	case nmd.NetworkMode_INBAND.String():
		naplesMode = delphiProto.NaplesStatus_NETWORK_MANAGED_INBAND
	case nmd.NetworkMode_OOB.String():
		naplesMode = delphiProto.NaplesStatus_NETWORK_MANAGED_OOB
	default:
		naplesMode = delphiProto.NaplesStatus_HOST_MANAGED
		naplesStatus := delphiProto.NaplesStatus{
			NaplesMode:   naplesMode,
			ID:           naplesConfig.Spec.ID,
			DSCName: naplesConfig.Status.DSCName,
			Fru: &delphiProto.NaplesFru{
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
		file, _ := json.MarshalIndent(naplesStatus, "", "\t")
		s := "\n"
		file = append(file, s...)
		_ = ioutil.WriteFile("/sysconfig/config0/naplesStatus.json", file, 0644)
		_ = ioutil.WriteFile("/sysconfig/config1/naplesStatus.json", file, 0644)
		return nil
	}

	naplesStatus := delphiProto.NaplesStatus{
		Controllers:     naplesConfig.Status.Controllers,
		NaplesMode:      naplesMode,
		TransitionPhase: transitionPhase,
		MgmtIP:          mgmtIP,
		ID:              naplesConfig.Spec.ID,
		DSCName:    naplesConfig.Status.DSCName,
		Fru: &delphiProto.NaplesFru{
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
	file, _ := json.MarshalIndent(naplesStatus, "", "\t")
	s := "\n"
	file = append(file, s...)
	_ = ioutil.WriteFile("/sysconfig/config0/naplesStatus.json", file, 0644)
	_ = ioutil.WriteFile("/sysconfig/config1/naplesStatus.json", file, 0644)

	return nil
}

// NewPipeline returns apis for apollo
func NewPipeline() (*Pipeline, error) {
	apollo := &Pipeline{
		Type: globals.NaplesPipelineApollo,
	}
	return apollo, nil
}
