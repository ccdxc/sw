// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"

	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// main function of network controller
func main() {

	var (
		debugflag    = flag.Bool("debug", false, "Enable debug mode")
		logToFile    = flag.String("logtofile", "/var/log/pensando/npm.log", "Redirect logs to file")
		listenURL    = flag.String("listen-url", ":"+globals.NpmRPCPort, "gRPC listener URL")
		resolverURLs = flag.String("resolver-urls", ":"+globals.CMDGRPCPort, "comma separated list of resolver URLs <IP:Port>")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "NPM",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10, // TODO: These needs to be part of Service Config Object
			MaxBackups: 3,  // TODO: These needs to be part of Service Config Object
			MaxAge:     7,  // TODO: These needs to be part of Service Config Object
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	// create the controller
	ctrler, err := npm.NewNetctrler(*listenURL, globals.APIServer, globals.VCHub, *resolverURLs)
	if err != nil || ctrler == nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	// wait forever
	<-waitCh
}
