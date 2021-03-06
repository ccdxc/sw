package main

import (
	"context"
	"flag"
	"fmt"
	"net/http"
	"path/filepath"
	"strings"

	"google.golang.org/grpc/grpclog"

	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/hooks/apigw"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	_ "github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/globals"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	_ "github.com/pensando/sw/venice/utils/bootstrapper/auth"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"

	_ "github.com/pensando/sw/api/generated/swagger"
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

// small function to redirect all http requests to https port
func redirect(w http.ResponseWriter, req *http.Request) {
	http.Redirect(w, req,
		"https://"+req.Host+req.URL.String(),
		http.StatusMovedPermanently)
}

func main() {
	var (
		httpaddr        = flag.String("port", ":"+globals.APIGwRESTPort, "HTTP port to listen on")
		debugflag       = flag.Bool("debug", false, "enable debug mode")
		host            = flag.String("host", globals.Localhost, "host identity")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.APIGw)), "redirect logs to file")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:port>")
		devmode         = flag.Bool("devmode", false, "Development mode where tracing options are enabled")
		override        = flag.String("override", "", "backend override map eg: 'pen-apiserver=localhost:5000,pen-search=localhost:5005'")
		skip            = flag.String("skip", "", "comma seperated list of services to skip initializing eg: 'search,events'")
		skipauth        = flag.Bool("skipauth", false, "skip authentication")
		skipauthz       = flag.Bool("skipauthz", false, "skip authorization")
		disableEvents   = flag.Bool("no-events", false, "disable events proxy")
		skipaudit       = flag.Bool("no-audit", false, "disable external auditing")
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
			Filter:      log.AllowInfoFilter,
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
		if *debugflag {
			logConfig.Filter = log.AllowAllFilter
		}
		pl = log.SetConfig(logConfig)

		// Add ApiGw specific context data
		pl = pl.WithContext("host", *host+*httpaddr)
	}
	defer pl.Close()
	grpclog.SetLoggerV2(pl)

	// create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component:     globals.APIGw,
		SkipEvtsProxy: *disableEvents}, pl)
	if err != nil {
		pl.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

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
		if *skipaudit {
			config.Auditor = auditmgr.WithAuditors(auditmgr.NewLogAuditor(context.TODO(), pl))
		}
	}
	if config.DevMode {
		trace.Init(globals.APIGw)
	}

	// redirect every http request to https
	go http.ListenAndServe(":80", http.HandlerFunc(redirect))

	pl.Log("msg", "Starting Run")

	gw := apigwpkg.MustGetAPIGateway()

	pl.Infof("%s is running {%+v}", globals.APIGw, gw)
	gw.Run(config)
}
