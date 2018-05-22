// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package writers

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	emgrpc "github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/evtsmgrproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	maxRetry = 15
	pkgName  = "venice_events_writer"
)

// Proxy through dispatcher -> Venice Writer -> Events Manager -> Elasticsearch

// VeniceWriter represents the venice writer
type VeniceWriter struct {
	// writer details
	name   string     // name of the writer
	chLen  int        // buffer or channel len
	logger log.Logger // logger

	// to connect with the events manager
	eventsMgr *eventsMgr // event manager

	// to receive events from the proxy (dispatcher)
	eventsChan          events.Chan          // channel to receive
	eventsOffsetTracker events.OffsetTracker // to track events file offset - bookmark indicating events till this point are processed successfully by this writer

	// to stop the writer
	stop     sync.Once      // for stopping the writer
	wg       sync.WaitGroup // to wait for the workers to finish
	shutdown chan struct{}  // to send shutdown signal to the writer
}

// eventsMgr encapsulates all the events manager details including connection string
// and context to be used while making calls.
type eventsMgr struct {
	sync.Mutex
	url             string                  // events manager gPRC listen URL
	rpcClient       *rpckit.RPCClient       // RPC client to connect with events manager
	client          emgrpc.EvtsMgrAPIClient // client to connect with events manager
	ctx             context.Context         // context to use for events manager gRPC calls
	connectionAlive bool                    // represents the connection status; alive or dead
}

// NewVeniceWriter creates a venice writer which sends events to events manager
func NewVeniceWriter(name string, chLen int, evtsMgrURL string,
	logger log.Logger) (events.Writer, error) {
	if utils.IsEmpty(name) || utils.IsEmpty(evtsMgrURL) || chLen <= 0 {
		return nil, fmt.Errorf("all parameters are required")
	}

	veniceWriter := &VeniceWriter{
		name:   name,
		chLen:  chLen,
		logger: logger.WithContext("submodule", pkgName),
		eventsMgr: &eventsMgr{
			url: evtsMgrURL,
			ctx: context.Background(),
		},
		shutdown: make(chan struct{}, 1),
	}

	// create events manager client
	if err := veniceWriter.initEvtsMgrGrpcClient(maxRetry); err != nil {
		veniceWriter.logger.Errorf("failed to create events manager client, err: %v", err)
		return nil, err
	}

	return veniceWriter, nil
}

// Start start the daemon go routines
func (v *VeniceWriter) Start(eventsCh events.Chan, offsetTracker events.OffsetTracker) {
	v.eventsChan = eventsCh
	v.eventsOffsetTracker = offsetTracker

	// start watching events using the given event channel
	v.wg.Add(1)
	go v.receiveEvents()

	v.logger.Info("started venice events writer")
}

// Stop stops the writer
func (v *VeniceWriter) Stop() {
	v.logger.Infof("stopping the venice writer")
	v.stop.Do(func() {
		if v.eventsChan != nil {
			v.eventsChan.Stop()
		}

		// wait for the writer to finish
		close(v.shutdown)
		v.wg.Wait()

		// close events manager client
		if v.eventsMgr.rpcClient != nil {
			v.eventsMgr.rpcClient.Close()
		}
	})
}

// Name returns the name of the writer
func (v *VeniceWriter) Name() string {
	return v.name
}

// ChLen returns the channel length; will be used to dispatcher to set channel len
func (v *VeniceWriter) ChLen() int {
	return v.chLen
}

// WriteEvents writes list of events to the venice events manager service
func (v *VeniceWriter) WriteEvents(events []*monitoring.Event) error {
	v.eventsMgr.Lock()
	if !v.eventsMgr.connectionAlive {
		v.eventsMgr.Unlock()
		return fmt.Errorf("failed to send event; connection unavailable")
	}
	v.eventsMgr.Unlock()

	// send events to events manager
	_, err := v.eventsMgr.client.SendEvents(v.eventsMgr.ctx, &monitoring.EventsList{Events: events})
	if err == nil {
		return nil
	}

	// check if the connection needs to be reset
	if strings.Contains(err.Error(), "Unavailable") {
		v.eventsMgr.Lock()
		v.eventsMgr.connectionAlive = false
		v.eventsMgr.Unlock()
		go v.reconnect()
	}

	return err
}

// GetLastProcessedOffset returns the last bookmarked offset by this writer
func (v *VeniceWriter) GetLastProcessedOffset() (int64, error) {
	return v.eventsOffsetTracker.GetOffset()
}

// startWorker watches the events using the event channel from dispatcher.
func (v *VeniceWriter) receiveEvents() {
	defer v.wg.Done()
	for {
		select {
		// this channel will be closed once the evenChan receives the stop signal from
		// this writer or when dispatcher shuts down.
		case batch, ok := <-v.eventsChan.ResultChan():
			if !ok { // channel closed
				return
			}

			// all the incoming batch of events needs to be processed in order to avoid losing track of events
			for {
				if err := v.WriteEvents(batch.GetEvents()); err != nil {
					v.logger.Debugf("failed to send events to the events manager, retrying.. err: %v", err)
					time.Sleep(1 * time.Second)
					continue
				}

				// successfully sent the event to events manager
				v.eventsOffsetTracker.UpdateOffset(batch.GetOffset())
				break
			}
		case <-v.shutdown:
			return
		}
	}
}

// reconnect helper function to re-establish the connection with events manager
func (v *VeniceWriter) reconnect() {
	for {
		if err := v.eventsMgr.rpcClient.Reconnect(); err != nil {
			log.Debugf("failed to reconnect to events manager, retrying.. err: %v", err)
			time.Sleep(1 * time.Second)
			continue
		}

		v.eventsMgr.Lock()
		defer v.eventsMgr.Unlock()
		v.eventsMgr.client = emgrpc.NewEvtsMgrAPIClient(v.eventsMgr.rpcClient.ClientConn)
		v.eventsMgr.connectionAlive = true
		log.Info("reconnnected with events manager")
		return
	}
}

// init creates events manager grpc client
func (v *VeniceWriter) initEvtsMgrGrpcClient(retry int) error {
	for i := 0; i < retry; i++ {
		// create a grpc client
		client, err := rpckit.NewRPCClient(pkgName, v.eventsMgr.url, rpckit.WithRemoteServerName(globals.EvtsMgr))
		if err != nil {
			time.Sleep(2 * time.Second)
			v.logger.Warnf("failed to connect to {%s}, err: %v, retry", globals.EvtsMgr, err)
			continue
		}

		v.eventsMgr.rpcClient = client
		v.eventsMgr.client = emgrpc.NewEvtsMgrAPIClient(client.ClientConn)
		v.eventsMgr.connectionAlive = true
		return nil
	}

	return fmt.Errorf("failed to connect to {%s}, exhausted all attempts (%d)", globals.EvtsMgr, retry)
}
