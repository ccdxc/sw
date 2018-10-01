// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/ctrler/rollout/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// RPCServer is the RPC server object
type RPCServer struct {
	stateMgr              *statemgr.Statemgr // reference to state manager
	grpcServer            *rpckit.RPCServer  // gRPC server instance
	smartNICRolloutServer *SmartNICRolloutRPCServer
	veniceRolloutServer   *VeniceRolloutRPCServer
	serviceRolloutServer  *ServiceRolloutRPCServer
}

// Stop stops the rpc server
func (rs *RPCServer) Stop() error {
	// stop the rpc server
	return rs.grpcServer.Stop()
}

// NewRPCServer creates a new instance of
func NewRPCServer(listenURL string, stateMgr *statemgr.Statemgr) (*RPCServer, error) {
	// create an RPC server
	grpcServer, err := rpckit.NewRPCServer(globals.Rollout, listenURL)
	if err != nil {
		log.Fatalf("Error creating rpc server. Err; %v", err)
	}

	veniceRolloutServer, err := NewVeniceRolloutRPCServer(stateMgr)
	if err != nil {
		log.Fatalf("Error creating VeniceRollout rpc server. Err; %v", err)
	}
	smartNICRolloutServer, err := NewSmartNICRolloutRPCServer(stateMgr)
	if err != nil {
		log.Fatalf("Error creating SmartNICRollout rpc server. Err; %v", err)
	}
	serviceRolloutServer, err := NewServiceRolloutRPCServer(stateMgr)
	if err != nil {
		log.Fatalf("Error creating ServiceRollout rpc server. Err; %v", err)
	}

	// register the RPC handlers and start the server
	protos.RegisterSmartNICRolloutApiServer(grpcServer.GrpcServer, smartNICRolloutServer)
	protos.RegisterVeniceRolloutApiServer(grpcServer.GrpcServer, veniceRolloutServer)
	protos.RegisterServiceRolloutApiServer(grpcServer.GrpcServer, serviceRolloutServer)
	grpcServer.Start()

	// create rpc server object
	rpcServer := RPCServer{
		stateMgr:              stateMgr,
		grpcServer:            grpcServer,
		smartNICRolloutServer: smartNICRolloutServer,
		veniceRolloutServer:   veniceRolloutServer,
		serviceRolloutServer:  serviceRolloutServer,
	}

	return &rpcServer, nil
}

// GetListenURL returns the listen URL for the server.
func (rs *RPCServer) GetListenURL() string {
	return rs.grpcServer.GetListenURL()
}
