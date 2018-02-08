// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"

	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
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

		//FIXME: read elastic address using resolver
		elasticURL = flag.String(
			"elastic-url",
			"",
			"Elasticsearch server address",
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
		LogToStdout: true,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	logger := log.SetConfig(config)

	// create the controller
	emgr, err := evtsmgr.NewEventsManager(globals.EvtsMgr,
		*listenURL, *elasticURL, logger)
	if err != nil {
		log.Fatalf("error creating events manager instance: %v", err)
	}

	log.Debug("events manager server started")

	// wait till the server stops
	<-emgr.RPCServer.Done()
	log.Debug("server stopped serving, exiting")
	os.Exit(0)
}
