// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"fmt"
	"os"
	"path"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/google/uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/writers"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	dummyEvtSource = &monitoring.EventSource{
		Component: "test",
		NodeName:  "test",
	}

	dummyEvt = &monitoring.Event{
		TypeMeta: api.TypeMeta{
			Kind: "Event",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			UUID:      uuid.New().String(),
		},
		EventAttributes: monitoring.EventAttributes{
			Source:   dummyEvtSource,
			Severity: "INFO",
			Type:     "DUMMY",
			Count:    1,
		},
	}

	dedupInterval = 100 * time.Second
	sendInterval  = 10 * time.Millisecond

	writerChLen = 30

	logger    = log.GetNewLogger(log.GetDefaultConfig("dispatcher-test"))
	eventsDir = "/tmp"
)

// TestEventsDispatcher tests the dispatcher's general functionality (distribute and receive)
func TestEventsDispatcher(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(dedupInterval, sendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter(fmt.Sprintf("mock.%s", t.Name()), writerChLen, logger)
	writerEventCh, offsetTracker, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh, offsetTracker)
	defer mockWriter.Stop()
	defer dispatcher.UnregisterWriter(mockWriter.Name())

	dispatcher.ProcessFailedEvents()

	// copy event and set type
	event := *dummyEvt
	NICDisconnectedEvtUUID := uuid.New().String()
	NICDisconnectedEvt := *(&event)
	NICDisconnectedEvt.EventAttributes.Type = "TestNICDisconnected"
	NICDisconnectedEvt.ObjectMeta.UUID = NICDisconnectedEvtUUID

	NICConnectedEvtUUID := uuid.New().String()
	NICConnectedEvt := *(&event)
	NICConnectedEvt.EventAttributes.Type = "TestNICConnected"
	NICConnectedEvt.ObjectMeta.UUID = NICConnectedEvtUUID

	// run multiple iterations
	iterations := 10
	for i := 0; i < iterations; i++ {
		for i := 0; i < 10; i++ { // all events will be deduped under the first (1st occurrence of an event in dedup interval) UUID
			creationTime, _ := types.TimestampProto(time.Now())
			timeNow := api.Timestamp{Timestamp: *creationTime}
			NICDisconnectedEvt.ObjectMeta.CreationTime = timeNow
			NICDisconnectedEvt.ObjectMeta.ModTime = timeNow

			NICConnectedEvt.ObjectMeta.CreationTime = timeNow
			NICConnectedEvt.ObjectMeta.ModTime = timeNow

			AssertOk(t, dispatcher.Action(NICDisconnectedEvt), "failed to send event")
			AssertOk(t, dispatcher.Action(NICConnectedEvt), "failed to send event")

			// keep updating uuid to make sure they're deduped under the first occurrence
			NICDisconnectedEvt.ObjectMeta.UUID = uuid.New().String()
			NICConnectedEvt.ObjectMeta.UUID = uuid.New().String()
		}
	}

	// ensure the writer received all the events that're sent
	totalEvents := iterations * 10 // 10 iterations sent 10 events each

	AssertEventually(t, func() (bool, interface{}) {
		disconnected := mockWriter.GetEventsByType("TestNICDisconnected")
		connected := mockWriter.GetEventsByType("TestNICConnected")
		if totalEvents == disconnected && totalEvents == connected {
			return true, nil
		}

		return false, fmt.Sprintf("expected: %d events, got: (TestNICDisconnected %d + TestNICConnected %d)\n",
			totalEvents*2, disconnected, connected)
	}, "unexpected number of events", string("5ms"), string("5s"))

	// send an event with missing attributes
	incomEvent := *(&event)
	incomEvent.ObjectMeta.Name = "empty severity"
	incomEvent.EventAttributes.Severity = ""
	err = dispatcher.Action(incomEvent)
	Assert(t, events.IsMissingEventAttributes(err), "expected failure, should not accept incomplete event")

	dispatcher.Shutdown()

	// sending events after shutdown should fail
	testEvt := *(&event)
	testEvt.EventAttributes.Type = "Test"
	err = dispatcher.Action(testEvt)
	Assert(t, strings.Contains(err.Error(), "dispatcher stopped, cannot process events"),
		"expected failure")

	// ensure the events are deduped
	AssertEventually(t, func() (bool, interface{}) {
		disconnectedEvt := mockWriter.GetEventByUUID(NICDisconnectedEvtUUID)
		connectedEvt := mockWriter.GetEventByUUID(NICConnectedEvtUUID)
		if disconnectedEvt == nil || connectedEvt == nil {
			return false, fmt.Sprintf("something wrong, couldn't find the expected event")
		}

		if disconnectedEvt.GetCount() > 1 && connectedEvt.GetCount() > 1 {
			return true, nil
		}

		return false, nil
	}, "events are not deduped", string("5ms"), string("5s"))
}

// TestEventsDispatcherShutdown tests the graceful shutdown of the dispatcher.
func TestEventsDispatcherShutdown(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(dedupInterval, sendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// create and start mock writers
	mockWriter1 := writers.NewMockWriter(fmt.Sprintf("mock1.%s", t.Name()), writerChLen, logger)
	writerEventCh1, offsetTracker1, err := dispatcher.RegisterWriter(mockWriter1)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter1.Start(writerEventCh1, offsetTracker1)
	defer mockWriter1.Stop()
	defer dispatcher.UnregisterWriter(mockWriter1.Name())

	mockWriter2 := writers.NewMockWriter(fmt.Sprintf("mock2.%s", t.Name()), writerChLen, logger)
	writerEventCh2, offsetTracker2, err := dispatcher.RegisterWriter(mockWriter2)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter2.Start(writerEventCh2, offsetTracker2)
	defer mockWriter2.Stop()
	defer dispatcher.UnregisterWriter(mockWriter2.Name())

	// flushes and closes the writers gracefully
	dispatcher.Shutdown()

	// no-op; they are already stopped druing shutdown
	mockWriter1.Stop()
	mockWriter2.Stop()

	// no-op
	dispatcher.Shutdown()
}

// TestEventsDispatcherFlush tests dispatcher's flush functionality.
// events are flushed internally when shutdown is called.
func TestEventsDispatcherFlush(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// large value to make sure the interval does not kick in before the flush
	interval := 5 * time.Second

	dispatcher, err := NewDispatcher(10*time.Second, interval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter(fmt.Sprintf("mock.%s", t.Name()), writerChLen, logger)
	writerEventCh, offsetTracker, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh, offsetTracker)
	defer mockWriter.Stop()
	defer dispatcher.UnregisterWriter(mockWriter.Name())

	// copy event and set type
	event := *dummyEvt
	NICDisconnectedEvt := *(&event)
	NICDisconnectedEvt.EventAttributes.Type = "TestNICDisconnected"
	NICDisconnectedEvt.ObjectMeta.UUID = uuid.New().String()

	NICConnectedEvt := *(&event)
	NICConnectedEvt.EventAttributes.Type = "TestNICConnected"
	NICConnectedEvt.ObjectMeta.UUID = uuid.New().String()

	// send events
	for i := 0; i < 100; i++ {
		creationTime, _ := types.TimestampProto(time.Now())
		timeNow := api.Timestamp{Timestamp: *creationTime}

		NICDisconnectedEvt.ObjectMeta.CreationTime = timeNow
		NICDisconnectedEvt.ObjectMeta.ModTime = timeNow

		NICConnectedEvt.ObjectMeta.CreationTime = timeNow
		NICConnectedEvt.ObjectMeta.ModTime = timeNow

		AssertOk(t, dispatcher.Action(NICDisconnectedEvt), "failed to send event")
		AssertOk(t, dispatcher.Action(NICConnectedEvt), "failed to send event")
	}

	// writer would have not received any deduped event becase the acitve internal is not hit
	temp := mockWriter.GetEventsByType("TestNICConnected")
	Assert(t, temp == 0, "expected 0 events, got:%v", temp)

	temp = mockWriter.GetEventsByType("TestNICDisconnected")
	Assert(t, temp == 0, "expected 0 events, got:%v", temp)

	// deduped events from the dispatcher will be flushed to all the writers
	dispatcher.Shutdown()

	// writer would have not received any deduped event becase the acitve internal is not hit
	AssertEventually(t, func() (bool, interface{}) {
		disconnected := mockWriter.GetEventsByType("TestNICDisconnected")
		connected := mockWriter.GetEventsByType("TestNICConnected")

		if disconnected == 100 && connected == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected 100 events for each type, got: (TestNICDisconnected %d, TestNICConnected %d)",
			disconnected, connected)
	}, "writer did not receive all the events or flush operation failed", string("5ms"), string("5s"))

	// thus the writers received the events before the distpatcher could hit active interval
}

// TestEventsDispatcherRegisterWriter tests dispatcher's register writer funtionality
func TestEventsDispatcherRegisterWriter(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	dispatcher, err := NewDispatcher(dedupInterval, sendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter(fmt.Sprintf("mock.%s", t.Name()), writerChLen, logger)
	eventsCh, offsetTracker, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register writer")
	mockWriter.Start(eventsCh, offsetTracker)
	defer mockWriter.Stop()

	_, _, err = dispatcher.RegisterWriter(mockWriter)
	Assert(t, strings.Contains(err.Error(), "name exists already"),
		"expected failure; writer with the same name exists already")

	mockWriter1 := writers.NewMockWriter(fmt.Sprintf("mock1.%s", t.Name()), writerChLen, logger)
	eventsCh, offsetTracker, err = dispatcher.RegisterWriter(mockWriter1)
	AssertOk(t, err, "failed to register writer")
	mockWriter1.Start(eventsCh, offsetTracker)
	defer mockWriter1.Stop()

	// dispatcher should not register any more writers
	dispatcher.Shutdown()

	mockWriter2 := writers.NewMockWriter(fmt.Sprintf("mock2.%s", t.Name()), writerChLen, logger)
	_, _, err = dispatcher.RegisterWriter(mockWriter2)
	Assert(t, strings.Contains(err.Error(), "dispatcher stopped"),
		"expected failure; stopped dispatcher should not accept register requests")
}

// TestEventsDispatcherWithSingleWriter tests the dispatcher with single writer.
func TestEventsDispatcherWithSingleWriter(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	testEventDispatcherWithWriters(t, 1, eventsStorePath)
}

// TestEventsDispatcherWithMultipleWriters tests the dispatcher with multiple writers.
func TestEventsDispatcherWithMultipleWriters(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	testEventDispatcherWithWriters(t, 10, eventsStorePath)
}

// TestEventsDispatcherWithSingleSource tests the dispatcher with single source
// producing events.
func TestEventsDispatcherWithSingleSource(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	testEventsDispatcherWithSources(t, 1, eventsStorePath)
}

// TestEventsDispatcherWithMultipleSources tests the dispatcher with multiple sources
// producing events.
func TestEventsDispatcherWithMultipleSources(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	testEventsDispatcherWithSources(t, 10, eventsStorePath)
}

// TestEventsDispatcherWithMultipleSourceAndWriters tests the dispatcher with multiple
// writer and multiple sources.
func TestEventsDispatcherWithMultipleSourceAndWriters(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(dedupInterval, sendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	numWriters := 3
	numSources := 10

	// copy event
	event := *dummyEvt

	// create wait groups for producers and writers
	writerWG := new(sync.WaitGroup)
	writerWG.Add(numWriters)

	producerWG := new(sync.WaitGroup)
	producerWG.Add(numSources + 1) // +1 for the main go routine spinning all the sources

	// to receive an error from the writers
	errs := make(chan error, numWriters)
	defer close(errs)

	// channel to stop the event source (producer)
	stopSendingEvents := make(chan struct{}, numSources)

	// create all the writers
	mockWriters := make([]*writers.MockWriter, numWriters)
	for i := 0; i < numWriters; i++ {
		writer := writers.NewMockWriter(fmt.Sprintf("mock%d.%s", i, t.Name()), writerChLen, logger)
		writerEventCh, offsetTracker, err := dispatcher.RegisterWriter(writer)
		AssertOk(t, err, "failed to register mock writer with the dispatcher")
		writer.Start(writerEventCh, offsetTracker)
		mockWriters[i] = writer
	}

	// monitor the writers to ensure they're receiving the events
	// error from any writer will be captured in errs channel.
	for i := 0; i < numWriters; i++ {
		go func(mockWriter *writers.MockWriter, dispatcher events.Dispatcher) {
			defer writerWG.Done()
			defer mockWriter.Stop()
			defer dispatcher.UnregisterWriter(mockWriter.Name())

			ticker := time.NewTicker(sendInterval + (10 * time.Millisecond))

			// stop after 5 iterations
			iterations := 0

			totalEventsBySource := map[string]int{}

			// check if the mock writer is receiving events from the dispatcher
			// totalevents received by the writer from each source should increase after each iteration
			// as the sources are continuously producing events.
			for iterations < 5 {
				select {
				case <-ticker.C:
					for s := 0; s < numSources; s++ {
						src := &monitoring.EventSource{
							NodeName:  fmt.Sprintf("node-name%v", s),
							Component: fmt.Sprintf("component%v", s),
						}

						temp := mockWriter.GetEventsBySourceAndType(src, "TestNICDisconnected") +
							mockWriter.GetEventsBySourceAndType(src, "TestNICConnected")
						sourceKey := fmt.Sprintf("%v-%v", src.GetNodeName(), src.GetComponent())
						// should be > than the previous iteration
						if !(temp > totalEventsBySource[sourceKey]) {
							errs <- fmt.Errorf("expected totalEventsBySource>%v, got:%v, source: %v",
								totalEventsBySource[sourceKey], temp, src)
							return
						}
						totalEventsBySource[sourceKey] = temp
					}
					iterations++
				}
			}
		}(mockWriters[i], dispatcher)
	}

	// start writing from multiple sources
	go func() {
		for i := 0; i < numSources; i++ {
			go func(source int) {
				src := &monitoring.EventSource{
					NodeName:  fmt.Sprintf("node-name%v", source),
					Component: fmt.Sprintf("component%v", source),
				}

				// make a copy of the event and set event source and type
				evt1 := *(&event)
				evt1.EventAttributes.Source = src
				evt1.EventAttributes.Type = "TestNICDisconnected"

				evt2 := *(&event)
				evt2.EventAttributes.Source = src
				evt2.EventAttributes.Type = "TestNICConnected"

				for {
					select {
					case <-stopSendingEvents:
						producerWG.Done()
						return
					default:
						creationTime, _ := types.TimestampProto(time.Now())
						timeNow := api.Timestamp{Timestamp: *creationTime}

						evt1.ObjectMeta.CreationTime = timeNow
						evt1.ObjectMeta.ModTime = timeNow
						evt1.ObjectMeta.UUID = uuid.New().String()

						evt2.ObjectMeta.CreationTime = timeNow
						evt2.ObjectMeta.ModTime = timeNow
						evt2.ObjectMeta.UUID = uuid.New().String()

						if err := dispatcher.Action(evt1); err != nil {
							errs <- fmt.Errorf("failed to send event %v", evt1)
						}

						if err := dispatcher.Action(evt2); err != nil {
							errs <- fmt.Errorf("failed to send event %v", evt2)
						}
					}
				}
			}(i)
		}

		producerWG.Done()
	}()

	// wait for all the writers to finish
	writerWG.Wait()

	close(stopSendingEvents)

	// wait for all the producers to stop
	producerWG.Wait()

	// non-blocking receive on the error channel
	select {
	case er := <-errs:
		t.Fatalf("failed with err: %v", er)
	default:
	}
}

// testEventsDispatcherWithSources helper function to test dispatcher with varying
// number of sources.
func testEventsDispatcherWithSources(t *testing.T, numSources int, eventsStorePath string) {
	// create dispatcher
	dispatcher, err := NewDispatcher(dedupInterval, sendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter(fmt.Sprintf("mock.%s", t.Name()), writerChLen, logger)
	writerEventCh, offsetTracker, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh, offsetTracker)
	defer mockWriter.Stop()
	defer dispatcher.UnregisterWriter(mockWriter.Name())

	// copy event
	event := *dummyEvt

	// let multiple workers start sending events from multiple sources
	wg := new(sync.WaitGroup)
	workers := runtime.NumCPU()
	wg.Add(workers * numSources)

	// run the workers
	for w := 0; w < workers; w++ {
		// events are sent from multiple sources
		for s := 0; s < numSources; s++ {
			go func(s int) {
				src := &monitoring.EventSource{
					NodeName:  fmt.Sprintf("node-name%v", s),
					Component: fmt.Sprintf("component%v", s),
				}

				// set event source
				evt1 := *(&event)
				evt1.EventAttributes.Source = src
				evt1.EventAttributes.Type = "TestNICDisconnected"

				evt2 := *(&event)
				evt2.EventAttributes.Source = src
				evt2.EventAttributes.Type = "TestNICConnected"

				// send events to the dispatcher
				for i := 0; i < 10; i++ {
					creationTime, _ := types.TimestampProto(time.Now())
					timeNow := api.Timestamp{Timestamp: *creationTime}

					evt1.ObjectMeta.CreationTime = timeNow
					evt1.ObjectMeta.ModTime = timeNow
					evt1.ObjectMeta.UUID = uuid.New().String()

					evt2.ObjectMeta.CreationTime = timeNow
					evt2.ObjectMeta.ModTime = timeNow
					evt2.ObjectMeta.UUID = uuid.New().String()

					dispatcher.Action(evt1)
					dispatcher.Action(evt2)
				}

				wg.Done()
			}(s)
		}
	}

	wg.Wait()

	// to make sure all the events are flushed to the writers
	dispatcher.Shutdown()

	// ensure the writer received all the events that're sent from each source
	for s := 0; s < numSources; s++ {
		src := &monitoring.EventSource{
			NodeName:  fmt.Sprintf("node-name%v", s),
			Component: fmt.Sprintf("component%v", s),
		}

		expected := workers * 10 // workers * total events sent by each go routine

		// stop only after receiving all the events or timeout
		AssertEventually(t, func() (bool, interface{}) {
			disconnectedEvents := mockWriter.GetEventsBySourceAndType(src, "TestNICDisconnected")
			connectedEvents := mockWriter.GetEventsBySourceAndType(src, "TestNICDisconnected")

			// make sure atleast half the total events are reached before stopping
			if (disconnectedEvents >= expected/2) && (connectedEvents >= expected/2) {
				return true, nil
			}

			return false, fmt.Sprintf("expected atleast: %d events, got: (%d, %d)", expected/2, disconnectedEvents, connectedEvents)
		}, "did not receive all the events produced", string("5ms"), string("120s"))
	}
}

// testEventDispatcherWithWriters helper function to test dispatcher with varying
// number of writers.
func testEventDispatcherWithWriters(t *testing.T, numWriters int, eventsStorePath string) {
	// dispatcher sends events to all the registered writers
	dispatcher, err := NewDispatcher(dedupInterval, sendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// copy event
	event := *dummyEvt

	writerWG := new(sync.WaitGroup)
	writerWG.Add(numWriters)

	producerWG := new(sync.WaitGroup)
	producerWG.Add(1)

	errs := make(chan error, numWriters)
	defer close(errs)

	stopSendingEvents := make(chan struct{}, 1)

	// create all the writers
	mockWriters := make([]*writers.MockWriter, numWriters)
	for i := 0; i < numWriters; i++ {
		writer := writers.NewMockWriter(fmt.Sprintf("mock%d.%s", i, t.Name()), writerChLen, logger)
		writerEventCh, offsetTracker, err := dispatcher.RegisterWriter(writer)
		AssertOk(t, err, "failed to register mock writer with the dispatcher")
		writer.Start(writerEventCh, offsetTracker)
		mockWriters[i] = writer
	}

	// ensure all the writers receive the events
	// any error from the writers will be captured by errs channel.
	for i := 0; i < numWriters; i++ {
		go func(mockWriter *writers.MockWriter, dispatcher events.Dispatcher) {
			defer writerWG.Done()
			defer mockWriter.Stop()
			defer dispatcher.UnregisterWriter(mockWriter.Name())

			ticker := time.NewTicker(sendInterval + (10 * time.Millisecond))

			// stop after 5 iterations
			iterations := 0

			TestNICConnectedEvents := 0
			TestNICDisconnectedEvents := 0

			// check if the mock writer is receiving events from the dispatcher
			// number of events received should keep increasing for every iteration
			// as the producer is continuously generating events.
			for iterations < 5 {
				select {
				case <-ticker.C:
					// make sure the writer received these events
					temp := mockWriter.GetEventsByType("TestNICConnected")
					// should > than the earlier iteration
					if !(temp > TestNICConnectedEvents) {
						errs <- fmt.Errorf("expected TestNICConnectedEvents>%v, got:%v",
							TestNICConnectedEvents, temp)
						return
					}
					TestNICConnectedEvents = temp

					temp = mockWriter.GetEventsByType("TestNICDisconnected")
					if !(temp > TestNICDisconnectedEvents) {
						errs <- fmt.Errorf("expected TestNICDisconnectedEvents>%v, got:%v",
							TestNICDisconnectedEvents, temp)
						return
					}
					TestNICDisconnectedEvents = temp

					iterations++
				}
			}
		}(mockWriters[i], dispatcher)
	}

	// send some events to the dispatcher
	go func() {
		// copy event and set event type
		evt1 := *(&event)
		evt1.EventAttributes.Type = "TestNICDisconnected"

		evt2 := *(&event)
		evt2.EventAttributes.Type = "TestNICConnected"

		for {
			select {
			case <-stopSendingEvents:
				producerWG.Done()
				return
			default:
				creationTime, _ := types.TimestampProto(time.Now())
				timeNow := api.Timestamp{Timestamp: *creationTime}
				evt1.ObjectMeta.CreationTime = timeNow
				evt1.ObjectMeta.ModTime = timeNow
				evt1.ObjectMeta.UUID = uuid.New().String()

				evt2.ObjectMeta.CreationTime = timeNow
				evt2.ObjectMeta.ModTime = timeNow
				evt2.ObjectMeta.UUID = uuid.New().String()

				dispatcher.Action(evt1)
				dispatcher.Action(evt2)
			}
		}
	}()

	// wait for all the writers to finish
	writerWG.Wait()

	close(stopSendingEvents)

	// wait for the producer to stop
	producerWG.Wait()

	// non-blocking receive on the error channel
	select {
	case er := <-errs:
		t.Fatalf("failed with err: %v", er)
	default:
	}
}

// TestEventsDispatcherRestart tests how the dispatcher behaves during restart.
func TestEventsDispatcherRestart(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(dedupInterval, sendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter(fmt.Sprintf("mock.%s", t.Name()), writerChLen, logger)
	writerEventCh, offsetTracker, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh, offsetTracker)
	defer mockWriter.Stop()
	defer dispatcher.UnregisterWriter(mockWriter.Name())

	// process failed events; this will be a NO-OP as there are no events recorded yet
	dispatcher.ProcessFailedEvents()

	// ensure the mock writer receive no events
	Assert(t, mockWriter.GetTotalEvents() == 0, "expected 0 events")

	// send some events
	evt := *dummyEvt
	for i := 0; i < 100; i++ {
		creationTime, _ := types.TimestampProto(time.Now())
		timeNow := api.Timestamp{Timestamp: *creationTime}

		evt.ObjectMeta.CreationTime = timeNow
		evt.ObjectMeta.ModTime = timeNow
		evt.ObjectMeta.UUID = uuid.New().String()

		AssertOk(t, dispatcher.Action(evt), "failed to send event")
	}

	// ensure the mock writer received these events
	AssertEventually(t, func() (bool, interface{}) {
		totalEvents := mockWriter.GetTotalEvents()
		if totalEvents == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 100 events, got: %d", totalEvents)
	}, "did not receive all the events from the bookmark", string("5ms"), string("5s"))

	// let us assume proxy restarted at this point

	// create new dispatcher after a restart
	dispatcher, err = NewDispatcher(dedupInterval, sendInterval, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// create writers after a restart
	mockWriter = writers.NewMockWriter(fmt.Sprintf("mock.%s", t.Name()), writerChLen, logger)
	writerEventCh, offsetTracker, err = dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh, offsetTracker)
	defer mockWriter.Stop()
	defer dispatcher.UnregisterWriter(mockWriter.Name())

	// to test whether the events are replayed during restart; lets reset the offset in the writer's offset file
	// open the offset file and update the offset. so that the disatcher can replay events for this writer.
	fh, err := os.OpenFile(path.Join(eventsStorePath, "offset", fmt.Sprintf("mock.%s", t.Name())),
		os.O_RDWR, 0) // open file in read write mode
	AssertOk(t, err, "failed to open offset file")
	defer fh.Close() // close the file handler

	if _, err := fh.WriteAt([]byte("0"), 0); err != nil {
		log.Errorf("could not write to offset file, err: %v", err)
	}

	fh.Truncate(1)

	// ensure the writer received 0 events before processing the failed events (new writer)
	Assert(t, mockWriter.GetTotalEvents() == 0, "expected 0 events")

	dispatcher.ProcessFailedEvents()

	AssertEventually(t, func() (bool, interface{}) {
		totalEvents := mockWriter.GetTotalEvents()
		if totalEvents >= 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: >=100 events, got: %d", totalEvents)
	}, "did not receive all the events from the bookmark", string("5ms"), string("5s"))

	// once the failed events are processed; the proxy will starts it's normal operation
	evt = *dummyEvt
	evt.EventAttributes.Type = "DUMMY2"
	for i := 0; i < 10; i++ {
		creationTime, _ := types.TimestampProto(time.Now())
		timeNow := api.Timestamp{Timestamp: *creationTime}

		evt.ObjectMeta.CreationTime = timeNow
		evt.ObjectMeta.ModTime = timeNow
		evt.ObjectMeta.UUID = uuid.New().String()

		AssertOk(t, dispatcher.Action(evt), "failed to send event")
	}

	// now, the writer should have replayed events + new events
	AssertEventually(t, func() (bool, interface{}) {
		totalEvents := mockWriter.GetTotalEvents()
		if totalEvents >= 110 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: >=110 events, got: %d", totalEvents)
	}, "did not receive all the events from the bookmark", string("5ms"), string("5s"))

	AssertEventually(t, func() (bool, interface{}) {
		numEvents := mockWriter.GetEventsByType("DUMMY2")
		if numEvents >= 10 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: >=10 events, got: %d", numEvents)
	}, "did not receive all the events from the bookmark", string("5ms"), string("5s"))
}

// TestEventsDispatcherExpiry tests the expiry of dedup cache items
func TestEventsDispatcherExpiry(t *testing.T) {
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher; all the events should be expired after a second
	dispatcher, err := NewDispatcher(1*time.Second, 10*time.Millisecond, eventsStorePath, logger)
	AssertOk(t, err, "failed to create dispatcher")
	defer dispatcher.Shutdown()

	// create writer
	mockWriter := writers.NewMockWriter(fmt.Sprintf("mock.%s", t.Name()), writerChLen, logger)
	writerEventCh, offsetTracker, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh, offsetTracker)
	defer mockWriter.Stop()
	defer dispatcher.UnregisterWriter(mockWriter.Name())

	// ensure the mock writer received no events
	Assert(t, mockWriter.GetTotalEvents() == 0, "expected 0 events")

	// send some events
	evt := *dummyEvt
	evtUUID := uuid.New().String() // all the consequetive duplicate events will be deduped under this event
	evt.ObjectMeta.UUID = evtUUID
	for i := 0; i < 100; i++ {
		creationTime, _ := types.TimestampProto(time.Now())
		timeNow := api.Timestamp{Timestamp: *creationTime}

		evt.ObjectMeta.CreationTime = timeNow
		evt.ObjectMeta.ModTime = timeNow

		AssertOk(t, dispatcher.Action(evt), "failed to send event")
		evt.ObjectMeta.UUID = uuid.New().String()
	}

	AssertEventually(t, func() (bool, interface{}) {
		evt := mockWriter.GetEventByUUID(evtUUID)
		if evt != nil && evt.GetCount() == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 100 events, got: %v", evt)
	}, "did not receive all the events from the dispatcher", string("5ms"), string("5s"))

	// make sure there are no other events received by the writer
	Assert(t, mockWriter.GetTotalEvents() == 100, "expected 100 events")

	// sleep for a second; the existing events will be expired
	time.Sleep(time.Second)

	// any event sent now will be a new event in the cache and deduplication will start from there on
	newEvtUUID := uuid.New().String()
	evt.ObjectMeta.UUID = newEvtUUID
	for i := 0; i < 100; i++ {
		creationTime, _ := types.TimestampProto(time.Now())
		timeNow := api.Timestamp{Timestamp: *creationTime}

		evt.ObjectMeta.CreationTime = timeNow
		evt.ObjectMeta.ModTime = timeNow

		AssertOk(t, dispatcher.Action(evt), "failed to send event")
		evt.ObjectMeta.UUID = uuid.New().String()
	}

	// old UUID will remain with same event count as there should not be any updates to the expired event.
	fmt.Println(mockWriter.GetEventByUUID(evtUUID).GetCount())
	Assert(t, mockWriter.GetEventByUUID(evtUUID).GetCount() == 100, "expected no updates to the expired event")

	// now, the new UUID should have all the duplicates aggregated
	AssertEventually(t, func() (bool, interface{}) {
		evt := mockWriter.GetEventByUUID(newEvtUUID)
		if evt != nil && evt.GetCount() == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 100 events, got: %v", evt)
	}, "did not receive all the events from the dispatcher", string("5ms"), string("5s"))

	// old + new events
	Assert(t, mockWriter.GetTotalEvents() == 200, "expected 100 events")
}
