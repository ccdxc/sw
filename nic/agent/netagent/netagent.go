// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"sync"

	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	grpcDatapath "github.com/pensando/sw/nic/agent/netagent/datapath"
	delphiDatapath "github.com/pensando/sw/nic/agent/netagent/datapath/delphidp"
	protos "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
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
	sync.Mutex
	datapath      types.NetDatapathAPI
	NetworkAgent  *state.Nagent
	npmClient     *ctrlerif.NpmClient
	RestServer    *restapi.RestServer
	Mode          protos.AgentMode
	SysmgrClient  *sysmgr.Client
	DelphiClient  clientApi.Client
	mountComplete bool
}

// NewAgent creates an agent instance
func NewAgent(datapath string, dbPath, ctrlerURL string, resolverClient resolver.Interface, mode protos.AgentMode) (*Agent, error) {
	var ag Agent
	var dp types.NetDatapathAPI

	cl, err := gosdk.NewClient(&ag)
	if err != nil {
		log.Fatalf("Error creating delphi client. Err: %v", err)
	}

	ag.DelphiClient = cl

	// create sysmgr client
	ag.SysmgrClient = sysmgr.NewClient(ag.DelphiClient, globals.Netagent)

	// Create the appropriate datapath
	// ToDo Remove mock hal datapath prior to FCS
	if datapath == "hal" {
		dp, err = grpcDatapath.NewHalDatapath("hal")
		if err != nil {
			log.Fatalf("Error creating hal datapath. Err: %v", err)
		}
	} else if datapath == "delphi" {
		dp, err = delphiDatapath.NewDelphiDatapath(cl)
		if err != nil {
			log.Fatalf("Error creating delphi datapath. Err: %v", err)
		}
	} else {
		// Set expectations to allow mock testing
		dp, err = grpcDatapath.NewHalDatapath("mock")
		if err != nil {
			log.Fatalf("Error creating mock datapath. Err: %v", err)
		}
	}

	// create a new network agent
	nagent, err := state.NewNetAgent(dp, mode, dbPath)
	ag.NetworkAgent = nagent
	ag.datapath = dp
	ag.Mode = mode

	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	// Mount delphi naples status object
	delphiProto.NaplesStatusMount(ag.DelphiClient, delphi.MountMode_ReadMode)

	if mode == protos.AgentMode_MANAGED {
		// create the NPM client
		npmClient, err := ctrlerif.NewNpmClient(nagent, ctrlerURL, resolverClient)
		if err != nil {
			log.Errorf("Error creating NPM client. Err: %v", err)
			return nil, err
		}

		log.Infof("NPM client {%+v} is running", npmClient)
		ag.npmClient = npmClient
		return &ag, nil
	}

	// Run delphi client
	go ag.DelphiClient.Run()

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
	// TODO Uncomment this when delphi hub is successfully integrated with venice integ test.
	//	ag.DelphiClient.Close()
}

// OnMountComplete gets called after all the objectes are mounted
func (ag *Agent) OnMountComplete() {
	log.Infof("On mount complete got called")

	// let sysmgr know init completed
	ag.SysmgrClient.InitDone()

	ag.Lock()
	defer ag.Unlock()
	ag.mountComplete = true
}

// Name returns the name of the service
func (ag *Agent) Name() string {
	return globals.Netagent
}

// IsMountComplete returns true if delphi on mount complete callback is completed
func (ag *Agent) IsMountComplete() bool {
	ag.Lock()
	defer ag.Unlock()
	return ag.mountComplete
}
