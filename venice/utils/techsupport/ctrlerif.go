package techsupport

import (
	"context"
	"sync"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	tsconfig "github.com/pensando/sw/venice/ctrler/tsm/config"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"

	action "github.com/pensando/sw/venice/utils/techsupport/actionengine"
	rest "github.com/pensando/sw/venice/utils/techsupport/restapi"
)

const (
	// kindTechSupportRequest is the kind of SmartNIC objects in string form
	kindTechSupportRequest = string(monitoring.KindTechSupportRequest)
	maxChanLength          = 2000
)

// TSMClient keeps the techsupport state
type TSMClient struct {
	sync.Mutex
	waitGrp sync.WaitGroup
	stopped bool

	// the name of the node. Must match the name of the Node/SmartNIC object
	name string
	// the kind of the node: Node (controller) or SmartNIC
	kind string
	// primary mac of the node
	mac string
	// the notifications that have been received
	notifications []*tsproto.TechSupportRequestEvent
	// a flag used to know if agent is watching or not -- access using atomics
	watching int32 // 0 --> not watching, 1 --> watching
	// rpc client used to watch and send updates
	tsGrpcClient *rpckit.RPCClient
	// API client used to watch and send updates
	tsAPIClient          tsproto.TechSupportApiClient
	diagnosticsAPIClient tsproto.DiagnosticsApiClient
	ctx                  context.Context
	// cancel function
	cancelFn       context.CancelFunc
	resolverClient resolver.Interface
	restServer     *rest.RestServer
	tsCh           chan tsproto.TechSupportRequest
	moduleCh       chan diagnostics.Module
	cfg            *tsconfig.TechSupportConfig
}

// NewTSMClient creates a new techsupport agent
func NewTSMClient(name string, mac string, kind string, configPath string, controllers []string, techsupportURL string, diagnosticsURL string) *TSMClient {
	log.Infof("Name : %v MAC : %v Controllers : %v", name, mac, controllers)
	var resolverClient resolver.Interface
	if len(controllers) != 0 {
		resolverClient = resolver.New(&resolver.Config{Name: globals.Tsm, Servers: controllers})
	} else {
		resolverClient = nil
	}

	res, err := action.ReadConfig(configPath)
	if err != nil {
		log.Errorf("Failed to read config file :%v. Err: %v", configPath, err)
		return nil
	}

	ctxt, cancel := context.WithCancel(context.Background())
	agent := &TSMClient{
		ctx:            ctxt,
		cancelFn:       cancel,
		name:           name,
		mac:            mac,
		kind:           kind,
		resolverClient: resolverClient,
		stopped:        false,
		tsCh:           make(chan tsproto.TechSupportRequest, maxChanLength),
		moduleCh:       make(chan diagnostics.Module, maxChanLength),
		cfg:            res,
	}

	if techsupportURL != "" {
		log.Infof("Setting techsupportURL number to : %v", techsupportURL)
		res.RESTUri = techsupportURL
	}

	// RESTUri is expected to be non-null for techsupport agent on Naples
	if res.RESTUri != "" {
		log.Info("REST URI is non null")
		agent.restServer = rest.NewRestServer(res.RESTUri, diagnosticsURL, agent.tsCh, agent.cfg)
	}

	return agent
}

func (ag *TSMClient) isStopped() bool {
	ag.Lock()
	defer ag.Unlock()
	return ag.stopped
}

// Start the operation of the techsupport agent
func (ag *TSMClient) Start() {
	log.Infof("Starting Tech Support TSMClient")
	if ag.restServer != nil {
		go ag.restServer.Start()
	}

	ag.waitGrp.Add(1)
	go ag.StartWorking()

	if ag.resolverClient != nil {
		for {
			tsGrpcClient, err := rpckit.NewRPCClient(ag.name, globals.Tsm, rpckit.WithBalancer(balancer.New(ag.resolverClient)))
			if err != nil {
				log.Errorf("Error creating TSM client %s: %v", ag.name, err)

				if ag.isStopped() {
					return
				}
			} else {
				ag.tsGrpcClient = tsGrpcClient
				break
			}

			time.Sleep(time.Second)
		}

		ag.waitGrp.Add(1)
		go ag.runTechSupportWatcher()

		ag.waitGrp.Add(1)
		go ag.RunModuleWatcher()

		ag.waitGrp.Add(1)
		go ag.StartModuleWorker()
	}
}

func (ag *TSMClient) isWatching() bool {
	return atomic.LoadInt32(&ag.watching) != 0
}

// Stop stops the TSMClient
func (ag *TSMClient) Stop() {
	ag.Lock()
	ag.stopped = true
	ag.Unlock()
	ag.cancelFn()
	ag.waitGrp.Wait()

	if ag.tsGrpcClient != nil {
		ag.tsGrpcClient.Close()
		ag.tsGrpcClient = nil
	}
	ag.tsAPIClient = nil

	if ag.restServer != nil {
		ag.restServer.Stop()
	}
	ag.restServer = nil
}
