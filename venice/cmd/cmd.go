package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"time"

	cmd "github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/server"
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/cmd/startup"
	configs "github.com/pensando/sw/venice/cmd/systemd-configs"
	cmdutils "github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/systemd"
)

var (
	// GitVersion is the version derived from git tag
	GitVersion string
	// GitCommit is the commit id
	GitCommit string
	// BuildDate is the date+time of the build
	BuildDate string
)

func main() {
	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Cmd)), "Redirect logs to file")
		registryURL     = flag.String("registry-url", "registry.test.pensando.io:5000", "URL to docker registry")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
	)

	var err error

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Cmd,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10, // TODO: These needs to be part of Service Config Object
			MaxBackups: 3,  // TODO: These needs to be part of Service Config Object
			MaxAge:     7,  // TODO: These needs to be part of Service Config Object
		},
	}

	// Initialize logger config
	env.Logger = log.SetConfig(logConfig)
	env.GitVersion = GitVersion
	env.GitCommit = GitCommit
	env.BuildDate = BuildDate

	// create events recorder
	// FIXME: eventSource.NodeName should match with the name in node object; either we
	// enforce user to provide hostname as the node name or we find workaround to
	// update the event source.
	if env.Recorder, err = recorder.NewRecorder(&recorder.Config{
		SkipEvtsProxy: true,
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: globals.Cmd},
		EvtTypes:      append(cmd.GetEventTypes(), evtsapi.GetEventTypes()...)}); err != nil {
		fmt.Printf("failed to create events recorder, err: %v", err)
	}

	env.RegistryURL = *registryURL
	env.Scheme = runtime.NewScheme()
	env.Scheme.AddKnownTypes(&cmd.Cluster{}, &cmd.Node{})
	env.Options = options.NewServerRunOptions()

	if err = configs.GenerateRegistryConfig(env.RegistryURL); err != nil {
		fmt.Printf("Error %v while generating registry config at startup", err)
	}

	if diOpt := cmdutils.GetConfigProperty("SmartNICDeadInterval"); diOpt != "" {
		di, err := time.ParseDuration(diOpt)
		if err == nil {
			log.Infof("Overriding default SmartNIC DeadInterval. New value: %v", di)
			smartnic.DeadInterval = di
		} else {
			log.Errorf("Invalid SmartNICDeadInterval: %v", diOpt)
		}
	}

	if hwiOpt := cmdutils.GetConfigProperty("SmartNICHealthWatchInterval"); hwiOpt != "" {
		hwi, err := time.ParseDuration(hwiOpt)
		if err == nil {
			log.Infof("Overriding default SmartNIC DeadInterval. New value: %v", hwi)
			smartnic.HealthWatchInterval = hwi
		} else {
			log.Errorf("Invalid HealthWatchInterval: %v", hwiOpt)
		}
	}

	// We need to issue equivalent of 'systemctl daemon-reload' before anything else to make systemd read the config files
	s := systemd.New()
	err = s.DaemonReload()
	if err != nil {
		fmt.Printf("Error %v while issuing systemd.DaemonReload at startup", err)
	}

	startup.OnStart()

	env.Logger.Infof("Launching server %s(ver:%s) (commit:%s) (builddate:%s) is running", globals.Cmd, GitVersion, GitCommit, BuildDate)
	server.Run(env.Options)
}
