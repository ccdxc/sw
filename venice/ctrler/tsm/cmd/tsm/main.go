// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"strings"

	_ "net/http/pprof"

	"github.com/pensando/sw/venice/ctrler/tsm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// main function of trouble shooting controller
func main() {
	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", "/var/log/pensando/tsm.log", "Redirect logs to file")
		listenURL       = flag.String("listen-url", ":"+globals.TsmRPCPort, "gRPC listener URL")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
		restURL         = flag.String("rest-url", ":"+globals.TsmRESTPort, "rest listener URL")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Tsm,
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

	// Initialize logger config
	log.SetConfig(logConfig)

	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	r := resolver.New(&resolver.Config{Name: globals.Tsm, Servers: strings.Split(*resolverURLs, ",")})
	// create the controller
	ctrler, err := tsm.NewTsCtrler(*listenURL, *restURL, globals.APIServer, r)
	if err != nil || ctrler == nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	log.Infof("%s is running {%+v}", globals.Tsm, ctrler)

	// wait forever
	<-waitCh
}
