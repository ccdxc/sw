// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/nic/agent/tpa"
	"github.com/pensando/sw/nic/agent/troubleshooting"

	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
	tsctrler "github.com/pensando/sw/nic/agent/troubleshooting/ctrlerif"

	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	grpcDatapath "github.com/pensando/sw/nic/agent/netagent/datapath"
	delphiDatapath "github.com/pensando/sw/nic/agent/netagent/datapath/delphidp"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/delphi/gosdk"
	clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/tsdb"
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
	datapath       types.NetDatapathAPI
	NetworkAgent   *state.Nagent
	ResolverClient resolver.Interface
	NpmClient      *ctrlerif.NpmClient
	RestServer     *restapi.RestServer
	mgmtIPAddr     string
	SysmgrClient   *sysmgr.Client
	DelphiClient   clientApi.Client
	NaplesStatus   delphiProto.NaplesStatus
	Tmagent        *tpa.PolicyAgent
	mountComplete  bool
	TroubleShoot   *troubleshooting.Agent
}

// NewAgent creates an agent instance
func NewAgent(datapath string, dbPath, ctrlerURL string, resolverClient resolver.Interface) (*Agent, error) {
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
	nagent, err := state.NewNetAgent(dp, dbPath, cl)
	ag.NetworkAgent = nagent
	ag.datapath = dp
	ag.ResolverClient = resolverClient

	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	// Mount delphi naples status object
	delphiProto.NaplesStatusMount(ag.DelphiClient, delphi.MountMode_ReadMode)

	// Set up watches
	delphiProto.NaplesStatusWatch(ag.DelphiClient, &ag)

	// Run delphi client
	go ag.DelphiClient.Run()

	return &ag, nil
}

// Stop stops the agent
func (ag *Agent) Stop() {
	if ag.NpmClient != nil {
		ag.NpmClient.Stop()
	}
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
	ag.mountComplete = true
	ag.Unlock()

	// walk naples status object
	nslist := delphiProto.NaplesStatusList(ag.DelphiClient)
	for _, ns := range nslist {
		ag.handleVeniceCoordinates(ns)
	}
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

// OnNaplesStatusCreate event handler
func (ag *Agent) OnNaplesStatusCreate(obj *delphiProto.NaplesStatus) {
	ag.handleVeniceCoordinates(obj)
	return
}

// OnNaplesStatusUpdate event handler
func (ag *Agent) OnNaplesStatusUpdate(old, new *delphiProto.NaplesStatus) {
	ag.handleVeniceCoordinates(new)
	return
}

// OnNaplesStatusDelete event handler
func (ag *Agent) OnNaplesStatusDelete(obj *delphiProto.NaplesStatus) {
	return
}

func (ag *Agent) handleVeniceCoordinates(obj *delphiProto.NaplesStatus) {
	log.Infof("Netagent reactor called with %v", obj)
	if (obj.NaplesMode == delphiProto.NaplesStatus_NETWORK_MANAGED_INBAND || obj.NaplesMode == delphiProto.NaplesStatus_NETWORK_MANAGED_OOB) && len(obj.Controllers) != 0 {
		ag.Lock()
		defer ag.Unlock()
		var controllers []string

		for _, ip := range obj.Controllers {
			controllers = append(controllers, fmt.Sprintf("%s:%s", ip, globals.CMDGRPCAuthPort))
		}

		isNewResolver := false
		if ag.ResolverClient == nil {
			ag.ResolverClient = resolver.New(&resolver.Config{Name: globals.Netagent, Servers: controllers})
			isNewResolver = true
			log.Infof("Populating Venice Co-ordinates with %v", controllers)
		} else {
			ag.ResolverClient.UpdateServers(controllers)
			log.Infof("Updating Venice Co-ordinates with %v", controllers)
		}

		// set Node UUID from FRU mac addr
		ag.NetworkAgent.NodeUUID = obj.SmartNicName
		ag.mgmtIPAddr = obj.MgmtIP

		// initialize netagent's tsdb client
		opts := &tsdb.Opts{
			ClientName:              globals.Netagent + ag.NetworkAgent.NodeUUID,
			ResolverClient:          ag.ResolverClient,
			Collector:               globals.Collector,
			DBName:                  "default",
			SendInterval:            time.Duration(30) * time.Second,
			ConnectionRetryInterval: 100 * time.Millisecond,
		}
		tsdb.Init(context.Background(), opts)
		if isNewResolver {
			// Lock netagent state
			ag.NetworkAgent.Lock()

			// Stop the existing npm client
			if ag.NpmClient != nil {
				ag.NpmClient.Stop()
			}

			ag.NetworkAgent.ControllerIPs = controllers

			// Clear previously registered controller interface.
			ag.NetworkAgent.Ctrlerif = nil
			ag.NetworkAgent.Mode = "network-managed"

			// create the NPM client
			npmClient, err := ctrlerif.NewNpmClient(ag.NetworkAgent, globals.Npm, ag.ResolverClient)
			if err != nil {
				log.Errorf("Error creating NPM client. Err: %v", err)
			}
			// Unlock netagent state
			ag.NetworkAgent.Unlock()

			log.Infof("NPM client {%+v} is running", npmClient)
			ag.NpmClient = npmClient

			if ag.TroubleShoot != nil {
				ag.TroubleShoot.TsClient, err = tsctrler.NewTsClient(ag.TroubleShoot.TroubleShootingAgent, globals.Tsm, ag.ResolverClient)
				if err != nil {
					log.Errorf("Error creating TroubleShooting client. Err: %v", err)
				}
				log.Infof("TroubleShooting client {%+v} is running", ag.TroubleShoot.TroubleShootingAgent)
			}

			if ag.Tmagent != nil { // tmagent was enableds
				if err := ag.Tmagent.NewTpClient(ag.NetworkAgent.NodeUUID, ag.ResolverClient); err != nil {
					log.Errorf("Error creating telemetry policy client, Err: %v", err)
				}
				log.Infof("telemetry policy client {%+v} is running", ag.Tmagent.TpState)
			}
		}
	}
}

// IsNpmClientConnected returns true if NPM client is connected to server
func (ag *Agent) IsNpmClientConnected() bool {
	return ag.NpmClient.IsConnected()
}

// GetMgmtIPAddr returns management ip address received in Naple status
func (ag *Agent) GetMgmtIPAddr() string {
	ag.Lock()
	defer ag.Unlock()
	return ag.mgmtIPAddr
}
