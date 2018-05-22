// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// main (command source) for events proxy
func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", "/var/log/pensando/evtsproxy.log", "Path of the log file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		listenURL       = flag.String("listen-url", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), "RPC listen URL")
		evtsMgrURL      = flag.String("evts-mgr-url", fmt.Sprintf(":%s", globals.EvtsMgrRPCPort), "RPC listen URL of events manager")
		dedupInterval   = flag.Duration("dedup-interval", 100*time.Second, "Events deduplication interval")
		batchInterval   = flag.Duration("batch-interval", 10*time.Second, "Events batching inteval")
		evtsStoreDir    = flag.String("evts-store-dir", globals.EventsDir, "Local events store directory")
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
	eps, err := evtsproxy.NewEventsProxy(globals.EvtsProxy, *listenURL, *evtsMgrURL, *dedupInterval, *batchInterval, *evtsStoreDir, logger)
	if err != nil {
		logger.Fatalf("error creating events proxy instance: %v", err)
	}

	logger.Debug("events proxy server started")

	// channel to receive signal
	gracefulStop := make(chan os.Signal, 1)
	signal.Notify(gracefulStop, syscall.SIGTERM, syscall.SIGINT)

	// done channel signals the signal handler that the application has completed
	var done = make(chan struct{})

	go func() {
		for {
			select {
			case <-done:
				return
			case <-gracefulStop:
				if err := eps.RPCServer.Stop(); err != nil {
					logger.Errorf("failed to stop RPC server, err %v", err)
				}
			}
		}
	}()

	// wait till the server stops
	<-eps.RPCServer.Done()
	done <- struct{}{}

	logger.Debug("server stopped serving, exiting")
}
