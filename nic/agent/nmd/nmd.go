// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package nmd

import (
	"time"

	"github.com/pensando/sw/nic/agent/nmd/cmdif"
	"github.com/pensando/sw/nic/agent/nmd/rolloutif"

	"github.com/pensando/sw/nic/agent/nmd/state"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// Agent is the wrapper object that contains
// NMD, CmdClient and Platform components
type Agent struct {

	// NMD object
	nmd *state.NMD

	// CMD client object
	cmdClient *cmdif.CmdClient

	// Platform object
	platform state.PlatformAPI

	// Rollout controller client
	roClient state.RolloutCtrlAPI

	// Upgrademgr Interface
	upgmgr state.UpgMgrAPI

	// DelphiClient for NMD
	DelphiClient clientAPI.Client
}

// NewAgent creates an agent instance
func NewAgent(platform state.PlatformAPI, upgmgr state.UpgMgrAPI,
	nmdDbPath, hostName, macAddr, cmdRegURL, cmdUpdURL, nmdListenURL, certsListenURL, cmdAuthCertsURL, mode string,
	regInterval, updInterval time.Duration,
	resolverClient resolver.Interface) (*Agent, error) {

	// create new NMD instance
	nm, err := state.NewNMD(platform, upgmgr, nmdDbPath, hostName, macAddr, nmdListenURL, certsListenURL, cmdAuthCertsURL, cmdRegURL, cmdUpdURL, mode, regInterval, updInterval)
	if err != nil {
		log.Errorf("Error creating NMD. Err: %v", err)
		return nil, err
	}
	log.Infof("NMD {%+v} is running", nm)

	// create the CMD client
	cmdClient, err := cmdif.NewCmdClient(nm, cmdRegURL, cmdUpdURL, resolverClient)
	if err != nil {
		log.Errorf("Error creating CMD client. Err: %v", err)
		return nil, err
	}
	log.Infof("CMD client {%+v} is running", cmdClient)

	roClient, err := rolloutif.NewRoClient(nm, resolverClient)
	if err != nil {
		log.Errorf("Error creating Rollout Controller client. Err: %v", err)
		return nil, err
	}
	log.Infof("Rollout client {%+v} is running", roClient)

	// create the agent instance
	ag := Agent{
		nmd:       nm,
		cmdClient: cmdClient,
		platform:  platform,
		roClient:  roClient,
		upgmgr:    upgmgr,
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
	d.Agent.nmd.CreateIPClient(d.DelphiClient)
	d.Agent.nmd.UpdateMgmtIP()
}

// Name returns the name of the delphi service.
func (d *DelphiService) Name() string {
	return "NMD delphi client"
}

// Stop stops the agent
func (ag *Agent) Stop() {

	log.Infof("NMD Stop")
	// Stop NMD and CmdClient
	ag.cmdClient.Stop()
	ag.roClient.Stop()
	ag.nmd.Stop()
}

// GetNMD return the naples manager instance
func (ag *Agent) GetNMD() *state.NMD {
	if ag != nil {
		return ag.nmd
	}
	return nil
}
