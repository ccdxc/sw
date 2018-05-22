// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	epgrpc "github.com/pensando/sw/venice/evtsproxy/rpcserver/evtsproxyproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/writers"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	testServerURL = "localhost:0"

	logger = log.GetNewLogger(log.GetDefaultConfig("evtsproxy_server_test"))

	source = &monitoring.EventSource{NodeName: "test", Component: "test"}

	event = monitoring.Event{
		TypeMeta: api.TypeMeta{
			Kind: "Event",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
		},
		EventAttributes: monitoring.EventAttributes{
			Source:   source,
			Severity: "INFO",
			Type:     "DUMMY",
			Count:    1,
		},
	}

	mockWriterChLen = 30

	testDedupInterval = 100 * time.Second
	testSendInterval  = 10 * time.Millisecond

	eventsDir = "/tmp"
)

// setup helper function to create RPC server and client instances
func setup(t *testing.T, eventsStorePath string, dedupInterval, batchInterval time.Duration) (*RPCServer,
	*rpckit.RPCClient, *writers.MockWriter, epgrpc.EventsProxyAPIClient) {
	// create dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(dedupInterval, batchInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")

	// create mock writer; register it with the dispatcher and start the writer
	mockWriter := writers.NewMockWriter("mock", mockWriterChLen, logger)
	mockEventsCh, offsetTracker, err := evtsDispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer")
	mockWriter.Start(mockEventsCh, offsetTracker)

	// create grpc server
	rpcServer, err := NewRPCServer(globals.EvtsProxy, testServerURL, evtsDispatcher, logger)
	AssertOk(t, err, "failed to create rpc server")
	testServerURL := rpcServer.GetListenURL()

	// create grpc client
	rpcClient, err := rpckit.NewRPCClient(globals.EvtsProxy, testServerURL)
	AssertOk(t, err, "failed to create rpc client")
	proxyClient := epgrpc.NewEventsProxyAPIClient(rpcClient.ClientConn)
	Assert(t, proxyClient != nil, "failed to created events proxy client")

	return rpcServer, rpcClient, mockWriter, proxyClient
}

// TestEventsProxyRPCServer tests events proxy server
func TestEventsProxyRPCServer(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	rpcServer, rpcClient, mockWriter, proxyClient := setup(t, eventsStorePath, testDedupInterval, testSendInterval)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()

	ctx := context.Background()

	// ensure the handlers work
	_, err := proxyClient.ForwardEvent(ctx, &monitoring.Event{})
	Assert(t, err != nil, "missing event attributes; expected failure")

	_, err = proxyClient.ForwardEvents(ctx, &monitoring.EventsList{})
	AssertOk(t, err, "failed to forward events")

	// send valid events
	for i := 0; i < 10; i++ {
		tmpEvent := event
		tmpEvent.EventAttributes.Message = fmt.Sprintf("event %d", i)
		tmpEvent.ObjectMeta.UUID = uuid.NewV4().String()
		_, err := proxyClient.ForwardEvent(ctx, &tmpEvent)
		AssertOk(t, err, "failed to forward event")
	}

	// make sure the writer received all the events;
	// ensure all the events are received and they're deduped
	AssertEventually(t, func() (bool, interface{}) {
		numEvents := mockWriter.GetTotalEvents()
		if numEvents == 10 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: %d, got: %v", 10, numEvents)
	}, "did not receive all the events produced", string("20ms"), string("2s"))
}

// TestDedupedEvents tests to make sure the events are deduped before sent to the writers.
func TestDedupedEvents(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	rpcServer, rpcClient, mockWriter, proxyClient := setup(t, eventsStorePath, testDedupInterval, testSendInterval)
	defer rpcServer.Stop()
	// all the writers will be stopped once the server stops -> dispatcher shutdown
	// so, this is just to make sure that it doesn't break when `Stop` is called multiple times.
	defer mockWriter.Stop()
	defer rpcClient.ClientConn.Close()

	ctx := context.Background()

	// send valid events
	evtUUIDs := []string{}
	for i := 0; i < 10; i++ {
		events := make([]*monitoring.Event, 10)
		// except the first event; the remaining 9 should be compressed
		event.EventAttributes.Message = fmt.Sprintf("event %d", i)
		event.ObjectMeta.UUID = uuid.NewV4().String()
		evtUUIDs = append(evtUUIDs, event.GetUUID())

		for j := 0; j < 10; j++ {
			tmpEvent := event
			events[j] = &tmpEvent
			event.ObjectMeta.UUID = uuid.NewV4().String() // update the UUID for each event
		}

		// forward all the events
		_, err := proxyClient.ForwardEvents(ctx, &monitoring.EventsList{Events: events})
		AssertOk(t, err, "failed to forward event")

	}

	for _, evtUUID := range evtUUIDs {
		// ensure all the events are received and they're deduped
		AssertEventually(t, func() (bool, interface{}) {
			evt := mockWriter.GetEventByUUID(evtUUID)
			if evt != nil && evt.GetCount() == 10 {
				return true, nil
			}

			return false, fmt.Sprintf("expected: %d, got: %v", 10, evt)
		}, "did not receive all the events produced", string("20ms"), string("2s"))
	}
}

// TestEventsProxyShutdown test ensures the events are flushed during shutdown
func TestEventsProxyShutdown(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// set large send interval (30s default) so that it is not hit before the flush call
	rpcServer, rpcClient, mockWriter, proxyClient := setup(t, eventsStorePath, 100*time.Second, 30*time.Second)
	defer rpcServer.Stop() // will shutdown the dispatcher and writers
	defer rpcClient.ClientConn.Close()

	ctx := context.Background()

	// send valid events
	evtUUIDs := []string{}
	for i := 0; i < 10; i++ {
		events := make([]*monitoring.Event, 10)
		// except the first event; the remaining 9 should be compressed
		event.EventAttributes.Message = fmt.Sprintf("event %d", i)
		event.ObjectMeta.UUID = uuid.NewV4().String()
		evtUUIDs = append(evtUUIDs, event.GetUUID())
		for j := 0; j < 10; j++ {
			tmpEvent := event
			events[j] = &tmpEvent
			event.ObjectMeta.UUID = uuid.NewV4().String() // update the UUID for each event
		}

		// forward all the events
		_, err := proxyClient.ForwardEvents(ctx, &monitoring.EventsList{Events: events})
		AssertOk(t, err, "failed to forward event")
	}

	// send interval is set large; no events received by the writer so far
	Assert(t, mockWriter.GetTotalEvents() == 0, "expected 0 events")

	// flush all the events
	rpcServer.handler.dispatcher.Shutdown()

	for _, evtUUID := range evtUUIDs {
		// ensure all the events are received and they're deduped
		AssertEventually(t, func() (bool, interface{}) {
			evt := mockWriter.GetEventByUUID(evtUUID)
			if evt != nil && evt.GetCount() == 10 {
				return true, nil
			}

			return false, fmt.Sprintf("expected: %d, got: %v", 10, evt)
		}, "did not receive all the events produced", string("20ms"), string("2s"))
	}
}

// TestEventsProxyRPCServerShutdown tests the graceful shutdown
func TestEventsProxyRPCServerShutdown(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	rpcServer, rpcClient, _, _ := setup(t, eventsStorePath, testDedupInterval, testSendInterval)

	go func() {
		// buffer to have `Done()` called before the stopping the server
		time.Sleep(60)
		rpcClient.ClientConn.Close()
		rpcServer.Stop()
	}()

	log.Info("waiting for the shutdown signal")
	<-rpcServer.Done()
	log.Infof("server stopped, exiting")
}

// TestEventsProxyRPCServerInstantiation tests the RPC server instantiation cases
func TestEventsProxyRPCServerInstantiation(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	dispatcher, err := dispatcher.NewDispatcher(time.Second, 10*time.Millisecond, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")

	// no listenURL name
	_, err = NewRPCServer("server-name", "", dispatcher, logger)
	Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no server name
	_, err = NewRPCServer("", "listen-url", dispatcher, logger)
	Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no server name and listen URL
	_, err = NewRPCServer("", "", dispatcher, logger)
	Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// nil dispatcher
	_, err = NewRPCServer("server-name", "listen-url", nil, logger)
	Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// nil logger
	_, err = NewRPCServer("server-name", "listen-url", dispatcher, nil)
	Assert(t, err != nil, "expected failure, RPCServer init succeeded")
}
