// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	types "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

/* Rough Architecture for Pensando Agent
 * -------------------------------------------
 *
 *
 *
 *               +----------------------+----------------------+
 *               | REST Handler         |  gRPC handler        |
 *               +----------------------+----------------------+
 *                                      |
 *                            Controller Interface
 *                                      |
 *                                      V
 *  +---------+  +---------------------------------------------+
 *  |  K8s    +->+                    NetAgent                 |
 *  | Plugin  |  |                                    +--------+----+
 *  +---------+  |                                    | Embedded DB |
 *  +---------+  |                 Core Objects       +--------+----+
 *  | Docker  +->+                                             |
 *  +---------+  +-----------------------+---------------------+
 *                                       |
 *                               Datapath Interface
 *                                       |
 *                                       V
 *               +-----------------------+---------------------+
 *               |  Fake Datapath        |    HAL Interface    |
 *               +-----------------------+---------------------+
 */

// Agent contains agent state
type Agent struct {
	datapath     state.NetDatapathAPI
	NetworkAgent *state.NetAgent
	npmClient    *ctrlerif.NpmClient
	RestServer   *restapi.RestServer
	Mode         types.AgentMode
}

// NewAgent creates an agent instance
func NewAgent(dp state.NetDatapathAPI, dbPath, nodeUUID, ctrlerURL, restListenURL string, resolverClient resolver.Interface, mode types.AgentMode) (*Agent, error) {
	var ag Agent
	// create a new network agent
	nagent, err := state.NewNetAgent(dp, mode, dbPath, nodeUUID)

	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	// create REST api server
	// ToDo Open Agent REST Server only in Classic Mode prior to FCS for security reasons.
	restServer, err := restapi.NewRestServer(nagent, restListenURL)
	if err != nil {
		log.Errorf("Error creating the rest API server. Err: %v", err)
		return nil, err
	}

	if mode == types.AgentMode_MANAGED {
		// create the NPM client
		npmClient, err := ctrlerif.NewNpmClient(nagent, ctrlerURL, resolverClient)
		if err != nil {
			log.Errorf("Error creating NPM client. Err: %v", err)
			return nil, err
		}

		log.Infof("NPM client {%+v} is running", npmClient)
		ag = Agent{
			datapath:     dp,
			NetworkAgent: nagent,
			npmClient:    npmClient,
			RestServer:   restServer,
			Mode:         mode,
		}
		return &ag, nil
	}
	ag = Agent{
		datapath:     dp,
		NetworkAgent: nagent,
		RestServer:   restServer,
		npmClient:    nil,
		Mode:         mode,
	}
	return &ag, nil
}

// Stop stops the agent
func (ag *Agent) Stop() {
	if ag.Mode == types.AgentMode_CLASSIC {
		ag.RestServer.Stop()
		ag.NetworkAgent.Stop()
		return
	}
	ag.npmClient.Stop()
	ag.NetworkAgent.Stop()
	ag.RestServer.Stop()
}
