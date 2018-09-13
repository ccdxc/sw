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
	"github.com/pensando/sw/venice/utils/log"
)

// main (command source) for events proxy
func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.EvtsProxy)), "Path of the log file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "Enable logging to stdout")
		listenURL       = flag.String("listen-url", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), "RPC listen URL")
		evtsMgrURL      = flag.String("evts-mgr-url", fmt.Sprintf(":%s", globals.EvtsMgrRPCPort), "RPC listen URL of events manager")
		dedupInterval   = flag.Duration("dedup-interval", 24*(60*time.Minute), "Events deduplication interval") // default dedup 24hrs
		batchInterval   = flag.Duration("batch-interval", 10*time.Second, "Events batching inteval")            // default batch 10s
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
	eps, err := evtsproxy.NewEventsProxy(globals.EvtsProxy, *listenURL, *evtsMgrURL, nil,
		*dedupInterval, *batchInterval, *evtsStoreDir, []evtsproxy.WriterType{evtsproxy.Venice}, logger)
	if err != nil {
		logger.Fatalf("error creating events proxy instance: %v", err)
	}

	logger.Infof("%s is running {%+v}", globals.EvtsProxy, *eps)

	// channel to receive signal
	gracefulStop := make(chan os.Signal, 1)
	signal.Notify(gracefulStop, syscall.SIGTERM, syscall.SIGINT)

	select {
	case <-gracefulStop:
		logger.Debug("got signal, exiting")
		if err := eps.RPCServer.Stop(); err != nil {
			logger.Errorf("failed to stop RPC server, err %v", err)
		}
	case <-eps.RPCServer.Done():
		logger.Debug("server stopped serving, exiting")
	}
}
