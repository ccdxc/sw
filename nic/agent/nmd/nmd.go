// {C} Copyright 2017 Pensando Systems Inc. All rights reserved .

package nmd

import (
	"time"

	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/venice/utils/log"
)

// Agent alias
type Agent state.Agent

// NewAgent creates an agent instance
func NewAgent(p state.Pipeline, nmdDbPath, nmdListenURL, revProxyURL string, regInterval, updInterval time.Duration) (*Agent, error) {

	// create new NMD instance
	nm, err := state.NewNMD(
		p,
		nmdDbPath,
		nmdListenURL,
		revProxyURL,
		regInterval, updInterval)
	if err != nil {
		log.Errorf("Error creating NMD. Err: %v", err)
		return nil, err
	}
	log.Infof("NMD {%+v} is running", nm)
	nm.Pipeline = p
	if p != nil {
		p.SetNmd(nm)
	}

	// create the agent instance
	ag := Agent{
		Nmd:      nm,
		Platform: nm.Platform,
		Upgmgr:   nm.Upgmgr,
	}

	return &ag, nil
}

// Stop stops the agent
func (ag *Agent) Stop() {
	log.Infof("NMD Stop")
	if ag != nil {
		if ag.Nmd != nil {
			ag.Nmd.Stop()
		}
		ag.Nmd = nil
	}
}

// GetNMD return the naples manager instance
func (ag *Agent) GetNMD() *state.NMD {
	if ag != nil {
		return ag.Nmd
	}
	return nil
}
