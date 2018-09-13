// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"github.com/gorilla/mux"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var dbgSock = "/var/run/pensando/tpm.sock"
var pkgName = globals.Tpm

// main function of the Telemetry Policy Manager
func main() {

	var (
		nsURLs = flag.String("resolver-urls", ":"+globals.CMDResolverPort,
			"comma separated list of resolver URLs of the form 'ip:port'")
		listenURL       = flag.String("listen-url", ":"+globals.TpmRPCPort, "gRPC listener URL")
		logFile         = flag.String("logfile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Tpm)), "redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		debugFlag       = flag.Bool("debug", false, "enable debug mode")
	)

	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      pkgName,
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		Debug:       *debugFlag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	// create events recorder
	if _, err := recorder.NewRecorder(&recorder.Config{
		Source:   &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: globals.Tpm},
		EvtTypes: evtsapi.GetEventTypes()}); err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}

	log.Infof("starting telemetry controller with args : {%+v}", os.Args)
	nsClient := resolver.New(&resolver.Config{Name: pkgName, Servers: strings.Split(*nsURLs, ",")})

	// init policy manager
	pm, err := tpm.NewPolicyManager(*listenURL, nsClient)
	if err != nil {
		// let the scheduler restart tpm
		log.Fatalf("failed to init policy agent, %s", err)
	}

	// debug
	router := mux.NewRouter()
	router.HandleFunc("/debug", pm.Debug).Methods("GET")
	// sudo curl --unix-socket /var/run/pensando/tpm.sock http://localhost/debug
	os.Remove(dbgSock)
	l, err := net.Listen("unix", dbgSock)
	if err != nil {
		log.Fatalf("failed to initialize debug, %s", err)
	}
	defer l.Close()
	go func() {
		log.Fatal(http.Serve(l, router))
	}()

	log.Infof("%s is running {%+v}", globals.Tpm, pm)
	recorder.Event(evtsapi.ServiceRunning, evtsapi.SeverityLevel_INFO, fmt.Sprintf("Service %s running on %s", globals.Tpm, utils.GetHostname()), nil)

	select {}
}
