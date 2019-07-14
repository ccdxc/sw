package apollo

import (
	"github.com/pensando/sw/nic/agent/nmd/state"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/globals"
)

// Pipeline state info for apollo Pipeline
type Pipeline struct {
	Type state.Kind
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
func (p *Pipeline) SetNmd(interface{}) {
	return
}

// WriteDelphiObjects ...
func (p *Pipeline) WriteDelphiObjects() (err error) {
	return nil
}

// NewPipeline returns apis for apollo
func NewPipeline() (*Pipeline, error) {
	apollo := &Pipeline{
		Type: globals.NaplesPipelineApollo,
	}
	return apollo, nil
}
