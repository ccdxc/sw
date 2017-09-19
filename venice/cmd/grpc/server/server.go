// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package server

import (
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// RunServer creates a gRPC server for cluster operations.
func RunServer(url, certFile, keyFile, caFile string, stopChannel chan bool) {
	// create an RPC server.
	rpcServer, err := rpckit.NewRPCServer("cmd", url)
	if err != nil {
		log.Fatalf("Error creating grpc server: %v", err)
	}
	defer func() { rpcServer.Stop() }()

	env.RPCServer = rpcServer

	// create and register the RPC handler for cluster object.
	grpc.RegisterClusterServer(rpcServer.GrpcServer, &clusterRPCHandler{})

	if env.ResolverService != nil {
		types.RegisterServiceAPIServer(env.RPCServer.GrpcServer, service.NewRPCHandler(env.ResolverService))
	}

	// wait forever
	<-stopChannel
}
