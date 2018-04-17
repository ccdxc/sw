// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package writers

import (
	"fmt"
	"sync"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

// number of workers to process the incoming events from dispatcher.
const numWorkers = 8

// MockWriter implements the `Writer` interface. this has the logic on how the event
// from dispatcher should be processed.
type MockWriter struct {
	sync.Mutex                                             // to protect this object
	wg                           sync.WaitGroup            // to wait for the watch channel to close
	stop                         sync.Once                 // for stopping the writer
	eventsChan                   events.EventReceiverChan  // to watch/stop events from dispatcher
	stat                         map[string]*eventsStat    // event stats
	totalEventsBySourceAndEvents map[string]map[string]int // total events received by source and event

	name  string // name of the writer
	chLen int    // buffer or channel len

	logger log.Logger
}

// eventsStat represents the number of unique events and repeated events
// unique events are the ones with count = 1 (first occurrence of the event from cache)
// repeated events are the ones with count >1
type eventsStat struct {
	uniqueEvents   int
	repeatedEvents int
}

// NewMockWriter creates and returns  the mock writer interface.
func NewMockWriter(name string, chLen int, logger log.Logger) *MockWriter {
	mockWriter := &MockWriter{
		stat: map[string]*eventsStat{},
		totalEventsBySourceAndEvents: map[string]map[string]int{},
		name:   name,
		chLen:  chLen,
		logger: logger.WithContext("submodule", "mock-writer"),
	}

	return mockWriter
}

// Start starts the writer
func (m *MockWriter) Start(eventsChan events.EventReceiverChan) {
	m.eventsChan = eventsChan

	// to wait for all the workers to be done
	m.wg.Add(numWorkers)

	// divide the work among multiple workers
	for i := 0; i < numWorkers; i++ {
		// start watching events using the given event channel
		go m.startWorker(i)
	}

	m.logger.Info("started mock events writer")
}

// Stop stops the watch by calling `Stop` on the event channel.
func (m *MockWriter) Stop() {
	m.stop.Do(func() {
		m.logger.Info("stopping the mock writer")
		m.eventsChan.Stop()

		// wait for the writer to finish
		m.wg.Wait()
	})
}

// Name returns the name of the mock writer
func (m *MockWriter) Name() string {
	return m.name
}

// ChLen returns the channel length to be set of this mock writer
func (m *MockWriter) ChLen() int {
	return m.chLen
}

// WriteEvents writes list of events
func (m *MockWriter) WriteEvents(events []*evtsapi.Event) error {
	if events == nil {
		return nil
	}

	for _, evt := range events {
		m.writeCount(evt)
	}

	return nil
}

// GetUniqueEvents returns the number of unique events received of the given type.
func (m *MockWriter) GetUniqueEvents(eType string) int {
	m.Lock()
	defer m.Unlock()

	res, ok := m.stat[eType]
	if ok {
		return res.uniqueEvents
	}

	return 0
}

// GetRepeatedEvents returns the number of repeated events received of the given type.
func (m *MockWriter) GetRepeatedEvents(eType string) int {
	m.Lock()
	defer m.Unlock()

	res, ok := m.stat[eType]
	if ok {
		return res.repeatedEvents
	}

	return 0
}

// GetTotalEventsBySourceAndEvent returns the total number of events received from the given source
// of the given event type.
func (m *MockWriter) GetTotalEventsBySourceAndEvent(source *evtsapi.EventSource, eventType string) int {
	m.Lock()
	defer m.Unlock()

	sourceKey := fmt.Sprintf("%v-%v", source.GetNodeName(), source.GetComponent())
	if m.totalEventsBySourceAndEvents[sourceKey] != nil {
		return m.totalEventsBySourceAndEvents[sourceKey][eventType]
	}

	return 0
}

// writeCount helper function to update the stats and counter based on the received event.
func (m *MockWriter) writeCount(event *evtsapi.Event) {
	m.Lock()
	defer m.Unlock()

	eventType := event.GetType()
	count := int(event.GetCount())

	if _, ok := m.stat[eventType]; !ok {
		m.stat[eventType] = &eventsStat{}
	}

	if count == 1 {
		// update unique event
		m.stat[eventType].uniqueEvents++
	} else {
		// update repeated event
		m.stat[eventType].repeatedEvents += count
	}

	// update total events by source and event type
	source := event.GetSource()
	sourceKey := fmt.Sprintf("%v-%v", source.GetNodeName(), source.GetComponent())
	if m.totalEventsBySourceAndEvents[sourceKey] == nil {
		m.totalEventsBySourceAndEvents[sourceKey] = map[string]int{}
	}

	src := m.totalEventsBySourceAndEvents[sourceKey]
	src[eventType] += count
}

// startWorker watches the events using the event channel from dispatcher.
func (m *MockWriter) startWorker(id int) {
	defer m.wg.Done()
	for {
		select {
		// this channel will be closed once the eventsChan receives the stop signal from
		// this writer or when dispatcher shuts down.
		case events, ok := <-m.eventsChan.ResultChan():
			if !ok { // channel closed
				return
			}

			m.WriteEvents(events)
		}
	}
}
