// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"

	"github.com/pensando/sw/ctrler/npm"
	"github.com/pensando/sw/ctrler/npm/rpcserver"
	"github.com/pensando/sw/utils/log"
)

// main function of network controller
func main() {

	var (
		debugflag = flag.Bool("debug", false, "Enable debug mode")
		logToFile = flag.String("logtofile", "/var/log/pensando/npm.log", "Redirect logs to file")
	)

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "NPM",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       *debugflag,
		Context:     true,
		LogToStdout: true,
		LogToFile:   false,
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
	ctrler, err := npm.NewNetctrler(rpcserver.NetctrlerURL, "localhost:8082", "")
	if err != nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	// FIXME: dummy code to emulate network create event from API server
	ctrler.Watchr.CreateNetwork("default", "default", "10.1.1.0/24", "10.1.1.254")

	// wait forever
	<-waitCh
}
