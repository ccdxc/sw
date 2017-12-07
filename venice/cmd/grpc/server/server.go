// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package server

import (
	"github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
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

	env.K8sService = services.NewK8sService()
	env.ResolverService = services.NewResolverService(env.K8sService)
	env.StateMgr = cache.NewStatemgr()
	env.CfgWatcherService = apiclient.NewCfgWatcherService(env.Logger, globals.APIServer, env.StateMgr)

	// create and register the RPC handler for service object.
	types.RegisterServiceAPIServer(env.RPCServer.GrpcServer, service.NewRPCHandler(env.ResolverService))

	// Create and register the RPC handler for SmartNIC service
	RegisterSmartNICServer(env.StateMgr)

	rpcServer.Start()

	// wait forever
	<-stopChannel
}
