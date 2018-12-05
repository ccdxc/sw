// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"

	"errors"
	"time"

	"golang.org/x/net/context"

	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/collector"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver"
	"github.com/pensando/sw/venice/citadel/data"
	"github.com/pensando/sw/venice/citadel/http"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/query"
	"github.com/pensando/sw/venice/citadel/watcher"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const maxRetry = 120

func main() {
	// command line flags
	var (
		kvstoreURL      = flag.String("kvstore", "", "KVStore URL where etcd is accessible")
		nodeURL         = flag.String("url", "", "listen URL where citadel's gRPC server runs")
		httpURL         = flag.String("http", "127.0.0.1:"+globals.CitadelHTTPPort, "HTTP server URL where citadel's REST api is available")
		queryURL        = flag.String("query-url", ":"+globals.CitadelQueryRPCPort, "HTTP server URL where citadel's metrics query api is available")
		nodeUUID        = flag.String("uuid", "", "Node UUID (unique identifier for this citadel instance)")
		dbPath          = flag.String("db", "/tmp/tstore/", "DB path where citadel's data will be stored")
		collectorURL    = flag.String("collector-url", fmt.Sprintf(":%s", globals.CollectorRPCPort), "listen URL where citadel metrics collector's gRPC server runs")
		logFile         = flag.String("logfile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Citadel)), "redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		resolverURLs    = flag.String(
			"resolver-urls",
			":"+globals.CMDResolverPort,
			"comma separated list of resolver URLs of the form 'ip:port'",
		)
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Citadel,
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
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

	// get host name and use that for node url & uuid
	if *nodeURL == "" || *nodeUUID == "" || *kvstoreURL == "" {
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

		if *kvstoreURL == "" {
			ku := hostname + ":" + globals.KVStoreClientPort
			kvstoreURL = &ku
		}
	}

	// cluster config
	cfg := meta.DefaultClusterConfig()
	cfg.MetastoreType = store.KVStoreTypeEtcd
	cfg.MetastoreURL = *kvstoreURL

	// create the data node
	dn, err := data.NewDataNode(cfg, *nodeUUID, *nodeURL, *dbPath)
	if err != nil {
		log.Fatalf("Error creating data node. Err: %v", err)
	}

	br, err := broker.NewBroker(cfg, *nodeUUID)
	if err != nil {
		log.Fatalf("Error creating broker. Err: %v", err)
	}

	if err := checkClusterHealth(br); err != nil {
		log.Fatalf("%s", err)
	}

	log.Infof("Datanode %+v and broker %+v are running", dn, br)

	// start the http server
	hsrv, err := httpserver.NewHTTPServer(*httpURL, br)
	if err != nil {
		log.Fatalf("Error creating HTTP server. Err: %v", err)
	}
	log.Infof("HTTP server is listening on %s", hsrv.GetAddr())

	// start collector, use citadel
	c := collector.NewCollector(br)

	// setup an rpc server
	srv, err := rpcserver.NewCollRPCSrv(*collectorURL, c)
	if err != nil {
		log.Fatalf(err.Error())
	}

	log.Infof("%s is running {%+v}", globals.Citadel, srv)

	qsrv, err := query.NewQueryService(*queryURL, br)

	log.Infof("query server is listening on %+v", qsrv)

	resolverClient := resolver.New(&resolver.Config{
		Name:    globals.Citadel,
		Servers: strings.Split(*resolverURLs, ",")})

	watcher := watcher.NewWatcher(globals.APIServer, br, resolverClient)

	// We should be waiting forever in tenantWatch
	// In case the watch is closed, we wrap in a for loop in order
	// to restart the watch
	for {
		watcher.WatchTenant(context.Background())
	}
}

func checkClusterHealth(br *broker.Broker) error {
	// wait till cluster is ready
	for i := 0; i < maxRetry; i++ {
		if err := br.ClusterCheck(); err == nil {
			return nil
		}
		time.Sleep(time.Second)
	}

	return errors.New("cluster check failed")
}
