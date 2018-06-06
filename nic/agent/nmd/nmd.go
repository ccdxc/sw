// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package nmd

import (
	"time"

	"github.com/pensando/sw/nic/agent/nmd/cmdif"
	"github.com/pensando/sw/nic/agent/nmd/state"
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
}

// NewAgent creates an agent instance
func NewAgent(platform state.PlatformAPI,
	nmdDbPath, hostName, macAddr, cmdRegURL, cmdUpdURL, nmdListenURL, mode string,
	regInterval, updInterval time.Duration,
	resolverClient resolver.Interface) (*Agent, error) {

	// create new NMD instance
	nm, err := state.NewNMD(platform, nmdDbPath, hostName, macAddr, nmdListenURL, mode, regInterval, updInterval)
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

	// create the agent instance
	ag := Agent{
		nmd:       nm,
		cmdClient: cmdClient,
		platform:  platform,
	}

	return &ag, nil
}

// Stop stops the agent
func (ag *Agent) Stop() {

	log.Infof("NMD Stop")
	// Stop NMD and CmdClient
	ag.cmdClient.Stop()
	ag.nmd.Stop()
}

// GetNMD return the naples manager instance
func (ag *Agent) GetNMD() *state.NMD {
	if ag != nil {
		return ag.nmd
	}
	return nil
}
