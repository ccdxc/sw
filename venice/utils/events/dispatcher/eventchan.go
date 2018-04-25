// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"sync"

	evtsapi "github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/events"
)

// eventReceiverChanImpl implements `EventReceiverChan` interface. This is what will be returned
// to the caller upon calling dispatcher's `RegisterWriter()`. The caller
// can watch for events using the resultCh.
type eventReceiverChanImpl struct {
	resultCh   chan []*evtsapi.Event // channel where the events are sent to the writers
	stopped    chan struct{}         // used for closing the channel
	stop       sync.Once             // used for closing the channel
	dispatcher events.Dispatcher
	writer     events.Writer
}

// newEventReceiverChan creates a new event channel instance with the given configuration.
func newEventReceiverChan(writer events.Writer, dispatcher events.Dispatcher) *eventReceiverChanImpl {
	return &eventReceiverChanImpl{
		resultCh:   make(chan []*evtsapi.Event, writer.ChLen()),
		stopped:    make(chan struct{}),
		dispatcher: dispatcher,
		writer:     writer,
	}
}

// ResultChan returns the channel to watch for events.
func (e *eventReceiverChanImpl) ResultChan() <-chan []*evtsapi.Event {
	return e.resultCh
}

// Stop stops the channel. Any `Stop()` after the first call will result in no-op.
func (e *eventReceiverChanImpl) Stop() {
	e.dispatcher.UnregisterWriter(e.writer.Name())
}
