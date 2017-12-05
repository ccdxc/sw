// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package ckm

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/ctrler/ckm/rpcserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/keymgr"
)

// CKM is the Venice component responsible for all aspects of Key Management

// CKMctrler is an instance of CKM
type CKMctrler struct {
	// CertificateMgr deals with creating, storing and rotating certificates
	CertMgr *certmgr.CertificateMgr

	// RPCServer is the gRPC endpoint that exposes CKM APIs to cluster members
	RPCServer *rpcserver.RPCServer
}

// NewCKMctrler returns a controller instance
func NewCKMctrler(serverURL, keyStoreDir string) (*CKMctrler, error) {
	// create key manager instance
	be, err := keymgr.NewDefaultBackend("ckm")
	defer func() {
		if err != nil {
			be.Close()
		}
	}()
	if err != nil {
		return nil, errors.Wrap(err, "Error instantiating key manager backend")
	}
	keymgr, err := keymgr.NewKeyMgr(be)
	if err != nil {
		return nil, errors.Wrap(err, "Error instantiating key manager")
	}

	// create certificate manager
	cm, err := certmgr.NewCertificateMgr(keymgr)
	if err != nil {
		return nil, errors.Wrap(err, "Error instantiating certificate manager")
	}
	err = cm.StartCa(true)
	if err != nil {
		return nil, errors.Wrap(err, "Error starting certificate manager CA")
	}

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(globals.CKMEndpointName, serverURL, cm)
	if err != nil {
		return nil, errors.Wrap(err, "Error instantiating RPC server")
	}

	// create the controller instance
	ctrler := CKMctrler{
		CertMgr:   cm,
		RPCServer: rpcServer,
	}

	return &ctrler, err
}
