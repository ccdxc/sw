// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/nevtsproxy/reader"
	"github.com/pensando/sw/nic/agent/nevtsproxy/shm"
	"github.com/pensando/sw/nic/agent/nevtsproxy/upg"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	dproto "github.com/pensando/sw/nic/delphi/proto/delphi"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/events/policy"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// agent mode - network or host
const (
	networkMode string = "network"
	hostMode           = "host"
)

// configs required to start the services
type config struct {
	restURL       string
	grpcListenURL string
	evtsStoreDir  string
	agentDbPath   string
	dedupInterval time.Duration
	batchInterval time.Duration
}

// represents the list of event services (includes both network and host modes)
type evtServices struct {
	sync.RWMutex
	config                   *config
	resolverClient           resolver.Interface
	eps                      *evtsproxy.EventsProxy // events proxy server
	policyWatcher            *policy.Watcher        // policy watcher responsible for watching event policies from evtsmgr; will be nil in host mode
	policyMgr                *policy.Manager        // responsible for creating/deleting syslog exporters for the incoming event policies
	shmReader                *reader.Reader         // shared memory reader
	restServer               *restapi.RestServer    // REST server serving event policy APIs in host mode
	agentStore               emstore.Emstore        // agent store
	running                  bool                   // indicates whether the services are running or not
	wg                       sync.WaitGroup         // for the shm reader
	ctx                      context.Context        // ctx for each start/stop
	cancelCtx                context.CancelFunc     // to stop shm reader routine
	logger                   log.Logger             // logger
	veniceExporterRegistered bool                   // whether events exporter to Venice is registered
	nodeName                 string                 // NAPLES host/node name
}

// NAPLES Clients for all the south bound connections
type nClient struct {
	name         string
	sysmgrClient *sysmgr.Client       // NAPLES sys manager client
	DelphiClient clientApi.Client     // NAPLES delphi client
	upgClient    *upg.NaplesUpgClient // NAPLES upgrade client
	evtServices  *evtServices         // list of event services
	logger       log.Logger
}

func (n *nClient) OnMountComplete() {
	n.logger.Infof("OnMountComplete() done for %s", n.name)
	n.sysmgrClient.InitDone()

	// walk naples status object
	nsList := delphiProto.DistributedServiceCardStatusList(n.DelphiClient)
	for _, ns := range nsList {
		n.handleVeniceCoordinates(ns)
	}
}

func (n *nClient) Name() string {
	return n.name
}

// OnDistributedServiceCardStatusCreate event handler
func (n *nClient) OnDistributedServiceCardStatusCreate(obj *delphiProto.DistributedServiceCardStatus) {
	n.handleVeniceCoordinates(obj)
	return
}

// OnDistributedServiceCardStatusUpdate event handler
func (n *nClient) OnDistributedServiceCardStatusUpdate(old, new *delphiProto.DistributedServiceCardStatus) {
	n.handleVeniceCoordinates(new)
	return
}

// OnDistributedServiceCardStatusDelete event handler
func (n *nClient) OnDistributedServiceCardStatusDelete(obj *delphiProto.DistributedServiceCardStatus) {
	return
}

// Stop closes all the clients
func (n *nClient) Stop() {
	if n.DelphiClient != nil {
		n.DelphiClient.Close()
		n.DelphiClient = nil
	}
}

func (n *nClient) handleVeniceCoordinates(obj *delphiProto.DistributedServiceCardStatus) {
	n.logger.Infof("%s reactor called with %v", globals.EvtsProxy, obj)

	n.evtServices.Lock()
	defer n.evtServices.Unlock()

	if obj.DistributedServiceCardMode == delphiProto.DistributedServiceCardStatus_NETWORK_MANAGED_INBAND || obj.DistributedServiceCardMode == delphiProto.DistributedServiceCardStatus_NETWORK_MANAGED_OOB {
		n.evtServices.nodeName = obj.GetID()
		var controllers []string

		for _, ip := range obj.Controllers {
			controllers = append(controllers, fmt.Sprintf("%s:%s", ip, globals.CMDGRPCAuthPort))
		}
		n.evtServices.resolverClient.UpdateServers(controllers)

		if n.evtServices.running {
			n.logger.Infof("changing mode to {%s}", networkMode)
			n.evtServices.startNetworkModeServices()
			return
		}
		n.evtServices.start(networkMode, n.upgClient.IsUpgradeInProcess())
		n.upgClient.RegisterEvtsProxy(n.evtServices.eps)
	} else {
		n.evtServices.nodeName = obj.GetDSCName() // use smart nic name for reporting events in host mode
		if n.evtServices.running {
			n.logger.Infof("changing mode to {%s}", hostMode)
			n.evtServices.stopNetworkModeServices()
			n.evtServices.resolverClient.UpdateServers([]string{})
			if err := n.evtServices.policyMgr.Reset(); err != nil { // nuke the existing policies
				n.logger.Fatalf("failed to delete the existing event polices, err: %v", err)
			}
			return
		}
		n.evtServices.start(hostMode, n.upgClient.IsUpgradeInProcess())
		n.upgClient.RegisterEvtsProxy(n.evtServices.eps)
	}
}

// main (command source) for events proxy
func main() {

	var (
		agentDbPath     = flag.String("agent-db", "/tmp/naples-nevtsproxy.db", "Agent database file")
		evtsStoreDir    = flag.String("evts-store-dir", globals.EventsDir, "Local events store directory")
		dedupInterval   = flag.Duration("dedup-interval", 10*time.Second, "Events de-duplication interval") // default 24hrs
		batchInterval   = flag.Duration("batch-interval", 10*time.Second, "Events batching interval")       // default 10s
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("log-to-file", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.EvtsProxy)), "Path of the log file")
		logToStdoutFlag = flag.Bool("log-to-stdout", false, "Enable logging to stdout")
	)

	flag.Parse()

	// Fill logger config params
	lConfig := &log.Config{
		Module:      globals.EvtsProxy,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    2,
			MaxBackups: 1,
			MaxAge:     7,
		},
	}

	logger := log.SetConfig(lConfig)
	defer logger.Close()

	// start event services based to the mode
	es := &evtServices{
		config: &config{
			restURL:       fmt.Sprintf("localhost:" + globals.EvtsProxyRESTPort),
			grpcListenURL: fmt.Sprintf("localhost:%s", globals.EvtsProxyRPCPort),
			evtsStoreDir:  *evtsStoreDir,
			agentDbPath:   *agentDbPath,
			dedupInterval: *dedupInterval,
			batchInterval: *batchInterval,
		},
		resolverClient: resolver.New(&resolver.Config{
			Name: globals.EvtsProxy,
		}),
		logger: logger,
	}
	defer es.stop()

	var nClient = &nClient{
		name:        globals.EvtsProxy,
		evtServices: es,
		logger:      logger,
	}
	defer nClient.Stop()

	delphiClient, err := delphi.NewClient(nClient)
	if err != nil {
		log.Fatalf("failed to create new delphi client")
	}

	nClient.DelphiClient = delphiClient
	nClient.sysmgrClient = sysmgr.NewClient(delphiClient, nClient.Name())
	nClient.upgClient, err = upg.NewNaplesUpgClient(delphiClient, logger)
	if err != nil {
		log.Fatalf("failed to create naples upgrade client")
	}

	// Mount delphi naples status object
	delphiProto.DistributedServiceCardStatusMount(delphiClient, dproto.MountMode_ReadMode)

	// Set up watches
	delphiProto.DistributedServiceCardStatusWatch(delphiClient, nClient)

	// run delphi thread in background
	go delphiClient.Run()

	select {}
}

// startNetworkModeServices helper function to start network mode services
func (e *evtServices) startNetworkModeServices() {
	e.logger.Infof("starting network mode services")

	// starting watching policies from venice
	if e.policyWatcher == nil {
		e.wg.Add(1)
		go e.startPolicyWatcher()
	}

	// to start exporting events to venice
	if !e.veniceExporterRegistered {
		e.wg.Add(1)
		go e.registerVeniceExporter()
	}
}

// Network mode functionality; start a watcher to watch events from venice(evtsmgr)
func (e *evtServices) startPolicyWatcher() {
	var err error
	defer e.wg.Done()

	for {
		select {
		case <-e.ctx.Done():
			return
		default:
			// start events policy watcher to watch events from events manager
			if e.policyWatcher, err = policy.NewWatcher(e.policyMgr, e.logger,
				policy.WithResolverClient(e.resolverClient)); err != nil {
				e.logger.Errorf("failed to create events policy watcher, err: %v, retrying...", err)
				time.Sleep(1 * time.Second)
				continue
			}
			e.logger.Infof("running policy watcher")
			return
		}
	}
}

// Network mode functionality; register venice exporter, to export events to elastic
func (e *evtServices) registerVeniceExporter() {
	defer e.wg.Done()

	for {
		select {
		case <-e.ctx.Done():
			return
		default:
			e.RLock()
			if e.veniceExporterRegistered {
				e.RUnlock()
				return
			}
			e.RUnlock()
			// register venice exporter (exports events to evtsmgr -> elastic)
			// it will fail if it is already registered
			if _, err := e.eps.RegisterEventsExporter(exporters.Venice, nil); err != nil {
				e.logger.Errorf("failed to register venice events exporter with events proxy, err: %v, retrying...", err)
				time.Sleep(1 * time.Second)
				continue
			}
			e.logger.Infof("registered venice exporter")
			e.Lock()
			e.veniceExporterRegistered = true
			e.Unlock()
			return
		}
	}
}

// stopNetworkModeServices helper function to stop network mode services
func (e *evtServices) stopNetworkModeServices() {
	e.logger.Infof("stopping network mode services")

	if e.policyWatcher != nil {
		e.policyWatcher.Stop()
		e.policyWatcher = nil
	}

	// unregister venice exporter (exports events to evtsmgr -> elastic)
	if e.veniceExporterRegistered {
		e.eps.UnregisterEventsExporter(exporters.Venice.String())
		e.veniceExporterRegistered = false
	}
}

// helper function to start services based on the given mode
func (e *evtServices) start(mode string, maintenanceMode bool) {
	e.logger.Infof("initializing {%s} mode", mode)

	var err error
	e.ctx, e.cancelCtx = context.WithCancel(context.Background())

	// create agent data store
	if utils.IsEmpty(e.config.agentDbPath) {
		e.agentStore, err = emstore.NewEmstore(emstore.MemStoreType, "")
	} else {
		e.agentStore, err = emstore.NewEmstore(emstore.BoltDBType, e.config.agentDbPath)
	}
	if err != nil {
		e.logger.Fatalf("error opening the embedded db, err: %v", err)
	}

	// create events proxy
	if e.eps, err = evtsproxy.NewEventsProxy(e.nodeName, globals.EvtsProxy, e.config.grpcListenURL, e.resolverClient,
		e.config.dedupInterval, e.config.batchInterval, &events.StoreConfig{Dir: e.config.evtsStoreDir}, e.logger,
		evtsproxy.WithDefaultObjectRef(getDefaultObjectRef(e.nodeName)), evtsproxy.WithMaintenanceMode(maintenanceMode)); err != nil {
		e.logger.Fatalf("error creating events proxy instance: %v", err)
	}

	// start events policy manager
	if e.policyMgr, err = policy.NewManager(e.nodeName, e.eps, e.logger, policy.WithStore(e.agentStore)); err != nil {
		e.logger.Fatalf("failed to create event policy manager, err: %v", err)
	}

	// REST server should run on both modes (in network mode, it is used for querying debugs)
	if e.restServer, err = restapi.NewRestServer(e.config.restURL, e.policyMgr, e.logger); err != nil {
		e.logger.Fatalf("error starting REST server, err: %v", err)
	}

	switch mode {
	case networkMode: // start venice events policy watcher and register events exporter to elastic
		e.startNetworkModeServices()
	}

	// start dispatching events to its registered exporters
	e.eps.StartDispatch()

	// create shared memory events reader
	e.shmReader = reader.NewReader(e.nodeName, shm.GetSharedMemoryDirectory(), 50*time.Millisecond, e.eps.GetEventsDispatcher(), e.logger)
	e.wg.Add(1)
	go func() {
		defer e.wg.Done()
		for {
			select {
			case <-e.ctx.Done():
				return
			case <-time.After(1 * time.Second):
				err := e.shmReader.Start()
				if err == nil {
					e.logger.Infof("shared memory events reader is running on dir: %s", shm.GetSharedMemoryDirectory())
					return
				}

				e.logger.Debugf("failed to start file watcher, err: %v, retrying...", err)
			}
		}
	}()

	e.running = true
	e.logger.Infof("%s is running {%+v}", globals.EvtsProxy, e.eps)
}

// stop stops all the running services
func (e *evtServices) stop() {
	e.cancelCtx()
	e.wg.Wait()

	if e.restServer != nil {
		e.restServer.Stop()
		e.restServer = nil
	}

	if e.shmReader != nil {
		e.shmReader.Stop()
		e.shmReader = nil
	}

	if e.policyWatcher != nil {
		e.policyWatcher.Stop()
		e.policyWatcher = nil
	}

	if e.policyMgr != nil {
		e.policyMgr.Stop()
		e.policyMgr = nil
	}

	if e.eps != nil {
		e.eps.Stop()
		e.eps = nil
	}

	if e.agentStore != nil {
		e.agentStore.Close()
		e.agentStore = nil
	}
	e.running = false
}

// returns the default object ref for events
func getDefaultObjectRef(nodeName string) *api.ObjectRef {
	dsc := &cluster.DistributedServiceCard{}
	dsc.Defaults("all")
	return &api.ObjectRef{
		Kind:      dsc.GetKind(),
		Name:      nodeName,
		Tenant:    dsc.GetTenant(),
		Namespace: dsc.GetNamespace(),
		URI:       dsc.GetSelfLink(),
	}
}
