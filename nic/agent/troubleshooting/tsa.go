// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package troubleshooting

import (
	netAgentState "github.com/pensando/sw/nic/agent/netagent/state"
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
	TsClient             *ctrlerif.TsClient
}

// NewTsAgent creates troubleshooting agent instance
func NewTsAgent(dp types.TsDatapathAPI, nodeUUID, ctrlerURL string, resolverClient resolver.Interface, na *netAgentState.Nagent, getMgmtIPAddr func() string) (*Agent, error) {

	var tsClient *ctrlerif.TsClient

	tsAgent, err := state.NewTsAgent(dp, nodeUUID, na, getMgmtIPAddr)
	if err != nil {
		log.Errorf("Error creating trouble shooting agent, Err: %v", err)
		return nil, err
	}

	tsClient, err = ctrlerif.NewTsClient(tsAgent, ctrlerURL, resolverClient)
	if err != nil {
		log.Errorf("Error creating TroubleShooting client. Err: %v", err)
		return nil, err
	}
	log.Infof("TroubleShooting client {%+v} is running", tsClient)

	agent := Agent{
		datapath:             dp,
		TroubleShootingAgent: tsAgent,
		TsClient:             tsClient,
	}
	return &agent, nil
}

// Stop stops the agent
func (agent *Agent) Stop() {
	agent.TroubleShootingAgent.Stop()
	if agent.TsClient != nil {
		agent.TsClient.Stop()
		agent.TsClient = nil
	}
}
