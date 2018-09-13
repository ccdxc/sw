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

	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// main (command source) for events proxy
func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("log-to-file", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.EvtsProxy)), "Path of the log file")
		logToStdoutFlag = flag.Bool("log-to-stdout", false, "Enable logging to stdout")
		listenURL       = flag.String("listen-url", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), "RPC listen URL")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "Comma separated list of resolver URLs of the form 'ip:port'")
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

	// create resolver client
	resolverClient := resolver.New(&resolver.Config{
		Name:    globals.EvtsProxy,
		Servers: strings.Split(*resolverURLs, ",")})

	// create events proxy
	// FIXME: start venice writer only in managed mode
	eps, err := evtsproxy.NewEventsProxy(globals.EvtsProxy, *listenURL, "", resolverClient,
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
