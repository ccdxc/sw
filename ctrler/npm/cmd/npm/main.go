// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"

	"github.com/pensando/sw/ctrler/npm"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/log"
)

// main function of network controller
func main() {

	var (
		debugflag = flag.Bool("debug", false, "Enable debug mode")
		logToFile = flag.String("logtofile", "/var/log/pensando/npm.log", "Redirect logs to file")
		listenURL = flag.String("listen", ":"+globals.NpmRPCPort, "Listen URL (eg. :9004)")
		apisrvURL = flag.String("apisrv", "localhost:"+globals.APIServerPort, "API server URL (eg. localhost:8082)")
		vmmURL    = flag.String("vmm", "localhost:"+globals.VCHubAPIPort, "Vchub URL (eg. localhost:9003)")
	)

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
	ctrler, err := npm.NewNetctrler(*listenURL, *apisrvURL, *vmmURL)
	if err != nil || ctrler == nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	// wait forever
	<-waitCh
}
