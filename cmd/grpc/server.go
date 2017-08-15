package grpc

import (
	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/rpckit"
)

// RunServer creates a gRPC server for cluster operations.
func RunServer(url, certFile, keyFile, caFile string, stopChannel chan bool) {
	// create an RPC server.
	rpcServer, err := rpckit.NewRPCServer("cmd", url, certFile, keyFile, caFile)
	if err != nil {
		log.Fatalf("Error creating grpc server: %v", err)
	}
	defer func() { rpcServer.Stop() }()

	env.RPCServer = rpcServer

	// create and register the RPC handler for cluster object.
	RegisterClusterServer(rpcServer.GrpcServer, &clusterRPCHandler{})

	// wait forever
	<-stopChannel
}
