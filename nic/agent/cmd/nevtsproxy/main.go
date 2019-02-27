// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"strings"
	"syscall"
	"time"

	"github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/nevtsproxy/reader"
	"github.com/pensando/sw/nic/agent/nevtsproxy/shm"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/policy"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// represents the list of event services (includes both network and host modes)
type evtServices struct {
	logger        log.Logger             // logger
	eps           *evtsproxy.EventsProxy // events proxy server
	policyWatcher *policy.Watcher        // policy watcher responsible for watching event policies from evtsmgr; will be nil in host mode
	policyMgr     *policy.Manager        // responsible for creating/deleting syslog exporters for the incoming event policies
	shmReader     *reader.Reader         // shared memory reader
	restServer    *restapi.RestServer    // REST server serving event policy APIs in host mode
	agentStore    emstore.Emstore        // agent store
}

// main (command source) for events proxy
func main() {

	var (
		agentDbPath     = flag.String("agent-db", "/tmp/naples-nevtsproxy.db", "Agent database file")
		grpcListenURL   = flag.String("grpc-listen-url", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), "gRPC listen URL")
		restURL         = flag.String("rest-url", ":"+globals.EvtsProxyRESTPort, "specify agent REST URL")
		mode            = flag.String("mode", "host", "Specify the agent mode either host or network")
		evtsStoreDir    = flag.String("evts-store-dir", globals.EventsDir, "Local events store directory")
		dedupInterval   = flag.Duration("dedup-interval", 24*(60*time.Minute), "Events de-duplication interval") // default 24hrs
		batchInterval   = flag.Duration("batch-interval", 10*time.Second, "Events batching interval")            // default 10s
		resolverURLs    = flag.String("resolver-urls", "", "Comma separated list of resolver URLs of the form 'ip:port'")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("log-to-file", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.EvtsProxy)), "Path of the log file")
		logToStdoutFlag = flag.Bool("log-to-stdout", false, "Enable logging to stdout")
	)

	flag.Parse()

	// Fill logger config params
	config := &log.Config{
		Module:      globals.EvtsProxy,
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	logger := log.SetConfig(config)

	// start event services according to the mode
	es := &evtServices{logger: logger}
	es.start(*mode, *grpcListenURL, *restURL, *agentDbPath, *evtsStoreDir, *resolverURLs, *dedupInterval, *batchInterval, logger)
	defer es.stop()

	logger.Infof("%s is running {%+v}", globals.EvtsProxy, es.eps)

	// channel to receive signal
	gracefulStop := make(chan os.Signal, 1)
	signal.Notify(gracefulStop, syscall.SIGTERM, syscall.SIGINT)

	select {
	case <-gracefulStop:
		logger.Debug("got signal, exiting")
		es.stop()
	case <-es.eps.RPCServer.Done():
		logger.Debug("server stopped serving, exiting")
		es.stop()
	}
}

// helper function to start services based on the given mode
func (e *evtServices) start(mode, grpcListenURL, restURL, agentDbPath, evtsStoreDir, resolverURLs string, dedupInterval,
	batchInterval time.Duration, logger log.Logger) {
	var agentStore emstore.Emstore
	var err error

	// create agent data store
	if utils.IsEmpty(agentDbPath) {
		agentStore, err = emstore.NewEmstore(emstore.MemStoreType, "")
	} else {
		agentStore, err = emstore.NewEmstore(emstore.BoltDBType, agentDbPath)
	}
	if err != nil {
		log.Fatalf("error opening the embedded db, err: %v", err)
	}

	switch mode {
	// managed; start venice services and exporter
	case "network":
		e.eps, e.policyMgr, e.policyWatcher = startNetworkModeServices(grpcListenURL, evtsStoreDir, resolverURLs,
			dedupInterval, batchInterval, agentStore, logger)

	// non-managed; start REST service
	case "host":
		e.eps, e.restServer = startHostModeServices(restURL, grpcListenURL, evtsStoreDir,
			dedupInterval, batchInterval, agentStore, logger)

	default:
		logger.Fatalf("unsupported mode: %s", mode)
	}

	// create shared memory events reader
	e.shmReader = reader.NewReader(shm.GetSharedMemoryDirectory(), 50*time.Millisecond, e.eps.GetEventsDispatcher(), logger)
	go func() {
		for {
			err := e.shmReader.Start()
			if err == nil {
				logger.Infof("shared memory events reader is running on dir: %s", shm.GetSharedMemoryDirectory())
				return
			}

			logger.Debugf("failed to start file watcher, err: %v, retrying...", err)
			time.Sleep(1 * time.Second)
		}
	}()
}

// stop stops all the running services
func (e *evtServices) stop() {
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
		if err := e.eps.RPCServer.Stop(); err != nil {
			e.logger.Errorf("failed to stop RPC server, err %v", err)
		}
		e.eps = nil
	}

	if e.agentStore != nil {
		e.agentStore.Close()
		e.agentStore = nil
	}
}

// helper function to start network mode services
func startNetworkModeServices(listenURL, evtsStoreDir, resolverURLs string, dedupInterval,
	batchInterval time.Duration, agentStore emstore.Emstore, logger log.Logger) (*evtsproxy.EventsProxy, *policy.Manager, *policy.Watcher) {
	logger.Infof("initializing network mode")

	// create resolver client
	var resolverClient resolver.Interface
	if !utils.IsEmpty(resolverURLs) {
		resolverClient = resolver.New(&resolver.Config{
			Name:    globals.EvtsProxy,
			Servers: strings.Split(resolverURLs, ",")})
	}

	// create events proxy
	eps, err := evtsproxy.NewEventsProxy(globals.EvtsProxy, listenURL, resolverClient, dedupInterval,
		batchInterval, &events.StoreConfig{Dir: evtsStoreDir}, logger)
	if err != nil {
		logger.Fatalf("error creating events proxy instance: %v", err)
	}

	// register venice exporter (exports events to evtsmgr -> elastic)
	if _, err := eps.RegisterEventsExporter(evtsproxy.Venice, nil); err != nil {
		logger.Fatalf("failed to register venice events exporter with events proxy, err: %v", err)
	}

	// start events policy manager
	policyMgr, err := policy.NewManager(eps, logger, policy.WithStore(agentStore))
	if err != nil {
		log.Fatalf("failed to create event policy manager, err: %v", err)
	}

	// start events policy watcher
	policyWatcher, err := policy.NewWatcher(policyMgr, logger, policy.WithResolverClient(resolverClient))
	if err != nil {
		log.Fatalf("failed to create events policy watcher, err: %v", err)
	}

	// start dispatching events to its registered exporters
	eps.StartDispatch()

	return eps, policyMgr, policyWatcher
}

// helper function to start host mode services
func startHostModeServices(restURL, grpcListenURL, evtsStoreDir string, dedupInterval,
	batchInterval time.Duration, agentStore emstore.Emstore, logger log.Logger) (*evtsproxy.EventsProxy, *restapi.RestServer) {
	logger.Infof("initializing host mode")

	// create events proxy
	eps, err := evtsproxy.NewEventsProxy(globals.EvtsProxy, grpcListenURL, nil, dedupInterval,
		batchInterval, &events.StoreConfig{Dir: evtsStoreDir}, logger)
	if err != nil {
		logger.Fatalf("error creating events proxy instance: %v", err)
	}

	// start events policy manager
	policyMgr, err := policy.NewManager(eps, logger, policy.WithStore(agentStore))
	if err != nil {
		log.Fatalf("failed to create event policy manager, err: %v", err)
	}

	restServer, err := restapi.NewRestServer(restURL, policyMgr, logger)
	if err != nil {
		logger.Fatalf("error starting REST server, err: %v", err)
	}

	return eps, restServer
}
