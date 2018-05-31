// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"strings"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", "/var/log/pensando/spyglass.log", "Redirect logs to file")
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
		Filter:      log.AllowAllFilter,
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

	// Create a dummy channel to wait forever
	waitCh := make(chan bool)
	ctx := context.Background()

	rslr := resolver.New(&resolver.Config{Name: "spyglass",
		Servers: strings.Split(*resolverAddrs, ",")})

	// Create the finder and associated search endpoint
	fdr, err := finder.NewFinder(ctx,
		*finderAddr,
		rslr,
		logger)
	if err != nil || fdr == nil {
		log.Fatalf("Failed to create finder, err: %v", err)
	}

	// Start finder service
	err = fdr.Start()
	if err != nil {
		log.Fatalf("Failed to start finder, err: %v", err)
	}

	// Create the indexer
	idxer, err := indexer.NewIndexer(ctx,
		*apiServerAddr,
		rslr,
		logger)

	if err != nil || idxer == nil {
		log.Fatalf("Failed to create indexer, err: %v", err)
	}

	// Start indexer service
	err = idxer.Start()
	if err != nil {
		log.Fatalf("Failed to start indexer, err: %v", err)
	}

	log.Infof("%s is running {%+v} {%v}", globals.Spyglass, fdr, idxer)
	// wait forever
	<-waitCh
}
