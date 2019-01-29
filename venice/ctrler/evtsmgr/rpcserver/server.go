// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/ctrler/evtsmgr/alertengine"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	emgrpc "github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/protos"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// RPCServer is the RPC server object with all event APIs
type RPCServer struct {
	evtsMgrHandler    *EvtsMgrRPCHandler
	evtsPolicyHandler *EventPolicyRPCHandler
	server            *rpckit.RPCServer // rpckit server instance
}

// Done returns RPC server's error channel which will error out when the server is stopped
func (rs *RPCServer) Done() <-chan error {
	return rs.server.DoneCh
}

// Stop stops the RPC server
func (rs *RPCServer) Stop() error {
	return rs.server.Stop() // this will stop accepting further requests
}

// GetListenURL returns the listen URL for the server.
func (rs *RPCServer) GetListenURL() string {
	return rs.server.GetListenURL()
}

// NewRPCServer creates a new instance of events RPC server
func NewRPCServer(serverName, listenURL string, esclient elastic.ESClient, alertEngine alertengine.Interface,
	memdb *memdb.MemDb) (*RPCServer, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(listenURL) || esclient == nil || alertEngine == nil || memdb == nil {
		return nil, errors.New("all parameters are required")
	}

	// create a RPC server
	rpcServer, err := rpckit.NewRPCServer(serverName, listenURL)
	if err != nil {
		return nil, errors.Wrap(err, "error creating rpc server")
	}

	// instantiate events handlers which carries the implementation of the service
	evtsMgrHandler, err := NewEvtsMgrRPCHandler(esclient, alertEngine)
	if err != nil {
		return nil, errors.Wrap(err, "error creating evtsmgr rpc server")
	}

	evtsPolicyHandler, err := NewEventPolicyRPCHandler(memdb.Memdb)
	if err != nil {
		return nil, errors.Wrap(err, "error creating event policy rpc server")
	}

	// register the servers
	emgrpc.RegisterEvtsMgrAPIServer(rpcServer.GrpcServer, evtsMgrHandler)
	emgrpc.RegisterEventPolicyAPIServer(rpcServer.GrpcServer, evtsPolicyHandler)
	rpcServer.Start()

	return &RPCServer{
		evtsMgrHandler:    evtsMgrHandler,
		evtsPolicyHandler: evtsPolicyHandler,
		server:            rpcServer,
	}, nil
}
