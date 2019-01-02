package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"strings"

	"google.golang.org/grpc/grpclog"

	evtsapi "github.com/pensando/sw/api/generated/events"
	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/hooks/apigw"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	_ "github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	_ "github.com/pensando/sw/venice/utils/bootstrapper/auth"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"
)

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
		host            = flag.String("host", globals.Localhost, "host identity")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.APIGw)), "redirect logs to file")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:port>")
		devmode         = flag.Bool("devmode", true, "Development mode where tracing options are enabled")
		override        = flag.String("override", "", "backend override map eg: 'pen-apiserver=localhost:5000,pen-search=localhost:5005'")
		skip            = flag.String("skip", "", "comma seperated list of services to skip initializing eg: 'search,events'")
		skipauth        = flag.Bool("skipauth", false, "skip authentication")
		skipauthz       = flag.Bool("skipauthz", false, "skip authorization")
		disableEvents   = flag.Bool("no-events", false, "disable events proxy")
	)

	flag.Parse()

	var pl log.Logger
	{
		logtoFileFlag := true
		if *logToFile == "" {
			logtoFileFlag = false
		}
		logConfig := &log.Config{
			Module:      globals.APIGw,
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
		pl = log.SetConfig(logConfig)

		// Add ApiGw specific context data
		pl = pl.WithContext("host", *host+*httpaddr)
	}

	// create events recorder
	if _, err := recorder.NewRecorder(&recorder.Config{
		Source:   &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: globals.APIGw},
		EvtTypes: evtsapi.GetEventTypes(), SkipEvtsProxy: *disableEvents}); err != nil {
		pl.Fatalf("failed to create events recorder, err: %v", err)
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
		config.SkipAuth = *skipauth
		config.SkipAuthz = *skipauthz
	}
	trace.Init(globals.APIGw)
	pl.Log("msg", "Starting Run")

	gw := apigwpkg.MustGetAPIGateway()
	grpclog.SetLogger(pl)
	pl.Infof("%s is running {%+v}", globals.APIGw, gw)
	gw.Run(config)
}
