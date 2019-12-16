// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"expvar"
	"flag"
	"fmt"
	"net/http"
	"net/http/pprof"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/gorilla/mux"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/nic/agent/httputils"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	emgr   = &evtsmgr.EventsManager{}
	err    error
	logger log.Logger
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
			fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.EvtsMgr)),
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

	logger = log.SetConfig(config)
	defer logger.Close()

	// create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component: globals.EvtsMgr}, logger)
	if err != nil {
		logger.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

	// create resolver client
	resolverClient := resolver.New(&resolver.Config{
		Name:    globals.EvtsMgr,
		Servers: strings.Split(*resolverURLs, ",")})

	// start module watcher
	moduleChangeCb := func(diagmod *diagapi.Module) {
		logger.ResetFilter(diagnostics.GetLogFilter(diagmod.Spec.LogLevel))
		logger.InfoLog("method", "moduleChangeCb", "msg", "setting log level", "moduleLogLevel", diagmod.Spec.LogLevel)
	}
	watcherOption := evtsmgr.WithModuleWatcher(module.GetWatcher(fmt.Sprintf("%s-%s", k8s.GetNodeName(), globals.EvtsMgr), globals.APIServer, resolverClient, logger, moduleChangeCb))

	// add diagnostics service
	diagOption := evtsmgr.WithDiagnosticsService(diagsvc.GetDiagnosticsServiceWithDefaults(globals.EvtsMgr, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, resolverClient, logger))

	// create the controller
	emgr, err = evtsmgr.NewEventsManager(globals.EvtsMgr, *listenURL,
		resolverClient, logger, watcherOption, diagOption)
	if err != nil {
		log.Fatalf("error creating events manager instance: %v", err)
	}

	// run a REST server for pprof
	router := mux.NewRouter()
	router.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/", pprof.Index)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/cmdline", pprof.Cmdline)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/profile", pprof.Profile)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/symbol", pprof.Symbol)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/trace", pprof.Trace)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/allocs", pprof.Handler("allocs").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/block", pprof.Handler("block").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/heap", pprof.Handler("heap").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/mutex", pprof.Handler("mutex").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/goroutine", pprof.Handler("goroutine").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/threadcreate", pprof.Handler("threadcreate").ServeHTTP)

	// garbage collect event-based alerts
	router.Methods("GET").Subrouter().HandleFunc("/gcalerts", httputils.MakeHTTPHandler(GCAlerts))

	go http.ListenAndServe(fmt.Sprintf("127.0.0.1:%s", globals.EvtsMgrRESTPort), router)

	logger.Infof("%s is running {%+v}", globals.EvtsMgr, emgr)
	recorder.Event(eventtypes.SERVICE_RUNNING,
		fmt.Sprintf("Service %s running on %s", globals.EvtsMgr, utils.GetHostname()), nil)

	// wait till the server stops
	<-emgr.RPCServer.Done()
	logger.Debug("server stopped serving, exiting")
	os.Exit(0)
}

// GCAlerts garbage collect alerts
func GCAlerts(r *http.Request) (interface{}, error) {
	if emgr != nil {
		emgr.GCAlerts(100 * time.Millisecond)
		return struct{}{}, nil
	}

	logger.Errorf("cannot GC alerts")
	return nil, nil
}
