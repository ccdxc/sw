// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"fmt"
	"strings"
	"sync"
	"time"

	evtsapi "github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils"
	memcache "github.com/pensando/sw/venice/utils/cache"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// defaultSendInterval to be used if the user given value is invalid <=0
	defaultSendInterval = 30 * time.Second
)

// secondaryCache is a buffer cache which is swapped with the primary cache
// every active interval. Secondary cache deals with processing deduped/cached
// events without blocking the primary cache. And, it is set to never expire.
// secondaryCache is cleared once the events are sent to all the writers.
var secondaryCache = &cache{m: map[string]memcache.Cache{}, ttl: -1}

// dispatcherImpl implements the `Dispatcher` interface. It is responsible for
// dispatching events to all the registered writers.
type dispatcherImpl struct {
	sync.Mutex                 // for protecting the dispatcher object
	cache        *cache        // in-memory cache to hold events
	sendInterval time.Duration // events are sent to the writers in this interval

	// used to wait for the graceful shutdown of notifier and incoming events processor
	wg sync.WaitGroup

	// writers
	writers *eventWriters

	// used for shutting down the dispatcher
	stop     sync.Once
	shutdown chan struct{}

	// to stop receving any more events/register requests once the dispatcher is shutdown
	stopped bool

	logger log.Logger
}

// upon registering the writer, each writers get a events channel to watch for
// events. `eventWriters` maintains the list of registered writers.
type eventWriters struct {
	sync.Mutex                                   // to protect the writers map
	list       map[string]*eventReceiverChanImpl // list of registered writers
}

// NewDispatcher creates a new dispatcher instance with the given send interval.
func NewDispatcher(sendInterval time.Duration, logger log.Logger) events.Dispatcher {
	if sendInterval <= 0 {
		sendInterval = defaultSendInterval
	}

	// expire events after 2x the given send interval. Dispatcher should process the
	// deduped events every send interval but if it cannot process due to CPU wait time, the events
	// are retained for this extra buffer time to avoid losing them.

	dispatcher := &dispatcherImpl{
		sendInterval: sendInterval,
		cache:        &cache{m: map[string]memcache.Cache{}, ttl: 2 * sendInterval},
		writers:      &eventWriters{list: map[string]*eventReceiverChanImpl{}},
		shutdown:     make(chan struct{}),
		logger:       logger.WithContext("submodule", "dispatcher"),
	}

	// to wait for the notifier to complete
	dispatcher.wg.Add(1)

	// start notifying writers of the events every send interval
	go dispatcher.notifyWriters()

	dispatcher.logger.Info("started events dispatcher")

	return dispatcher
}

// Action implements the action to be taken when the event reaches the dispatcher.
// It adds the event to respective cache and sends it to the writers if this is
// 1st occurrence of an event in send interval.
func (d *dispatcherImpl) Action(event evtsapi.Event) error {
	if d.stopped {
		d.logger.Errorf("dispatcher stopped, cannot process event: {%s}", event.GetSelfLink())
		return fmt.Errorf("dispatcher stopped, cannot process events")
	}

	hashKey, err := d.getEventKey(&event)
	if err != nil {
		d.logger.Errorf("failed to create key for the event: {%s}, err: %v", event.GetSelfLink(), err)
		return err
	}

	d.cache.Lock()
	srcCache := d.getCacheByEventSource(event.GetSource())

	existingEvt, ok := srcCache.Get(hashKey)
	if ok { // found, update the count of the existing event and timestamp
		d.logger.Debugf("event {%s} found in cache, updating the counter and timestamp", event.GetSelfLink())
		event = existingEvt.(evtsapi.Event)
		event.EventAttributes.Count++
	}

	srcCache.Add(hashKey, event)
	d.cache.Unlock()

	// first event; send it to the writers
	// It is an optimistic attempt to send the first event in an interval to the writers. So, that
	// when process/node restarts, we'll lose only the duplicates (atleast one occurrence is already recordered).
	if !ok {
		d.logger.Debugf("{first event in the interval} send it to the writers: {%s}", event.GetSelfLink())
		d.distributeEvents([]*evtsapi.Event{&event})
	}

	return nil
}

// distributeEvents helper function to distribute given event list to all writers.
func (d *dispatcherImpl) distributeEvents(evts []*evtsapi.Event) {
	if len(evts) == 0 {
		return
	}

	d.writers.Lock()
	defer d.writers.Unlock()

	// notify all the watchers
	for _, w := range d.writers.list {
		select {
		case <-w.stopped:
			d.logger.Debugf("event reciever channel for writer {%s} stopped; cannot deliver events", w.writer.Name())
		case w.resultCh <- evts:
			// slow writers will block this. So, it is highly recommended to set a large enough
			// channel length for them.
		default:
			// for non-blocking send; writer failing to receive the event for a any reason (e.g. channel full)
			// will lose the event.
		}
	}
}

// RegisterWriter creates a watch channel for the caller and returns it.
// the caller can watch the channel for events and once done, can stop the channel.
// each channel maintains a name which is useful for stopping the watch.
func (d *dispatcherImpl) RegisterWriter(writer events.Writer) (events.EventReceiverChan, error) {
	if d.stopped {
		d.logger.Errorf("dispatcher stopped, cannot register writer: {%s}", writer.Name())
		return nil, fmt.Errorf("dispatcher stopped, cannot register writers")
	}

	d.writers.Lock()
	defer d.writers.Unlock()

	writerName := writer.Name()
	if _, ok := d.writers.list[writerName]; ok {
		d.logger.Errorf("writer {%s} exists already", writerName)
		return nil, fmt.Errorf("writer with the given name exists already")
	}

	e := newEventReceiverChan(writer, d)
	d.writers.list[writerName] = e
	d.logger.Debugf("writer {%s} registered with the dispatcher successfully", writerName)

	return e, nil
}

// UnregisterWriter removes the writer identified by given name from the list of writers. As a result
// the channel associated with the given name will no more receive the events
// from the dispatcher. It does nothing if the writer identified by given name is not found in the
// disptacher's writer list.
func (d *dispatcherImpl) UnregisterWriter(name string) {
	d.writers.Lock()
	if w, ok := d.writers.list[name]; ok {
		w.stop.Do(func() {
			close(w.stopped)
			close(w.resultCh)
		})

		delete(d.writers.list, name)
		d.logger.Debugf("writer {%s} unregistered from the dispatcher successfully", name)
	}
	d.writers.Unlock()
}

// Shutdown sends shutdown signal to the notifier, flushes all the deudped events to all
// registered writers and closes all the writers.
func (d *dispatcherImpl) Shutdown() {
	d.Lock()
	defer d.Unlock()

	d.stop.Do(func() {
		d.stopped = true // helps to stop accepting any more events
		close(d.shutdown)

		// process the pending events and send to writers
		evts := d.processCachedEvents()

		if len(evts) > 0 {
			d.logger.Debug("flush the cached events to registered writers")
			d.writers.Lock()
			// notify all the watchers
			for _, w := range d.writers.list {
				err := w.writer.WriteEvents(evts)
				if err != nil {
					d.logger.Debugf("failed to flush events to writer {%s}", w.writer.Name())
				}
			}
			d.writers.Unlock()
		}
	})
	// wait for the notifier to complete
	d.wg.Wait()

	d.closeAllWriters()

	d.logger.Debugf("dispatcher shutdown")
}

// notifyWriters is a deamon which processes the deduped/cached events every send interval
// and distributes it to all the writers. This daemon stops when it receives shutdown
// signal.
func (d *dispatcherImpl) notifyWriters() {
	ticker := time.NewTicker(d.sendInterval)

breakforloop:
	for {
		select {
		case <-ticker.C:
			evts := d.processCachedEvents()
			d.distributeEvents(evts)
		case <-d.shutdown:
			break breakforloop
		}
	}

	d.wg.Done()
}

// processCachedEvents helper function to process the cached events. It will be called
// when its time to distribute events. This ensures the cache keys are retained across
// the intervals to avoid re-creating them for the repeating source in every interval.
// To avoid blocking the primary cache while processing cached events, the secondary cache is used.
// so, that primary cache can still receive incoming events while the secondary
// cache is processing cached events and dispatches it to all writers.
func (d *dispatcherImpl) processCachedEvents() []*evtsapi.Event {
	secondaryCache.Lock()

	d.cache.Lock()
	// swap the cache
	tempCache := d.cache.getCache()
	// secondaryCache.m is an empty cache at this point
	d.cache.m = secondaryCache.m
	// now, secondary contains all the items that needs to be sent to the watchers/writers
	secondaryCache.m = tempCache

	// populate cache keys to make sure they're not re-created over each interval.
	// this ensures the source keys are just created once.
	for key := range secondaryCache.m {
		d.cache.m[key] = d.cache.newMemCache()
	}

	d.cache.Unlock()

	// process the events from all the sources
	evts := []*evtsapi.Event{}
	for _, cache := range secondaryCache.m {
		cachedEvents := cache.Items()

		// run items from each source
		for _, event := range cachedEvents {
			tmp := event.(evtsapi.Event)
			if tmp.GetCount() > 1 {
				evts = append(evts, &tmp)
			}
		}
	}

	// reset the secondary cache
	secondaryCache.clear()

	secondaryCache.Unlock()

	return evts
}

// closeAllWriters helper function to close all the writers
func (d *dispatcherImpl) closeAllWriters() {
	d.logger.Debug("closing all the registered writers")
	d.writers.Lock()
	defer d.writers.Unlock()
	for _, w := range d.writers.list {
		// this ensures that any further call to `Stop()` watcher will do nothing.
		// otherwise, we'll be writing to some memory which we don't use anymore.
		w.stop.Do(func() {
			close(w.stopped)
			close(w.resultCh)
		})
	}

	// efficient than deleting the elements one by one
	d.writers.list = map[string]*eventReceiverChanImpl{}
}

// getCacheByEventSource helper function that fetches the underlying cache of the given source.
func (d *dispatcherImpl) getCacheByEventSource(source *evtsapi.EventSource) memcache.Cache {
	return d.cache.getSourceCache(d.getSourceKey(source))
}

// getSourceKey helper function to construct the source key given the event
// source. This key is used for maintaining a separate cache for each source.
func (d *dispatcherImpl) getSourceKey(source *evtsapi.EventSource) string {
	if source == nil {
		return ""
	}

	return fmt.Sprintf("%s-%s", source.GetNodeName(), source.GetComponent())
}

// getEventKey helper function to create a hashkey for the event. This is
// used for storing events in the cache.
func (d *dispatcherImpl) getEventKey(event *evtsapi.Event) (string, error) {
	if event.Source == nil || utils.IsEmpty(event.Type) || utils.IsEmpty(event.Severity) {
		return "", events.NewError(events.ErrMissingEventAttributes, "source/type/severity is required")
	}

	keyComponents := []string{
		event.Source.NodeName,
		event.Source.Component,
		event.Type,
		event.Severity,
		event.Message}

	if event.ObjectRef != nil {
		keyComponents = append(keyComponents,
			[]string{event.ObjectRef.Kind,
				event.ObjectRef.Namespace,
				event.ObjectRef.Name}...)
	}

	return strings.Join(keyComponents, ""), nil
}
