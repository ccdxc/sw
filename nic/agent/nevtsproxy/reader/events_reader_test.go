package reader

import (
	"context"
	"fmt"
	"io/ioutil"
	"os"
	"sync"
	"testing"
	"time"

	google_protobuf1 "github.com/gogo/protobuf/types"
	"github.com/golang/protobuf/proto"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/nevtsproxy/shm"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	evtsRdrlogger = log.GetNewLogger(log.GetDefaultConfig("events-reader-test"))
)

// TestEventsReaderBasic ensures the event read from shared memory is what was written to it.
func TestEventsReaderBasic(t *testing.T) {
	shmName := fmt.Sprintf("/tmp/%s", uuid.NewV4().String())
	defer os.Remove(shmName)

	sm, err := shm.CreateSharedMem(shmName, 1024)
	AssertOk(t, err, "failed to create shared memory, err: %v", err)
	defer sm.Close()

	ipc := sm.GetIPCInstance()

	hEvt := &halproto.Event{
		Type:      int32(eventtypes.SERVICE_RUNNING),
		Component: "reader-test",
		Message:   "test-msg",
	}
	msgSize := hEvt.Size()

	// reader to read from shared memory
	ipcR := shm.NewIPCReader(ipc, 50*time.Millisecond)
	defer ipcR.Stop()
	Assert(t, len(ipcR.Dump()) == 0, "expected empty shared memory but there are events") // empty shared memory

	// WRITE hEvt to shared memory
	ipcW := shm.NewIPCWriter(ipc)
	buf := ipcW.GetBuffer(msgSize) // get buffer
	Assert(t, buf != nil, "failed to get buffer from shared memory")
	_, err = writeEvent(buf, hEvt) // write event to the obtained buffer
	AssertOk(t, err, "failed to write event to shared memory")
	ipcW.PutBuffer(buf, msgSize)

	// dump from shared memory
	Assert(t, len(ipcR.Dump()) == 1, "failed to read event from shared memory; expected a dump of 1 event")
	// read process messages from shared memory
	errCh := make(chan error, 1)
	go ipcR.Receive(context.Background(), func(nEvt *halproto.Event) error {
		if nEvt.Message != hEvt.Message || nEvt.Component != hEvt.Component ||
			nEvt.Type != hEvt.Type {
			err := fmt.Errorf("mismatch between write and read")
			errCh <- err
			return err
		}

		errCh <- nil
		return nil
	})

	err = <-errCh
	AssertOk(t, err, "err: %v", err)

	// wait for the event to be read by the reader
	AssertEventually(t, func() (bool, interface{}) {
		pendingEvents := ipcR.NumPendingEvents()
		if pendingEvents == 0 {
			return true, nil
		}
		return false, fmt.Sprintf("expected 0 pending events, got: %d", pendingEvents)
	}, "", string("10ms"), string("2s"))

	// ensure there is no error
	Assert(t, ipcR.ErrCount == 0, "expected 0 err count, got: %d", ipcR.ErrCount)
}

// TestEventsReaderWithCorruptedMessage
// It tries to write a message of size ~30 bytes using a buffer smaller than that.
// As a result, the message will get corrupted and the reader will fail.
func TestEventsReaderWithCorruptedMessage(t *testing.T) {
	shmName := fmt.Sprintf("/tmp/%s", uuid.NewV4().String())
	defer os.Remove(shmName)

	sm, err := shm.CreateSharedMem(shmName, 1024)
	AssertOk(t, err, "failed to create shared memory, err: %v", err)
	defer sm.Close()

	ipc := sm.GetIPCInstance()

	hEvt := &halproto.Event{
		Type:      int32(eventtypes.SERVICE_RUNNING),
		Component: "reader-test",
		Message:   "test-msg",
	}
	msgSize := hEvt.Size()
	invalidMsgSize := msgSize - 10 // this will corrupt the message

	// WRITE halproto.Event to shared memory
	ipcW := shm.NewIPCWriter(ipc)
	buf := ipcW.GetBuffer(invalidMsgSize) // get a buffer of smaller size
	Assert(t, buf != nil, "failed to get buffer from shared memory")
	_, err = writeEvent(buf, hEvt) // write event to the obtained buffer (evt bytes that can fit into the buffer will only be written)
	AssertOk(t, err, "failed to write event to shared memory")
	ipcW.PutBuffer(buf, invalidMsgSize)

	// reader will try to receive *halproto.Event and fail
	ipcR := shm.NewIPCReader(ipc, 50*time.Millisecond)
	defer ipcR.Stop()
	go ipcR.Receive(context.Background(), func(nEvt *halproto.Event) error {
		return nil
	})

	// ensure the error is observed and no events are received
	AssertEventually(t, func() (bool, interface{}) {
		if ipcR.ErrCount == 1 {
			return true, nil
		}

		return false, fmt.Sprintf("expected err count: %d, got: %d", 1, ipcR.ErrCount)
	}, "", string("5ms"), string("5s"))

	Assert(t, ipcR.RxCount == 0, "expected 0 events, got: %d", ipcR.ErrCount)
}

// TestEventsReader tests the events reader by sending some events to shared memory and
// reading it off from the shared memory.
func TestEventsReader(t *testing.T) {
	tLogger := evtsRdrlogger.WithContext("t_name", t.Name())
	totalEventsSent := testEventsReader(t, nil, tLogger, 2, 3*time.Second)
	Assert(t, totalEventsSent > 0, "0 events sent?!! something went wrong")
}

// TestEventsReaderWithDispatcher tests the events reader by sending some events to shared memory
// and reading it off from the shared memory. Also, ensures the events were dispatched using the dispatcher.
func TestEventsReaderWithDispatcher(t *testing.T) {
	tLogger := evtsRdrlogger.WithContext("t_name", t.Name())
	dir, err := ioutil.TempDir("", "dispatcher")
	AssertOk(t, err, "failed to create temp dir")

	logger := log.GetNewLogger(log.GetDefaultConfig(t.Name()))

	// create dispatcher
	evtsD, err := dispatcher.NewDispatcher(t.Name(), 60*time.Second, 100*time.Millisecond, &events.StoreConfig{Dir: dir},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	evtsD.Start()
	defer evtsD.Shutdown()

	// create and start writer
	mockWriter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), 30, logger)
	writerEventCh, offsetTracker, err := evtsD.RegisterExporter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh, offsetTracker)
	defer mockWriter.Stop()
	defer evtsD.UnregisterExporter(mockWriter.Name())

	totalEventsSent := testEventsReader(t, evtsD, tLogger, 0, 3*time.Second)
	Assert(t, totalEventsSent > 0, "0 events sent?!! something went wrong")

	// ensure the mock writer received all the events through events dispatcher
	AssertEventually(t, func() (bool, interface{}) {
		receivedAtWriter := uint64(mockWriter.GetTotalEvents())
		if totalEventsSent == receivedAtWriter {
			return true, nil
		}

		return false, fmt.Sprintf("expected: %d, got: %d", totalEventsSent, receivedAtWriter)
	}, "unexpected number of events at the mock writer", string("5ms"), string("5s"))

	// stop the dispatcher; all the events should start failing (as it cannot send it to the dispatcher and error count keeps increasing)
	evtsD.Shutdown()
	testEventsReader(t, evtsD, tLogger, -1, 100*time.Millisecond) // error count == total events that was sent
	// mock writer should not have received any more events than the original events that was sent earlier (L175)
	// everything that was sent now has failed at the dispatcher; did not make it to the writer/exporter.
	Assert(t, totalEventsSent == uint64(mockWriter.GetTotalEvents()), "expected: %d, got: %d", totalEventsSent, mockWriter.GetTotalEvents())
}

// testEventsReader helper function to read and write some events and return
// the total number of events sent.
func testEventsReader(t *testing.T, evtsDipsatcher events.Dispatcher, logger log.Logger, wantedErrors int, runtime time.Duration) uint64 {
	dir, err := ioutil.TempDir("", "shm")
	AssertOk(t, err, "failed to create temp dir")
	defer os.RemoveAll(dir)

	shmName := fmt.Sprintf(dir, uuid.NewV4().String())
	defer shm.DeleteSharedMem(shmName)

	var wg sync.WaitGroup
	wg.Add(2) // writer and reader

	var totalEventsSent uint64
	stopWriter := make(chan struct{}, 1)
	go func() {
		// writer creates the shared memory and starts sending events until it is stopped.
		defer wg.Done()
		totalEventsSent = startEventWriter(shmName, 2048, stopWriter, wantedErrors)
	}()

	go func() {
		defer wg.Done()

		eRdr := startEventReader(t.Name(), shmName, evtsDipsatcher, logger)
		Assert(t, eRdr != nil, "failed to start reader")
		defer eRdr.Stop()

		// stop the reader once the writer is stopped and ensure all the events are received by the reader.
		<-stopWriter
		if wantedErrors >= 0 {
			AssertEventually(t, func() (bool, interface{}) {
				if totalEventsSent == eRdr.TotalEventsRead() {
					return true, nil
				}

				return false, fmt.Sprintf("expected: %d, got: %d", totalEventsSent, eRdr.TotalEventsRead())
			}, "unexpected number of events at the shm. events reader", string("5ms"), string("5s"))

			// ensure there is no error
			Assert(t, eRdr.TotalErrCount() == uint64(wantedErrors), "expected %d err count, got: %d", wantedErrors, eRdr.TotalErrCount())
		} else if wantedErrors == -1 { // eRdr.TotalErrCount() == totalEventsSent; everything that was sent resulted in an error
			// e.g if the dispatcher is stopped, all the messages will fail
			AssertEventually(t, func() (bool, interface{}) {
				if totalEventsSent == eRdr.TotalErrCount() {
					return true, nil
				}

				return false, fmt.Sprintf("expected error count: %d, got: %d", totalEventsSent, eRdr.TotalErrCount())
			}, "unexpected err count at the shm. events reader", string("5ms"), string("5s"))
		}
	}()

	<-time.After(runtime)
	close(stopWriter)
	wg.Wait()

	return totalEventsSent
}

// helper function to start the reader
// - start events reader to read events from given shmName
// - spin off a go routine to start receiving events
func startEventReader(nodeName, shmName string, evtsDispatcher events.Dispatcher, logger log.Logger) *EvtReader {
	var evtsReader *EvtReader
	var err error

	for i := 0; i < 10; i++ {
		if evtsReader, err = NewEventReader(nodeName, shmName, 50*time.Millisecond, logger, WithEventsDispatcher(evtsDispatcher)); err == nil {
			break
		}
		log.Errorf("failed to open shared memory, retrying..")
		time.Sleep(10 * time.Millisecond)
		continue
	}

	if err != nil {
		log.Errorf("failed to create shared memory: %s, err: %v", shmName, err)
		return nil
	}

	evtsReader.Start()
	return evtsReader
}

// helper function to start the writer
// - create shared memory with the given name and size
// - start recording events to shared memory
// - stop upon receiving signal from stopCh
func startEventWriter(shmName string, size int, stopCh chan struct{}, fakeErrors int) uint64 {
	sm, err := shm.CreateSharedMem(shmName, size)
	if err != nil {
		log.Errorf("failed to create shared memory: %s, err: %v", shmName, err)
		return 0
	}
	defer sm.Close()

	ipc := sm.GetIPCInstance()
	ipcW := shm.NewIPCWriter(ipc)
	var totalEventsSent uint64
	for {
		select {
		case <-stopCh:
			return totalEventsSent
		case <-time.After(10 * time.Millisecond):

			hEvt := &halproto.Event{
				Type:      int32(eventtypes.SERVICE_RUNNING),
				Component: "reader-test",
				Message:   fmt.Sprintf("test msg - %d", totalEventsSent),
			}
			if fakeErrors > 0 {
				hEvt.ObjectKey = &google_protobuf1.Any{}
				fakeErrors--
			}

			msgSize := hEvt.Size()
			buf := ipcW.GetBuffer(msgSize) // get buffer
			if buf == nil {
				log.Errorf("failed to get buffer from shared memory")
				return totalEventsSent
			}

			_, err := writeEvent(buf, hEvt) // write event to the obtained buffer
			if err != nil {
				log.Errorf("failed to write event to shared memory")
				return totalEventsSent
			}
			ipcW.PutBuffer(buf, msgSize)
			totalEventsSent++
			time.Sleep(10 * time.Millisecond)
		}
	}
}

// helper function to write given event to the buffer
func writeEvent(buf []byte, evt *halproto.Event) (int, error) {
	out, err := proto.Marshal(evt)
	if err != nil {
		return 0, err
	}

	copy(buf, out)
	return len(out), nil
}
