// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/utils/log"
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
}

// NewAgent creates an agent instance
func NewAgent(dp state.NetDatapathAPI, dbPath, nodeUUID, ctrlerURL, resolverURLs, restListenURL string) (*Agent, error) {
	// create a new network agent
	nagent, err := state.NewNetAgent(dp, dbPath, nodeUUID)

	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	// create the NPM client
	npmClient, err := ctrlerif.NewNpmClient(nagent, ctrlerURL, resolverURLs)
	if err != nil {
		log.Errorf("Error creating NPM client. Err: %v", err)
		return nil, err
	}

	log.Infof("NPM client {%+v} is running", npmClient)

	// create REST api server
	restServer, err := restapi.NewRestServer(nagent, restListenURL)
	if err != nil {
		log.Errorf("Error creating the rest API server. Err: %v", err)
		return nil, err
	}

	// create the agent instance
	ag := Agent{
		datapath:     dp,
		NetworkAgent: nagent,
		npmClient:    npmClient,
		RestServer:   restServer,
	}

	return &ag, nil
}

// Stop stops the agent
func (ag *Agent) Stop() {
	ag.npmClient.Stop()
	ag.NetworkAgent.Stop()
	ag.RestServer.Stop()
}
