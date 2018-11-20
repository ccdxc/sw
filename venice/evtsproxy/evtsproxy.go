// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsproxy

import (
	"fmt"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/writers"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// WriterType represents different writer types (venice, syslog, etc.)
type WriterType uint

const (
	// Venice represents the venice writer
	Venice WriterType = 0
)

// String returns the string name of the writer
func (w WriterType) String() string {
	switch w {
	case Venice:
		return "venice"
	}

	return ""
}

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

	// events dispatcher which dispatches the events to destinations (venice, syslog, etc..)
	// after applying de-duplication and batching
	evtsDispatcher events.Dispatcher

	// resolver to connect with events manager or other services
	resolverClient resolver.Interface

	logger log.Logger // logger
}

// NewEventsProxy creates and returns a events proxy instance
func NewEventsProxy(serverName, serverURL string, resolverClient resolver.Interface, dedupInterval, batchInterval time.Duration,
	eventsStoreDir string, logger log.Logger) (*EventsProxy, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) || logger == nil {
		return nil, errors.New("serverName, serverURL and logger is required")
	}

	// create the events dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(dedupInterval, batchInterval, eventsStoreDir, logger)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating events proxy RPC server")
	}

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(serverName, serverURL, evtsDispatcher, logger)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating events proxy RPC server")
	}

	return &EventsProxy{
		RPCServer:      rpcServer,
		evtsDispatcher: evtsDispatcher,
		resolverClient: resolverClient,
		logger:         logger,
	}, nil

}

// StartDispatch starts dispatching events to the registered writers
func (ep *EventsProxy) StartDispatch() {
	if ep.evtsDispatcher != nil {
		ep.evtsDispatcher.Start()
	}
}

// Stop stops events proxy
func (ep *EventsProxy) Stop() {
	if ep.RPCServer != nil {
		ep.RPCServer.Stop()
		ep.RPCServer = nil
	}

	if ep.evtsDispatcher != nil {
		ep.evtsDispatcher.Shutdown()
		ep.evtsDispatcher = nil
	}
}

// GetEventsDispatcher returns the underlying events dispatcher
func (ep *EventsProxy) GetEventsDispatcher() events.Dispatcher {
	return ep.evtsDispatcher
}

// RegisterEventsWriter creates the writer of given type and registers it with the dispatcher
func (ep *EventsProxy) RegisterEventsWriter(writerType WriterType, config interface{}) error {
	switch writerType {
	case Venice:
		var evtsMgrURL string
		if config != nil {
			var ok bool
			if evtsMgrURL, ok = config.(string); !ok {
				ep.logger.Errorf("failed to read venice writer config, %v", config)
			}
		}

		veniceWriter, err := writers.NewVeniceWriter(writerType.String(), writerChLen, evtsMgrURL, ep.resolverClient, ep.logger)
		if err != nil {
			return err
		}

		// register venice writer
		eventsChan, offsetTracker, err := ep.evtsDispatcher.RegisterWriter(veniceWriter)
		if err != nil {
			return err
		}

		// start all the workers
		veniceWriter.Start(eventsChan, offsetTracker)
	default:
		return fmt.Errorf("unsupported writer type: %v", writerType)
	}

	return nil
}
