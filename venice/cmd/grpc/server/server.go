// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package server

import (
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// RunUnauthServer creates a gRPC server for cluster operations.
func RunUnauthServer(clusterMgmtURL, certsURL, nicRegURL string, stopChannel chan bool) {
	// create an unauthenticated RPC server for initial cluster setup
	clusterMgmtRPCServer, err := rpckit.NewRPCServer(globals.Cmd, clusterMgmtURL, rpckit.WithTLSProvider(nil), rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Fatalf("Error creating ClusterMgmt server at %s: %v", clusterMgmtURL, err)
	}
	env.ClusterMgmtRPCServer = clusterMgmtRPCServer
	grpc.RegisterClusterServer(clusterMgmtRPCServer.GrpcServer, &clusterRPCHandler{})

	// create a local unauthenticated RPC server for certificate requests
	certsRPCServer, err := rpckit.NewRPCServer(globals.Cmd, certsURL, rpckit.WithTLSProvider(nil), rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Fatalf("Error creating Certificates server at %s: %v", certsURL, err)
	}
	env.LocalCertsRPCServer = certsRPCServer
	certRPCHandler := certificates.NewRPCHandler()
	certapi.RegisterCertificatesServer(certsRPCServer.GrpcServer, certRPCHandler)

	// create an unauthenticated RPC server for SmartNIC registration
	smartNICRegRPCServer, err := rpckit.NewRPCServer(globals.Cmd, nicRegURL, rpckit.WithTLSProvider(nil), rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Fatalf("Error creating NIC Registration server at %s: %v", nicRegURL, err)
	}
	env.SmartNICRegRPCServer = smartNICRegRPCServer
	RegisterSmartNICRegistrationServer(env.StateMgr)

	// start RPC servers
	certsRPCServer.Start()
	defer certsRPCServer.Stop()
	smartNICRegRPCServer.Start()
	defer smartNICRegRPCServer.Stop()
	clusterMgmtRPCServer.Start()
	defer clusterMgmtRPCServer.Stop()

	// wait until stop request
	<-stopChannel
}
