// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils/events"
)

// batch implements `Batch` interface. Dispatcher sends this batch to all the writers.
type batch struct {
	evts   []*evtsapi.Event
	offset int64
}

// newBatch creates a new batch
func newBatch(evts []*evtsapi.Event, offset int64) events.Batch {
	return &batch{evts: evts, offset: offset}
}

// GetOffset returns the offset from the this batch
func (b *batch) GetOffset() int64 {
	return b.offset
}

// GetEvents returns the list of events from this batch
func (b *batch) GetEvents() []*evtsapi.Event {
	return b.evts
}

// eventsBatcher batches the list of events which will be sent to the writers for further processing.
// the main goal is to preserve the order of incoming events. that's why a slice is used internally to maintain the order.
// map is used for O(1) look up to avoid iterating the slice.
// NOTE: the caller will need to ensure thread safety.
type eventsBatcher struct {
	meta        map[string]int   // event key (uniquely identifies the event) mapping to the actual index of an event in the slice
	evts        []*evtsapi.Event // helps to preserve the order of events
	expiredEvts []*evtsapi.Event // list of events that're expired from the cache but not dispatched to the writers yet
}

// newEventsBatcher creates a new events batcher
func newEventsBatcher() *eventsBatcher {
	return &eventsBatcher{meta: map[string]int{}, evts: []*evtsapi.Event{}, expiredEvts: []*evtsapi.Event{}}
}

// clear clears the events batch
func (e *eventsBatcher) clear() {
	e.meta = map[string]int{}
	e.evts = []*evtsapi.Event{}
	e.expiredEvts = []*evtsapi.Event{}
}

// add adds the given event to the batch or updates the existing event
func (e *eventsBatcher) add(key string, evt *evtsapi.Event) {
	if index, ok := e.meta[key]; ok {
		// different UUID indicates that the existing (old) event got expired from the cache
		// but not dispatched yet as it still exists in the batch.
		if e.evts[index].GetUUID() != evt.GetUUID() {
			e.expiredEvts = append(e.expiredEvts, e.evts[index])
		}

		// update the slice with new event
		e.evts[index] = evt
		return
	}

	e.evts = append(e.evts, evt)
	e.meta[key] = len(e.evts) - 1 // add the event index with key
}

// getEvents returns the list of events from the batcher
func (e *eventsBatcher) getEvents() []*evtsapi.Event {
	return append(e.evts, e.expiredEvts...)
}
