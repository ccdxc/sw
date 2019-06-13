// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package nmd

import (
	"time"

	nmdapi "github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/nic/agent/nmd/state"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/utils/log"
)

// Agent is the wrapper object that contains NMD and Platform components
type Agent struct {

	// NMD object
	nmd *state.NMD

	// Platform object
	platform nmdapi.PlatformAPI

	// Upgrademgr Interface
	upgmgr nmdapi.UpgMgrAPI

	// DelphiClient for NMD
	DelphiClient clientAPI.Client
}

// NewAgent creates an agent instance
func NewAgent(delphiClient clientAPI.Client, nmdDbPath, nmdListenURL, revProxyURL string, regInterval, updInterval time.Duration) (*Agent, error) {

	// create new NMD instance
	nm, err := state.NewNMD(
		delphiClient,
		nmdDbPath,
		nmdListenURL,
		revProxyURL,
		regInterval, updInterval)
	if err != nil {
		log.Errorf("Error creating NMD. Err: %v", err)
		return nil, err
	}
	log.Infof("NMD {%+v} is running", nm)

	// create the agent instance
	ag := Agent{
		nmd:      nm,
		platform: nm.Platform,
		upgmgr:   nm.Upgmgr,
	}

	return &ag, nil
}

// DelphiService struct helps to convert NMD into a Delphi Service
type DelphiService struct {
	DelphiClient clientAPI.Client
	Agent        *Agent
}

// NewDelphiService creates a new NMD delphi service
func NewDelphiService() *DelphiService {
	return &DelphiService{}
}

// OnMountComplete is the function which is called by Delphi when the mounting of Service objects is completed.
func (d *DelphiService) OnMountComplete() {
	log.Infof("OnMountComplete() done for %s", d.Name())
	if err := d.Agent.nmd.UpdateNaplesConfig(d.Agent.nmd.GetNaplesConfig()); err != nil {
		log.Errorf("Failed to update naples during onMountComplete. Err: %v", err)
	}
	d.Agent.nmd.UpdateCurrentManagementMode()
	d.Agent.nmd.CreateIPClient(d.DelphiClient)
}

// Name returns the name of the delphi service.
func (d *DelphiService) Name() string {
	return "NMD delphi client"
}

// Stop stops the agent
func (ag *Agent) Stop() {
	log.Infof("NMD Stop")
	if ag.nmd != nil {
		ag.nmd.Stop()
	}
	ag.nmd = nil
}

// GetNMD return the naples manager instance
func (ag *Agent) GetNMD() *state.NMD {
	if ag != nil {
		return ag.nmd
	}
	return nil
}
