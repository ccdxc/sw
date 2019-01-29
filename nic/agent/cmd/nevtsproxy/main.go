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

	"github.com/pensando/sw/nic/agent/nevtsproxy/reader"
	"github.com/pensando/sw/nic/agent/nevtsproxy/shm"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
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
}

// main (command source) for events proxy
func main() {

	var (
		listenURL       = flag.String("listen-url", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), "RPC listen URL")
		mode            = flag.String("mode", "host", "Specify the agent mode either host or network")
		evtsStoreDir    = flag.String("evts-store-dir", globals.EventsDir, "Local events store directory")
		dedupInterval   = flag.Duration("dedup-interval", 24*(60*time.Minute), "Events de-duplication interval") // default 24hrs
		batchInterval   = flag.Duration("batch-interval", 10*time.Second, "Events batching interval")            // default 10s
		resolverURLs    = flag.String("resolver-urls", "", "Comma separated list of resolver URLs of the form 'ip:port'")
		debugflag       = flag.Bool("debug", false, "enable debug mode")
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

	// create resolver client
	var resolverClient resolver.Interface
	if !utils.IsEmpty(*resolverURLs) {
		resolverClient = resolver.New(&resolver.Config{
			Name:    globals.EvtsProxy,
			Servers: strings.Split(*resolverURLs, ",")})
	}

	// start event services according to the mode
	es := &evtServices{logger: logger}
	es.start(*mode, *listenURL, *evtsStoreDir, resolverClient, *dedupInterval, *batchInterval, logger)
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
func (e *evtServices) start(mode, listenURL, evtsStoreDir string, resolverClient resolver.Interface, dedupInterval,
	batchInterval time.Duration, logger log.Logger) {
	switch mode {

	// managed; start venice services and exporter
	case "network":
		e.eps, e.policyMgr, e.policyWatcher = startNetworkModeServices(listenURL, evtsStoreDir, resolverClient,
			dedupInterval, batchInterval, logger)

	// non-managed; start REST service
	case "host":
		e.eps = startHostModeServices(listenURL, evtsStoreDir, resolverClient,
			dedupInterval, batchInterval, logger)

	default:
		logger.Fatalf("unsupported mode: %s", mode)
	}

	// create shared memory events reader
	e.shmReader = reader.NewReader(shm.GetSharedMemoryDirectory(), 50*time.Millisecond, e.eps.GetEventsDispatcher())
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
}

// helper function to start network mode services
func startNetworkModeServices(listenURL, evtsStoreDir string, resolverClient resolver.Interface, dedupInterval,
	batchInterval time.Duration, logger log.Logger) (*evtsproxy.EventsProxy, *policy.Manager, *policy.Watcher) {
	logger.Infof("initializing network mode")

	// create events proxy
	var result interface{}
	var err error
	for {
		result, err = utils.ExecuteWithRetry(func() (interface{}, error) {
			return evtsproxy.NewEventsProxy(globals.EvtsProxy, listenURL, resolverClient, dedupInterval, batchInterval,
				evtsStoreDir, logger)
		}, 2*time.Second, 30)
		if err != nil {
			logger.Fatalf("error creating events proxy instance: %v", err)
		}
		break
	}

	// register venice exporter (exports events to evtsmgr -> elastic)
	eps := result.(*evtsproxy.EventsProxy)
	if _, err := eps.RegisterEventsExporter(evtsproxy.Venice, nil); err != nil {
		logger.Fatalf("failed to register venice events exporter with events proxy, err: %v", err)
	}

	// start events policy manager
	policyMgr, err := policy.NewManager(eps, logger)
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
func startHostModeServices(listenURL, evtsStoreDir string, resolverClient resolver.Interface, dedupInterval,
	batchInterval time.Duration, logger log.Logger) *evtsproxy.EventsProxy {
	logger.Infof("initializing host mode")

	// create events proxy
	var result interface{}
	var err error
	for {
		result, err = utils.ExecuteWithRetry(func() (interface{}, error) {
			return evtsproxy.NewEventsProxy(globals.EvtsProxy, listenURL, resolverClient, dedupInterval, batchInterval,
				evtsStoreDir, logger)
		}, 2*time.Second, 30)
		if err != nil {
			logger.Fatalf("error creating events proxy instance: %v", err)
		}
		break
	}

	eps := result.(*evtsproxy.EventsProxy)

	// TODO: implement REST

	return eps
}
