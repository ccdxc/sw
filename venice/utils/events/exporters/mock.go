// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package exporters

import (
	"sync"
	"time"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

// MockExporter implements the `Exporter` interface. this has the logic on how the event
// from dispatcher should be processed.
type MockExporter struct {
	sync.Mutex // to protect this object

	// exporter details
	name   string // name of the exporter
	chLen  int    // buffer or channel len
	logger log.Logger

	// to receive events from the proxy (dispatcher)
	eventsChan          events.Chan          // to watch/stop events from dispatcher
	eventsOffsetTracker events.OffsetTracker // to track events file offset - bookmark indicating events till this point are processed successfully by this exporter

	// for mock operations
	eventsByUUID          map[string]*evtsapi.Event      // all the events received
	eventsBySourceAndType map[string]map[string][]string // events received by source and type

	// to stop the exporter
	wg       sync.WaitGroup // to wait for the watch channel to close
	stop     sync.Once      // for stopping the exporter
	shutdown chan struct{}  // to send shutdown signal to the exporter
}

// NewMockExporter creates and returns  the mock exporter interface.
func NewMockExporter(name string, chLen int, logger log.Logger) *MockExporter {
	mockExporter := &MockExporter{
		name:                  name,
		chLen:                 chLen,
		logger:                logger.WithContext("submodule", "mock_exporter"),
		eventsByUUID:          map[string]*evtsapi.Event{},
		eventsBySourceAndType: map[string]map[string][]string{},
		shutdown:              make(chan struct{}, 1),
	}

	return mockExporter
}

// Start starts the exporter
func (m *MockExporter) Start(eventsCh events.Chan, offsetTracker events.OffsetTracker) {
	m.eventsChan = eventsCh
	m.eventsOffsetTracker = offsetTracker

	// start events receiver
	m.wg.Add(1)
	go m.receiveEvents()

	m.logger.Info("started mock events exporter")
}

// Stop stops the watch by calling `Stop` on the event channel.
func (m *MockExporter) Stop() {
	m.logger.Info("stopping the mock exporter")
	m.stop.Do(func() {
		close(m.shutdown)
	})

	// wait for the exporter to finish
	m.wg.Wait()
}

// Name returns the name of the mock exporter
func (m *MockExporter) Name() string {
	return m.name
}

// ChLen returns the channel length to be set of this mock exporter
func (m *MockExporter) ChLen() int {
	return m.chLen
}

// WriteEvents writes list of events
func (m *MockExporter) WriteEvents(evts []*evtsapi.Event) error {
	if evts == nil {
		return nil
	}

	m.Lock()
	defer m.Unlock()

	for _, evt := range evts {
		temp := evt

		// update event by UUID
		evtUUID := evt.GetUUID()
		m.eventsByUUID[evtUUID] = temp

		// to update events by source and event type
		sourceKey := events.GetSourceKey(temp.GetSource())
		if m.eventsBySourceAndType[sourceKey] == nil {
			m.eventsBySourceAndType[sourceKey] = map[string][]string{}
		}

		found := false
		for _, uuid := range m.eventsBySourceAndType[sourceKey][evt.GetType()] {
			if uuid == evtUUID { // existing UUID
				found = true
				break
			}
		}

		if !found {
			m.eventsBySourceAndType[sourceKey][evt.GetType()] = append(m.eventsBySourceAndType[sourceKey][evt.GetType()], evtUUID)
		}
	}

	return nil
}

// GetLastProcessedOffset returns the last bookmarked offset by this exporter
func (m *MockExporter) GetLastProcessedOffset() (int64, error) {
	return m.eventsOffsetTracker.GetOffset()
}

// GetTotalEvents returns the number of events received.
func (m *MockExporter) GetTotalEvents() int {
	m.Lock()
	defer m.Unlock()

	res := 0
	for _, evt := range m.eventsByUUID {
		res += int(evt.GetCount())
	}

	return res
}

// GetEventByUUID returns the event matching the given UUID from the list of received events.
func (m *MockExporter) GetEventByUUID(uuid string) *evtsapi.Event {
	m.Lock()
	defer m.Unlock()

	for _, evt := range m.eventsByUUID {
		if evt.GetUUID() == uuid {
			return evt
		}
	}

	return nil
}

// GetEventsByType returns the number of events received of the given event type.
func (m *MockExporter) GetEventsByType(eType string) int {
	m.Lock()
	defer m.Unlock()

	res := 0
	for _, evt := range m.eventsByUUID {
		if evt.GetType() == eType {
			res += int(evt.GetCount())
		}
	}

	return res
}

// GetEventsBySourceAndType returns the total number of events received from the given source
// of the given event type.
func (m *MockExporter) GetEventsBySourceAndType(source *evtsapi.EventSource, eventType string) int {
	m.Lock()
	defer m.Unlock()

	res := 0
	sourceKey := events.GetSourceKey(source)
	if m.eventsBySourceAndType[sourceKey] != nil {
		for _, uuid := range m.eventsBySourceAndType[sourceKey][eventType] {
			res += int(m.eventsByUUID[uuid].GetCount())
		}
	}

	return res
}

// receiveEvents watches the events using the event channel from dispatcher.
func (m *MockExporter) receiveEvents() {
	defer m.wg.Done()
	for {
		select {
		// this channel will be closed once the Chan receives the stop signal from
		// this exporter or when dispatcher shuts down.
		case evts, ok := <-m.eventsChan.ResultChan():
			if !ok { // channel closed
				return
			}

			// all the incoming batch of events needs to be processed in order to avoid losing track of events
			for {
				if err := m.WriteEvents(evts.GetEvents()); err != nil {
					m.logger.Debugf("mock exporter failed to process events, err: %v", err)
					time.Sleep(1 * time.Second)
				} else { // successfully sent the event to events manager
					m.eventsOffsetTracker.UpdateOffset(evts.GetOffset())
					break
				}
			}
		case <-m.shutdown:
			return
		}
	}
}
