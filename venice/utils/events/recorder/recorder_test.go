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
	"github.com/pensando/sw/events/generated/eventtypes"
	epgrpc "github.com/pensando/sw/venice/evtsproxy/rpcserver"
	"github.com/pensando/sw/venice/evtsproxy/rpcserver/evtsproxyproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

var (
	testServerURL = "localhost:0"
	numEvents     = 1000

	logger        = log.GetNewLogger(log.GetDefaultConfig("recorder_test"))
	mockBufferLen = 10

	testDedupInterval = 100 * time.Second
	testSendInterval  = 10 * time.Millisecond

	eventsDir = "/tmp"
)

// createEventsProxy helper function that creates events proxy RPC server and client
func createEventsProxy(t *testing.T, proxyURL, eventsStorePath string) (*epgrpc.RPCServer,
	events.Dispatcher, *rpckit.RPCClient, *exporters.MockExporter) {
	// create events dispatcher
	evtsDispatcher, err := dispatcher.NewDispatcher(t.Name(), testDedupInterval, testSendInterval,
		&events.StoreConfig{Dir: eventsStorePath}, nil, logger)
	AssertOk(t, err, "failed to create dispatcher")

	// create mock writer
	mockWriter := exporters.NewMockExporter(fmt.Sprintf("mock-%s", t.Name()), mockBufferLen, logger)
	writerEventCh, offsetTracker, err := evtsDispatcher.RegisterExporter(mockWriter)
	AssertOk(t, err, "failed to register mock writer")
	mockWriter.Start(writerEventCh, offsetTracker)
	evtsDispatcher.Start()

	// create grpc server
	rpcServer, err := epgrpc.NewRPCServer(globals.EvtsProxy, proxyURL, evtsDispatcher, logger)
	AssertOk(t, err, "failed to create rpc server")

	// create grpc client
	rpcClient, err := rpckit.NewRPCClient(globals.EvtsProxy, rpcServer.GetListenURL(), rpckit.WithLogger(logger))
	AssertOk(t, err, "failed to create rpc client")

	return rpcServer, evtsDispatcher, rpcClient, mockWriter
}

// TestEventsRecorder tests the events recorder (record event)
func TestEventsRecorder(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	// run events proxy server
	rpcServer, evtsDispatcher, rpcClient, _ := createEventsProxy(t, testServerURL, eventsStorePath)
	defer rpcServer.Stop()
	defer evtsDispatcher.Shutdown()
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
		Component:    "test-component" + t.Name(),
		EvtsProxyURL: rpcServer.GetListenURL(),
		BackupDir:    recorderEventsDir}, logger)
	AssertOk(t, err, "failed to create events recorder")
	defer evtsRecorder.Close()

	evtsRecorder.Event(eventtypes.DSC_ADMITTED, "test event - 1", nil)
	evtsRecorder.Event(eventtypes.DSC_UNHEALTHY, "test event - 2", nil)

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
				evtsRecorder.Event(eventtypes.DSC_UNHEALTHY, message, nil)
				evtsRecorder.Event(eventtypes.DSC_ADMITTED, message, nil)

				// create test NIC object
				testNIC := policygen.CreateSmartNIC("0014.2201.2345",
					cluster.DistributedServiceCardStatus_ADMITTED.String(),
					"esx-1",
					&cluster.DSCCondition{
						Type:   cluster.DSCCondition_HEALTHY.String(),
						Status: cluster.ConditionStatus_FALSE.String(),
					})

				// record events with reference object
				evtsRecorder.Event(eventtypes.DSC_UNHEALTHY, message, testNIC)
				evtsRecorder.Event(eventtypes.DSC_ADMITTED, message, testNIC)
			}
			wg.Done()
		}(i, evtsRecorder)
	}

	wg.Wait()
}

// TestEventRecorderInstantiation tests event recorder instantiation cases
func TestEventRecorderInstantiation(t *testing.T) {
	// missing component name
	_, err := NewRecorder(&Config{EvtsProxyURL: testServerURL}, logger)
	Assert(t, err != nil, "expected failure, event recorder instantiation succeeded")

	// skip evts proxy
	r, err := NewRecorder(&Config{Component: t.Name(),
		EvtsProxyURL: testServerURL, BackupDir: "/tmp", SkipEvtsProxy: true}, logger)
	AssertOk(t, err, "expected success, event recorder instantiation failed")
	r.StartExport()
	r.Close()
}

// TestRecorderWithProxyRestart tests the events with events proxy restart
func TestRecorderWithProxyRestart(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath)

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	proxyRPCServer, evtsDispatcher, proxyRPCClient, _ := createEventsProxy(t, testServerURL, eventsStorePath)
	defer proxyRPCServer.Stop()
	defer proxyRPCClient.ClientConn.Close()

	stopEventRecorder := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(1)

	var totalEventsSent uint64
	go func() {
		defer wg.Done()
		var evtsRecorder events.Recorder

		evtsRecorder, err = NewRecorder(&Config{
			Component:    t.Name(),
			EvtsProxyURL: proxyRPCServer.GetListenURL(),
			BackupDir:    recorderEventsDir}, logger)
		if err != nil {
			log.Errorf("failed to create recorder, err: %v", err)
			return
		}
		defer evtsRecorder.Close()

		ticker := time.NewTicker(10 * time.Millisecond)
		for {
			select {
			case <-stopEventRecorder:
				return
			case <-ticker.C:
				evtsRecorder.Event(eventtypes.DSC_ADMITTED, "test event - 1", nil)
				atomic.AddUint64(&totalEventsSent, 1)

				evtsRecorder.Event(eventtypes.DSC_UNHEALTHY, "test event - 2", nil)
				atomic.AddUint64(&totalEventsSent, 1)
			}
		}
	}()

	// restart events proxy
	proxyURL := proxyRPCServer.GetListenURL()

	time.Sleep(2 * time.Second)
	Assert(t, atomic.LoadUint64(&totalEventsSent) > 0, "recorder did not record any event")
	totalEventsSentBeforeStop := atomic.LoadUint64(&totalEventsSent)
	proxyRPCServer.Stop()

	// proxy won't be able to accept any events for 2s but the events will be stored in a backup file
	time.Sleep(2 * time.Second)
	totalEventsSentAfterStop := atomic.LoadUint64(&totalEventsSent)

	// this indicates that the recorder was running fine when the proxy is unavailable
	Assert(t, totalEventsSentAfterStop > totalEventsSentBeforeStop, "recorder wasn't running when proxy went disconnected")
	// reset the counter
	atomic.StoreUint64(&totalEventsSent, 0)
	evtsDispatcher.Shutdown() // shutdown the existing dispatcher
	os.RemoveAll(eventsStorePath)

	// create events dispatcher
	evtsDispatcher, err = dispatcher.NewDispatcher(t.Name(), testDedupInterval, testSendInterval,
		&events.StoreConfig{Dir: eventsStorePath}, nil, logger)
	if err != nil {
		log.Errorf("failed to create events dispatcher, err: %v", err)
		return
	}
	evtsDispatcher.Start()
	defer evtsDispatcher.Shutdown()

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

	evtsRecorder, err := NewRecorder(&Config{
		Component: t.Name(),
		BackupDir: recorderEventsDir}, logger)
	AssertOk(t, err, "failed to create recorder")
	defer evtsRecorder.Close()

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
				evtsRecorder.Event(eventtypes.DSC_ADMITTED, "test event - 1", nil)
				atomic.AddUint64(&totalEventsSent, 1)

				evtsRecorder.Event(eventtypes.DSC_UNHEALTHY, "test event - 2", nil)
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

	proxyRPCServer, evtsDispatcher, proxyRPCClient, mockWriter := createEventsProxy(t, testServerURL, eventsStorePath)
	defer proxyRPCServer.Stop()
	defer evtsDispatcher.Shutdown()
	defer proxyRPCClient.ClientConn.Close()
	defer mockWriter.Stop()

	stopWorkers := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(2)

	evtsRecorder, err := NewRecorder(&Config{
		Component:    t.Name(),
		EvtsProxyURL: proxyRPCServer.GetListenURL(),
		BackupDir:    recorderEventsDir}, logger)
	AssertOk(t, err, "failed to create recorder")
	defer evtsRecorder.Close()

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
				evtsRecorder.Event(eventtypes.DSC_ADMITTED, "test event - 1", nil)
				atomic.AddUint64(&totalEventsSent, 1)

				evtsRecorder.Event(eventtypes.DSC_UNHEALTHY, "test event - 2", nil)
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
				proxyRPCServer, evtsDispatcher, proxyRPCClient, mockWriter = createEventsProxy(t, proxyURL, eventsStorePath)
			}
		}
	}()

	time.Sleep(3 * time.Second)

	close(stopWorkers)
	wg.Wait()

	// check if all the events has been received by the mock writer
	// It is expected to receive duplicates when the proxy restarts continuously because we
	// may come across situations where the proxy might restart while it is half way
	// processing the failed events from recorder. And the recorder will retry sending all of them once again.
	AssertEventually(t,
		func() (bool, interface{}) {
			totalReceived := totalEventsReceived + uint64(mockWriter.GetTotalEvents())
			if totalReceived >= totalEventsSent {
				return true, nil
			}
			return false, fmt.Sprintf("expected: > = %d, got: %d",
				totalEventsSent, totalReceived)
		}, "mock writer did not receive all the events recorded", "20ms", "6s")
}

func TestNewfile(t *testing.T) {
	f, err := ioutil.TempFile("/tmp", "event")
	AssertOk(t, err, "failed to create file")
	defer os.Remove(f.Name())

	_, err = newFile(f.Name(), "tmpfile")
	Assert(t, err != nil, "test didn't fail for invalid dir")
}
