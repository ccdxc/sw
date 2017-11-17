// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/ctrler/ckm/rpcserver/ckmproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// CKMctrlerURL is URL where CKMctrler is running
const CKMctrlerURL = "master.local:" + globals.CKMAPILocalPort

// RPCServer is the RPC server object for all CKM APIs
type RPCServer struct {
	certificateMgr *certmgr.CertificateMgr
	rpcServer      *rpckit.RPCServer // rpckit server instance
}

// Stop stops the rpc server
func (rs *RPCServer) Stop() error {
	// stop the rpc server
	return rs.rpcServer.Stop()
}

// GetListenURL returns the listen URL for the server (for testing).
func (rs *RPCServer) GetListenURL() string {
	return rs.rpcServer.GetListenURL()
}

// NewRPCServer creates a new instance of CKM
func NewRPCServer(serverName, listenURL string, cm *certmgr.CertificateMgr) (*RPCServer, error) {
	if serverName == "" || cm == nil {
		return nil, errors.New("All parameters are required")
	}
	// empty url might be valid, so we let rpckit check it and return an error if it is not

	// create an RPC server
	rpcServer, err := rpckit.NewRPCServer(serverName, listenURL)
	if err != nil {
		return nil, errors.Wrap(err, "Error creating rpc server")
	}

	// Instantiate handlers for certificates API
	cs, err := NewCertificatesRPCServer(cm)
	if err != nil {
		return nil, errors.Wrap(err, "Error certificates rpc server")
	}

	// register the RPC handlers and start the server
	ckmproto.RegisterCertificatesServer(rpcServer.GrpcServer, cs)
	rpcServer.Start()

	// create rpc server object
	server := RPCServer{
		certificateMgr: cm,
		rpcServer:      rpcServer,
	}

	return &server, nil
}
