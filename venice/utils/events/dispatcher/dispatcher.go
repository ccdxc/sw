// {C} Copyright 2017-2019 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"context"
	"encoding/json"
	"fmt"
	"path"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/utils"
	memcache "github.com/pensando/sw/venice/utils/cache"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// defaultSendInterval to be used if the user given value is invalid <=0
	defaultSendInterval = 10 * time.Second

	// defaultDedupInterval to be used if the user given value is invalid <=0
	defaultDedupInterval = 10 * time.Second

	// defaultYearSetOnNaples to check if the event can be skipped from processing
	// 2000-01-02 .. default date/time set on NAPLES; clock is set to the correct time
	// once it establishes a connection with venice.
	defaultYearSetOnNaples = 2000
)

// Option fills the optional params for dispatcher
type Option func(d *dispatcherImpl)

// WithMaintenanceMode passes maintenance mode flag for dispatcher
func WithMaintenanceMode(flag bool) Option {
	return func(d *dispatcherImpl) {
		d.maintenanceMode = flag
	}
}

// dispatcherImpl implements the `Dispatcher` interface. It is responsible for
// dispatching events to all the registered exporters.
type dispatcherImpl struct {
	sync.Mutex                  // for protecting the dispatcher object
	nodeName      string        // node name
	sendInterval  time.Duration // i.e, batch interval; events are sent to the exporters in this interval
	dedupInterval time.Duration // events are de-duped for the given interval
	logger        log.Logger    // logger

	// store, cache and batch needs to be in sync always; so, they should be handled through a common lock
	eventsStore   events.PersistentStore // persistent store for events
	dedupCache    *cache                 // in-memory cache to de-dup events
	eventsBatcher *eventsBatcher         // batcher batches the list of events to be sent out in the next send (batch) interval

	// any operation on exporters (events distribution, registration, un-registration) should not stall the events pipeline
	exporters *eventExporters // event exporters

	// default object ref to be included in the event that do not carry object-ref
	defaultObjectRef *api.ObjectRef

	// set to true when the system is going through upgrade
	maintenanceMode bool

	start sync.Once // used for starting the dispatcher

	stop     sync.Once      // used for shutting down the dispatcher
	shutdown chan struct{}  // to send shutdown signal to the daemon go routines (i.e. event distribution)
	wg       sync.WaitGroup // used to wait for the graceful shutdown of daemon go routines
	stopped  bool           // indicates whether the dispatcher is running or not
}

// upon registering the exporter, each exporter gets a events channel to watch for
// events. `eventExporters` maintains the list of registered exporters.
type eventExporters struct {
	sync.Mutex                          // to protect the exporters map without having to stall the events pipeline
	list       map[string]*evtsExporter // map of exporter with their name; exporters are given a name during creation NewExporter(...).
}

// evtsExporter ties the exporter with it's associated channel receiving events and the offset tracker.
type evtsExporter struct {
	eventsCh      events.Chan
	offsetTracker events.OffsetTracker
	wr            events.Exporter
}

// NewDispatcher creates a new dispatcher instance with the given send interval.
func NewDispatcher(nodeName string, dedupInterval, sendInterval time.Duration, storeConfig *events.StoreConfig,
	defaultObjectRef *api.ObjectRef, logger log.Logger, opts ...Option) (events.Dispatcher, error) {
	if utils.IsEmpty(nodeName) {
		return nil, fmt.Errorf("empty node name")
	}

	if dedupInterval <= 0 {
		dedupInterval = defaultDedupInterval
	}

	if sendInterval <= 0 { // batch interval
		sendInterval = defaultSendInterval
	}

	if storeConfig == nil || utils.IsEmpty(storeConfig.Dir) {
		return nil, fmt.Errorf("empty events store Path")
	}

	// create persistent event store
	eventsStore, err := newPersistentStore(storeConfig, logger)
	if err != nil {
		logger.Errorf("failed to create dispatcher; could not create events store, err: %v", err)
		return nil, fmt.Errorf("failed to create dispatcher, err: %v", err)
	}

	dispatcher := &dispatcherImpl{
		nodeName:         nodeName,
		dedupInterval:    dedupInterval,
		sendInterval:     sendInterval,
		defaultObjectRef: defaultObjectRef,
		logger:           logger.WithContext("submodule", "events_dispatcher"),
		dedupCache:       newDedupCache(dedupInterval),
		eventsBatcher:    newEventsBatcher(logger),
		eventsStore:      eventsStore,
		exporters:        &eventExporters{list: map[string]*evtsExporter{}},
		shutdown:         make(chan struct{}),
	}

	for _, opt := range opts {
		if opt != nil {
			opt(dispatcher)
		}
	}

	return dispatcher, nil
}

// start notifying writers of the events every send interval
func (d *dispatcherImpl) Start() {
	d.start.Do(func() {
		// start sending events from the cache
		d.wg.Add(1)
		go d.notifyExporters()

		d.logger.Info("started events dispatcher")
	})
}

// sets the maintenance mode to given `flag`
func (d *dispatcherImpl) SetMaintenanceMode(flag bool) {
	d.Lock()
	d.maintenanceMode = flag
	d.Unlock()
}

// Action implements the action to be taken when the event reaches the dispatcher.
// 1. Writes the events to persistent store.
// 2. Add event to the de-dup cache.
// 3. Add the de-duped event to the batch which will be sent to the exporters.
func (d *dispatcherImpl) Action(event evtsapi.Event) error {
	return d.addEvent(&event)
}

// helper function to write event to the persistent store and add events to the dedup cache & batch.
func (d *dispatcherImpl) addEvent(event *evtsapi.Event) error {
	d.Lock()
	defer d.Unlock()

	if d.stopped {
		d.logger.Errorf("dispatcher stopped, cannot process event: {%s}", event.GetSelfLink())
		return fmt.Errorf("dispatcher stopped, cannot process events")
	}

	if d.maintenanceMode && eventtypes.SuppressDuringMaintenance(event.Type) {
		return nil // suppress certain events during maintenance window
	}

	// skip events that were recorded before the clock is set
	cTime, _ := event.CreationTime.Time()
	if cTime.Year() == defaultYearSetOnNaples {
		d.logger.Errorf("received event {%s} that was recorded before the clock is set, skipping", event.Message)
		return nil // skip processing this event
	}

	if event.EventAttributes.Source == nil {
		event.EventAttributes.Source = &evtsapi.EventSource{}
	}
	event.EventAttributes.Source.NodeName = d.nodeName // set node name on the event

	// populate object ref
	if event.ObjectRef == nil {
		event.ObjectRef = d.defaultObjectRef
	}

	if err := events.ValidateEvent(event); err != nil {
		d.logger.Errorf("event {%s} validation failed, err: %v", event.GetUUID(), err)
		return err
	}

	// write event to the persistent store (i.e. file)
	if err := d.writeToEventsStore(event); err != nil {
		d.logger.Errorf("failed to write event {%s} to persistent store (file), err: %v", event.GetUUID(), err)
		return err
	}

	// de-dup and add the event to batch
	if err := d.dedupAndBatch(events.GetEventKey(event), event); err != nil {
		d.logger.Errorf("failed to dedup and batch event {%s}, err: %v", event.GetUUID(), err)
		return err
	}

	return nil
}

func (d *dispatcherImpl) processFailedEvents(exporter *evtsExporter, exporterOffset *events.Offset, currentEvtsFileOffset *events.Offset) error {
	exporterName := exporter.wr.Name()

	// nothing to be done; exporter up-to date
	if exporterOffset.Filename == currentEvtsFileOffset.Filename && exporterOffset.BytesRead == currentEvtsFileOffset.BytesRead {
		return nil
	}

	// get the list of events pending events from persistent store
	evts, err := d.eventsStore.GetEventsFromOffset(exporterOffset)
	if err != nil {
		d.logger.Errorf("cannot process failed/pending events; failed to get the events using offset {%v: %v}, err: %v",
			exporterOffset.Filename, exporterOffset.BytesRead, err)
		return err
	}

	// create the batch to be sent
	batch, err := newBatch(evts, currentEvtsFileOffset)
	if err != nil {
		d.logger.Errorf("failed to create batch, err: %v", err)
		return err
	}

	if len(evts) > 0 {
		d.logger.Infof("sending {%v} number of events to exporter {%s}, offset [%v:%v...%v:%v]", len(evts), exporterName,
			exporterOffset.Filename, exporterOffset.BytesRead, currentEvtsFileOffset.Filename, currentEvtsFileOffset.BytesRead)
		select {
		case <-exporter.eventsCh.Stopped():
			d.logger.Debugf("event receiver channel for exporter {%s} stopped; cannot deliver events", exporterName)
		case exporter.eventsCh.Chan() <- batch:
			d.logger.Infof("sent failed/pending events to the exporter {%s}", exporterName)
		default: // to avoid blocking
			d.logger.Debugf("could not send failed/pending events to the exporter {%s}", exporterName)
		}
	} else {
		d.logger.Debugf("exporter in sync with the proxy; no backlog of events to be sent to the exporter {%s}", exporterName)
	}
	return nil
}

// RegisterExporter creates a watch channel and offset tracker for the caller and returns it.
// the caller can watch the channel for events and once done, can stop the channel.
// each channel maintains a name which is useful for stopping the watch. Offset tracker is used to
// bookmark the offset.
func (d *dispatcherImpl) RegisterExporter(w events.Exporter) (events.Chan, events.OffsetTracker, error) {
	log.Debugf("registering exporter {%s}", w.Name())
	d.Lock()
	if d.stopped {
		d.logger.Errorf("dispatcher stopped, cannot register exporter: {%s}", w.Name())
		d.Unlock()
		return nil, nil, fmt.Errorf("dispatcher stopped, cannot register exporter")
	}
	d.Unlock()

	esCurrOffset, err := d.eventsStore.GetCurrentOffset()
	if err != nil {
		d.logger.Errorf("could not read current events store offset, err: %v", err)
		return nil, nil, errors.Wrap(err, "failed to register exporter")
	}

	d.exporters.Lock()
	defer d.exporters.Unlock()

	exporterName := w.Name()
	if _, ok := d.exporters.list[exporterName]; ok {
		d.logger.Errorf("exporter {%s} exists already", exporterName)
		return nil, nil, fmt.Errorf("exporter with the given name exists already")
	}

	// to record and manage file offset
	offsetTracker, err := newOffsetTracker(path.Join(d.eventsStore.GetStorePath(), "offset"), exporterName, d.logger)
	if err != nil {
		d.logger.Errorf("could not create offset tracker, err: %v", err)
		return nil, nil, errors.Wrap(err, "failed to register exporter")
	}

	exporterOffset, err := offsetTracker.GetOffset()
	if err != nil {
		d.logger.Errorf("could not read from offset tracker, err: %v", err)
		return nil, nil, errors.Wrap(err, "failed to register exporter")
	}

	// during restart, it is possible that the new exporter could end up receiving more events than
	// intended (ones that were generated before the exporter registration). To avoid such issue, new exporter
	// is given the current events store offset. So, that it starts receiving events from now on(from current offset).
	if utils.IsEmpty(exporterOffset.Filename) && exporterOffset.BytesRead == 0 { // new exporter
		// Whenever venice is getting registered, it should not miss any event that might have been recorded
		// before the registration. So, if venice is getting registered for the first time, then allow it to receive
		// all the events starting from offset 0.
		//
		// This case holds very well when NIC changes from host to network mode. In the host mode, events are not exported
		// to venice. when it changes to network mode, it should receive all (possibly) those events that were recorded
		// in host mode. And, the way to allow this behavior is to let the venice exporter read from offset 0 of
		// the (oldest) events file.
		//
		if w.Name() == exporters.Venice.String() {
			exporterOffset.Filename = esCurrOffset.Filename
			exporterOffset.BytesRead = 0

			oldestFilename := d.eventsStore.GetOldestFilename()
			if !utils.IsEmpty(oldestFilename) {
				exporterOffset.Filename = oldestFilename
			}
		} else { // other exporters e.g. syslog
			exporterOffset = esCurrOffset
		}

		// update offset tracker file
		if err := offsetTracker.UpdateOffset(exporterOffset); err != nil {
			d.logger.Errorf("could not update the exporter offset, err: %v", err)
			return nil, nil, errors.Wrap(err, "failed to register exporter")
		}
	} else {
		d.logger.Errorf("exporter {%s} restarting from offset: {%v: %v}", exporterName, exporterOffset.Filename, exporterOffset.BytesRead)
	}

	e := newEventsChan(w.ChLen())
	d.exporters.list[exporterName] = &evtsExporter{eventsCh: e, offsetTracker: offsetTracker, wr: w}

	// for any re-registration; failed events will be processed here
	if err := d.processFailedEvents(d.exporters.list[exporterName], exporterOffset, esCurrOffset); err != nil {
		delete(d.exporters.list, exporterName)
		e.Stop()
		return nil, nil, errors.Wrap(err, "failed to register exporter")
	}

	// update exporter offset to be up to date as the failed/pending events are sent
	if exporterOffset.BytesRead != esCurrOffset.BytesRead {
		exporterOffset.BytesRead = esCurrOffset.BytesRead
		offsetTracker.UpdateOffset(exporterOffset)
	}

	d.logger.Debugf("exporter {%s} registered with the dispatcher successfully, will start receiving events from offset: {%v: %v}", exporterName,
		exporterOffset.Filename, exporterOffset.BytesRead)
	return e, offsetTracker, nil
}

// UnregisterExporter removes the exporter identified by given name from the list of exporters. As a result
// the channel associated with the given name will no more receive the events
// from the dispatcher. And, offset tracker will be stopped as well.
// call does nothing if the exporter identified by given name is not found in the dispatcher's exporter list.
func (d *dispatcherImpl) UnregisterExporter(name string) {
	log.Debugf("unregistering exporter {%s}", name)
	d.exporters.Lock()
	if w, ok := d.exporters.list[name]; ok {
		delete(d.exporters.list, name)
		w.eventsCh.Stop()
		d.logger.Debugf("exporter {%s} unregistered from the dispatcher successfully", name)
	}
	d.exporters.Unlock()
}

// Shutdown sends shutdown signal to the notifier, flushes all the de-duped events to all
// registered exporters and closes all the exporters.
func (d *dispatcherImpl) Shutdown() {
	d.stop.Do(func() {
		d.Lock()
		d.stopped = true // helps to stop accepting any more events
		close(d.shutdown)
		d.Unlock()

		// wait for the notifier to complete
		d.wg.Wait()

		// process the pending events and send to exporters
		d.logger.Debug("flush the batched events to registered exporters")
		d.Lock()
		evts := d.eventsBatcher.getEvents()
		offset, err := d.eventsStore.GetCurrentOffset()
		if err != nil {
			d.logger.Errorf("failed to get the offset from events store to flush, err: %v", err)
		} else {
			d.distributeEvents(evts, offset)
		}

		// wait for a sec to let the exporters finish processing flushed events
		var wg sync.WaitGroup
		var maxRetries = 10
		var retryDelay = 100 * time.Millisecond

		d.exporters.Lock()
		d.logger.Info("waiting (1s) for the exporters to finish processing flushed events")
		for _, w := range d.exporters.list {
			wg.Add(1)
			go func(w *evtsExporter) {
				defer wg.Done()
				_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
					exporterOffset, err := w.offsetTracker.GetOffset()
					if err != nil {
						return false, err
					}
					if exporterOffset == offset {
						return true, nil
					}
					return false, fmt.Errorf("failed to flush events to exporter {%s}", w.wr.Name())
				}, retryDelay, maxRetries)

				if err != nil {
					d.logger.Error(err)
				}
			}(w)
		}
		wg.Wait()
		d.exporters.Unlock()

		d.eventsStore.Close()
		d.Unlock()

		// close all the exporters
		d.closeAllExporters()

		d.logger.Debugf("dispatcher shutdown, current events store offset {%v}", offset)
	})
}

// DeleteExporter unregisters the exporter and deletes the offset tracker file associated with it.
func (d *dispatcherImpl) DeleteExporter(name string) {
	log.Debugf("deleting exporter {%s}", name)
	d.exporters.Lock()
	if w, ok := d.exporters.list[name]; ok {
		delete(d.exporters.list, name)
		w.eventsCh.Stop()
		if err := w.offsetTracker.Delete(); err != nil {
			d.logger.Debugf("failed to delete offset tracker file for exporter {%s}, err: %v", name, err)
		}
		d.logger.Debugf("exporter {%s} unregistered from the dispatcher successfully", name)
	}
	d.exporters.Unlock()
}

// notifyExporters is a daemon which processes the de-duped/cached events every send interval
// and distributes it to all the exporters. This daemon stops when it receives shutdown
// signal.
func (d *dispatcherImpl) notifyExporters() {
	ticker := time.NewTicker(d.sendInterval)
	defer d.wg.Done()

	for {
		select {
		case <-ticker.C: // distribute current batch with offset
			d.Lock()
			evts := d.eventsBatcher.getEvents()
			if len(evts) == 0 {
				d.Unlock()
				continue
			}
			d.eventsBatcher.clear()

			offset, err := d.eventsStore.GetCurrentOffset()
			d.Unlock()

			if err != nil {
				d.logger.Errorf("failed to get the offset from events store to distribute events; skipping distribute, err: %v", err)
				continue
			}

			d.distributeEvents(evts, offset)
		case <-d.shutdown:
			return
		}
	}
}

// distributeEvents helper function to distribute given event list and offset to all exporters.
func (d *dispatcherImpl) distributeEvents(evts []*evtsapi.Event, offset *events.Offset) {
	if len(evts) == 0 {
		return
	}

	d.exporters.Lock()
	defer d.exporters.Unlock()

	resp, _ := newBatch(evts, offset)
	// notify all the watchers
	for _, w := range d.exporters.list {
		d.logger.Infof("distributing events of len {%v} to exporter {%s}", len(evts), w.wr.Name())
		select {
		case <-w.eventsCh.Stopped():
			d.logger.Errorf("exporter event channel {%s} stopped; cannot deliver events", w.wr.Name())
		case w.eventsCh.Chan() <- resp:
			// slow exporters will block this. So, it is highly recommended to set a large enough
			// channel length for them.
		default:
			d.logger.Errorf("exporter event channel {%s} failed to receive events, may be the buffer is full",
				w.wr.Name())
			// for non-blocking send; exporter failing to receive the event for a any reason (channel full)
			// will lose the event.
		}
	}
}

// helper function to write the given to persistent event store.
func (d *dispatcherImpl) writeToEventsStore(event *evtsapi.Event) error {
	evt, err := json.Marshal(event)
	if err != nil {
		return errors.Wrap(err, "failed to marshal the given event")
	}

	currentOffset, err := d.eventsStore.GetCurrentOffset()
	if err != nil {
		return errors.Wrap(err, "failed to get current events offset")
	}

	fileRotated, err := d.eventsStore.Write(append(evt, '\n'))
	if err != nil {
		return errors.Wrap(err, "failed to write event to file")
	}

	// flush the current batch with the offset obtained before write
	if fileRotated {
		evts := d.eventsBatcher.getEvents()
		if len(evts) != 0 {
			d.distributeEvents(evts, currentOffset)
		}
		d.eventsBatcher.clear()
	}

	return nil
}

// dedupAndBatch dedups the given event and adds it to the batch to be sent to the exporters.
func (d *dispatcherImpl) dedupAndBatch(hashKey string, event *evtsapi.Event) error {
	evt := *event

	// look for potential de-duplication
	srcCache := d.getCacheByEventSource(event.GetSource())
	if existingEvt, ok := srcCache.Get(hashKey); ok { // found, update the count of the existing event and timestamp
		evt = existingEvt.(evtsapi.Event)
		d.logger.Debugf("event {%s} found in cache, deduping with event {%s}", event.GetUUID(), evt.GetUUID())

		// update count and timestamp
		timestamp, _ := types.TimestampProto(time.Now())
		evt.EventAttributes.Count++
		evt.ObjectMeta.ModTime.Timestamp = *timestamp
	}

	// add to de-dup cache
	srcCache.Add(hashKey, evt)

	// add event to the batch
	d.eventsBatcher.add(hashKey, &evt)
	return nil
}

// closeAllExporters helper function to close all the exporters
func (d *dispatcherImpl) closeAllExporters() {
	d.logger.Debug("closing all the registered exporters")
	d.exporters.Lock()
	defer d.exporters.Unlock()

	for _, w := range d.exporters.list {
		delete(d.exporters.list, w.wr.Name())
		w.eventsCh.Stop()
		w.wr.Stop()
	}

	// efficient than deleting the elements one by one
	d.exporters.list = map[string]*evtsExporter{}
}

// getCacheByEventSource helper function that fetches the underlying cache of the given source.
func (d *dispatcherImpl) getCacheByEventSource(source *evtsapi.EventSource) memcache.Cache {
	return d.dedupCache.getSourceCache(events.GetSourceKey(source))
}
