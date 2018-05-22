// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"sync"

	"github.com/pensando/sw/venice/utils/events"
)

// eventsChanImpl implements `Chan` interface. This is what will be returned
// to the caller upon calling dispatcher's `RegisterWriter()`. The caller
// can watch for events using the resultCh.
type eventsChanImpl struct {
	resultCh chan events.Batch // channel where the events and file offset are sent to the writers
	stopped  chan struct{}     // used for closing the channel
	stop     sync.Once         // used for closing the channel
}

// newEventsChan creates a new event channel instance with the given configuration.
func newEventsChan(chLen int) events.Chan {
	return &eventsChanImpl{
		resultCh: make(chan events.Batch, chLen),
		stopped:  make(chan struct{}),
	}
}

// ResultChan returns the channel to watch for events.
func (e *eventsChanImpl) ResultChan() <-chan events.Batch {
	return e.resultCh
}

// Stopped returns the channel which get notified when the event channel is closed
func (e *eventsChanImpl) Stopped() <-chan struct{} {
	return e.stopped
}

// Chan returns the underlying events channel
func (e *eventsChanImpl) Chan() chan events.Batch {
	return e.resultCh
}

// Stop stops the channel. Any `Stop()` after the first call will result in no-op.
func (e *eventsChanImpl) Stop() {
	e.stop.Do(func() {
		close(e.stopped)
		close(e.resultCh)
	})
}
