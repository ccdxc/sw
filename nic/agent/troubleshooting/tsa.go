// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package troubleshooting

import (
	protos "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/troubleshooting/ctrlerif"
	"github.com/pensando/sw/nic/agent/troubleshooting/state"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

/* Trouble Shooting Agent and associated modules
 * ----------------------------------------------
 *
 *
 *
 *               +------------------------+----------------------+
 *               | NetAgent's REST Handler|  gRPC handler        |
 *               +------------------------+----------------------+
 *                                      |
 *                            Controller Interface [ troubleshooting/ctrlerif/*.go ]
 *                                      |
 *                                      V
 *  +---------+  +---------------------------------------------+
 *  |  K8s    +->+        TroubleShootingAgent                 |
 *  | Plugin  |  |                                    +--------+----+
 *  +---------+  |                                    | Embedded DB |
 *  +---------+  |                 Core Objects       +--------+----+
 *  |         |  |                                             |
 *  | Docker  +->+    troubleshooting/state/*.go               |
 *  |         |  |                                             |
 *  +---------+  +-----------------------+---------------------+
 *                                       |
 *                               Datapath Interface [ troubleshooting/datapath/*.go ]
 *                                       |
 *                                       V
 *               +-----------------------+---------------------+
 *               |  Fake Datapath        |    HAL Interface    |
 *               |                       |    (TS Apis)        |
 *               +-----------------------+---------------------+
 */

// Agent contains agent state
type Agent struct {
	datapath             types.TsDatapathAPI
	TroubleShootingAgent *state.Tagent
	tsClient             *ctrlerif.TsClient
	Mode                 protos.AgentMode
}

// NewTsAgent creates troubleshooting agent instance
func NewTsAgent(dp types.TsDatapathAPI, dbPath, nodeUUID, ctrlerURL string, resolverClient resolver.Interface, mode protos.AgentMode) (*Agent, error) {

	var tsClient *ctrlerif.TsClient

	tsAgent, err := state.NewTsAgent(dp, mode, dbPath, nodeUUID)
	if err != nil {
		log.Errorf("Error creating trouble shooting agent, Err: %v", err)
		return nil, err
	}

	if mode == protos.AgentMode_MANAGED {
		tsClient, err = ctrlerif.NewTsClient(tsAgent, ctrlerURL, resolverClient)
		if err != nil {
			log.Errorf("Error creating TroubleShooting client. Err: %v", err)
			return nil, err
		}
		log.Info("TroubleShooting client {%+v} is running", tsClient)
	}

	agent := Agent{
		datapath:             dp,
		TroubleShootingAgent: tsAgent,
		tsClient:             tsClient,
		Mode:                 mode,
	}
	return &agent, nil
}

// Stop stops the agent
func (agent *Agent) Stop() {
	agent.TroubleShootingAgent.Stop()
}
