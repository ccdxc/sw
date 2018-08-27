// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package auth

import (
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// RunAuthServer creates a gRPC server for services that require authentication.
// Authentication happens using mTLS, so the server can be started only after
// the certificates server is up and ready to serve certificates.
func RunAuthServer(url string, stopChannel chan bool) {
	// create and start authenticated RPC server.
	// These require TLS so depend on certificate services being available
	rpcServer, err := rpckit.NewRPCServer(globals.Cmd, url)
	if err != nil {
		log.Fatalf("Error creating grpc server at %v: %v", url, err)
	}
	defer func() { rpcServer.Stop() }()
	env.AuthRPCServer = rpcServer

	if env.ResolverService != nil {
		// create and register the RPC handler for service object.
		types.RegisterServiceAPIServer(env.AuthRPCServer.GrpcServer, service.NewRPCHandler(env.ResolverService))
	}

	// Create and register the RPC handler for SmartNIC service
	grpc.RegisterSmartNICUpdatesServer(env.AuthRPCServer.GrpcServer, env.NICService.(*smartnic.RPCServer))

	// The Certificates API runs both on the authenticated and unauthenticated port.
	//
	// The endpoint on the unauthenticated port is for local processes that can be attested
	// using local runtime mechanisms (OS system calls, docker runtime, kubelet API).
	//
	// The endpoint on the authenticated port is for processes running on NAPLES.
	// When the NIC is admitted to the cluster, NMD is given a certificate to authenticate
	// itself to CMD and other Venice components. NMD receives certificates API requests
	// from processes running on NAPLES and forwards them to CMD over a mTLS connection.
	//
	// CertMgr is thread-safe, so the same CertMgr instance can be shared between the two
	// Certificates API endpoints.
	certRPCHandler := certificates.NewRPCHandler(env.CertMgr)
	certapi.RegisterCertificatesServer(env.AuthRPCServer.GrpcServer, certRPCHandler)

	rpcServer.Start()

	// wait forever
	<-stopChannel
}
