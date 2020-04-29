// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"expvar"
	"flag"
	"fmt"
	"net/http"
	"net/http/pprof"
	"path/filepath"
	"strings"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Spyglass)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		apiServerAddr   = flag.String("api-server-addr", globals.APIServer, "ApiServer gRPC endpoint")
		finderAddr      = flag.String("finder-addr", ":"+globals.SpyglassRPCPort, "Finder search gRPC endpoint")
		resolverAddrs   = flag.String("resolver-addrs", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
	)

	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Spyglass,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		CtxSelector: log.ContextAll,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	logger := log.SetConfig(logConfig)
	defer logger.Close()

	// Create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component: globals.Spyglass}, logger)
	if err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

	// Create a dummy channel to wait forever
	waitCh := make(chan bool)
	ctx := context.Background()

	rslr := resolver.New(&resolver.Config{Name: "spyglass",
		Servers: strings.Split(*resolverAddrs, ",")})

	// Create new policy cache
	cache := cache.NewCache(logger)

	// Create the finder and associated search endpoint
	fdr, err := finder.NewFinder(ctx,
		*finderAddr,
		rslr,
		cache,
		logger,
	)
	if err != nil || fdr == nil {
		log.Fatalf("Failed to create finder, err: %v", err)
	}

	// Start finder service
	err = fdr.Start()
	if err != nil {
		log.Fatalf("Failed to start finder, err: %v", err)
	}

	startIndexer := func(idxer indexer.Interface) {
		for {
			err := idxer.Start()
			if err != nil {
				log.Errorf("Indexer failed with err, err: %v", err)
			}
		}
	}

	// Create the indexers
	idxerConfig, err := indexer.NewIndexer(ctx,
		*apiServerAddr,
		rslr,
		cache,
		logger,
		indexer.WithDisableVOSWatcher())
	if err != nil || idxerConfig == nil {
		log.Fatalf("Failed to create config indexer, err: %v", err)
	}

	idxerFwlogs, err := indexer.NewIndexer(ctx,
		*apiServerAddr,
		rslr,
		cache,
		logger,
		indexer.WithDisableAPIServerWatcher())
	if err != nil || idxerFwlogs == nil {
		log.Fatalf("Failed to create fwlogs indexer, err: %v", err)
	}

	// Start the indexers
	go startIndexer(idxerConfig)
	go startIndexer(idxerFwlogs)

	router := mux.NewRouter()

	// add pprof routes
	router.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/", pprof.Index)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/cmdline", pprof.Cmdline)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/profile", pprof.Profile)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/symbol", pprof.Symbol)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/trace", pprof.Trace)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/allocs", pprof.Handler("allocs").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/block", pprof.Handler("block").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/heap", pprof.Handler("heap").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/mutex", pprof.Handler("mutex").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/goroutine", pprof.Handler("goroutine").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/threadcreate", pprof.Handler("threadcreate").ServeHTTP)
	router.Methods("GET", "POST").Subrouter().Handle("/debug/config", indexer.HandleDebugConfig(idxerFwlogs))

	go http.ListenAndServe(fmt.Sprintf("127.0.0.1:%s", globals.SpyglassRESTPort), router)

	log.Infof("%s is running", globals.Spyglass)

	// wait forever
	<-waitCh
}
