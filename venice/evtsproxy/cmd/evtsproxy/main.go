// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"syscall"
	"time"

	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/events/policy"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// main (command source) for events proxy
func main() {

	var (
		listenURL       = flag.String("listen-url", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), "RPC listen URL")
		evtsStoreDir    = flag.String("evts-store-dir", globals.EventsDir, "Local events store directory")
		dedupInterval   = flag.Duration("dedup-interval", 24*(60*time.Minute), "Events de-duplication interval") // default 24hrs
		batchInterval   = flag.Duration("batch-interval", 10*time.Second, "Events batching interval")            // default 10s
		evtsMgrURL      = flag.String("evts-mgr-url", fmt.Sprintf(":%s", globals.EvtsMgrRPCPort), "RPC listen URL of events manager")
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

	// create events proxy
	eps, pMgr, pWatcher := createEvtsProxy(*listenURL, nil, *dedupInterval, *batchInterval,
		*evtsStoreDir, *evtsMgrURL, logger)
	logger.Infof("%s is running {%+v}", globals.EvtsProxy, eps)

	// channel to receive signal
	gracefulStop := make(chan os.Signal, 1)
	signal.Notify(gracefulStop, syscall.SIGTERM, syscall.SIGINT)

	select {
	case <-gracefulStop:
		logger.Debug("got signal, exiting")
		stopEvtsProxy(eps, pMgr, pWatcher, logger)
	case <-eps.RPCServer.Done():
		logger.Debug("server stopped serving, exiting")
		stopEvtsProxy(eps, pMgr, pWatcher, logger)
	}
}

// helper function to stop evtsproxy services (policy manager, watcher and rpc server)
func stopEvtsProxy(eps *evtsproxy.EventsProxy, pMgr *policy.Manager, pWatcher *policy.Watcher, logger log.Logger) {
	// stop policy watcher
	pWatcher.Stop()

	// stop policy manager
	pMgr.Stop()

	// stop evts proxy
	if err := eps.RPCServer.Stop(); err != nil {
		logger.Errorf("failed to stop RPC server, err %v", err)
	}
}

// helper function to start evtsproxy services (policy manager, watcher and rpc server)
func createEvtsProxy(listenURL string, resolverClient resolver.Interface, dedupInterval time.Duration,
	batchInterval time.Duration, evtsStoreDir string, evtsMgrURL string, logger log.Logger) (
	*evtsproxy.EventsProxy, *policy.Manager, *policy.Watcher) {

	// create events proxy
	eps, err := evtsproxy.NewEventsProxy(globals.EvtsProxy, listenURL, resolverClient, dedupInterval,
		batchInterval, evtsStoreDir, logger)
	if err != nil {
		logger.Fatalf("error creating events proxy instance: %v", err)
	}
	if _, err := eps.RegisterEventsExporter(evtsproxy.Venice,
		&exporters.VeniceExporterConfig{EvtsMgrURL: evtsMgrURL}); err != nil {
		log.Fatalf("failed to register venice events exporter with events proxy, err: %v", err)
	}

	// start events policy manager
	policyMgr, err := policy.NewManager(eps, logger)
	if err != nil {
		log.Fatalf("failed to create event policy manager, err: %v", err)
	}

	// start events policy watcher
	policyWatcher, err := policy.NewWatcher(policyMgr, logger, policy.WithEventsMgrURL(evtsMgrURL))
	if err != nil {
		log.Fatalf("failed to create events policy watcher, err: %v", err)
	}

	eps.StartDispatch()

	return eps, policyMgr, policyWatcher
}
