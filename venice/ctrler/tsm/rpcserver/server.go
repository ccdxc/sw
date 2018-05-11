// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// RPCServer is the RPC server object
type RPCServer struct {
	stateMgr   *statemgr.Statemgr // reference to state manager
	grpcServer *rpckit.RPCServer  // gRPC server instance
	// XXX unexport msServer
	MsServer *MirrorSessionRPCServer
}

// Stop stops the rpc server
func (rs *RPCServer) Stop() error {
	// stop the rpc server
	return rs.grpcServer.Stop()
}

// NewRPCServer creates a new instance of
func NewRPCServer(listenURL string, stateMgr *statemgr.Statemgr) (*RPCServer, error) {
	// create an RPC server
	grpcServer, err := rpckit.NewRPCServer(globals.Tsm, listenURL)
	if err != nil {
		log.Fatalf("Error creating rpc server. Err; %v", err)
	}

	// create mirror session RPC server
	msServer, err := NewMirrorSessionRPCServer(stateMgr)
	if err != nil {
		log.Fatalf("Error creating mirror session rpc server. Err; %v", err)
	}

	// register the RPC handlers and start the server
	tsproto.RegisterMirrorSessionApiServer(grpcServer.GrpcServer, msServer)
	grpcServer.Start()

	// create rpc server object
	rpcServer := RPCServer{
		stateMgr:   stateMgr,
		grpcServer: grpcServer,
		MsServer:   msServer,
	}

	return &rpcServer, nil
}

// GetListenURL returns the listen URL for the server.
func (rs *RPCServer) GetListenURL() string {
	return rs.grpcServer.GetListenURL()
}
