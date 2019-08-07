// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"strings"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/ctrler/rollout"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// main function of the Cluster Key Manager
func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("logtofile", "/var/log/pensando/rollout.log", "Redirect logs to file")
		listenURL       = flag.String("listen-url", ":"+globals.RolloutRPCPort, "gRPC listener URL")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Rollout,
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
	logger := log.SetConfig(logConfig)
	defer logger.Close()
	log.SetTraceDebug()

	// create a dummy channel to wait forever
	waitCh := make(chan bool)
	r := resolver.New(&resolver.Config{Name: globals.Rollout, Servers: strings.Split(*resolverURLs, ",")})

	// create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component: globals.Rollout}, logger)
	if err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

	// start module watcher
	moduleChangeCb := func(diagmod *diagapi.Module) {
		logger.ResetFilter(diagnostics.GetLogFilter(diagmod.Spec.LogLevel))
		logger.InfoLog("method", "moduleChangeCb", "msg", "setting log level", "moduleLogLevel", diagmod.Spec.LogLevel)
	}
	watcherOption := rollout.WithModuleWatcher(module.GetWatcher(fmt.Sprintf("%s-%s", k8s.GetNodeName(), globals.Rollout), globals.APIServer, r, logger, moduleChangeCb))

	// add diagnostics service
	diagOption := rollout.WithDiagnosticsService(diagsvc.GetDiagnosticsServiceWithDefaults(globals.Rollout, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, r, logger))

	// create the controller
	_, err = rollout.NewCtrler(*listenURL, globals.APIServer, r, evtsRecorder, watcherOption, diagOption)
	if err != nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	log.Infof("rollout controller is running")

	// wait forever
	<-waitCh
}
