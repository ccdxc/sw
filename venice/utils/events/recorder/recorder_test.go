// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package recorder

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	epgrpc "github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/evtsproxy/rpcserver/evtsproxyproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/writers"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	TestNICDisconnected = "TestNICDisconnected"
	TestNICConnected    = "TestNICConnected"
)

var (
	testServerURL = "localhost:0"
	numEvents     = 1000

	testEventTypes = []string{
		TestNICDisconnected,
		TestNICConnected,
	}

	logger        = log.GetNewLogger(log.GetDefaultConfig("recorder_test"))
	mockBufferLen = 10

	testDedupInterval = 100 * time.Second
	testSendInterval  = 10 * time.Millisecond

	eventsDir = "/tmp"
)

// createEventsProxy helper function that creates events proxy RPC server and client
func createEventsProxy(t *testing.T, eventsStorePath string) (*epgrpc.RPCServer, *rpckit.RPCClient) {
	// create events dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(testDedupInterval, testSendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")

	// create mock writer
	mockWriter := writers.NewMockWriter("mock", mockBufferLen, logger)
	writerEventCh, offsetTracker, err := evtsDispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer")
	mockWriter.Start(writerEventCh, offsetTracker)

	// create grpc server
	rpcServer, err := epgrpc.NewRPCServer(globals.EvtsProxy, testServerURL, evtsDispatcher, logger)
	AssertOk(t, err, "failed to create rpc server")

	// create grpc client
	rpcClient, err := rpckit.NewRPCClient(globals.EvtsProxy, rpcServer.GetListenURL())
	AssertOk(t, err, "failed to create rpc client")

	return rpcServer, rpcClient
}

// TestEventsRecorder tests the events recorder (record event)
func TestEventsRecorder(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	// run events proxy server
	rpcServer, rpcClient := createEventsProxy(t, eventsStorePath)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()

	// create events proxy client
	proxyClient := evtsproxyproto.NewEventsProxyAPIClient(rpcClient.ClientConn)
	Assert(t, proxyClient != nil, "failed to created events proxy client")

	// create recorder
	recorder, err := NewRecorder(
		&monitoring.EventSource{NodeName: "test-node", Component: "test-component"},
		testEventTypes, rpcServer.GetListenURL())
	AssertOk(t, err, "failed to create events recorder")

	err = recorder.Event(TestNICDisconnected, "INFO", "test event - 1", nil)
	AssertOk(t, err, "failed to send event to the proxy")

	err = recorder.Event(TestNICConnected, "INFO", "test event - 2", nil)
	AssertOk(t, err, "failed to send event to the proxy")

	// send events using multiple workers and check if things are still intact
	wg := new(sync.WaitGroup)
	workers := runtime.NumCPU()
	each := numEvents / workers
	wg.Add(workers)

	// start the workers
	for i := 0; i < workers; i++ {
		go func(threadID int, recorder events.Recorder) {
			for j := 0; j < each; j++ {
				message := fmt.Sprintf("thread: %v; event: %v", j, threadID)
				recorder.Event(TestNICDisconnected, "CRITICAL", message, nil)
				recorder.Event(TestNICConnected, "INFO", message, nil)
			}
			wg.Done()
		}(i, recorder)
	}

	wg.Wait()
}

// TestInvalidEventInputs tests invalid event inputs (severity, type)
func TestInvalidEventInputs(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	// run events proxy server
	rpcServer, rpcClient := createEventsProxy(t, eventsStorePath)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()

	// create events proxy client
	proxyClient := evtsproxyproto.NewEventsProxyAPIClient(rpcClient.ClientConn)
	Assert(t, proxyClient != nil, "failed to created events proxy client")

	// create recorder
	recorder, err := NewRecorder(
		&monitoring.EventSource{NodeName: "test-node", Component: "test-component"},
		testEventTypes, rpcServer.GetListenURL())
	AssertOk(t, err, "failed to create events recorder")

	// test invalid event type
	err = recorder.Event("InvalidEvent", "INFO", "test event - 1", nil)
	Assert(t, err.Error() == events.NewError(events.ErrInvalidEventType, "").Error(),
		"expected invalid event type error, found:", err.Error())

	// test invalid severity
	err = recorder.Event(TestNICConnected, "INVALID", "test event - 1", nil)
	Assert(t, err.Error() == events.NewError(events.ErrInvalidSeverity, "").Error(),
		"expected invalid event type error, found:", err.Error())
}

// TestEventRecorderInstantiation tests event recorder instantiation cases
func TestEventRecorderInstantiation(t *testing.T) {
	// nil event source
	_, err := NewRecorder(nil, testEventTypes, testServerURL)
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")

	// nil event types
	_, err = NewRecorder(&monitoring.EventSource{}, nil, testServerURL)
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")

	// empty event types
	_, err = NewRecorder(&monitoring.EventSource{}, []string{}, testServerURL)
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")

	// empty events proxy URL
	_, err = NewRecorder(&monitoring.EventSource{}, testEventTypes, "")
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")
}

// TestRecorderWithProxyRestart tests the events with events proxy restart
func TestRecorderWithProxyRestart(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	proxyRPCServer, proxyRPCClient := createEventsProxy(t, eventsStorePath)
	defer proxyRPCServer.Stop()
	defer proxyRPCClient.ClientConn.Close()

	stopEventRecorder := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(1)

	var totalEventsSent uint64
	go func() {
		defer wg.Done()
		testEventSource := &monitoring.EventSource{NodeName: "test-node", Component: t.Name()}
		recorder, err := NewRecorder(testEventSource, testEventTypes, proxyRPCServer.GetListenURL())
		if err != nil {
			log.Errorf("failed to create recorder, err: %v", err)
			return
		}

		ticker := time.NewTicker(10 * time.Millisecond)
		for {
			select {
			case <-stopEventRecorder:
				return
			case <-ticker.C:
				if err = recorder.Event(TestNICConnected, "INFO", "test event - 1", nil); err == nil {
					atomic.AddUint64(&totalEventsSent, 1)
				}

				if err = recorder.Event(TestNICDisconnected, "CRITICAL", "test event - 2", nil); err == nil {
					atomic.AddUint64(&totalEventsSent, 1)
				}
			}
		}
	}()

	// restart events proxy
	proxyURL := proxyRPCServer.GetListenURL()

	time.Sleep(2 * time.Second)
	Assert(t, atomic.LoadUint64(&totalEventsSent) > 0, "recorder did not record any event")
	proxyRPCServer.Stop()

	// reset the counter
	totalEventsSent = 0

	// proxy won't be able to accept any events for 2s
	time.Sleep(2 * time.Second)
	os.RemoveAll(eventsStorePath)

	// create events dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(testDedupInterval, testSendInterval, eventsStorePath, logger)
	if err != nil {
		log.Errorf("failed to create events dispatcher, err: %v", err)
		return
	}

	proxyRPCServer, err = epgrpc.NewRPCServer(globals.EvtsProxy, proxyURL, evtsDispatcher, logger)
	if err != nil {
		log.Errorf("failed to start events proxy, err: %v", err)
		return
	}

	// let the recorders send some events after the proxy restart
	time.Sleep(2 * time.Second)

	// recorder should be back to normal; and start recording events
	Assert(t, atomic.LoadUint64(&totalEventsSent) > 0, "recorder did not record any event")

	// stop all the recorders
	close(stopEventRecorder)

	wg.Wait()
}
