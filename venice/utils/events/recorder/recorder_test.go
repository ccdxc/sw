// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package recorder

import (
	"fmt"
	"runtime"
	"sync"
	"testing"

	evtsapi "github.com/pensando/sw/api/generated/monitoring"
	epgrpc "github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/evtsproxy/rpcserver/evtsproxyproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events"
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

	logger = log.GetNewLogger(log.GetDefaultConfig("recorder-test"))
)

// createEventsProxy helper function that creates events proxy RPC server and client
func createEventsProxy(t *testing.T) (*epgrpc.RPCServer, *rpckit.RPCClient) {
	// create grpc server
	rpcServer, err := epgrpc.NewRPCServer(globals.EvtsProxy, testServerURL, logger)
	AssertOk(t, err, "failed to create rpc server")
	testServerURL = rpcServer.GetListenURL()

	// create grpc client
	rpcClient, err := rpckit.NewRPCClient(globals.EvtsProxy, testServerURL)
	AssertOk(t, err, "failed to create rpc client")

	return rpcServer, rpcClient
}

// TestEventsRecorder tests the events recorder (record event)
func TestEventsRecorder(t *testing.T) {
	// run events proxy server
	rpcServer, rpcClient := createEventsProxy(t)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()

	// create events proxy client
	proxyClient := evtsproxyproto.NewEventsProxyAPIClient(rpcClient.ClientConn)
	Assert(t, proxyClient != nil, "failed to created events proxy client")

	// create recorder
	recorder, err := NewRecorder(
		&evtsapi.EventSource{NodeName: "test-node", Component: "test-component"},
		testEventTypes, testServerURL)
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
	// run events proxy server
	rpcServer, rpcClient := createEventsProxy(t)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()

	// create events proxy client
	proxyClient := evtsproxyproto.NewEventsProxyAPIClient(rpcClient.ClientConn)
	Assert(t, proxyClient != nil, "failed to created events proxy client")

	// create recorder
	recorder, err := NewRecorder(
		&evtsapi.EventSource{NodeName: "test-node", Component: "test-component"},
		testEventTypes, testServerURL)
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
	_, err = NewRecorder(&evtsapi.EventSource{}, nil, testServerURL)
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")

	// empty event types
	_, err = NewRecorder(&evtsapi.EventSource{}, []string{}, testServerURL)
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")

	// empty events proxy URL
	_, err = NewRecorder(&evtsapi.EventSource{}, testEventTypes, "")
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")
}
