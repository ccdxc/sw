// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"strings"

	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// main (command source) for events manager
func main() {

	var (
		debugflag = flag.Bool(
			"debug",
			false,
			"Enable debug mode",
		)

		logToFile = flag.String(
			"logtofile",
			"/var/log/pensando/evtsmgr.log",
			"Path of the log file",
		)

		logToStdoutFlag = flag.Bool(
			"logtostdout",
			false,
			"enable logging to stdout",
		)

		resolverURLs = flag.String(
			"resolver-urls",
			":"+globals.CMDResolverPort,
			"comma separated list of resolver URLs of the form 'ip:port'",
		)

		listenURL = flag.String(
			"listen-url",
			fmt.Sprintf(":%s", globals.EvtsMgrRPCPort),
			"RPC listen URL",
		)
	)

	flag.Parse()

	// Fill logger config params
	config := &log.Config{
		Module:      globals.EvtsMgr,
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
		Name:    globals.EvtsMgr,
		Servers: strings.Split(*resolverURLs, ",")})

	// create the controller
	emgr, err := evtsmgr.NewEventsManager(globals.EvtsMgr, *listenURL,
		resolverClient, logger)
	if err != nil {
		log.Fatalf("error creating events manager instance: %v", err)
	}

	logger.Infof("%s is running {%+v}", globals.EvtsMgr, *emgr)

	// wait till the server stops
	<-emgr.RPCServer.Done()
	logger.Debug("server stopped serving, exiting")
	os.Exit(0)
}
