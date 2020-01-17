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
	"sync/atomic"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/google/uuid"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	dummyEvtSource = &evtsapi.EventSource{
		Component: "test",
		NodeName:  "test",
	}

	dummyEvt = &evtsapi.Event{
		TypeMeta: api.TypeMeta{
			Kind: "Event",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			UUID:      uuid.New().String(),
		},
		EventAttributes: evtsapi.EventAttributes{
			Source:   dummyEvtSource,
			Severity: eventattrs.Severity_INFO.String(),
			Type:     "DUMMY",
			Count:    1,
		},
	}

	dedupInterval = 100 * time.Second
	sendInterval  = 10 * time.Millisecond

	exporterChLen = 30

	logger    = log.GetNewLogger(log.GetDefaultConfig("dispatcher-test"))
	eventsDir = "/tmp"
)

// TestEventsDispatcher tests the dispatcher's general functionality (distribute and receive)
func TestEventsDispatcher(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	dispatcher.Start() // start the dispatcher again; NO-OP
	defer dispatcher.Shutdown()

	// create and start exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)
	defer mockExporter.Stop()
	defer dispatcher.UnregisterExporter(mockExporter.Name())

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
		for i := 0; i < 10; i++ { // all events will be de-duped under the first (1st occurrence of an event in dedup interval) UUID
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

	// ensure the exporter received all the events that're sent
	totalEvents := iterations * 10 // 10 iterations sent 10 events each

	AssertEventually(t, func() (bool, interface{}) {
		disconnected := mockExporter.GetEventsByType("TestNICDisconnected")
		connected := mockExporter.GetEventsByType("TestNICConnected")
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

	// ensure the events are de-duped
	AssertEventually(t, func() (bool, interface{}) {
		disconnectedEvt := mockExporter.GetEventByUUID(NICDisconnectedEvtUUID)
		connectedEvt := mockExporter.GetEventByUUID(NICConnectedEvtUUID)
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
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create and start mock exporters
	mockExporter1 := exporters.NewMockExporter(fmt.Sprintf("mock1.%s", t.Name()), exporterChLen, logger)
	exporterEventCh1, offsetTracker1, err := dispatcher.RegisterExporter(mockExporter1)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter1.Start(exporterEventCh1, offsetTracker1)
	defer mockExporter1.Stop()
	defer dispatcher.UnregisterExporter(mockExporter1.Name())

	mockExporter2 := exporters.NewMockExporter(fmt.Sprintf("mock2.%s", t.Name()), exporterChLen, logger)
	exporterEventCh2, offsetTracker2, err := dispatcher.RegisterExporter(mockExporter2)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter2.Start(exporterEventCh2, offsetTracker2)
	defer mockExporter2.Stop()
	defer dispatcher.UnregisterExporter(mockExporter2.Name())

	// flushes and closes the exporters gracefully
	dispatcher.Shutdown()

	// no-op; they are already stopped during shutdown
	mockExporter1.Stop()
	mockExporter2.Stop()

	// no-op
	dispatcher.Shutdown()
}

// TestEventsDispatcherFlush tests dispatcher's flush functionality.
// events are flushed internally when shutdown is called.
func TestEventsDispatcherFlush(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// large value to make sure the interval does not kick in before the flush;
	// it is possible that events could be flushed before hitting the batch interval if there is a file rotation.
	interval := 5 * time.Second

	dispatcher, err := NewDispatcher(t.Name(), 10*time.Second, interval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create and start exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)
	defer mockExporter.Stop()
	defer dispatcher.UnregisterExporter(mockExporter.Name())

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

	// exporter would have not received any de-duped event because the batch internal is not hit
	temp := mockExporter.GetEventsByType("TestNICConnected")
	Assert(t, temp < 100, "expected: <100 events, got:%v", temp)

	temp = mockExporter.GetEventsByType("TestNICDisconnected")
	Assert(t, temp < 100, "expected: <100 events, got:%v", temp)

	// deduped events from the dispatcher will be flushed to all the exporters
	dispatcher.Shutdown()

	// exporter would have not received any de-duped event because the batch internal is not hit
	AssertEventually(t, func() (bool, interface{}) {
		disconnected := mockExporter.GetEventsByType("TestNICDisconnected")
		connected := mockExporter.GetEventsByType("TestNICConnected")

		if disconnected == 100 && connected == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected 100 events for each type, got: (TestNICDisconnected %d, TestNICConnected %d)",
			disconnected, connected)
	}, "exporter did not receive all the events or flush operation failed", string("5ms"), string("5s"))

	// thus the exporters received the events before the dispatcher could hit batch interval
}

// TestEventsDispatcherRegisterExporter tests dispatcher's register exporter functionality
func TestEventsDispatcherRegisterExporter(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	dispatcher, err := NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create and start exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	eventsCh, offsetTracker, err := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to register exporter")
	mockExporter.Start(eventsCh, offsetTracker)
	defer mockExporter.Stop()

	_, _, err = dispatcher.RegisterExporter(mockExporter)
	Assert(t, strings.Contains(err.Error(), "name exists already"),
		"expected failure; exporter with the same name exists already")

	mockExporter1 := exporters.NewMockExporter(fmt.Sprintf("mock1.%s", t.Name()), exporterChLen, logger)
	eventsCh, offsetTracker, err = dispatcher.RegisterExporter(mockExporter1)
	AssertOk(t, err, "failed to register exporter")
	mockExporter1.Start(eventsCh, offsetTracker)
	defer mockExporter1.Stop()

	// dispatcher should not register any more exporters
	dispatcher.Shutdown()

	mockExporter2 := exporters.NewMockExporter(fmt.Sprintf("mock2.%s", t.Name()), exporterChLen, logger)
	_, _, err = dispatcher.RegisterExporter(mockExporter2)
	Assert(t, strings.Contains(err.Error(), "dispatcher stopped"),
		"expected failure; stopped dispatcher should not accept register requests")
}

// TestEventsDispatcherWithSingleExporter tests the dispatcher with single exporter.
func TestEventsDispatcherWithSingleExporter(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	testEventDispatcherWithExporters(t, 1, eventsStorePath, logger)
}

// TestEventsDispatcherWithMultipleExporters tests the dispatcher with multiple exporters.
func TestEventsDispatcherWithMultipleExporters(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	testEventDispatcherWithExporters(t, 10, eventsStorePath, logger)
}

// TestEventsDispatcherWithSingleSource tests the dispatcher with single source
// producing events.
func TestEventsDispatcherWithSingleSource(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	testEventsDispatcherWithSources(t, 1, eventsStorePath, logger)
}

// TestEventsDispatcherWithMultipleSources tests the dispatcher with multiple sources
// producing events.
func TestEventsDispatcherWithMultipleSources(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	testEventsDispatcherWithSources(t, 10, eventsStorePath, logger)
}

// TestEventsDispatcherWithMultipleSourceAndExporters tests the dispatcher with multiple
// exporter and multiple sources.
func TestEventsDispatcherWithMultipleSourceAndExporters(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	numExporters := 3
	numSources := 10

	// copy event
	event := *dummyEvt

	// create wait groups for producers and exporters
	exporterWG := new(sync.WaitGroup)
	exporterWG.Add(numExporters)

	producerWG := new(sync.WaitGroup)
	producerWG.Add(numSources + 1) // +1 for the main go routine spinning all the sources

	// channel to stop the event source (producer)
	stopSendingEvents := make(chan struct{}, numSources)

	// create all the exporters
	mockExporters := make([]*exporters.MockExporter, numExporters)
	for i := 0; i < numExporters; i++ {
		exporter := exporters.NewMockExporter(fmt.Sprintf("mock%d.%s", i, t.Name()), exporterChLen, logger)
		exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(exporter)
		AssertOk(t, err, "failed to register mock exporter with the dispatcher")
		exporter.Start(exporterEventCh, offsetTracker)
		mockExporters[i] = exporter
	}

	// monitor the exporters to ensure they're receiving the events
	// error from any exporter will be captured in assertion.
	for i := 0; i < numExporters; i++ {
		go func(mockExporter *exporters.MockExporter, dispatcher events.Dispatcher) {
			defer exporterWG.Done()
			defer mockExporter.Stop()
			defer dispatcher.UnregisterExporter(mockExporter.Name())

			ticker := time.NewTicker(sendInterval + (10 * time.Millisecond))

			totalEventsBySource := map[string]int{}

			// check if the mock exporter is receiving events from the dispatcher.
			// total events received by the exporter from each source should increase after each iteration
			// as the sources are continuously producing events.
			for iterations := 0; iterations < 5; iterations++ {
				select {
				case <-ticker.C:
					for s := 0; s < numSources; s++ {
						src := &evtsapi.EventSource{
							NodeName:  t.Name(),
							Component: fmt.Sprintf("component%v", s),
						}
						sourceKey := fmt.Sprintf("%v-%v", src.GetNodeName(), src.GetComponent())

						AssertEventually(t, func() (bool, interface{}) {
							temp := mockExporter.GetEventsBySourceAndType(src, "TestNICDisconnected") +
								mockExporter.GetEventsBySourceAndType(src, "TestNICConnected")

							// should be > than the previous iteration
							if !(temp > totalEventsBySource[sourceKey]) {
								return false, fmt.Sprintf("exporter {%s}: expected totalEventsBySource: >%d, got: %d, source: %v",
									mockExporter.Name(), totalEventsBySource[sourceKey], temp, src)
							}
							totalEventsBySource[sourceKey] = temp

							return true, nil
						}, "did not receive all the events produced", string("5ms"), string("5s"))
					}
				}
			}
		}(mockExporters[i], dispatcher)
	}

	// start writing from multiple sources
	go func() {
		for i := 0; i < numSources; i++ {
			go func(source int) {
				src := &evtsapi.EventSource{
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
							AssertOk(t, err, "failed to send event %v", evt1)
						}

						if err := dispatcher.Action(evt2); err != nil {
							AssertOk(t, err, "failed to send event %v", evt1)
						}
					}

					time.Sleep(10 * time.Millisecond)
				}
			}(i)
		}

		producerWG.Done()
	}()

	// wait for all the exporters to finish
	exporterWG.Wait()

	close(stopSendingEvents)

	// wait for all the producers to stop
	producerWG.Wait()
}

// testEventsDispatcherWithSources helper function to test dispatcher with varying
// number of sources.
func testEventsDispatcherWithSources(t *testing.T, numSources int, eventsStorePath string, logger log.Logger) {
	// create dispatcher
	dispatcher, err := NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create and start exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)
	defer mockExporter.Stop()
	defer dispatcher.UnregisterExporter(mockExporter.Name())

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
				src := &evtsapi.EventSource{
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

	// to make sure all the events are flushed to the exporters
	dispatcher.Shutdown()

	// ensure the exporter received all the events that're sent from each source
	for s := 0; s < numSources; s++ {
		src := &evtsapi.EventSource{
			NodeName:  t.Name(),
			Component: fmt.Sprintf("component%v", s),
		}

		expected := workers * 10 // workers * total events sent by each go routine

		// stop only after receiving all the events or timeout
		AssertEventually(t, func() (bool, interface{}) {
			disconnectedEvents := mockExporter.GetEventsBySourceAndType(src, "TestNICDisconnected")
			connectedEvents := mockExporter.GetEventsBySourceAndType(src, "TestNICDisconnected")

			// make sure atleast half the total events are reached before stopping
			if (disconnectedEvents >= expected/2) && (connectedEvents >= expected/2) {
				return true, nil
			}

			return false, fmt.Sprintf("expected atleast: %d events, got: (%d, %d)", expected/2, disconnectedEvents, connectedEvents)
		}, "did not receive all the events produced", string("5ms"), string("120s"))
	}
}

// testEventDispatcherWithExporters helper function to test dispatcher with varying
// number of exporters.
func testEventDispatcherWithExporters(t *testing.T, numExporters int, eventsStorePath string, logger log.Logger) {
	// dispatcher sends events to all the registered exporter
	dispatcher, err := NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// copy event
	event := *dummyEvt

	exporterWG := new(sync.WaitGroup)
	exporterWG.Add(numExporters)

	producerWG := new(sync.WaitGroup)
	producerWG.Add(1)

	stopSendingEvents := make(chan struct{}, 1)

	// create all the exporters
	mockExporters := make([]*exporters.MockExporter, numExporters)
	for i := 0; i < numExporters; i++ {
		exporter := exporters.NewMockExporter(fmt.Sprintf("mock%d.%s", i, t.Name()), exporterChLen, logger)
		exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(exporter)
		AssertOk(t, err, "failed to register mock exporter with the dispatcher")
		exporter.Start(exporterEventCh, offsetTracker)
		mockExporters[i] = exporter
	}

	// ensure all the exporters receive the events
	// any error from the exporters will be captured in assertion.
	for i := 0; i < numExporters; i++ {
		go func(mockExporter *exporters.MockExporter, dispatcher events.Dispatcher) {
			defer exporterWG.Done()
			defer mockExporter.Stop()
			defer dispatcher.UnregisterExporter(mockExporter.Name())

			ticker := time.NewTicker(sendInterval + (10 * time.Millisecond))

			TestNICConnectedEvents := 0
			TestNICDisconnectedEvents := 0

			// check if the mock exporter is receiving events from the dispatcher
			// number of events received should keep increasing for every iteration
			// as the producer is continuously generating events.
			for iterations := 0; iterations < 5; iterations++ {
				select {
				case <-ticker.C:
					AssertEventually(t, func() (bool, interface{}) {
						// make sure the exporter received these events
						temp := mockExporter.GetEventsByType("TestNICConnected")
						// should > than the earlier iteration
						if !(temp > TestNICConnectedEvents) {
							return false, fmt.Sprintf("exporter {%s}: expected TestNICConnectedEvents: >%d, got: %d",
								mockExporter.Name(), TestNICConnectedEvents, temp)
						}
						TestNICConnectedEvents = temp

						temp = mockExporter.GetEventsByType("TestNICDisconnected")
						if !(temp > TestNICDisconnectedEvents) {
							return false, fmt.Sprintf("exporter {%s}: expected TestNICDisconnectedEvents: >%d, got: %d",
								mockExporter.Name(), TestNICDisconnectedEvents, temp)
						}
						TestNICDisconnectedEvents = temp

						return true, nil
					}, "did not receive all the events produced", string("10ms"), string("5s"))
				}
			}
		}(mockExporters[i], dispatcher)
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
			case <-time.After(10 * time.Millisecond):
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

	// wait for all the exporters to finish
	exporterWG.Wait()

	close(stopSendingEvents)

	// wait for the producer to stop
	producerWG.Wait()
}

// TestEventsDispatcherRestart tests how the dispatcher behaves during restart.
func TestEventsDispatcherRestart(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create and start exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)
	defer mockExporter.Stop()
	defer dispatcher.UnregisterExporter(mockExporter.Name())

	// ensure the mock exporter receive no events
	Assert(t, mockExporter.GetTotalEvents() == 0, "expected: 0 events, got: %v", mockExporter.GetTotalEvents())

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

	// ensure the mock exporter received these events
	AssertEventually(t, func() (bool, interface{}) {
		totalEvents := mockExporter.GetTotalEvents()
		if totalEvents == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 100 events, got: %d", totalEvents)
	}, "did not receive all the events from the bookmark", string("5ms"), string("5s"))

	// let us assume proxy restarted at this point

	// create new dispatcher after a restart
	dispatcher, err = NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create exporter after a restart
	mockExporter = exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err = dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)

	// to test whether the events are replayed during restart; lets reset the offset in the exporter's offset file
	// open the offset file and update the offset. so that the dispatcher can replay events for this exporter.
	fh, err := os.OpenFile(path.Join(eventsStorePath, "offset", fmt.Sprintf("mock.%s", t.Name())),
		os.O_RDWR, 0) // open file in read write mode
	AssertOk(t, err, "failed to open offset file")
	defer fh.Close() // close the file handler

	offsetBeforeReset, err := offsetTracker.GetOffset()
	AssertOk(t, err, "failed to read offset using the offset tracker")
	offsetMsg := fmt.Sprintf("%s %d", offsetBeforeReset.Filename, 0)
	if _, err := fh.WriteAt([]byte(offsetMsg), 0); err != nil {
		log.Errorf("could not write to offset file, err: %v", err)
	}

	fh.Truncate(int64(len(offsetMsg)))

	// ensure the exporter received 0 events before processing the failed events (new exporter)
	Assert(t, mockExporter.GetTotalEvents() == 0, "expected: 0 events, got: %v", mockExporter.GetTotalEvents())

	// restart the exporter (as the file is reset to 0, it should receive all the events from the beginning as part of the registration)
	mockExporter.Stop()
	dispatcher.UnregisterExporter(mockExporter.Name())
	mockExporter = exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err = dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to Register Exporter")
	mockExporter.Start(exporterEventCh, offsetTracker)

	AssertEventually(t, func() (bool, interface{}) {
		// it should go back to where it left off (after processing failed events)
		offsetAfterReset, _ := offsetTracker.GetOffset()
		if offsetAfterReset.Filename == offsetAfterReset.Filename && offsetBeforeReset.BytesRead == offsetAfterReset.BytesRead {
			return true, nil
		}

		totalEvents := mockExporter.GetTotalEvents()
		if totalEvents >= 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: >=100 events, got: %d", totalEvents)
	}, "did not receive all the events from the bookmark", string("5ms"), string("5s"))
	totalEventsAfterReset := mockExporter.GetTotalEvents()

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

	// now, the exporter should have replayed events + new events
	AssertEventually(t, func() (bool, interface{}) {
		totalEvents := mockExporter.GetTotalEvents()
		if totalEvents >= totalEventsAfterReset+10 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: >=%d events, got: %d", totalEventsAfterReset+10, totalEvents)
	}, "did not receive all the events from the bookmark", string("5ms"), string("5s"))

	AssertEventually(t, func() (bool, interface{}) {
		numEvents := mockExporter.GetEventsByType("DUMMY2")
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
	dispatcher, err := NewDispatcher(t.Name(), 1*time.Second, 10*time.Millisecond, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)
	defer mockExporter.Stop()
	defer dispatcher.UnregisterExporter(mockExporter.Name())

	// ensure the mock exporter received no events
	Assert(t, mockExporter.GetTotalEvents() == 0, "expected: 0 events, got: %v", mockExporter.GetTotalEvents())

	// send some events
	evt := *dummyEvt
	evtUUID := uuid.New().String() // all the consecutive duplicate events will be de-duped under this event
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
		evt := mockExporter.GetEventByUUID(evtUUID)
		if evt != nil && evt.GetCount() == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 100 events, got: %v", evt)
	}, "did not receive all the events from the dispatcher", string("5ms"), string("5s"))

	// make sure there are no other events received by the exporter
	Assert(t, mockExporter.GetTotalEvents() == 100, "expected: 100 events, got: %v", mockExporter.GetTotalEvents())

	// sleep for a second; the existing events will be expired
	time.Sleep(time.Second)

	// any event sent now will be a new event in the cache and de-duplication will start from there on
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
	Assert(t, mockExporter.GetEventByUUID(evtUUID).GetCount() == 100, "expected no updates to the expired event")

	// now, the new UUID should have all the duplicates aggregated
	AssertEventually(t, func() (bool, interface{}) {
		evt := mockExporter.GetEventByUUID(newEvtUUID)
		if evt != nil && evt.GetCount() == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 100 events, got: %v", evt)
	}, "did not receive all the events from the dispatcher", string("5ms"), string("5s"))

	// old + new events
	Assert(t, mockExporter.GetTotalEvents() == 200, "expected: 200 events, got: %v", mockExporter.GetTotalEvents())
}

// TestDispatcherWithDynamicExporterAndRestart tests the distribution of events to a new exporter which got added
// right before restart. The expectation is that the new exporter should not receive any events that were previously recorded (before this exporter's registraion).
func TestDispatcherWithDynamicExporterAndRestart(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher; all the events should be expired after a second
	dispatcher, err := NewDispatcher(t.Name(), 1*time.Second, 10*time.Millisecond, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create exporter
	var mockExporters []*exporters.MockExporter
	for i := 0; i < 3; i++ {
		mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s.%v", t.Name(), i), exporterChLen, logger)
		exporterEventCh, offsetTracker, dErr := dispatcher.RegisterExporter(mockExporter)
		AssertOk(t, dErr, "failed to register mock exporter with the dispatcher")
		mockExporter.Start(exporterEventCh, offsetTracker)
		defer mockExporter.Stop()
		defer dispatcher.UnregisterExporter(mockExporter.Name())
		mockExporters = append(mockExporters, mockExporter)
	}

	// ensure the mock exporters received no events
	for i := 0; i < 3; i++ {
		Assert(t, mockExporters[i].GetTotalEvents() == 0, "expected: 0 events, got: %v", mockExporters[i].GetTotalEvents())
	}

	// send some events
	evt := *dummyEvt
	evtUUID := uuid.New().String() // all the consecutive duplicate events will be de-duped under this event
	evt.ObjectMeta.UUID = evtUUID
	for i := 0; i < 100; i++ {
		creationTime, _ := types.TimestampProto(time.Now())
		timeNow := api.Timestamp{Timestamp: *creationTime}

		evt.ObjectMeta.CreationTime = timeNow
		evt.ObjectMeta.ModTime = timeNow

		AssertOk(t, dispatcher.Action(evt), "failed to send event")
		evt.ObjectMeta.UUID = uuid.New().String()
	}

	for i := 0; i < 3; i++ {
		AssertEventually(t, func() (bool, interface{}) {
			evt := mockExporters[i].GetEventByUUID(evtUUID)
			if evt != nil && evt.GetCount() == 100 {
				return true, nil
			}

			return false, fmt.Sprintf("expected: 100 events, got: %v", evt)
		}, "did not receive all the events from the dispatcher", string("5ms"), string("5s"))
	}

	// add a new exporter
	newMockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s.new", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(newMockExporter)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	newMockExporter.Start(exporterEventCh, offsetTracker)
	defer newMockExporter.Stop()
	defer dispatcher.UnregisterExporter(newMockExporter.Name())

	time.Sleep(1 * time.Second)

	// check the count of events on the new exporter
	Assert(t, newMockExporter.GetTotalEvents() == 0, "expected: 0 events, got: %v", newMockExporter.GetTotalEvents())

	// check the old mock exporters
	for i := 0; i < 3; i++ {
		Assert(t, mockExporters[i].GetTotalEvents() == 100, "expected: 100 events, got: %v", mockExporters[i].GetTotalEvents())
	}

	// start recording some events and check the count
	evtUUID = uuid.New().String() // all the consecutive duplicate events will be de-duped under this event
	evt.ObjectMeta.UUID = evtUUID
	for i := 0; i < 100; i++ {
		creationTime, _ := types.TimestampProto(time.Now())
		timeNow := api.Timestamp{Timestamp: *creationTime}

		evt.ObjectMeta.CreationTime = timeNow
		evt.ObjectMeta.ModTime = timeNow

		AssertOk(t, dispatcher.Action(evt), "failed to send event")
		evt.ObjectMeta.UUID = uuid.New().String()
	}

	AssertEventually(t, func() (bool, interface{}) { // new exporter should receive 100 events
		evt := newMockExporter.GetEventByUUID(evtUUID)
		if evt != nil && evt.GetCount() == 100 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 100 events, got: %v", evt)
	}, "did not receive all the events from the dispatcher", string("5ms"), string("5s"))
}

// TestEventsDispatcherCacheExpiry ensures any update to the expired event should result in a new event.
// Expiration is not reset during update.
func TestEventsDispatcherCacheExpiry(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher; all the events should be expired after a second
	dispatcher, err := NewDispatcher(t.Name(), 500*time.Millisecond, 10*time.Millisecond, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create mock exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, dErr := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, dErr, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)
	defer mockExporter.Stop()
	defer dispatcher.UnregisterExporter(mockExporter.Name())

	var wg sync.WaitGroup
	wg.Add(1)
	defer wg.Wait()

	stopUpdatingEvents := make(chan struct{}, 1)
	defer close(stopUpdatingEvents)

	// send some events
	evt := *dummyEvt
	evtUUID := uuid.New().String() // all the consecutive duplicate events will be de-duped under this event
	evt.ObjectMeta.UUID = evtUUID
	creationTime, _ := types.TimestampProto(time.Now())
	timeCreated := api.Timestamp{Timestamp: *creationTime} // first event creation time
	evt.ObjectMeta.CreationTime = timeCreated
	go func() {
		for {
			select {
			case <-stopUpdatingEvents:
				wg.Done()
				return
			default:
				AssertOk(t, dispatcher.Action(evt), "failed to send event")
				time.Sleep(4 * time.Millisecond)
				creationTime, _ := types.TimestampProto(time.Now())
				timeNow := api.Timestamp{Timestamp: *creationTime}
				evt.ObjectMeta.CreationTime = timeNow
			}
		}
	}()

	expTime := time.Now().Add(500 * time.Millisecond) // after expiry
	prevCount := uint32(0)

	for {
		select {
		case <-time.After(time.Until(expTime.Add(100 * time.Millisecond))): // after expiry + buffer(100ms)
			t.Logf("prevCount is %d after expiration time", prevCount)

			// look for a new event
			AssertEventually(t, func() (bool, interface{}) { // new exporter should receive 100 events
				evt := mockExporter.GetEventByUUID(evtUUID)
				if evt != nil {
					cTime, _ := evt.CreationTime.Time()           // event creation time
					firstEvtCreationTime, _ := timeCreated.Time() // fist event creation time

					if cTime.Equal(firstEvtCreationTime) {
						msg := fmt.Sprintf("new event should have been created after expiry instead got event count: %v", evt.GetCount())
						t.Logf(msg)
						return false, msg
					}

					// it is a new event when creation time is after the first event's creation time and prev count > event count
					if cTime.After(firstEvtCreationTime) && (evt.GetCount() > 0 && prevCount > evt.GetCount()) {
						t.Logf("old event is expired and new event got created, evt.Count: %v", evt.GetCount())
						return true, nil
					}

					return false, nil
				}

				return false, "nil event"
			}, "expected cache expiry did not happen", string("20ms"), string("5s"))
			return

		case <-time.After(100 * time.Millisecond): // after every few batch intervals
			evt := mockExporter.GetEventByUUID(evtUUID)
			Assert(t, evt != nil, "unexpected, event is nil")

			cTime, _ := evt.CreationTime.Time()           // event creation time
			firstEvtCreationTime, _ := timeCreated.Time() // fist event creation time

			// ensure the event is the same as the original event (first event) and check count
			if cTime.Equal(firstEvtCreationTime) {
				Assert(t, evt.GetCount() >= prevCount, "expected evt count: >= %v, got: %v", prevCount, evt.GetCount())
				prevCount = evt.GetCount()
			}
		}
	}
}

// TestEventsDispatcherWithSlowExporter tests the dispatcher pipeline with slow exporter.
// Exporter will be way behind in processing the events that were sent due to the slowness. As a result, more events
// will be piled up in consecutive event files (due to rotation and max. num files). So, where is a restart
// the exporter should receive all the events that it missed (from where it left off) and this list of events will come
// different event files.
func TestEventsDispatcherWithSlowExporter(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	var wg sync.WaitGroup

	// create dispatcher; all the events should be expired after a second
	dispatcher, err := NewDispatcher(t.Name(), 10*time.Second, 10*time.Millisecond,
		&events.StoreConfig{Dir: eventsStorePath, MaxFileSize: 100 * 1000, MaxNumFiles: 50}, nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create mock exporter
	oMockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), 10000, logger, exporters.WithSleepBetweenMockReads(100*time.Millisecond))
	exporterEventCh, offsetTracker, dErr := dispatcher.RegisterExporter(oMockExporter)
	AssertOk(t, dErr, "failed to register mock exporter with the dispatcher")
	oMockExporter.Start(exporterEventCh, offsetTracker)
	defer oMockExporter.Stop()

	// record events
	wg.Add(1)
	var count uint32
	stopSendingEvents := make(chan struct{})
	go func() {
		defer wg.Done()
		evt := *dummyEvt

		for {
			select {
			case <-stopSendingEvents:
				return
			case <-time.After(1 * time.Millisecond):
				evtUUID := uuid.New().String()
				evt.ObjectMeta.UUID = evtUUID
				evt.Message = fmt.Sprintf("message - %d", atomic.LoadUint32(&count))
				err := dispatcher.Action(evt)
				AssertOk(t, err, "failed to send event, err: %v", err)
				atomic.AddUint32(&count, 1)
			}
		}
	}()

	// meantime, events should be piled up in different event files as the syslog receiver is reading a slow pace.
	time.Sleep(5 * time.Second)

	// simulate a restart; during this restart, exporter should receive all the events that it missed.
	oMockExporter.Stop()
	dispatcher.UnregisterExporter(oMockExporter.Name())
	nMockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), 10000, logger)
	exporterEventCh, offsetTracker, dErr = dispatcher.RegisterExporter(nMockExporter)
	AssertOk(t, dErr, "failed to register mock exporter with the dispatcher")
	nMockExporter.Start(exporterEventCh, offsetTracker)
	defer nMockExporter.Stop()

	time.Sleep(4 * time.Second)
	close(stopSendingEvents)

	// ensure the syslog server receives
	AssertEventually(t,
		func() (bool, interface{}) {
			totalRecieved := uint32(oMockExporter.GetTotalEvents() + nMockExporter.GetTotalEvents())
			if totalRecieved == atomic.LoadUint32(&count) {
				return true, nil
			}
			return false, fmt.Sprintf("expected: %d, got: %d", count, totalRecieved)
		}, "did not receive expected messages on the mock exporter", "60ms", "100s")
}

// TestEventsDispatcherDeleteExporter tests the delete exporter behavior.
// As part of the delete, offset tracker file should be deleted.
func TestEventsDispatcherDeleteExporter(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher; all the events should be expired after a second
	dispatcher, err := NewDispatcher(t.Name(), 500*time.Millisecond, 10*time.Millisecond,
		&events.StoreConfig{Dir: eventsStorePath}, nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create mock exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, dErr := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, dErr, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)
	defer mockExporter.Stop()

	offsetTrackerFilepath := path.Join(eventsStorePath, "offset", mockExporter.Name())
	_, err = os.Stat(offsetTrackerFilepath)
	AssertOk(t, err, "offset tracker file does not exists, err: %v", err) // make sure the offset tracker file exists

	// delete the exporter
	dispatcher.DeleteExporter(mockExporter.Name())
	_, err = os.Stat(offsetTrackerFilepath)
	Assert(t, err != nil && os.IsNotExist(err), "offset tracker file should have been deleted, but still exists. err: %v", err)
}

// TestEventsSkip ensures events that were recorded with creation time 2000-MM-DD is skipped
func TestEventsSkip(t *testing.T) {
	logger := logger.WithContext("t_name", t.Name())
	eventsStorePath := filepath.Join(eventsDir, t.Name())
	defer os.RemoveAll(eventsStorePath) // cleanup

	// create dispatcher
	dispatcher, err := NewDispatcher(t.Name(), dedupInterval, sendInterval, &events.StoreConfig{Dir: eventsStorePath},
		nil, logger)
	AssertOk(t, err, "failed to create dispatcher")
	dispatcher.Start()
	defer dispatcher.Shutdown()

	// create and start exporter
	mockExporter := exporters.NewMockExporter(fmt.Sprintf("mock.%s", t.Name()), exporterChLen, logger)
	exporterEventCh, offsetTracker, err := dispatcher.RegisterExporter(mockExporter)
	AssertOk(t, err, "failed to register mock exporter with the dispatcher")
	mockExporter.Start(exporterEventCh, offsetTracker)
	defer mockExporter.Stop()
	defer dispatcher.UnregisterExporter(mockExporter.Name())

	event := *dummyEvt
	creationTime, _ := types.TimestampProto(time.Now())
	event.CreationTime = api.Timestamp{Timestamp: *creationTime}
	dispatcher.Action(event)
	AssertEventually(t, func() (bool, interface{}) { // ensure the event is received at the exporter
		if mockExporter.GetTotalEvents() == 1 {
			return true, nil
		}
		return false, nil
	}, "exporter did not receive event(s)", string("10ms"), string("2s"))

	// send events with time backing to 2000's
	creationTime, _ = types.TimestampProto(time.Date(2000, time.January, 2, 0, 0, 0, 0, time.UTC))

	event = *dummyEvt
	event.CreationTime = api.Timestamp{Timestamp: *creationTime}
	event.Message = "t1"
	dispatcher.Action(event)

	event = *dummyEvt
	event.CreationTime = api.Timestamp{Timestamp: *creationTime}
	event.Message = "t2"
	dispatcher.Action(event)

	time.Sleep(2 * sendInterval)
	Assert(t, mockExporter.GetTotalEvents() == 1, "expected event(s) are not skipped")

	event = *dummyEvt
	creationTime, _ = types.TimestampProto(time.Now())
	event.CreationTime = api.Timestamp{Timestamp: *creationTime}
	dispatcher.Action(event)
	AssertEventually(t, func() (bool, interface{}) { // ensure the event is received at the exporter
		if mockExporter.GetTotalEvents() == 2 {
			return true, nil
		}
		return false, nil
	}, "exporter did not receive event(s)", string("10ms"), string("2s"))
}
