package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"strings"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/perseus/env"
	"github.com/pensando/sw/venice/perseus/startup"
	"github.com/pensando/sw/venice/utils/log"
)

func main() {
	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Perseus)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
	)

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Perseus,
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

	// special logger with throttling for gRPC
	grpcLoggerConf := log.GetDefaultThrottledLoggerConfig(*logConfig)
	grpcLogger := log.GetNewLogger(grpcLoggerConf)
	grpclog.SetLoggerV2(grpcLogger)

	env.Options = options.NewServerRunOptions()

	startup.OnStart(strings.Split(*resolverURLs, ","))

	env.Logger.Infof("Launching server %s is running", globals.Perseus)

	// Wait forever
	waitCh := make(chan bool)
	<-waitCh
}
