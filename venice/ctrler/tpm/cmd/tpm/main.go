// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/tpm"
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

var dbgSock = globals.DebugDir + "/tpm.sock"
var pkgName = globals.Tpm

// main function of the Telemetry Policy Manager
func main() {

	var (
		nsURLs = flag.String("resolver-urls", ":"+globals.CMDResolverPort,
			"comma separated list of resolver URLs of the form 'ip:port'")
		listenURL       = flag.String("listen-url", ":"+globals.TpmRPCPort, "gRPC listener URL")
		logFile         = flag.String("logfile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Tpm)), "redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		restURL         = flag.String("rest-url", globals.Localhost+":"+globals.TpmRestPort, "rest listener URL")
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
	logger := log.SetConfig(logConfig)
	defer logger.Close()

	// create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component: globals.Tpm}, logger)
	if err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

	log.Infof("starting telemetry controller with args : {%+v}", os.Args)
	nsClient := resolver.New(&resolver.Config{Name: pkgName, Servers: strings.Split(*nsURLs, ",")})

	// start module watcher
	moduleChangeCb := func(diagmod *diagapi.Module) {
		logger.ResetFilter(diagnostics.GetLogFilter(diagmod.Spec.LogLevel))
		logger.InfoLog("method", "moduleChangeCb", "msg", "setting log level", "moduleLogLevel", diagmod.Spec.LogLevel)
	}
	watcherOption := tpm.WithModuleWatcher(module.GetWatcher(fmt.Sprintf("%s-%s", k8s.GetNodeName(), globals.Tpm), globals.APIServer, nsClient, logger, moduleChangeCb))

	// add diagnostics service
	diagOption := tpm.WithDiagnosticsService(diagsvc.GetDiagnosticsServiceWithDefaults(globals.Tpm, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, nsClient, logger))

	// init policy manager
	pm, err := tpm.NewPolicyManager(*listenURL, nsClient, *restURL, watcherOption, diagOption)
	if err != nil {
		// let the scheduler restart tpm
		log.Fatalf("failed to init policy agent, %s", err)
	}

	log.Infof("%s is running {%+v}", globals.Tpm, pm)
	recorder.Event(eventtypes.SERVICE_RUNNING,
		fmt.Sprintf("Service %s running on %s", globals.Tpm, utils.GetHostname()), nil)

	select {}
}
