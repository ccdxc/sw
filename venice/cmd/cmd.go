package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"time"

	"google.golang.org/grpc/grpclog"

	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/server"
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/startup"
	"github.com/pensando/sw/venice/cmd/systemd-configs"
	cmdutils "github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/nodemetrics"
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
			MaxSize:    10, // MB    TODO: These needs to be part of Service Config Object
			MaxBackups: 10, // files TODO: These needs to be part of Service Config Object
			MaxAge:     30, // days  TODO: These needs to be part of Service Config Object
		},
	}

	// Initialize logger config
	env.Logger = log.SetConfig(logConfig)
	defer env.Logger.Close()

	env.GitVersion = GitVersion
	env.GitCommit = GitCommit
	env.BuildDate = BuildDate

	// special logger with throttling for gRPC
	grpcLoggerConf := log.GetDefaultThrottledLoggerConfig(*logConfig)
	grpcLogger := log.GetNewLogger(grpcLoggerConf)
	grpclog.SetLoggerV2(grpcLogger)

	// create events recorder
	// FIXME: eventSource.NodeName should match with the name in node object; either we
	// enforce user to provide hostname as the node name or we find workaround to
	// update the event source.
	if env.Recorder, err = recorder.NewRecorder(&recorder.Config{
		SkipEvtsProxy: true,
		Component:     globals.Cmd}, env.Logger); err != nil {
		fmt.Printf("failed to create events recorder, err: %v", err)
	}
	defer env.Recorder.Close()

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

	if cmsiOpt := cmdutils.GetConfigProperty("ClusterMetricsSendInterval"); cmsiOpt != "" {
		cmsi, err := time.ParseDuration(cmsiOpt)
		if err == nil {
			log.Infof("Overriding default Cluster Metrics Send Interval. New value: %v", cmsi)
			services.MetricsSendInterval = cmsi
			if cmsi < nodemetrics.GetMinimumFrequency() {
				nodemetrics.SetMinimumFrequency(cmsi)
			}
		} else {
			log.Errorf("Invalid ClusterMetricsSendInterval: %v", cmsiOpt)
		}
	}

	if csuiOpt := cmdutils.GetConfigProperty("ClusterStatusUpdateInterval"); csuiOpt != "" {
		csui, err := time.ParseDuration(csuiOpt)
		if err == nil {
			log.Infof("Overriding default Cluster Status Update Interval. New value: %v", csui)
			services.ClusterStatusUpdateInterval = csui
		} else {
			log.Errorf("Invalid ClusterStatusUpdateInterval: %v", csuiOpt)
		}
	}

	cInfo := cmdutils.GetContainerInfo()
	ntpContainerName := cInfo[globals.Ntp]
	cmdutils.NtpContainer = ntpContainerName.ImageName

	// We need to issue equivalent of 'systemctl daemon-reload' before anything else to make systemd read the config files
	s := systemd.New()
	err = s.DaemonReload()
	if err != nil {
		fmt.Printf("Error %v while issuing systemd.DaemonReload at startup", err)
	}

	startup.OnStart()

	env.Logger.Infof("Launching server %s(ver:%s) (commit:%s) (builddate:%s) is running", globals.Cmd, GitVersion, GitCommit, BuildDate)
	server.Run(env.Options)

	// Wait forever
	waitCh := make(chan bool)
	<-waitCh
}
