// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsproxy

import (
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/writers"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	writerChLen = 1000
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
func NewEventsProxy(serverName, serverURL, evtsMgrURL string, dedupInterval, batchInterval time.Duration, eventsStoreDir string, logger log.Logger) (*EventsProxy, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) || utils.IsEmpty(evtsMgrURL) || logger == nil {
		return nil, errors.New("all parameters are required")
	}

	// create the events dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(dedupInterval, batchInterval, eventsStoreDir, logger)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating events proxy RPC server")
	}

	// add venice writer
	if err = addDefaultWriters(evtsDispatcher, evtsMgrURL, logger); err != nil {
		return nil, errors.Wrap(err, "failed to register default writers with the dispatcher")
	}

	// start processing any pending/failed events
	evtsDispatcher.ProcessFailedEvents()

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(serverName, serverURL, evtsDispatcher, logger)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating events proxy RPC server")
	}

	return &EventsProxy{
		RPCServer: rpcServer,
	}, nil
}

// addDefaultWriters registers default writer with the dispatcher
func addDefaultWriters(dispatcher events.Dispatcher, evtsMgrURL string, logger log.Logger) error {
	veniceWriter, err := writers.NewVeniceWriter("venice_writer", writerChLen, evtsMgrURL, logger)
	if err != nil {
		return err
	}

	// register venice writer
	eventsChan, offsetTracker, err := dispatcher.RegisterWriter(veniceWriter)
	if err != nil {
		return err
	}

	// start all the workers
	veniceWriter.Start(eventsChan, offsetTracker)
	return nil
}
