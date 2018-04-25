// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/monitoring"
)

// from Venice, Events Recorder -> Events Proxy -> {Events Dispatcher -> Events Writers}
//
// Event dispatcher and writers are library modules that are shared between
// venice and NAPLES.
//
// Recorder calls are returned from the proxy. Further processing at the
// dispatcher and the writers are asynchronous.

// Recorder records events on behalf of the events sources.
// This will carry the event source and event types validator.
// Events Recorder -> Events Proxy
type Recorder interface {
	// Event creates the event object which will be sent to the proxy server for
	// further processing in the pipeline.
	//
	// if  objRef != nil, the event will be created in the same tenant/namespace
	// as the reference object.
	// Otherwise, it will be created under the default tenant `system`.
	//
	// eventType should be one of the valid event type
	// severity shoule be one of INFO, WARNING, CRITICAL
	// message is a free form text explaining the reason of the event
	Event(eventType, severity, message string, objRef *api.ObjectRef) error
}

// Dispatcher processes all the incoming events for any duplication to avoid
// overwhelming the system with events. It aggregates duplicate events and sends
// it to the registered writers in the intervals specified in `sendInterval`.
// Writers can send it to their respective destinations. Dispatcher maintains
// a cache for each event source separtely.
// Events Proxy -> Events Dispatcher
type Dispatcher interface {
	// event writers or other processes can register themselves to get notified of
	// the events.
	RegisterWriter(Writer) (EventReceiverChan, error)

	// removes the writer identified by given name from the list of registered writers.
	UnregisterWriter(name string)

	// dispatcher stops accepting any more events once shutdown is issued. And, it
	// flushes all the pending(cached) events from the dispatcher to all the
	// writers. This flushes all the sources and will be called during palnned maintence/upgrade.
	Shutdown()

	// action to be performed by the proxy when an event is received from the recorder.
	Action(event evtsapi.Event) error
}

// EventReceiverChan represents the channel that will be used by the dispatcher to send
// events to the registered writers.
type EventReceiverChan interface {
	// closes the channel gracefully
	Stop()

	// channel to get notified of the events
	ResultChan() <-chan []*evtsapi.Event
}

// Writer - all the writers (venice, syslog, etc.) should implement the functions
// defined here.
type Writer interface {
	// starts the writer(s) to process the events
	Start(EventReceiverChan)

	// stop the writer; the writer will not receive any more events from the dispatcher once stopped
	Stop()

	// writes list of events to the targeted destination
	WriteEvents(events []*evtsapi.Event) error

	// returns the name of the writer
	Name() string

	// returns the length of the channel; this is used to set the channel length at the dispatcher
	ChLen() int

	// TODO Filters()
}
