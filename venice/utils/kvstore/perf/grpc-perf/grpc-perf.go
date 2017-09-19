package main

import (
	"flag"
	"fmt"
	"strings"

	"github.com/pensando/sw/venice/utils/kvstore/perf/grpc-perf/client"
	"github.com/pensando/sw/venice/utils/kvstore/perf/grpc-perf/server"
)

func main() {
	var kvServers string
	var numWatchers, numKVClients, numGRPCClients, numPuts, putRate, restPort, gRPCPort int

	flag.StringVar(&kvServers, "kv-servers", "http://localhost:2379", "comma seperated URLs for kvstore servers")
	flag.IntVar(&numWatchers, "num-watchers", 30, "number of watchers")
	flag.IntVar(&numGRPCClients, "num-grpc-clients", 30, "number of gRPC clients (nodes in the system)")
	flag.IntVar(&numKVClients, "num-kv-clients", 30, "number of KV clients")
	flag.IntVar(&numPuts, "num-puts", 100, "number of puts")
	flag.IntVar(&putRate, "put-rate", 100, "number of puts per second")
	flag.IntVar(&restPort, "rest-port", 9001, "REST server port")
	flag.IntVar(&gRPCPort, "grpc-port", 9002, "gRPC server port")

	flag.Parse()

	restURL := fmt.Sprintf(":%v", restPort)
	gRPCURL := fmt.Sprintf(":%v", gRPCPort)

	// Start the clients.
	go client.RunClient(gRPCURL, numWatchers, numGRPCClients, numPuts, putRate)

	// Start the REST and gRPC servers.
	server.RunServers(restURL, gRPCURL, strings.Split(kvServers, ","), numKVClients)
}
