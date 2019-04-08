// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"

	"github.com/pensando/sw/api/generated/events"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/collector"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver"
	"github.com/pensando/sw/venice/citadel/data"
	"github.com/pensando/sw/venice/citadel/http"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/query"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/debug"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const maxRetry = 120

func main() {
	// command line flags
	var (
		nodeURL         = flag.String("url", "", "listen URL where citadel's gRPC server runs")
		httpURL         = flag.String("http", "127.0.0.1:"+globals.CitadelHTTPPort, "HTTP server URL where citadel's REST api is available")
		queryURL        = flag.String("query-url", ":"+globals.CitadelQueryRPCPort, "HTTP server URL where citadel's metrics query api is available")
		nodeUUID        = flag.String("uuid", "", "Node UUID (unique identifier for this citadel instance)")
		dbPath          = flag.String("db", "/var/lib/pensando/citadel/", "DB path where citadel's data will be stored")
		queryDbPath     = flag.String("queryDb", "", "query DB path for query aggregation")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs of the form 'ip:port'")
		collectorURL    = flag.String("collector-url", fmt.Sprintf(":%s", globals.CollectorRPCPort), "listen URL where citadel metrics collector's gRPC server runs")
		logFile         = flag.String("logfile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Citadel)), "redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Citadel,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)
	logger := log.GetNewLogger(logConfig)

	// create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Source:   &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: globals.Citadel},
		EvtTypes: evtsapi.GetEventTypes()}, logger)
	if err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

	// get host name and use that for node url & uuid
	if *nodeURL == "" || *nodeUUID == "" {
		// read my host name
		hostname, err := os.Hostname()
		if err != nil {
			log.Fatalf("Failed to read the hostname. Err: %v", err)
		}

		if *nodeURL == "" {
			lu := hostname + ":" + globals.CitadelRPCPort
			nodeURL = &lu
		}

		if *nodeUUID == "" {
			nodeUUID = &hostname
		}
	}

	// cluster config
	cfg := meta.DefaultClusterConfig()
	cfg.MetastoreType = store.KVStoreTypeEtcd
	cfg.ResolverClient = resolver.New(&resolver.Config{Name: globals.Citadel, Servers: strings.Split(*resolverURLs, ",")})

	// create the data node
	dn, err := data.NewDataNode(cfg, *nodeUUID, *nodeURL, *dbPath, *queryDbPath, logger)
	if err != nil {
		log.Fatalf("Error creating data node. Err: %v", err)
	}

	br, err := broker.NewBroker(cfg, *nodeUUID, logger)
	if err != nil {
		log.Fatalf("Error creating broker. Err: %v", err)
	}

	checkClusterHealth(br)

	log.Infof("Datanode %+v and broker %+v are running", dn, br)

	// start collector, use citadel
	c := collector.NewCollector(br)

	// setup an rpc server
	srv, err := rpcserver.NewCollRPCSrv(*collectorURL, c)
	if err != nil {
		log.Fatalf(err.Error())
	}

	log.Infof("=== %s is running {%+v}", globals.Citadel, srv)

	// Creating debug instance
	dbg := debug.New(srv.Debug)

	// start the http server
	hsrv, err := httpserver.NewHTTPServer(*httpURL, br, dbg)
	if err != nil {
		log.Fatalf("Error creating HTTP server. Err: %v", err)
	}
	log.Infof("HTTP server is listening on %s", hsrv.GetAddr())

	qsrv, err := query.NewQueryService(*queryURL, br)

	log.Infof("query server is listening on %+v", qsrv)

	// Wait forever
	select {}
}

func checkClusterHealth(br *broker.Broker) {
	for { // wait till cluster is ready
		var err error
		for i := 0; i < maxRetry; i++ {
			if err = br.ClusterCheck(); err == nil {
				log.Infof("cluster is ready")
				recorder.Event(events.ServiceRunning, events.SeverityLevel_INFO, globals.Citadel+" service is ready", nil)
				return
			}
			log.Errorf("cluster failed %v", err)
			time.Sleep(time.Second)
		}

		// log event
		recorder.Event(events.ServiceUnresponsive, events.SeverityLevel_WARNING, globals.Citadel+" service failed, "+err.Error(), nil)
	}
}
