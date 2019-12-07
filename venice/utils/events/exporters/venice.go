// {C} Copyright 2017-2019 Pensando Systems Inc. All rights reserved.

package exporters

import (
	"context"
	"fmt"
	"sync"
	"time"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	evtsapi "github.com/pensando/sw/api/generated/events"
	emgrpc "github.com/pensando/sw/nic/agent/protos/evtprotos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	maxRetry = 60
)

// Proxy through dispatcher -> Venice exporter -> Events Manager -> Elasticsearch

// VeniceExporter represents the venice exporter
type VeniceExporter struct {
	// exporter details
	name   string     // name of the exporter
	chLen  int        // buffer or channel len
	logger log.Logger // logger

	// to connect with the events manager
	eventsMgr *eventsMgr // event manager

	// to receive events from the proxy (dispatcher)
	eventsChan          events.Chan          // channel to receive
	eventsOffsetTracker events.OffsetTracker // to track events file offset - bookmark indicating events till this point are processed successfully by this writer

	// context to use for any outgoing calls and to stop exporter go routines
	ctx        context.Context
	cancelFunc context.CancelFunc

	// to stop the exporter
	stop     sync.Once      // for stopping the exporter
	wg       sync.WaitGroup // to wait for the worker to finish
	shutdown chan struct{}  // to send shutdown signal to the exporter
}

// eventsMgr encapsulates all the events manager details including connection string
// and context to be used while making calls. Either URL or resolver client is used to establish
// connection with events manager
type eventsMgr struct {
	sync.Mutex
	url             string                  // events manager gPRC listen URL
	resolverClient  resolver.Interface      // resolver
	rpcClient       *rpckit.RPCClient       // RPC client to connect with events manager
	client          emgrpc.EvtsMgrAPIClient // client to connect with events manager
	connectionAlive bool                    // represents the connection status; alive or dead
}

// NewVeniceExporter creates a venice exporter which sends events to events manager
func NewVeniceExporter(name string, chLen int, evtsMgrURL string, resolverClient resolver.Interface,
	logger log.Logger) (events.Exporter, error) {
	if utils.IsEmpty(name) || chLen <= 0 || logger == nil {
		return nil, fmt.Errorf("exporter name, channel length and logger is required")
	}

	if (!utils.IsEmpty(evtsMgrURL) && resolverClient != nil) || (utils.IsEmpty(evtsMgrURL) && resolverClient == nil) {
		return nil, fmt.Errorf("provide either evtsMgrURL or resolverClient")
	}

	ctx, cancelFunc := context.WithCancel(context.Background())
	veniceExporter := &VeniceExporter{
		name:   name,
		chLen:  chLen,
		logger: logger.WithContext("pkg", name),
		eventsMgr: &eventsMgr{
			url:            evtsMgrURL,
			resolverClient: resolverClient,
		},
		ctx:        ctx,
		cancelFunc: cancelFunc,
		shutdown:   make(chan struct{}, 1),
	}

	// create events manager client
	if err := veniceExporter.initEvtsMgrGrpcClient(maxRetry); err != nil {
		veniceExporter.logger.Errorf("failed to create events manager client, err: %v", err)
		return nil, err
	}

	return veniceExporter, nil
}

// Start start the daemon go routines
func (v *VeniceExporter) Start(eventsCh events.Chan, offsetTracker events.OffsetTracker) {
	v.eventsChan = eventsCh
	v.eventsOffsetTracker = offsetTracker

	// start watching events using the given event channel
	v.wg.Add(1)
	go v.receiveEvents()

	v.logger.Infof("{exporter %s} started", v.name)
}

// Stop stops the exporter
func (v *VeniceExporter) Stop() {
	v.logger.Infof("{exporter %s} stopping", v.name)
	v.stop.Do(func() {
		if v.eventsChan != nil {
			v.eventsChan.Stop()
		}

		// cancel exporter context
		v.cancelFunc()

		// wait for the exporter to finish
		close(v.shutdown)

		v.wg.Wait()

		// close events manager client
		if v.eventsMgr.rpcClient != nil {
			v.eventsMgr.rpcClient.Close()
		}

		if fileOffset, err := v.GetLastProcessedOffset(); err != nil {
			v.logger.Debugf("exporter {%s} failed to get offset, err: %v", v.name, err)
		} else {
			v.logger.Debugf("exporter {%s} stopping at offset: {%v: %v}", v.name, fileOffset.Filename,
				fileOffset.BytesRead)
		}
	})
}

// AddWriter adds the given writer to venice exporter
func (v *VeniceExporter) AddWriter(writer interface{}) {
}

// Name returns the name of the exporter
func (v *VeniceExporter) Name() string {
	return v.name
}

// ChLen returns the channel length; will be used to dispatcher to set channel len
func (v *VeniceExporter) ChLen() int {
	return v.chLen
}

// WriteEvents writes list of events to the venice events manager service
func (v *VeniceExporter) WriteEvents(events []*evtsapi.Event) error {
	v.eventsMgr.Lock()
	if !v.eventsMgr.connectionAlive {
		v.eventsMgr.Unlock()
		return fmt.Errorf("failed to send event; connection unavailable")
	}
	v.eventsMgr.Unlock()

	// send events to events manager
	ctx, cancel := context.WithTimeout(v.ctx, 200*time.Second)
	defer cancel()
	_, err := v.eventsMgr.client.SendEvents(ctx, &evtsapi.EventList{Items: events})
	if err == nil {
		return nil
	}

	// check if the connection needs to be reset
	errStatus, ok := status.FromError(err)
	if ok && (errStatus.Code() == codes.Unavailable || errStatus.Code() == codes.Internal) {
		v.eventsMgr.Lock()
		v.eventsMgr.connectionAlive = false
		v.eventsMgr.Unlock()
		go v.reconnect()
	}

	return err
}

// GetLastProcessedOffset returns the last bookmarked offset by this exporter
func (v *VeniceExporter) GetLastProcessedOffset() (*events.Offset, error) {
	if v.eventsOffsetTracker != nil {
		return v.eventsOffsetTracker.GetOffset()
	}

	return nil, fmt.Errorf("offset tracker not available")
}

// startWorker watches the events using the event channel from dispatcher.
func (v *VeniceExporter) receiveEvents() {
	defer v.wg.Done()
	for {
		select {
		// this channel will be closed once the evenChan receives the stop signal from
		// this exporter or when dispatcher shuts down.
		case batch, ok := <-v.eventsChan.ResultChan():
			if !ok { // channel closed
				v.logger.Errorf("{exporter %s} exiting events receiver channel", v.name)
				return
			}

			v.logger.Infof("{exporter %s} received events batch, len: %v", v.name, len(batch.GetEvents()))
			v.logger.Debugf("{exporter %s} received events: %v, offset: %v", v.name, events.Minify(batch.GetEvents()), batch.GetOffset())

			// all the incoming batch of events needs to be processed in order to avoid losing track of events
			for {
				if err := v.ctx.Err(); err != nil {
					v.logger.Errorf("{exporter %s} context closed, err: %v", v.name, err)
					return
				}

				if err := v.WriteEvents(batch.GetEvents()); err != nil {
					v.logger.Errorf("{exporter %s} failed to send events to the events manager, retrying.. err: %v",
						v.name, err)
					time.Sleep(1 * time.Second)
					continue
				}

				// successfully sent the event(s) to events manager
				v.eventsOffsetTracker.UpdateOffset(batch.GetOffset())
				break
			}
			v.logger.Infof("{exporter %s} done processing events batch", v.name)
		case <-v.shutdown:
			v.logger.Errorf("{exporter %s} shutting down events receiver channel", v.name)
			return
		}
	}
}

// reconnect helper function to re-establish the connection with events manager
func (v *VeniceExporter) reconnect() {
	var client *rpckit.RPCClient
	var err error
	for {
		if !utils.IsEmpty(v.eventsMgr.url) {
			v.logger.Infof("{exporter %s} reconnecting to events manager client using URL: {%v}", v.name, v.eventsMgr.url)
			client, err = rpckit.NewRPCClient(v.name, v.eventsMgr.url, rpckit.WithRemoteServerName(globals.EvtsMgr), rpckit.WithLogger(v.logger))
		} else {
			v.logger.Infof("{exporter %s} reconnecting to events manager client using resolver", v.name)
			client, err = rpckit.NewRPCClient(v.name, globals.EvtsMgr, rpckit.WithBalancer(balancer.New(v.eventsMgr.resolverClient)), rpckit.WithLogger(v.logger))
		}
		if err != nil {
			log.Errorf("{exporter %s} failed to reconnect to events manager, retrying.. err: %v", v.name, err)
			time.Sleep(1 * time.Second)
			continue
		}

		v.eventsMgr.Lock()
		v.eventsMgr.rpcClient = client
		v.eventsMgr.client = emgrpc.NewEvtsMgrAPIClient(v.eventsMgr.rpcClient.ClientConn)
		v.eventsMgr.connectionAlive = true
		log.Infof("{exporter %s} reconnected with events manager", v.name)
		v.eventsMgr.Unlock()
		return
	}
}

// init creates events manager grpc client
func (v *VeniceExporter) initEvtsMgrGrpcClient(maxRetries int) error {
	var client interface{}
	var err error

	if !utils.IsEmpty(v.eventsMgr.url) {
		v.logger.Infof("{exporter %s} creating events manager client using URL: {%v}", v.name, v.eventsMgr.url)
		client, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			return rpckit.NewRPCClient(v.name, v.eventsMgr.url, rpckit.WithRemoteServerName(globals.EvtsMgr),
				rpckit.WithLogger(v.logger))
		}, 2*time.Second, maxRetries)
	} else { // use resolver client
		v.logger.Infof("{exporter %s} creating events manager client using resolver", v.name)
		client, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			return rpckit.NewRPCClient(v.name, globals.EvtsMgr, rpckit.WithBalancer(balancer.New(v.eventsMgr.resolverClient)),
				rpckit.WithLogger(v.logger))
		}, 2*time.Second, maxRetries)
	}

	if err != nil {
		return fmt.Errorf("failed to connect to {%s}, err: %v", globals.EvtsMgr, err)
	}

	v.eventsMgr.rpcClient = client.(*rpckit.RPCClient)
	v.eventsMgr.client = emgrpc.NewEvtsMgrAPIClient(v.eventsMgr.rpcClient.ClientConn)
	v.eventsMgr.connectionAlive = true
	return nil
}
