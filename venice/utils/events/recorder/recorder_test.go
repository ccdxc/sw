// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package recorder

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"runtime"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	epgrpc "github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/evtsproxy/rpcserver/evtsproxyproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/writers"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
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
func createEventsProxy(t *testing.T, proxyURL, eventsStorePath string) (*epgrpc.RPCServer, *rpckit.RPCClient, *writers.MockWriter) {
	// create events dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(testDedupInterval, testSendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	evtsDispatcher.Start()

	// create mock writer
	mockWriter := writers.NewMockWriter(fmt.Sprintf("mock-%s", t.Name()), mockBufferLen, logger)
	writerEventCh, offsetTracker, err := evtsDispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer")
	mockWriter.Start(writerEventCh, offsetTracker)

	// create grpc server
	rpcServer, err := epgrpc.NewRPCServer(globals.EvtsProxy, proxyURL, evtsDispatcher, logger)
	AssertOk(t, err, "failed to create rpc server")

	// create grpc client
	rpcClient, err := rpckit.NewRPCClient(globals.EvtsProxy, rpcServer.GetListenURL())
	AssertOk(t, err, "failed to create rpc client")

	return rpcServer, rpcClient, mockWriter
}

// TestEventsRecorder tests the events recorder (record event)
func TestEventsRecorder(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	// run events proxy server
	rpcServer, rpcClient, _ := createEventsProxy(t, testServerURL, eventsStorePath)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	// create events proxy client
	proxyClient := evtsproxyproto.NewEventsProxyAPIClient(rpcClient.ClientConn)
	Assert(t, proxyClient != nil, "failed to created events proxy client")

	// create recorder
	evtsRecorder, err := NewRecorder(&Config{
		Source:       &evtsapi.EventSource{NodeName: "test-node", Component: "test-component"},
		EvtTypes:     testEventTypes,
		EvtsProxyURL: rpcServer.GetListenURL(),
		BackupDir:    recorderEventsDir})
	AssertOk(t, err, "failed to create events recorder")

	evtsRecorder.Event(TestNICDisconnected, evtsapi.SeverityLevel_INFO, "test event - 1", nil)
	evtsRecorder.Event(TestNICConnected, evtsapi.SeverityLevel_INFO, "test event - 2", nil)

	// send events using multiple workers and check if things are still intact
	wg := new(sync.WaitGroup)
	workers := runtime.NumCPU()
	each := numEvents / workers
	wg.Add(workers)

	// start the workers
	for i := 0; i < workers; i++ {
		go func(threadID int, recorder events.Recorder) {
			for j := 0; j < each; j++ {
				// record events w/o reference object
				message := fmt.Sprintf("thread: %v; event: %v", j, threadID)
				evtsRecorder.Event(TestNICDisconnected, evtsapi.SeverityLevel_CRITICAL, message, nil)
				evtsRecorder.Event(TestNICConnected, evtsapi.SeverityLevel_INFO, message, nil)

				// create test NIC object
				testNIC := policygen.CreateSmartNIC("00-14-22-01-23-45",
					cluster.SmartNICStatus_ADMITTED.String(),
					"esx-1",
					&cluster.SmartNICCondition{
						Type:   cluster.SmartNICCondition_HEALTHY.String(),
						Status: cluster.ConditionStatus_FALSE.String(),
					})

				// record events with reference object
				evtsRecorder.Event(TestNICDisconnected, evtsapi.SeverityLevel_CRITICAL, message, testNIC)
				evtsRecorder.Event(TestNICConnected, evtsapi.SeverityLevel_INFO, message, testNIC)
			}
			wg.Done()
		}(i, evtsRecorder)
	}

	wg.Wait()
}

// TestInvalidEventInputs tests invalid event inputs (severity, type)
func TestInvalidEventInputs(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	// run events proxy server
	rpcServer, rpcClient, _ := createEventsProxy(t, testServerURL, eventsStorePath)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	// create events proxy client
	proxyClient := evtsproxyproto.NewEventsProxyAPIClient(rpcClient.ClientConn)
	Assert(t, proxyClient != nil, "failed to created events proxy client")

	// create recorder
	evtsRecorder, err := NewRecorder(&Config{
		Source:       &evtsapi.EventSource{NodeName: "test-node", Component: "test-component"},
		EvtTypes:     testEventTypes,
		EvtsProxyURL: rpcServer.GetListenURL(),
		BackupDir:    recorderEventsDir})
	AssertOk(t, err, "failed to create events recorder")

	// send events using multiple workers and check if things are still intact
	wg := new(sync.WaitGroup)
	wg.Add(2)

	// test invalid event type
	go func() {
		defer func() {
			if r := recover(); r != nil {
				wg.Done()
			}
		}()

		evtsRecorder.Event("InvalidEvent", evtsapi.SeverityLevel_INFO, "test event - 1", nil)
	}()

	// test invalid severity - 100
	go func() {
		defer func() {
			if r := recover(); r != nil {
				wg.Done()
			}
		}()

		evtsRecorder.Event(TestNICConnected, 100, "test event - 1", nil)
	}()

	wg.Wait()
}

// TestEventRecorderInstantiation tests event recorder instantiation cases
func TestEventRecorderInstantiation(t *testing.T) {
	// missing event source
	_, err := NewRecorder(&Config{EvtTypes: testEventTypes, EvtsProxyURL: testServerURL})
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")

	// empty event types
	_, err = NewRecorder(&Config{Source: &evtsapi.EventSource{}, EvtsProxyURL: testServerURL})
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")

	// skip evts proxy
	_, err = NewRecorder(&Config{Source: &evtsapi.EventSource{}, EvtTypes: []string{"DUMMY"},
		EvtsProxyURL: testServerURL, BackupDir: "/tmp", SkipEvtsProxy: true})
	AssertOk(t, err, "expected success, event recorder instantiation failed")
}

// TestRecorderWithProxyRestart tests the events with events proxy restart
func TestRecorderWithProxyRestart(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	proxyRPCServer, proxyRPCClient, _ := createEventsProxy(t, testServerURL, eventsStorePath)
	defer proxyRPCServer.Stop()
	defer proxyRPCClient.ClientConn.Close()

	stopEventRecorder := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(1)

	var totalEventsSent uint64
	go func() {
		defer wg.Done()
		var evtsRecorder events.Recorder

		testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: t.Name()}
		evtsRecorder, err = NewRecorder(&Config{
			Source:       testEventSource,
			EvtTypes:     testEventTypes,
			EvtsProxyURL: proxyRPCServer.GetListenURL(),
			BackupDir:    recorderEventsDir})
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
				evtsRecorder.Event(TestNICConnected, evtsapi.SeverityLevel_INFO, "test event - 1", nil)
				atomic.AddUint64(&totalEventsSent, 1)

				evtsRecorder.Event(TestNICDisconnected, evtsapi.SeverityLevel_CRITICAL, "test event - 2", nil)
				atomic.AddUint64(&totalEventsSent, 1)
			}
		}
	}()

	// restart events proxy
	proxyURL := proxyRPCServer.GetListenURL()

	time.Sleep(2 * time.Second)
	Assert(t, atomic.LoadUint64(&totalEventsSent) > 0, "recorder did not record any event")
	proxyRPCServer.Stop()

	// reset the counter
	atomic.StoreUint64(&totalEventsSent, 0)

	// proxy won't be able to accept any events for 2s but the events will be stored in a backup file
	time.Sleep(2 * time.Second)
	os.RemoveAll(eventsStorePath)

	// create events dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(testDedupInterval, testSendInterval, eventsStorePath, logger)
	if err != nil {
		log.Errorf("failed to create events dispatcher, err: %v", err)
		return
	}
	evtsDispatcher.Start()

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

// TestRecorderFileBackup make sure the events are recorded to a file when the proxy is not reachable
func TestRecorderFileBackup(t *testing.T) {
	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: t.Name()}
	evtsRecorder, err := NewRecorder(&Config{
		Source:    testEventSource,
		EvtTypes:  testEventTypes,
		BackupDir: recorderEventsDir})
	AssertOk(t, err, "failed to create recorder")

	stopEventRecorder := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(1)

	// start recording events
	var totalEventsSent uint64
	go func() {
		defer wg.Done()

		ticker := time.NewTicker(10 * time.Millisecond)
		for {
			select {
			case <-stopEventRecorder:
				return
			case <-ticker.C:
				evtsRecorder.Event(TestNICConnected, evtsapi.SeverityLevel_INFO, "test event - 1", nil)
				atomic.AddUint64(&totalEventsSent, 1)

				evtsRecorder.Event(TestNICDisconnected, evtsapi.SeverityLevel_CRITICAL, "test event - 2", nil)
				atomic.AddUint64(&totalEventsSent, 1)
			}
		}
	}()

	time.Sleep(60 * time.Millisecond)
	close(stopEventRecorder)
	wg.Wait()

	// make sure events are recorded to a backup file
	AssertEventually(t, func() (bool, interface{}) {
		var size int64
		_ = filepath.Walk(recorderEventsDir, func(_ string, info os.FileInfo, err error) error {
			if !info.IsDir() {
				size += info.Size()
			}
			return err
		})

		if size > 0 {
			return true, nil
		}

		return false, fmt.Sprintf("expected size of the file to be >0, got: %v", size)
	}, "failed to record events to backup file", string("5ms"), string("5s"))
}

// TestRecorderFailedEventsForwarder tests the recorder with continuous evtsproxy restarts
func TestRecorderFailedEventsForwarder(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir(eventsDir, "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	proxyRPCServer, proxyRPCClient, mockWriter := createEventsProxy(t, testServerURL, eventsStorePath)
	defer proxyRPCServer.Stop()
	defer proxyRPCClient.ClientConn.Close()
	defer mockWriter.Stop()

	stopWorkers := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(2)

	testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: t.Name()}
	evtsRecorder, err := NewRecorder(&Config{
		Source:       testEventSource,
		EvtTypes:     testEventTypes,
		EvtsProxyURL: proxyRPCServer.GetListenURL(),
		BackupDir:    recorderEventsDir})
	AssertOk(t, err, "failed to create recorder")

	// record events
	var totalEventsSent uint64
	go func() {
		defer wg.Done()

		ticker := time.NewTicker(10 * time.Millisecond)
		for {
			select {
			case <-stopWorkers:
				return
			case <-ticker.C:
				evtsRecorder.Event(TestNICConnected, evtsapi.SeverityLevel_INFO, "test event - 1", nil)
				atomic.AddUint64(&totalEventsSent, 1)

				evtsRecorder.Event(TestNICDisconnected, evtsapi.SeverityLevel_CRITICAL, "test event - 2", nil)
				atomic.AddUint64(&totalEventsSent, 1)
			}
		}
	}()

	var totalEventsReceived uint64
	// restart proxy in short intervals
	go func() {
		defer wg.Done()
		proxyURL := proxyRPCServer.GetListenURL()
		ticker := time.NewTicker(60 * time.Millisecond)
		for {
			select {
			case <-stopWorkers:
				return
			case <-ticker.C:
				proxyRPCServer.Stop()
				time.Sleep(20 * time.Millisecond)
				atomic.AddUint64(&totalEventsReceived, uint64(mockWriter.GetTotalEvents()))
				proxyRPCServer, proxyRPCClient, mockWriter = createEventsProxy(t, proxyURL, eventsStorePath)
			}
		}
	}()

	time.Sleep(3 * time.Second)

	close(stopWorkers)
	wg.Wait()

	// let the mock writer receive all the events
	time.Sleep(2 * time.Second)
	atomic.AddUint64(&totalEventsReceived, uint64(mockWriter.GetTotalEvents()))

	// check if all the events has been received by the mock writer
	// It is expected to receive duplicates when the proxy restarts continuously because we
	// may come across situations where the proxy might restart while it is half way
	// processing the failed events from recorder. And the recorder will retry sending all of them once again.
	Assert(t, totalEventsSent <= totalEventsReceived, "mock writer did not receive all the events recorded, expected: >=%d, got: %d",
		totalEventsSent, totalEventsReceived)
}
