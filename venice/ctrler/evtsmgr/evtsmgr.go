// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsmgr

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
)

// EventsManager instance of events manager; responsible for all aspects of events
// including management of elastic connections.
type EventsManager struct {
	// RPCServer that exposes the server implementation of event manager APIs
	RPCServer *rpcserver.RPCServer
}

// NewEventsManager returns a events manager/controller instance
func NewEventsManager(serverName, serverURL, elasticURL string, logger log.Logger) (*EventsManager, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) || utils.IsEmpty(elasticURL) {
		return nil, errors.New("all parameters are required")
	}

	esClient, err := elastic.NewClient(elasticURL, logger.WithContext("submodule", "elastic"))
	if err != nil {
		return nil, errors.Wrap(err, "error creating ES client")
	}

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(serverName, serverURL, esClient)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating RPC server")
	}

	return &EventsManager{
		RPCServer: rpcServer,
	}, nil
}
