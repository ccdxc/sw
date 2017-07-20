package main

import (
	"flag"

	"google.golang.org/grpc/grpclog"

	_ "github.com/pensando/sw/api/generated/exports/apigw"
	"github.com/pensando/sw/apigw"
	apigwpkg "github.com/pensando/sw/apigw/pkg"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/log"
	trace "github.com/pensando/sw/utils/trace"
)

const srvName = "ApiGw"

func main() {
	var (
		httpaddr        = flag.String("port", ":"+globals.APIGwRESTPort, "HTTP port to listen on")
		debugflag       = flag.Bool("debug", false, "enable debug mode")
		host            = flag.String("host", "localhost", "host identity")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("logtofile", "/var/log/pensando/apigw.log", "redirect logs to file")
	)

	flag.Parse()

	var pl log.Logger
	{
		logtoFileFlag := true
		if *logToFile == "" {
			logtoFileFlag = false
		}
		logConfig := &log.Config{
			Module:      srvName,
			Format:      log.LogFmt,
			Debug:       *debugflag,
			LogToStdout: *logToStdoutFlag,
			LogToFile:   logtoFileFlag,
			FileCfg: log.FileConfig{
				Filename:   *logToFile,
				MaxSize:    10, // TODO: These needs to be part of Service Config Object
				MaxBackups: 3,  // TODO: These needs to be part of Service Config Object
				MaxAge:     7,  // TODO: These needs to be part of Service Config Object
			},
		}
		pl = log.GetNewLogger(logConfig)

		// Add ApiGw specific context data
		pl = pl.WithContext("host", *host+*httpaddr)
	}

	var config apigw.Config
	{
		config.HTTPAddr = *httpaddr
		config.DebugMode = *debugflag
		config.Logger = pl
	}
	trace.Init("ApiGateway")
	pl.Log("msg", "Starting Run")
	gw := apigwpkg.MustGetAPIGateway()
	grpclog.SetLogger(pl)
	gw.Run(config)
}
