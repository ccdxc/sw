// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"github.com/pensando/sw/api/generated/events"
)

// from Venice, Events Recorder -> Events Proxy -> {Events Dispatcher -> Events Exporters}
//
// Event dispatcher and exporters are library modules that are shared between
// venice and NAPLES.
//
// Recorder calls are returned from the proxy. Further processing at the
// dispatcher and the exporters are asynchronous.

// Recorder records events on behalf of the events sources.
// This will carry the event source and list of supported event types.
// Events Recorder -> Events Proxy
type Recorder interface {
	// Event creates the event object which will be sent to the proxy server for
	// further processing in the pipeline.
	//
	// if  objRef != nil, the event will be created in the same tenant/namespace
	// as the reference object.
	// Otherwise, it will be created under the default tenant `globals.DefaultTenant` and `globals.DefaultNamespace`.
	//
	// eventType should be one of the valid event type
	// severity should be one of INFO, WARNING, CRITICAL
	// message is a free form text explaining the reason of the event
	Event(eventType string, severity events.SeverityLevel, message string, objRef interface{})

	// StartExport to start the client
	StartExport()

	// Close the recorder
	Close()
}

// Dispatcher processes all the incoming events for any duplication to avoid
// overwhelming the system with events.
// 1. Dispatcher performs de-duplication and batching. De-dup interval spans for a longer period than batch/send interval.
// 2. Dispatcher batches the de-duped events and distributes it to all the registered exporters.
// 3. Dispatcher maintains a de-dup cache for each event source separately.
// Events Proxy -> Events Dispatcher
type Dispatcher interface {
	// event exporters or other processes can register themselves to get notified of
	// the events.
	RegisterExporter(Exporter) (Chan, OffsetTracker, error)

	// removes the exporter identified by given name from the list of registered exporters.
	UnregisterExporter(name string)

	// unregisters the exporter and deletes the exporter offset file from the system; invoked when event policy is deleted.
	DeleteExporter(name string)

	// start notifying the writers of the events
	Start()

	// dispatcher stops accepting any more events once shutdown is issued. And, it
	// flushes all the pending(cached) events from the dispatcher to all the
	// exporters. This flushes all the sources and will be called during planned maintenance/upgrade.
	Shutdown()

	// action to be performed by the proxy when an event is received from the recorder.
	Action(event events.Event) error
}

// Chan represents the channel that will be used by the dispatcher to send
// events to the registered exporters.
type Chan interface {
	// closes the channel gracefully
	Stop()

	// channel to get notified of the events
	ResultChan() <-chan Batch

	// returns a channel which gets notified when the result channel stops
	Stopped() <-chan struct{}

	// returns the underlying channel which transmits events from dispatcher to exporter
	Chan() chan Batch
}

// Batch represents the list of events and offset which is sent to the exporters.
// the exporters can update the offset once the sent events are processed.
type Batch interface {
	// returns the list of events
	GetEvents() []*events.Event

	// returns the offset
	GetOffset() *Offset
}

// Exporter - all the exporters (venice, syslog, etc.) should implement the functions
// defined here. Each exporter maintains the offset which indicates the events till that
// point is successfully processed.
type Exporter interface {
	// starts the exporter to process the events
	Start(Chan, OffsetTracker)

	// stop the exporter; the exporter will not receive any more events from the dispatcher once stopped
	Stop()

	// writes list of events to the targeted destination
	WriteEvents(events []*events.Event) error

	// returns the name of the exporter
	Name() string

	// returns the length of the channel; this is used to set the channel length at the dispatcher
	ChLen() int

	// returns the last bookmarked offset i.e, the point till where it was successful
	// when the proxy restarts, it starts processing events from this offset for this exporter to avoid
	// losing any event.
	GetLastProcessedOffset() (*Offset, error)

	// adds the given writer to exporter
	AddWriter(interface{})

	// TODO Filters()
}

// OffsetTracker helper to help the exporters update offset.
type OffsetTracker interface {
	// updates/bookmarks the offset to the given value
	UpdateOffset(*Offset) error

	// returns the current bookmarked offset
	GetOffset() (*Offset, error)

	// deletes the underlying offset file that is tracking the offset values
	Delete() error
}

// PersistentStore represents the persistent store for events
// e.g. file
type PersistentStore interface {
	// returns the current events file name in use
	GetFilename() string

	// writes the given data to persistent store and return whether there was a rotation or not
	// true indicates a file rotation
	Write(data []byte) (bool, error)

	// returns the current offset of the persistent store; this is used by the exporters to bookmark.
	GetCurrentOffset() (*Offset, error)

	// returns the list of events starting from the given offset.
	GetEventsFromOffset(offset *Offset) ([]*events.Event, error)

	// returns the path of the persistent store
	GetStorePath() string

	// closes the persistent store connection/handler
	Close()
}

// Offset represents file offset data that is tracked for each exporter.
// Since there can be multiple events files (backed up ones), we need to track the filename
// to know where exactly the exporter is left processing.
type Offset struct {
	Filename  string
	BytesRead int64
}

// StoreConfig represents the config for underlying file/persistent store which stores
// all the events until they are rotated/deleted based on the config policy (max. size and max. num files).
// Events file(s) will be replayed for each exporter from where they left off during process restarts.
// And, this way we ensure no events are lost.
type StoreConfig struct {
	Dir         string // events store directory where the events (including backups) and exporter offsets are stored
	MaxFileSize int64  // maximum size of the events
	MaxNumFiles int    // maximum number of event files to be kept
}
