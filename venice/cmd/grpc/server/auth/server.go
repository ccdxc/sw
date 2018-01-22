// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package auth

import (
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// RunAuthServer creates a gRPC server for services that require authentication.
// Authentication happens using mTLS, so the server can be started only after
// the certificates server is up and ready to serve certificates.
func RunAuthServer(url string, stopChannel chan bool) {
	// create and start authenticated RPC server.
	// These require TLS so depend on certificate services being available
	rpcServer, err := rpckit.NewRPCServer("cmd", url)
	if err != nil {
		log.Fatalf("Error creating grpc server at %v: %v", url, err)
	}
	defer func() { rpcServer.Stop() }()
	env.AuthRPCServer = rpcServer

	if env.ResolverService != nil {
		// create and register the RPC handler for service object.
		types.RegisterServiceAPIServer(env.AuthRPCServer.GrpcServer, service.NewRPCHandler(env.ResolverService))
	}

	rpcServer.Start()

	// wait forever
	<-stopChannel
}
