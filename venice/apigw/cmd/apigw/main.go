package main

import (
	"flag"
	"strings"

	"google.golang.org/grpc/grpclog"

	_ "github.com/pensando/sw/api/generated/exports/apigw"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	trace "github.com/pensando/sw/venice/utils/trace"
)

const srvName = "ApiGw"

func main() {
	var (
		httpaddr        = flag.String("port", ":"+globals.APIGwRESTPort, "HTTP port to listen on")
		debugflag       = flag.Bool("debug", false, "enable debug mode")
		host            = flag.String("host", "localhost", "host identity")
		logToStdoutFlag = flag.Bool("logtostdout", true, "enable logging to stdout")
		logToFile       = flag.String("logtofile", "/var/log/pensando/apigw.log", "redirect logs to file")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:port>")
		devmode         = flag.Bool("devmode", true, "Development mode where tracing options are enabled")
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
			Format:      log.JSONFmt,
			Filter:      log.AllowAllFilter,
			Debug:       *debugflag,
			CtxSelector: log.ContextAll,
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
		config.Resolvers = strings.Split(*resolverURLs, ",")
		config.DevMode = *devmode
	}
	trace.Init("ApiGateway")
	pl.Log("msg", "Starting Run")
	gw := apigwpkg.MustGetAPIGateway()
	grpclog.SetLogger(pl)
	gw.Run(config)
}
