// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package nmd

import (
	"time"

	"github.com/pensando/sw/nic/agent/nmd/cmdif"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/agent/nmd/rolloutif"
	"github.com/pensando/sw/nic/agent/nmd/state"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type delphiService struct {
}

func (d *delphiService) OnMountComplete() {
	log.Infof("OnMountComplete() done for %s", d.Name())
}

func (d *delphiService) Name() string {
	return "NMD delphi client"
}

// NewDelphiService returns Service object used to initialize delphi clients
func NewDelphiService() clientAPI.Service {
	return &delphiService{}
}

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

	delphiClient clientAPI.Client
}

// NewAgent creates an agent instance
func NewAgent(platform state.PlatformAPI, upgmgr state.UpgMgrAPI,
	nmdDbPath, hostName, macAddr, cmdRegURL, cmdUpdURL, nmdListenURL, certsListenURL, cmdAuthCertsURL, mode string,
	regInterval, updInterval time.Duration,
	resolverClient resolver.Interface,
	delphiClient clientAPI.Client) (*Agent, error) {

	// create new NMD instance
	nm, err := state.NewNMD(platform, upgmgr, nmdDbPath, hostName, macAddr, nmdListenURL, certsListenURL, cmdAuthCertsURL, mode, regInterval, updInterval)
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
		nmd:          nm,
		cmdClient:    cmdClient,
		platform:     platform,
		roClient:     roClient,
		upgmgr:       upgmgr,
		delphiClient: delphiClient,
	}

	// run delphi event loop in the background
	if delphiClient != nil {
		go delphiClient.Run()
	}

	// TODO Replace this with the real dhcp parsed information
	naplesStatus := delphiProto.NaplesStatus{
		Meta: &delphi.ObjectMeta{
			Kind: "NaplesStatus",
		},
		Controllers: []string{"A.B.C.D", "E.F.G.H"},
	}

	// TODO Remove this check prior to FCS as nmd is always expected to be started with delphi
	if delphiClient != nil {
		if err = ag.delphiClient.SetObject(&naplesStatus); err != nil {
			log.Errorf("Error writing the naples status object. Err: %v", err)
			return nil, err
		}
	}

	return &ag, nil
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
