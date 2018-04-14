// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsmgr

import (
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	// maxRetries maximum number of retries for fetching elasticsearch URLs
	// and creating client.
	maxRetries = 60

	// delay between retries
	retryDelay = 2 * time.Second
)

// EventsManager instance of events manager; responsible for all aspects of events
// including management of elastic connections.
type EventsManager struct {
	// RPCServer that exposes the server implementation of event manager APIs
	RPCServer *rpcserver.RPCServer
}

// NewEventsManager returns a events manager/controller instance
func NewEventsManager(serverName, serverURL string, resolverClient resolver.Interface,
	logger log.Logger) (*EventsManager, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) || resolverClient == nil || logger == nil {
		return nil, errors.New("all parameters are required")
	}

	// create elastic client
	result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
		return elastic.NewClient("", resolverClient, logger.WithContext("submodule", "elastic"))
	}, retryDelay, maxRetries)
	if err != nil {
		logger.Errorf("failed to create elastic client, err: %v", err)
		return nil, err
	}
	logger.Debugf("created elastic client")
	esClient := result.(elastic.ESClient)

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(serverName, serverURL, esClient)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating RPC server")
	}

	return &EventsManager{
		RPCServer: rpcServer,
	}, nil
}
