// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package auth

import (
	"time"

	"github.com/pensando/sw/api/generated/tokenauth"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/cmd/grpc/server/health"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	tokenauthsrv "github.com/pensando/sw/venice/cmd/grpc/server/tokenauth"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	tokenauthsvc "github.com/pensando/sw/venice/cmd/services/tokenauth"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const heartbeatTimeout = 1 * time.Minute

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

	if env.TokenAuthService == nil {
		env.TokenAuthService = tokenauthsvc.NewTokenAuthService(env.ClusterName, env.CertMgr.Ca())
	}
	tokenAuthRPCHandler := tokenauthsrv.NewRPCHandler(env.TokenAuthService)
	tokenauth.RegisterTokenAuthV1Server(env.AuthRPCServer.GrpcServer, tokenAuthRPCHandler)

	rpcServer.Start()

	log.Infof("Started CMD authenticated service at %v", url)

	// wait forever
	<-stopChannel
}

// RunLeaderInstanceServer creates a gRPC server for authenticated services offered by leader CMD.
func RunLeaderInstanceServer(url string, stopChannel chan bool) {
	// Leader service require TLS so it depends on certificate services being available
	rpcServer, err := rpckit.NewRPCServer(globals.CmdNICUpdatesSvc, url)
	if err != nil {
		log.Fatalf("Error creating grpc server at %v: %v", url, err)
	}

	// Create and register the RPC handler for SmartNIC service
	grpc.RegisterSmartNICUpdatesServer(rpcServer.GrpcServer, env.NICService.(*smartnic.RPCServer))

	// Create and register the RPC handler for Health service
	healthService := health.NewRPCServer(heartbeatTimeout)
	grpc.RegisterNodeHeartbeatServer(rpcServer.GrpcServer, healthService)

	rpcServer.Start()

	log.Infof("Started CMD leader instance service at %v", url)

	// wait until stop signal
	<-stopChannel
	rpcServer.Stop()
	healthService.Stop()

	log.Infof("Stopped CMD leader instance service at %v", url)
}
