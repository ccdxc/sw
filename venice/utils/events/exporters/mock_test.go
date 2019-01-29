// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package exporters

import (
	"fmt"
	"sync"
	"testing"

	"github.com/google/uuid"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	mockBufferLen = 30

	dummyEvt = &evtsapi.Event{
		ObjectMeta: api.ObjectMeta{
			Namespace: globals.DefaultNamespace,
			Tenant:    globals.DefaultTenant,
		},
		EventAttributes: evtsapi.EventAttributes{
			Type:  "DUMMY",
			Count: 1,
			Source: &evtsapi.EventSource{
				Component: "test",
				NodeName:  "test",
			},
		},
	}
)

// mock events channel implementation
type mockEventChanImpl struct {
	result  chan events.Batch // channel where the events are sent to the exporters
	stop    sync.Once
	stopped chan struct{}
}

// ResultChan returns the mock channel to watch for events
func (e *mockEventChanImpl) ResultChan() <-chan events.Batch {
	return e.result
}

// Stop stops the mock channel
func (e *mockEventChanImpl) Stop() {
	e.stop.Do(func() {
		close(e.result)
	})
}

// Chan returns the channel which transmits events to exporter
func (e *mockEventChanImpl) Chan() chan events.Batch {
	return e.result
}

// Stopped returns the channel which gets notified when the event channel stops
func (e *mockEventChanImpl) Stopped() <-chan struct{} {
	return e.stopped
}

// mockBatch mock implementation of channel response
type mockBatch struct {
	events []*evtsapi.Event
	offset int64
}

// GetEvents returns list of events from this response
func (e *mockBatch) GetEvents() []*evtsapi.Event {
	return e.events
}

// GetOffset returns the offset from this response
func (e *mockBatch) GetOffset() int64 {
	return e.offset
}

// newMockBatch creates a new chan response which will be sent out to exporter
func newMockBatch(events []*evtsapi.Event, offset int64) events.Batch {
	return &mockBatch{events: events, offset: offset}
}

// mockOffsetTrackerImpl mock implementation of offset tracker
type mockOffsetTrackerImpl struct {
	sync.Mutex
	offset int64
}

// UpdateOffset updates/bookmarks the offset to given value
func (e *mockOffsetTrackerImpl) UpdateOffset(offset int64) error {
	e.Lock() // because multiple workers can do this
	defer e.Unlock()

	e.offset = offset
	return nil
}

// GetOffset returns the bookmarked offset
func (e *mockOffsetTrackerImpl) GetOffset() (int64, error) {
	e.Lock()
	defer e.Unlock()

	return e.offset, nil
}

// Stop stops the offset tracker
func (e *mockOffsetTrackerImpl) Stop() error {
	return nil
}

// TestMockEventsExporter tests mock exporter
func TestMockEventsExporter(t *testing.T) {
	logger := log.GetNewLogger(log.GetDefaultConfig("mock_exporter_test"))
	mockEventsChan := &mockEventChanImpl{
		result:  make(chan events.Batch, mockBufferLen),
		stopped: make(chan struct{}),
	}

	mockOffsetTracker := &mockOffsetTrackerImpl{}

	// create exporter
	mockExporter := NewMockExporter("mock", mockBufferLen, logger)
	mockExporter.Start(mockEventsChan, mockOffsetTracker)
	defer mockExporter.Stop()

	Assert(t, mockExporter.Name() == "mock", "expected `mock`")
	Assert(t, mockExporter.ChLen() == mockBufferLen, "expected %d", mockBufferLen)

	// send different events of same type (with the count = 1)
	var evts []*evtsapi.Event
	for i := 0; i < 10; i++ {
		temp := *dummyEvt
		temp.ObjectMeta.UUID = uuid.New().String()
		temp.EventAttributes.Type = "TEST"
		evts = append(evts, &temp)
	}

	// send events to mock exporter
	mockEventsChan.result <- newMockBatch(evts, 0)

	AssertEventually(t, func() (bool, interface{}) {
		obtained := mockExporter.GetEventsByType("TEST")
		if obtained == 10 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 10, got: %v", obtained)
	}, "unexpected unique number of events", string("20ms"), string("2s"))

	// test deduped events (1 event with count = 10)
	evts = []*evtsapi.Event{}
	temp := *dummyEvt
	temp.ObjectMeta.UUID = uuid.New().String()
	temp.EventAttributes.Count = 10
	evts = append(evts, &temp)

	mockEventsChan.result <- newMockBatch(evts, 0)

	AssertEventually(t, func() (bool, interface{}) {
		obtained := mockExporter.GetEventByUUID(temp.GetUUID())
		if obtained != nil && obtained.GetCount() == 10 {
			return true, nil
		}

		return false, fmt.Sprintf("expected: 10, got: %v", obtained)
	}, "unexpected unique number of events", string("20ms"), string("2s"))

	// test event by source and type
	evts = []*evtsapi.Event{}
	evtType1 := "TYPE1"
	evtType2 := "TYPE2"
	for i := 0; i < 10; i++ {
		src := &evtsapi.EventSource{Component: fmt.Sprintf("comp%d", i), NodeName: fmt.Sprintf("node%d", i)}

		// type 1
		for j := 0; j < 5; j++ {
			temp := *dummyEvt
			temp.ObjectMeta.UUID = uuid.New().String()
			temp.EventAttributes.Source = src
			temp.Type = evtType1
			evts = append(evts, &temp)
		}

		// type 2
		for j := 0; j < 5; j++ {
			temp := *dummyEvt
			temp.ObjectMeta.UUID = uuid.New().String()
			temp.EventAttributes.Source = src
			temp.Type = evtType2
			evts = append(evts, &temp)
		}
	}

	mockEventsChan.result <- newMockBatch(evts, 0)

	for i := 0; i < 10; i++ {
		src := &evtsapi.EventSource{Component: fmt.Sprintf("comp%d", i), NodeName: fmt.Sprintf("node%d", i)}
		AssertEventually(t, func() (bool, interface{}) {
			type1 := mockExporter.GetEventsBySourceAndType(src, evtType1)
			type2 := mockExporter.GetEventsBySourceAndType(src, evtType1)
			if type1 == 5 && type2 == 5 {
				return true, nil
			}

			return false, fmt.Sprintf("expected type1: 5, type2: 5, got type1: %v, type2: %v", type1, type2)
		}, "unexpected unique number of events", string("20ms"), string("2s"))
	}

	Assert(t, mockExporter.GetEventByUUID("xxx") == nil, "expected failure, get by dummy uuid succeeded")
	Assert(t, mockExporter.GetEventsByType("xxx") == 0, "expected failure, get by dummy type succeeded")
	Assert(t, mockExporter.GetEventsBySourceAndType(nil, "xxx") == 0, "expected failure, get by dummy source and type succeeded")
}
