// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsproxy

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
)

// EventsProxy instance of events proxy; responsible for all aspects of managing
// proxy server and it's functionalities:
// 1. receive event from recorder
// 2. forward it to dispatcher
type EventsProxy struct {
	// RPCServer that exposes the server implementation of event proxy APIs
	RPCServer *rpcserver.RPCServer
}

// NewEventsProxy creates and returns a events proxy instance
func NewEventsProxy(serverName, serverURL string, logger log.Logger) (*EventsProxy, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) {
		return nil, errors.New("all parameters are required")
	}

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(serverName, serverURL, logger)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating events proxy RPC server")
	}

	return &EventsProxy{
		RPCServer: rpcServer,
	}, nil
}
