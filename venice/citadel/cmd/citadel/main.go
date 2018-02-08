// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"os"

	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"

	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/collectors"
	"github.com/pensando/sw/venice/citadel/data"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
)

func main() {
	// command line flags
	var (
		kvstoreURL = flag.String("kvstore", "", "KVStore URL where etcd is accessible")
		nodeURL    = flag.String("url", "", "listen URL where citadel's gRPC server runs")
		httpURL    = flag.String("http", ":"+globals.CitadelHTTPPort, "HTTP server URL where citadel's REST api is available")
		nodeUUID   = flag.String("uuid", "", "Node UUID (unique identifier for this citadel instance)")
		dbPath     = flag.String("db", "/tmp/tstore/", "DB path where citadel's data will be stored")
	)
	flag.Parse()

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

	log.Infof("Datanode %+v and broker %+v are running", dn, br)

	// start the http server
	hsrv, err := collectors.NewHTTPServer(*httpURL, br)
	if err != nil {
		log.Fatalf("Error creating HTTP server. Err: %v", err)
	}
	log.Infof("HTTP server is listening on %s", hsrv.GetAddr())

	// create a dummy channel to wait forver
	waitCh := make(chan bool)
	// wait forever
	<-waitCh
}
