// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsproxy

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// ExporterType represents different exporter types (venice, syslog, etc.)
type ExporterType uint

const (
	// Venice represents the venice events exporter
	Venice ExporterType = 0

	// Syslog represents the exporter exporting to external syslog server(s)
	Syslog ExporterType = 1
)

// String returns the string name of the exporter
func (w ExporterType) String() string {
	switch w {
	case Venice:
		return "venice"
	}

	return ""
}

var (
	// len of the exporter channel
	exporterChLen = 1000
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

	// map storing list of registered exporters; used to stop the exporters when proxy stops
	sync.Mutex
	exporters map[string]events.Exporter

	// context details
	ctx        context.Context
	cancelFunc context.CancelFunc

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

	ctx, cancel := context.WithCancel(context.Background())

	return &EventsProxy{
		RPCServer:      rpcServer,
		evtsDispatcher: evtsDispatcher,
		resolverClient: resolverClient,
		exporters:      make(map[string]events.Exporter),
		logger:         logger,
		ctx:            ctx,
		cancelFunc:     cancel,
	}, nil

}

// StartDispatch starts dispatching events to the registered exporters
func (ep *EventsProxy) StartDispatch() {
	if ep.evtsDispatcher != nil {
		ep.evtsDispatcher.Start()
	}
}

// Stop stops events proxy
func (ep *EventsProxy) Stop() {
	ep.logger.Info("stopping event proxy server")
	if ep.RPCServer != nil {
		ep.RPCServer.Stop()
		ep.RPCServer = nil
	}

	if ep.evtsDispatcher != nil {
		ep.evtsDispatcher.Shutdown()
		ep.evtsDispatcher = nil
	}

	// stop all the exporters
	for _, exporter := range ep.exporters {
		exporter.Stop()
	}

	ep.cancelFunc() // this will exit the watcher go routine if it is running
}

// GetEventsDispatcher returns the underlying events dispatcher
func (ep *EventsProxy) GetEventsDispatcher() events.Dispatcher {
	return ep.evtsDispatcher
}

// RegisterEventsExporter creates the exporter of given type and registers it with the dispatcher
func (ep *EventsProxy) RegisterEventsExporter(exporterType ExporterType, config interface{}) (events.Exporter, error) {
	switch exporterType {
	case Venice:
		exporterConfig := &exporters.VeniceExporterConfig{}
		if config != nil {
			var ok bool
			if exporterConfig, ok = config.(*exporters.VeniceExporterConfig); !ok {
				ep.logger.Errorf("failed to read venice exporter config, %v", config)
			}
		}

		veniceExporter, err := exporters.NewVeniceExporter("venice", exporterChLen, exporterConfig.EvtsMgrURL, ep.resolverClient, ep.logger)
		if err != nil {
			return nil, err
		}

		if err := ep.registerExporter("venice", veniceExporter); err != nil {
			return nil, err
		}

		return veniceExporter, nil
	case Syslog:
		exporterConfig := &exporters.SyslogExporterConfig{}
		if config != nil {
			var ok bool
			if exporterConfig, ok = config.(*exporters.SyslogExporterConfig); !ok {
				ep.logger.Errorf("failed to read syslog exporter config, %v", config)
			}
		}

		syslogExporter, err := exporters.NewSyslogExporter(exporterConfig.Name, exporterChLen, exporterConfig.Writers, ep.logger)
		if err != nil {
			return nil, err
		}

		if err := ep.registerExporter(exporterConfig.Name, syslogExporter); err != nil {
			return nil, err
		}

		return syslogExporter, nil
	default:
		return nil, fmt.Errorf("unsupported exporter type: %v", exporterType)
	}
}

// UnregisterEventsExporter unregisters the given exporter from events dispatcher
func (ep *EventsProxy) UnregisterEventsExporter(name string) {
	ep.evtsDispatcher.UnregisterExporter(name)
	ep.Lock()
	delete(ep.exporters, name)
	ep.Unlock()
}

// helper function to register the exporter with dispatcher
func (ep *EventsProxy) registerExporter(name string, exporter events.Exporter) error {
	// register the given exporter
	eventsChan, offsetTracker, err := ep.evtsDispatcher.RegisterExporter(exporter)
	if err != nil {
		return err
	}

	// start all the workers
	exporter.Start(eventsChan, offsetTracker)
	ep.Lock()
	ep.exporters[name] = exporter
	ep.Unlock()
	return nil
}
