// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"fmt"
	"runtime"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/writers"
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
		},
		EventAttributes: evtsapi.EventAttributes{
			Source:   dummyEvtSource,
			Severity: "INFO",
			Type:     "DUMMY",
			Count:    1,
		},
	}

	sendInterval = 30 * time.Millisecond

	writerChLen = 30

	logger = log.GetNewLogger(log.GetDefaultConfig("dispatcher-test"))
)

// TestEventsDispatcher tests the dispatcher's general functionality (distribute and receive)
func TestEventsDispatcher(t *testing.T) {
	// create dispatcher
	dispatcher := NewDispatcher(sendInterval, logger)
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter("mock", writerChLen, logger)
	writerEventCh, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh)
	defer mockWriter.Stop()

	// let multiple workers start sending events
	wg := new(sync.WaitGroup)
	workers := runtime.NumCPU()
	wg.Add(workers)

	// copy event and set type
	event := *dummyEvt
	NICDisconnectedEvt := *(&event)
	NICDisconnectedEvt.EventAttributes.Type = "TestNICDisconnected"

	NICConnectedEvt := *(&event)
	NICConnectedEvt.EventAttributes.Type = "TestNICConnected"

	// run the workers
	for w := 0; w < workers; w++ {
		go func() {
			for i := 0; i < 10; i++ {
				AssertOk(t, dispatcher.Action(NICDisconnectedEvt), "failed to send event")
				AssertOk(t, dispatcher.Action(NICConnectedEvt), "failed to send event")
			}

			wg.Done()
		}()

		// to make sure the events reach the writers before next iteration
		time.Sleep(sendInterval * 2)
	}

	wg.Wait()

	// ensure the writer received all the events that're sent
	expectedUniqueEvents := workers
	expectedRepeatedEvents := workers * 10
	totalEvents := expectedUniqueEvents + expectedRepeatedEvents

	AssertEventually(t, func() (bool, interface{}) {
		uniqueTestNICDisconnectedEvents := mockWriter.GetUniqueEvents("TestNICDisconnected")
		if uniqueTestNICDisconnectedEvents >= expectedUniqueEvents {
			return true, nil
		}

		fmt.Printf("expected: %d events, got: %d\n", expectedUniqueEvents, uniqueTestNICDisconnectedEvents)
		return false, nil
	}, "unexpected number of unique events", string("5ms"), string("5s"))

	AssertEventually(t, func() (bool, interface{}) {
		uniqueTestNICConnectedEvents := mockWriter.GetUniqueEvents("TestNICConnected")
		if uniqueTestNICConnectedEvents >= expectedUniqueEvents {
			return true, nil
		}

		fmt.Printf("expected: %d events, got: %d\n", expectedUniqueEvents, uniqueTestNICConnectedEvents)
		return false, nil
	}, "unexpected number of unique events", string("5ms"), string("5s"))

	AssertEventually(t, func() (bool, interface{}) {
		repeatedTestNICDisconnectedEvents := mockWriter.GetRepeatedEvents("TestNICDisconnected")
		totalTestNICDisconnectedEvents := mockWriter.GetTotalEventsBySourceAndEvent(dummyEvtSource, "TestNICDisconnected")
		if repeatedTestNICDisconnectedEvents >= expectedRepeatedEvents ||
			totalTestNICDisconnectedEvents == totalEvents {
			return true, nil
		}

		fmt.Printf("expected: %d events, got: %d, total events received: %d\n", expectedRepeatedEvents, repeatedTestNICDisconnectedEvents,
			totalTestNICDisconnectedEvents)
		return false, nil
	}, "unexpected number of repeated events", string("5ms"), string("5s"))

	AssertEventually(t, func() (bool, interface{}) {
		repeatedTestNICConnectedEvents := mockWriter.GetRepeatedEvents("TestNICConnected")
		totalTestNICConnectedEvents := mockWriter.GetTotalEventsBySourceAndEvent(dummyEvtSource, "TestNICConnected")
		if repeatedTestNICConnectedEvents >= expectedRepeatedEvents ||
			totalTestNICConnectedEvents == totalEvents {
			return true, nil
		}

		fmt.Printf("expected: %d events, got: %d, total events received: %d\n", expectedRepeatedEvents, repeatedTestNICConnectedEvents,
			totalTestNICConnectedEvents)
		return false, nil
	}, "unexpected repeated number of events", string("5ms"), string("5s"))

	// ensure the writer received all the events
	expected := (workers * 10) + workers // + workers for the first occurrence
	connectedEvts := mockWriter.GetTotalEventsBySourceAndEvent(dummyEvtSource, "TestNICConnected")
	Assert(t, connectedEvts >= expected,
		fmt.Sprintf("unexpected total number of events received. expected: %v, got:%v", expected, connectedEvts))

	disconnectedEvts := mockWriter.GetTotalEventsBySourceAndEvent(dummyEvtSource, "TestNICDisconnected")
	Assert(t, disconnectedEvts >= expected,
		fmt.Sprintf("unexpected total number of events received. expected: %v, got:%v", expected, disconnectedEvts))

	// send an event with missing attributes
	incomEvent := *(&event)
	incomEvent.Severity = ""
	err = dispatcher.Action(incomEvent)
	Assert(t, events.IsMissingEventAttributes(err), "expected failure, should not accept incomplete event")

	dispatcher.Shutdown()

	// sending events after shutdown should fail
	testEvt := *(&event)
	testEvt.EventAttributes.Type = "Test"
	err = dispatcher.Action(testEvt)
	Assert(t, strings.Contains(err.Error(), "dispatcher stopped, cannot process events"),
		"expected failure")
}

// TestEventsDispatcherShutdown tests the graceful shutdown of the dispatcher.
func TestEventsDispatcherShutdown(t *testing.T) {
	// create dispatcher
	dispatcher := NewDispatcher(sendInterval, logger)
	defer dispatcher.Shutdown()

	// create and start mock writers
	mockWriter1 := writers.NewMockWriter("mock1", writerChLen, logger)
	writerEventCh1, err := dispatcher.RegisterWriter(mockWriter1)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter1.Start(writerEventCh1)
	defer mockWriter1.Stop()

	mockWriter2 := writers.NewMockWriter("mock2", writerChLen, logger)
	writerEventCh2, err := dispatcher.RegisterWriter(mockWriter2)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter2.Start(writerEventCh2)
	defer mockWriter2.Stop()

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
	// large value to make sure the interval does not kick in before the flush
	interval := 5 * time.Second

	dispatcher := NewDispatcher(interval, logger)
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter("mock", writerChLen, logger)
	writerEventCh, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh)
	defer mockWriter.Stop()

	// copy event and set type
	event := *dummyEvt
	NICDisconnectedEvt := *(&event)
	NICDisconnectedEvt.EventAttributes.Type = "TestNICDisconnected"

	NICConnectedEvt := *(&event)
	NICConnectedEvt.EventAttributes.Type = "TestNICConnected"

	// send events
	for i := 0; i < 100; i++ {
		AssertOk(t, dispatcher.Action(NICDisconnectedEvt), "failed to send event")
		AssertOk(t, dispatcher.Action(NICConnectedEvt), "failed to send event")
	}

	// writer would have not received any deduped event becase the acitve internal is not hit
	temp := mockWriter.GetRepeatedEvents("TestNICConnected")
	Assert(t, temp == 0, "expected 0 events, got:%v", temp)

	temp = mockWriter.GetRepeatedEvents("TestNICDisconnected")
	Assert(t, temp == 0, "expected 0 events, got:%v", temp)

	// deduped events from the dispatcher will be flushed to all the writers
	dispatcher.Shutdown()

	// give enough buffer to let some events reach writer
	time.Sleep(time.Second)

	temp = mockWriter.GetRepeatedEvents("TestNICConnected")
	Assert(t, temp > 0, "expected >0 events, got:%v", temp)

	temp = mockWriter.GetRepeatedEvents("TestNICDisconnected")
	Assert(t, temp > 0, "expected >0 events, got:%v", temp)

	// thus the writers received the events before the distpatcher could hit active interval
}

// TestEventsDispatcherRegisterWriter tests dispatcher's register writer funtionality
func TestEventsDispatcherRegisterWriter(t *testing.T) {
	dispatcher := NewDispatcher(sendInterval, logger)
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter("mock", writerChLen, logger)
	_, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register writer")

	_, err = dispatcher.RegisterWriter(mockWriter)
	Assert(t, strings.Contains(err.Error(), "name exists already"),
		"expected failure; writer with the same name exists already")

	mockWriter = writers.NewMockWriter("mock1", writerChLen, logger)
	_, err = dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register writer")

	// dispatcher should not register any more writers
	dispatcher.Shutdown()

	mockWriter = writers.NewMockWriter("mock2", writerChLen, logger)
	_, err = dispatcher.RegisterWriter(mockWriter)
	Assert(t, strings.Contains(err.Error(), "dispatcher stopped"),
		"expected failure; stopped dispatcher should not accept register requests")
}

// TestEventsDispatcherWithSingleWriter tests the dispatcher with single writer.
func TestEventsDispatcherWithSingleWriter(t *testing.T) {
	testEventDispatcherWithWriters(t, 1)
}

// TestEventsDispatcherWithMultipleWriters tests the dispatcher with multiple writers.
func TestEventsDispatcherWithMultipleWriters(t *testing.T) {
	testEventDispatcherWithWriters(t, 10)
}

// TestEventsDispatcherWithSingleSource tests the dispatcher with single source
// producing events.
func TestEventsDispatcherWithSingleSource(t *testing.T) {
	testEventsDispatcherWithSources(t, 1)
}

// TestEventsDispatcherWithMultipleSources tests the dispatcher with multiple sources
// producing events.
func TestEventsDispatcherWithMultipleSources(t *testing.T) {
	testEventsDispatcherWithSources(t, 10)
}

// TestEventsDispatcherWithMultipleSourceAndWriters tests the dispatcher with multiple
// writer and multiple sources.
func TestEventsDispatcherWithMultipleSourceAndWriters(t *testing.T) {
	// create dispatcher
	dispatcher := NewDispatcher(sendInterval, logger)
	defer dispatcher.Shutdown()

	numWriters := 3
	numSources := 10

	// copy event
	event := *dummyEvt

	// create wait groups for producers and writers
	writerWG := new(sync.WaitGroup)
	writerWG.Add(numWriters)

	producerWG := new(sync.WaitGroup)
	producerWG.Add(1)

	// to receive an error from the writers
	errs := make(chan error, numWriters)
	defer close(errs)

	// channel to stop the event source (producer)
	stopSendingEvents := make(chan struct{}, numSources)

	// create all the writers
	mockWriters := make([]*writers.MockWriter, numWriters)
	for i := 0; i < numWriters; i++ {
		writer := writers.NewMockWriter(fmt.Sprintf("mock%d", i), writerChLen, logger)
		writerEventCh, err := dispatcher.RegisterWriter(writer)
		AssertOk(t, err, "failed to register mock writer with the dispatcher")
		writer.Start(writerEventCh)
		mockWriters[i] = writer
	}

	// monitor the writers to ensure they're receiving the events
	// error from any writer will be captured in errs channel.
	for i := 0; i < numWriters; i++ {
		go func(mockWriter *writers.MockWriter) {
			defer writerWG.Done()
			defer mockWriter.Stop()

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
						src := &evtsapi.EventSource{
							NodeName:  fmt.Sprintf("node-name%v", s),
							Component: fmt.Sprintf("component%v", s),
						}

						temp := mockWriter.GetTotalEventsBySourceAndEvent(src, "TestNICDisconnected") +
							mockWriter.GetTotalEventsBySourceAndEvent(src, "TestNICConnected")
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
		}(mockWriters[i])
	}

	// start writing from multiple sources
	go func() {
		for {
			for i := 0; i < numSources; i++ {
				src := &evtsapi.EventSource{
					NodeName:  fmt.Sprintf("node-name%v", i),
					Component: fmt.Sprintf("component%v", i),
				}

				// make a copy of the event and set event source and type
				evt1 := *(&event)
				evt1.EventAttributes.Source = src
				evt1.EventAttributes.Type = "TestNICDisconnected"

				evt2 := *(&event)
				evt2.EventAttributes.Source = src
				evt2.EventAttributes.Type = "TestNICConnected"

				select {
				case <-stopSendingEvents:
					producerWG.Done()
					return
				default:
					if err := dispatcher.Action(evt1); err != nil {
						errs <- fmt.Errorf("failed to send event %v", evt1)
					}

					if err := dispatcher.Action(evt2); err != nil {
						errs <- fmt.Errorf("failed to send event %v", evt2)
					}
				}
			}
		}
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
func testEventsDispatcherWithSources(t *testing.T, numSources int) {
	// create dispatcher
	dispatcher := NewDispatcher(sendInterval, logger)
	defer dispatcher.Shutdown()

	// create and start writer
	mockWriter := writers.NewMockWriter("mock", writerChLen, logger)
	writerEventCh, err := dispatcher.RegisterWriter(mockWriter)
	AssertOk(t, err, "failed to register mock writer with the dispatcher")
	mockWriter.Start(writerEventCh)
	defer mockWriter.Stop()

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
		src := &evtsapi.EventSource{
			NodeName:  fmt.Sprintf("node-name%v", s),
			Component: fmt.Sprintf("component%v", s),
		}

		expected := workers * 10 // workers * total events sent by each go routine (10 #iterations)

		// stop only after receiving all the events or timeout
		AssertEventually(t, func() (bool, interface{}) {
			disconnectedEvents := mockWriter.GetTotalEventsBySourceAndEvent(src, "TestNICDisconnected")
			connectedEvents := mockWriter.GetTotalEventsBySourceAndEvent(src, "TestNICDisconnected")

			// make sure atleast half the total events are reached before stopping
			if (disconnectedEvents >= expected/2) && (connectedEvents >= expected/2) {
				return true, nil
			}

			return false, nil
		}, "did not receive all the events produced", string("5ms"), string("120s"))
	}
}

// testEventDispatcherWithWriters helper function to test dispatcher with varying
// number of writers.
func testEventDispatcherWithWriters(t *testing.T, numWriters int) {
	// dispatcher sends events to all the registered writers
	dispatcher := NewDispatcher(sendInterval, logger)
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
		writer := writers.NewMockWriter(fmt.Sprintf("mock%d", i), writerChLen, logger)
		writerEventCh, err := dispatcher.RegisterWriter(writer)
		AssertOk(t, err, "failed to register mock writer with the dispatcher")
		writer.Start(writerEventCh)
		mockWriters[i] = writer
	}

	// ensure all the writers receive the events
	// any error from the writers will be captured by errs channel.
	for i := 0; i < numWriters; i++ {
		go func(mockWriter *writers.MockWriter) {
			defer writerWG.Done()
			defer mockWriter.Stop()

			ticker := time.NewTicker(sendInterval + (10 * time.Millisecond))

			// stop after 5 iterations
			iterations := 0

			uniqueTestNICConnectedEvents := 0
			repeatedTestNICConnectedEvents := 0

			uniqueTestNICDisconnectedEvents := 0
			repeatedTestNICDisconnectedEvents := 0

			// check if the mock writer is receiving events from the dispatcher
			// number of events received should keep increasing for every iteration
			// as the producer is continuously generating events.
			for iterations < 5 {
				select {
				case <-ticker.C:
					// make sure the writer received these events
					temp := mockWriter.GetUniqueEvents("TestNICConnected")
					// should > than the earlier iteration
					if !(temp > uniqueTestNICConnectedEvents) {
						errs <- fmt.Errorf("expected uniqueTestNICConnectedEvents>%v, got:%v",
							uniqueTestNICConnectedEvents, temp)
						return
					}
					uniqueTestNICConnectedEvents = temp

					temp = mockWriter.GetUniqueEvents("TestNICDisconnected")
					if !(temp > uniqueTestNICDisconnectedEvents) {
						errs <- fmt.Errorf("expected uniqueTestNICDisconnectedEvents>%v, got:%v",
							uniqueTestNICDisconnectedEvents, temp)
						return
					}
					uniqueTestNICDisconnectedEvents = temp

					temp = mockWriter.GetRepeatedEvents("TestNICConnected")
					if !(temp > repeatedTestNICConnectedEvents) {
						errs <- fmt.Errorf("expected repeatedTestNICConnectedEvents>%v, got:%v",
							repeatedTestNICConnectedEvents, temp)
						return
					}
					repeatedTestNICConnectedEvents = temp

					temp = mockWriter.GetRepeatedEvents("TestNICDisconnected")
					if !(temp > repeatedTestNICDisconnectedEvents) {
						errs <- fmt.Errorf("expected repeatedTestNICDisconnectedEvents>%v, got:%v",
							repeatedTestNICDisconnectedEvents, temp)
						return
					}
					repeatedTestNICDisconnectedEvents = temp

					iterations++
				}
			}
		}(mockWriters[i])
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
