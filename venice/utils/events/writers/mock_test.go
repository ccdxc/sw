// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package writers

import (
	"sync"
	"testing"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	bufferLen = 30

	dummyEvt = &evtsapi.Event{
		EventAttributes: evtsapi.EventAttributes{
			Type:  "TEST",
			Count: 1,
		},
	}

	logger = log.GetNewLogger(log.GetDefaultConfig("mock-writer-test"))
)

// mock events channel implementation
type mockEventChanImpl struct {
	result chan []*evtsapi.Event // channel where the events are sent to the writers
	stop   sync.Once
}

// ResultChan returns the mock channel to watch for events
func (e *mockEventChanImpl) ResultChan() <-chan []*evtsapi.Event {
	return e.result
}

// Stop stops the mock channel
func (e *mockEventChanImpl) Stop() {
	e.stop.Do(func() {
		close(e.result)
	})
}

// TestMockEventsWriter tests mock writer
func TestMockEventsWriter(t *testing.T) {
	mockEventsChan := &mockEventChanImpl{
		result: make(chan []*evtsapi.Event, bufferLen),
	}

	// create writer
	mockWriter := NewMockWriter("mock", bufferLen, logger)
	mockWriter.Start(mockEventsChan)
	defer mockWriter.Stop()

	// test unique events
	var evts []*evtsapi.Event
	for i := 0; i < 10; i++ {
		temp := *dummyEvt
		temp.EventAttributes.Type = "TEST"
		evts = append(evts, &temp)
	}

	// send events to mock writer
	mockEventsChan.result <- evts

	AssertEventually(t, func() (bool, interface{}) {
		if mockWriter.GetUniqueEvents("TEST") == 10 {
			return true, nil
		}

		return false, nil
	}, "unexpected unique number of events", string("10ms"), string("2s"))

	// test repeated events
	evts = []*evtsapi.Event{}
	temp := *dummyEvt
	temp.EventAttributes.Count = 10
	evts = append(evts, &temp)

	// send events to mock writer
	mockEventsChan.result <- evts

	AssertEventually(t, func() (bool, interface{}) {
		if mockWriter.GetRepeatedEvents("TEST") == 10 {
			return true, nil
		}

		return false, nil
	}, "unexpected unique number of events", string("10ms"), string("2s"))
}
