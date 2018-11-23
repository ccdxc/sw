package reader

import (
	"context"
	"fmt"
	"io/ioutil"
	"os"
	"sync"
	"testing"
	"time"

	"github.com/golang/protobuf/proto"
	"github.com/satori/go.uuid"

	google_protobuf1 "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/nevtsproxy/shm"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/dispatcher"
	"github.com/pensando/sw/venice/utils/events/writers"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestEventsReaderBasic ensures the event read from shared memory is what was written to it.
func TestEventsReaderBasic(t *testing.T) {
	shmName := fmt.Sprintf("/tmp/%s", uuid.NewV4().String())
	sm, err := shm.CreateSharedMem(shmName, 512)
	AssertOk(t, err, "failed to create shared memory, err: %v", err)
	ipc := sm.GetIPCInstance()

	hEvt := &halproto.Event{
		Severity:  halproto.Severity_INFO,
		Type:      "DUMMY",
		Component: "reader-test",
		Message:   "test-msg",
	}
	msgSize := hEvt.Size()

	// WRITE hEvt to shared memory
	ipcW := shm.NewIPCWriter(ipc)
	buf := ipcW.GetBuffer(msgSize) // get buffer
	Assert(t, buf != nil, "failed to get buffer from shared memory")
	_, err = writeEvent(buf, hEvt) // write event to the obtained buffer
	AssertOk(t, err, "failed to write event to shared memory")
	ipcW.PutBuffer(buf, msgSize)

	// READ from shared memory
	ipcR := shm.NewIPCReader(ipc, 50*time.Millisecond)
	errCh := make(chan error, 1)
	go ipcR.Receive(context.Background(), halproto.Event{}, func(msg interface{}) error {
		var err error
		nEvt, ok := msg.(*halproto.Event)
		if !ok {
			err = fmt.Errorf("failed to type cast the message from shared memory to event")
			errCh <- err
			return err
		}

		if nEvt.Message != hEvt.Message || nEvt.Component != hEvt.Component ||
			nEvt.Severity != hEvt.Severity || nEvt.Type != hEvt.Type {
			err = fmt.Errorf("mismatch between write and read")
			errCh <- err
			return err
		}

		errCh <- nil
		return nil
	})

	err = <-errCh
	AssertOk(t, err, "err: %v", err)

	// ensure there is no error
	Assert(t, ipcR.ErrCount == 0, "expected 0 err count, got: %d", ipcR.ErrCount)
}

// TestEventsReaderReceiveInvalidObjectType
// write halproto.Event and try to receive halproto.NetworkSpec{}
func TestEventsReaderReceiveInvalidObjectType(t *testing.T) {
	shmName := fmt.Sprintf("/tmp/%s", uuid.NewV4().String())
	sm, err := shm.CreateSharedMem(shmName, 512)
	AssertOk(t, err, "failed to create shared memory, err: %v", err)
	ipc := sm.GetIPCInstance()

	hEvt := &halproto.Event{
		Severity:  halproto.Severity_INFO,
		Type:      "DUMMY",
		Component: "reader-test",
		Message:   "test-msg",
	}
	msgSize := hEvt.Size()

	// WRITE halproto.Event to shared memory
	ipcW := shm.NewIPCWriter(ipc)
	buf := ipcW.GetBuffer(msgSize) // get buffer
	Assert(t, buf != nil, "failed to get buffer from shared memory")
	_, err = writeEvent(buf, hEvt) // write event to the obtained buffer
	AssertOk(t, err, "failed to write event to shared memory")
	ipcW.PutBuffer(buf, msgSize)

	// READ halproto.NetworkSpec from shared memory; it should fail
	ipcR := shm.NewIPCReader(ipc, 50*time.Millisecond)
	go ipcR.Receive(context.Background(), halproto.NetworkSpec{}, func(msg interface{}) error {
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
	totalEventsSent := testEventsReader(t, nil, 2)
	Assert(t, totalEventsSent > 0, "0 events sent?!! something went wrong")
}

// TestEventsReaderWithDispatcher tests the events reader by sending some events to shared memory
// and reading it off from the shared memory. Also, ensures the events were dispatched using the dispatcher.
func TestEventsReaderWithDispatcher(t *testing.T) {
	dir, err := ioutil.TempDir("", "dispatcher")
	AssertOk(t, err, "failed to create temp dir")

	logger := log.GetNewLogger(log.GetDefaultConfig(t.Name()))

	// create dispatcher
	evtsD, err := dispatcher.NewDispatcher(60*time.Second, 100*time.Millisecond, dir, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer evtsD.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter(fmt.Sprintf("mock.%s", t.Name()), 30, logger)
	writerEventCh, offsetTracker, err := evtsD.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh, offsetTracker)
	defer mockWriter.Stop()
	defer evtsD.UnregisterWriter(mockWriter.Name())

	totalEventsSent := testEventsReader(t, evtsD, 0)
	Assert(t, totalEventsSent > 0, "0 events sent?!! something went wrong")

	// ensure the mock writer received all the events through events dispatcher
	AssertEventually(t, func() (bool, interface{}) {
		receivedAtWriter := uint64(mockWriter.GetTotalEvents())
		if totalEventsSent == receivedAtWriter {
			return true, nil
		}

		return false, fmt.Sprintf("expected: %d, got: %d", totalEventsSent, receivedAtWriter)
	}, "unexpected number of events at the mock writer", string("5ms"), string("5s"))
}

// testEventsReader helper function to read and write some events and return
// the total number of events sent.
func testEventsReader(t *testing.T, evtsDipsatcher events.Dispatcher, wantedErrors int) uint64 {
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
		totalEventsSent = startEventWriter(shmName, 1024, stopWriter, wantedErrors)
	}()

	go func() {
		defer wg.Done()

		eRdr := startEventReader(shmName, evtsDipsatcher)
		Assert(t, eRdr != nil, "failed to start reader")
		defer eRdr.Stop()

		// stop the reader once the writer is stopped and ensure all the events are received by the reader.
		<-stopWriter
		AssertEventually(t, func() (bool, interface{}) {
			if totalEventsSent == eRdr.TotalEventsRead() {
				return true, nil
			}

			return false, fmt.Sprintf("expected: %d, got: %d", totalEventsSent, eRdr.TotalEventsRead())
		}, "unexpected number of events at the shm. events reader", string("5ms"), string("5s"))

		// ensure there is no error
		Assert(t, eRdr.TotalErrCount() == uint64(wantedErrors), "expected %d err count, got: %d", wantedErrors, eRdr.TotalErrCount())
	}()

	<-time.After(3 * time.Second)
	close(stopWriter)
	wg.Wait()

	return totalEventsSent
}

// helper function to start the reader
// - start events reader to read events from given shmName
// - spin off a go routine to start receiving events
func startEventReader(shmName string, evtsDispatcher events.Dispatcher) *EvtReader {
	var evtsReader *EvtReader
	var err error

	for i := 0; i < 10; i++ {
		if evtsReader, err = NewEventReader(shmName, 50*time.Millisecond, WithEventsDispatcher(evtsDispatcher)); err == nil {
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

	ipc := sm.GetIPCInstance()
	ipcW := shm.NewIPCWriter(ipc)
	var totalEventsSent uint64
	for {
		select {
		case <-stopCh:
			return totalEventsSent
		case <-time.After(10 * time.Millisecond):

			hEvt := &halproto.Event{
				Severity:  halproto.Severity_INFO,
				Type:      "DUMMY",
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
				return 0
			}

			_, err := writeEvent(buf, hEvt) // write event to the obtained buffer
			if err != nil {
				log.Errorf("failed to write event to shared memory")
				return 0
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
