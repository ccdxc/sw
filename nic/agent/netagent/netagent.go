// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	protos "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
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
	datapath     types.NetDatapathAPI
	NetworkAgent *state.Nagent
	npmClient    *ctrlerif.NpmClient
	RestServer   *restapi.RestServer
	Mode         protos.AgentMode
}

// NewAgent creates an agent instance
func NewAgent(dp types.NetDatapathAPI, dbPath, nodeUUID, ctrlerURL string, resolverClient resolver.Interface, mode protos.AgentMode) (*Agent, error) {
	var ag Agent
	// create a new network agent
	nagent, err := state.NewNetAgent(dp, mode, dbPath, nodeUUID)

	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	if mode == protos.AgentMode_MANAGED {
		// create the NPM client
		npmClient, err := ctrlerif.NewNpmClient(nagent, ctrlerURL, resolverClient)
		if err != nil {
			log.Errorf("Error creating NPM client. Err: %v", err)
			return nil, err
		}

		log.Infof("NPM client {%+v} is running", npmClient)

		// create the agent instance
		ag = Agent{
			datapath:     dp,
			NetworkAgent: nagent,
			npmClient:    npmClient,
			Mode:         mode,
		}
		return &ag, nil
	}
	ag = Agent{
		datapath:     dp,
		NetworkAgent: nagent,
		npmClient:    nil,
		Mode:         mode,
	}
	return &ag, nil
}

// Stop stops the agent
func (ag *Agent) Stop() {
	if ag.Mode == protos.AgentMode_CLASSIC {
		ag.RestServer.Stop()
		ag.NetworkAgent.Stop()
		return
	}
	ag.npmClient.Stop()
	ag.NetworkAgent.Stop()
	ag.RestServer.Stop()
}
