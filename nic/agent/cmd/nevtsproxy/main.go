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
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

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

	// create events proxy
	var result interface{}
	var err error
	for {
		result, err = utils.ExecuteWithRetry(func() (interface{}, error) {
			return evtsproxy.NewEventsProxy(globals.EvtsProxy, *listenURL, resolverClient, *dedupInterval, *batchInterval,
				*evtsStoreDir, logger)
		}, 2*time.Second, 30)
		if err != nil {
			logger.Fatalf("error creating events proxy instance: %v", err)
		}
		break
	}

	eps := result.(*evtsproxy.EventsProxy)
	defer eps.Stop()

	// handle different agent modes
	handleModes(*mode, eps, logger)

	// start dispatching events to its registered writers
	eps.StartDispatch()

	logger.Infof("%s is running {%+v}", globals.EvtsProxy, *eps)

	// create shared memory events reader
	shmEvtsReader := reader.NewReader(shm.GetSharedMemoryDirectory(), 50*time.Millisecond, eps.GetEventsDispatcher())
	defer shmEvtsReader.Stop()
	go func() {
		for {
			err := shmEvtsReader.Start()
			if err == nil {
				logger.Infof("shared memory events reader is running on dir: %s", shm.GetSharedMemoryDirectory())
				return
			}

			logger.Debugf("failed to start file watcher, err: %v, retrying...", err)
			time.Sleep(1 * time.Second)
		}
	}()

	// channel to receive signal
	gracefulStop := make(chan os.Signal, 1)
	signal.Notify(gracefulStop, syscall.SIGTERM, syscall.SIGINT)

	select {
	case <-gracefulStop:
		// stop shm reader
		shmEvtsReader.Stop()

		// stop evtsproxy RPC server
		logger.Debug("got signal, exiting")
		if err := eps.RPCServer.Stop(); err != nil {
			logger.Errorf("failed to stop RPC server, err %v", err)
		}
	case <-eps.RPCServer.Done():
		shmEvtsReader.Stop()
		logger.Debug("server stopped serving, exiting")
	}
}

// helper function to support different modes of operation
func handleModes(mode string, eps *evtsproxy.EventsProxy, logger log.Logger) {
	switch mode {
	case "network": // managed; start venice exporter
		logger.Infof("initializing network mode")
		if err := eps.RegisterEventsWriter(evtsproxy.Venice, nil); err != nil {
			log.Fatalf("failed to register venice writer with events proxy, err: %v", err)
		}
	case "host": // non-managed; start REST service
		logger.Infof("initializing host mode")
		// TODO: implement REST

	default:
		logger.Fatalf("unsupported mode: %s", mode)
	}
}
