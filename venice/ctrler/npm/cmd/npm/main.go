// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"strings"

	_ "net/http/pprof"

	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// main function of network controller
func main() {

	var (
		debugflag    = flag.Bool("debug", false, "Enable debug mode")
		logToFile    = flag.String("logtofile", "/var/log/pensando/npm.log", "Redirect logs to file")
		listenURL    = flag.String("listen-url", ":"+globals.NpmRPCPort, "gRPC listener URL")
		resolverURLs = flag.String("resolver-urls", ":"+globals.CMDGRPCPort, "comma separated list of resolver URLs <IP:Port>")
		restURL      = flag.String("rest-url", ":"+globals.NpmRESTPort, "rest listener URL")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "NPM",
		Format:      log.JSONFmt,
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

	r := resolver.New(&resolver.Config{Name: "npm", Servers: strings.Split(*resolverURLs, ",")})
	// create the controller
	ctrler, err := npm.NewNetctrler(*listenURL, *restURL, globals.APIServer, globals.VCHub, r)
	if err != nil || ctrler == nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	// wait forever
	<-waitCh
}
