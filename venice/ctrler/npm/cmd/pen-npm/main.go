// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"strings"

	_ "net/http/pprof"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// main function of network controller
func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Npm)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		listenURL       = flag.String("listen-url", ":"+globals.NpmRPCPort, "gRPC listener URL")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
		restURL         = flag.String("rest-url", ":"+globals.NpmRESTPort, "rest listener URL")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Npm,
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

	// create events recorder
	if _, err := recorder.NewRecorder(&recorder.Config{
		Source:   &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: globals.Npm},
		EvtTypes: evtsapi.GetEventTypes()}); err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}

	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	r := resolver.New(&resolver.Config{Name: "npm", Servers: strings.Split(*resolverURLs, ",")})
	// create the controller
	ctrler, err := npm.NewNetctrler(*listenURL, *restURL, globals.APIServer, globals.VCHub, r)
	if err != nil || ctrler == nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	log.Infof("%s is running {%+v}", globals.Npm, ctrler)
	recorder.Event(evtsapi.ServiceRunning, evtsapi.SeverityLevel_INFO, fmt.Sprintf("Service %s running on %s", globals.Npm, utils.GetHostname()), nil)

	// wait forever
	<-waitCh
}
