// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package agent

import (
	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
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
	datapath  netagent.NetDatapathAPI
	Netagent  *netagent.NetAgent
	npmClient *ctrlerif.NpmClient
}

// NewAgent creates an agent instance
func NewAgent(dp netagent.NetDatapathAPI, dbPath, nodeUUID, ctrlerURL, resolverURLs string) (*Agent, error) {

	// create new network agent
	nagent, err := netagent.NewNetAgent(dp, dbPath, nodeUUID)
	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	log.Infof("NetAgent {%+v} is running", nagent)

	// pass the agent back to datapath
	err = dp.SetAgent(nagent)
	if err != nil {
		log.Errorf("Error linking datapath and agent. Err: %v", err)
		return nil, err
	}

	// create the NPM client
	npmClient, err := ctrlerif.NewNpmClient(nagent, ctrlerURL, resolverURLs)
	if err != nil {
		log.Errorf("Error creating NPM client. Err: %v", err)
		return nil, err
	}

	log.Infof("NPM client {%+v} is running", npmClient)

	// create the agent instance
	ag := Agent{
		Netagent:  nagent,
		datapath:  dp,
		npmClient: npmClient,
	}

	return &ag, nil
}

// Stop stops the agent
func (ag *Agent) Stop() {
	ag.npmClient.Stop()
	ag.Netagent.Stop()
}
