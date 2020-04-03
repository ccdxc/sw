// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsproxy

import (
	"context"
	"fmt"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

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

	// context details
	ctx        context.Context
	cancelFunc context.CancelFunc

	logger log.Logger // logger
}

// EventsProxyOptions represents the custom options for events proxy
type EventsProxyOptions struct {
	// default object ref to be included in the event that do not carry object-ref
	defaultObjectRef *api.ObjectRef

	// used to suppress non-upgrade related events during upgrade process
	maintenanceMode bool
}

// Option fills the optional params for events proxy
type Option func(epo *EventsProxyOptions)

// WithDefaultObjectRef passes a custom object ref for events going through this proxy
func WithDefaultObjectRef(objRef *api.ObjectRef) Option {
	return func(epo *EventsProxyOptions) {
		epo.defaultObjectRef = objRef
	}
}

// WithMaintenanceMode passes a maintenance mode flag
func WithMaintenanceMode(flag bool) Option {
	return func(epo *EventsProxyOptions) {
		epo.maintenanceMode = flag
	}
}

// NewEventsProxy creates and returns a events proxy instance
func NewEventsProxy(nodeName, serverName, serverURL string, resolverClient resolver.Interface, dedupInterval, batchInterval time.Duration,
	storeConfig *events.StoreConfig, logger log.Logger, opts ...Option) (*EventsProxy, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) || utils.IsEmpty(nodeName) || logger == nil {
		return nil, errors.New("serverName, serverURL, nodeName and logger is required")
	}

	epo := &EventsProxyOptions{}
	for _, opt := range opts {
		opt(epo)
	}

	// create the events dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(nodeName, dedupInterval, batchInterval, storeConfig,
		epo.defaultObjectRef, logger, dispatcher.WithMaintenanceMode(epo.maintenanceMode))
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

// SetMaintenanceMode sets the maintenance flag.
// certain events will be suppressed if the maintenance flag is set.
func (ep *EventsProxy) SetMaintenanceMode(flag bool) {
	if ep.evtsDispatcher != nil {
		ep.evtsDispatcher.SetMaintenanceMode(flag)
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

	ep.cancelFunc() // this will exit the watcher go routine if it is running
}

// GetEventsDispatcher returns the underlying events dispatcher
func (ep *EventsProxy) GetEventsDispatcher() events.Dispatcher {
	return ep.evtsDispatcher
}

// RegisterEventsExporter creates the exporter of given type and registers it with the dispatcher
func (ep *EventsProxy) RegisterEventsExporter(exporterType exporters.Type, config interface{}) (events.Exporter, error) {
	switch exporterType {
	case exporters.Venice:
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

		if err := ep.registerExporter(veniceExporter); err != nil {
			// close connections that got established to evtsmgr if any
			veniceExporter.Stop()
			return nil, err
		}

		return veniceExporter, nil
	case exporters.Syslog:
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

		if err := ep.registerExporter(syslogExporter); err != nil {
			syslogExporter.Stop()
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
}

// DeleteEventsExporter deletes the given exporter from events dispatcher
func (ep *EventsProxy) DeleteEventsExporter(name string) {
	ep.evtsDispatcher.DeleteExporter(name)
}

// helper function to register the exporter with dispatcher
func (ep *EventsProxy) registerExporter(exporter events.Exporter) error {
	// register the given exporter
	eventsChan, offsetTracker, err := ep.evtsDispatcher.RegisterExporter(exporter)
	if err != nil {
		return err
	}

	// start all the workers
	exporter.Start(eventsChan, offsetTracker)
	return nil
}
