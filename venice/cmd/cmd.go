package main

import (
	"flag"
	"fmt"

	cmd "github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/server"
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/cmd/startup"
	configs "github.com/pensando/sw/venice/cmd/systemd-configs"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/systemd"
)

func main() {
	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", "/var/log/pensando/cmd.log", "Redirect logs to file")
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

	// create events recorder
	// FIXME: eventSource.NodeName should match with the name in node object; either we
	// enforce user to provide hostname as the node name or we find workaround to
	// update the event source.
	if _, err = recorder.NewRecorder(
		&evtsapi.EventSource{NodeName: utils.GetHostname(), Component: globals.Cmd},
		cmd.GetEventTypes(), "", ""); err != nil {
		fmt.Printf("failed to create events recorder, err: %v", err)
	}

	env.RegistryURL = *registryURL
	env.Scheme = runtime.NewScheme()
	env.Scheme.AddKnownTypes(&cmd.Cluster{}, &cmd.Node{})
	env.Options = options.NewServerRunOptions()

	if err = configs.GenerateRegistryConfig(env.RegistryURL); err != nil {
		fmt.Printf("Error %v while generating registry config at startup", err)
	}

	// We need to issue equivalent of 'systemctl daemon-reload' before anything else to make systemd read the config files
	s := systemd.New()
	err = s.DaemonReload()
	if err != nil {
		fmt.Printf("Error %v while issuing systemd.DaemonReload at startup", err)
	}

	startup.OnStart()

	env.Logger.Debugln("Launching server")
	server.Run(env.Options)
	env.Logger.Infof("%s is running", globals.Cmd)
}
