package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"strings"

	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/tsdb"
)

func main() {
	var (
		listenURL = flag.String(
			"listen-url",
			fmt.Sprintf(":%s", globals.OrchHubAPIPort),
			"RPC listen URL",
		)
		debugflag = flag.Bool(
			"debug",
			false,
			"enable debug mode",
		)
		logToStdoutFlag = flag.Bool(
			"logtostdout",
			false,
			"enable logging to stdout",
		)
		logToFile = flag.String(
			"logtofile",
			fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.OrchHub)),
			"redirect logs to file",
		)
		resolverURLs = flag.String(
			"resolver-urls",
			":"+globals.CMDResolverPort,
			"comma separated list of resolver URLs <IP:port>",
		)
		vcList = flag.String(
			"vcenter-list",
			"",
			"Comma separated list of vc URL of the form 'https://user:pass@ip:port'",
		)
		disableEvents = flag.Bool(
			"no-events",
			false,
			"disable events proxy",
		)
	)

	flag.Parse()

	var pl log.Logger
	{
		logtoFileFlag := true
		if *logToFile == "" {
			logtoFileFlag = false
		}

		logConfig := &log.Config{
			Module:      globals.OrchHub,
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
	}

	// Initialize logger config
	defer pl.Close()

	// create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component:     globals.OrchHub,
		SkipEvtsProxy: *disableEvents}, pl)
	if err != nil {
		pl.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

	r := resolver.New(&resolver.Config{Name: globals.OrchHub, Servers: strings.Split(*resolverURLs, ",")})
	tsdb.Init(context.Background(), &tsdb.Opts{})

	opts := orchhub.Opts{
		VcList:    *vcList,
		ListenURL: *listenURL,
		Resolver:  r,
		Logger:    pl,
		DebugMode: *debugflag,
	}
	ctrler, err := orchhub.NewOrchCtrler(opts)
	if err != nil || ctrler == nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	pl.Infof("%s is running {%+v}", globals.OrchHub, ctrler)
	recorder.Event(eventtypes.SERVICE_RUNNING,
		fmt.Sprintf("Service %s running on %s", globals.OrchHub, utils.GetHostname()), nil)

	waitForever()
}

func waitForever() {
	select {}
}
