package main

import (
	"flag"
	"strings"

	"google.golang.org/grpc/grpclog"

	_ "github.com/pensando/sw/api/generated/exports/apigw"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	_ "github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	trace "github.com/pensando/sw/venice/utils/trace"
)

const srvName = "ApiGw"

// makeOverrideMap creates a map from a string. examples for input strings
//   "pen-apiserver=localhost:5000" -> map[pen-apiserver:localhost:5000]
//   "pen-apiserver=localhost:5000,pen-search=localhost:5005" ->
//      map[pen-apiserver:localhost:5000 pen-search:localhost:5005]
func makeOverrideMap(in string) map[string]string {
	if in == "" {
		return nil
	}
	ret := make(map[string]string)
	kvs := strings.Split(in, ",")
	for _, v := range kvs {
		if v == "" {
			continue
		}
		kv := strings.Split(v, "=")
		if len(kv) == 2 {
			if kv[0] != "" && kv[1] != "" {
				ret[kv[0]] = kv[1]
			}
		}
	}
	return ret
}

func main() {
	var (
		httpaddr        = flag.String("port", ":"+globals.APIGwRESTPort, "HTTP port to listen on")
		debugflag       = flag.Bool("debug", false, "enable debug mode")
		host            = flag.String("host", "localhost", "host identity")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("logtofile", "/var/log/pensando/apigw.log", "redirect logs to file")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:port>")
		devmode         = flag.Bool("devmode", true, "Development mode where tracing options are enabled")
		override        = flag.String("override", "", "backend override map eg: 'pen-apiserver=localhost:5000,pen-search=localhost:5005'")
		skip            = flag.String("skip", "", "comma seperated list of services to skip initializing eg: 'search,events'")
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
		if *override != "" {
			config.BackendOverride = makeOverrideMap(*override)
			config.Resolvers = []string{}
		}
		if *skip != "" {
			config.SkipBackends = strings.Split(*skip, ",")
		}
	}
	trace.Init("ApiGateway")
	pl.Log("msg", "Starting Run")
	gw := apigwpkg.MustGetAPIGateway()
	grpclog.SetLogger(pl)
	gw.Run(config)
}
