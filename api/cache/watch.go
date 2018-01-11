package cache

import (
	"container/list"
	"context"
	"expvar"
	"fmt"
	"math"
	"net/http"
	"strconv"
	"sync"
	"time"

	"github.com/tchap/go-patricia/patricia"

	"github.com/pensando/sw/api"
	hdr "github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/safelist"
)

var (
	defSweepInterval     = (3 * time.Second)
	defRetentionDuration = (30 * time.Second)
	defRetentionDepthMax = 1024 * 1024
	defEvictInterval     = (10 * time.Second)
)

type eventHandlerFn func(evType kvstore.WatchEventType, item runtime.Object)

// WatchEventQConfig specifies the behavior of the event queue
type WatchEventQConfig struct {
	// SweepInterval is the frequency at which the janitor is run
	SweepInterval time.Duration
	// RetentionDuration is window of history that is retained in the
	//  queue.
	RetentionDuration time.Duration
	// RetentionDepthMax is the highest number of events janitor will allow
	//  in the queue.
	RetentionDepthMax int
	// EvictInterval is the max tolerated lag interval for the watchers.
	//  if a watcher falls behind more than this duration the watcher is
	//  evicted.
	EvictInterval time.Duration
}

// WatchEventQ is a interface for a Watch Q which is used to mux events to watchers.
type WatchEventQ interface {
	Enqueue(evType kvstore.WatchEventType, obj runtime.Object) error
	Dequeue(ctx context.Context, fromver uint64, cb eventHandlerFn, cleanupfn func())
	Stop()
}

// WatchedPrefixes is an interface for managing WatchEventQueues
type WatchedPrefixes interface {
	Add(path string) WatchEventQ
	Del(path string) WatchEventQ
	Get(path string) []WatchEventQ
	GetExact(path string) WatchEventQ
}

// watcher is used track individual client side watchers
type watcher struct {
	sync.RWMutex
	ctx    context.Context
	cancel context.CancelFunc
	// this is the current version for the watcher
	version uint64
	// time for last update
	lastUpd time.Time
}

// GetState returns (current version, last update time)
func (w *watcher) GetState() (uint64, time.Time) {
	defer w.RUnlock()
	w.RLock()
	return w.getState()
}

// getState is internal implementation for GetState
func (w *watcher) getState() (uint64, time.Time) {
	return w.version, w.lastUpd
}

// watchEvent is an object used to track watch events.
type watchEvent struct {
	version  uint64
	refCount int
	evType   kvstore.WatchEventType
	enqts    time.Time
	item     runtime.Object
}

// watchEventQStats is a container for stats for the watchEventQ
type watchEventQStats struct {
	enqueues *expvar.Int
	dequeues *expvar.Int
	// ageOutEviction count the number of events removed from queue
	//  due to age
	ageoutEvictions *expvar.Int
	// depthEviction count the number of events removed from queue
	//  due to queue being too deep
	depthEvictions *expvar.Int
	// clientEvictions count the number of watch clients evicted
	//  due to watcher being too slow.
	clientEvictions *expvar.Int
}

// watchEventQ is an implementation of the WatchEventQ interface
type watchEventQ struct {
	//TBD Pending Heap
	sync.Mutex
	// path is the KV path pertaining to the eventQueue
	path string
	// Store for the cache
	store Store
	// eventList is the queue of events
	eventList *safelist.SafeList
	// watcherList is the list of watchers currently active
	watcherList *safelist.SafeList
	// refCount is the number of references to this watchEventQ
	refCount int
	// cond is the condition variable that will be signalled on enqueue
	cond *sync.Cond
	// stopCh stops the watchEventQ
	stopCh chan error
	// notifyCh is used to broadcast to all watchers on enqueue
	notifyCh chan error
	// log is the logger for events
	log log.Logger
	// wg for all watchers
	wg sync.WaitGroup
	// config defines the behavior of watchEventQs
	config WatchEventQConfig
	stats  watchEventQStats
	// janitorVerMu protects the janitorVer
	janitorVerMu sync.Mutex
	// janitorVer is the version on which the janitor is acting on
	janitorVer uint64
}

// watchedPrefixes is an implementation of the WatchedPrefixes interface
type watchedPrefixes struct {
	sync.RWMutex
	trie        *patricia.Trie // Entries of type watchEventQ
	log         log.Logger
	store       Store
	watchConfig WatchEventQConfig
}

func (w *watchedPrefixes) init() {
	w.trie = patricia.NewTrie()
}

// NewWatchedPrefixes returns an implementation of WatchedPrefixes
func NewWatchedPrefixes(logger log.Logger, store Store, config WatchEventQConfig) WatchedPrefixes {
	ret := &watchedPrefixes{
		log:         logger,
		store:       store,
		watchConfig: config,
	}
	ret.init()
	return ret
}

// Add adds a new Watcher to the cache.
func (w *watchedPrefixes) Add(path string) WatchEventQ {
	w.log.DebugLog("oper", "AddWatchedPrefix", "prefix", path)
	defer w.Unlock()
	w.Lock()
	prefix := patricia.Prefix(path)
	i := w.trie.Get(prefix)
	if i != nil {
		q := i.(*watchEventQ)
		defer q.Unlock()
		q.Lock()
		q.refCount++
		return q
	}
	ret := &watchEventQ{
		log: w.log,
	}
	ret.eventList = safelist.New()
	ret.watcherList = safelist.New()
	ret.refCount = 1
	ret.cond = &sync.Cond{L: &sync.Mutex{}}
	ret.stopCh = make(chan error)
	ret.notifyCh = make(chan error)
	ret.path = path
	ret.store = w.store
	ret.config = w.watchConfig
	w.trie.Insert(prefix, ret)
	w.log.DebugLog("oper", "AddWatchedPrefix", "prefix", path, "msg", "starting janitor")
	ret.start()
	return ret
}

// Del removes a Watcher from the cache. This will free up any events pending in the queue
// and cleanup any watchers using the Queue.
func (w *watchedPrefixes) Del(path string) WatchEventQ {
	w.log.DebugLog("oper", "DelWatchedPrefix", "prefix", path)
	prefix := patricia.Prefix(path)
	i := w.trie.Get(prefix)
	if i != nil {
		q := i.(*watchEventQ)
		q.Lock()
		q.refCount--
		refcount := q.refCount
		q.Unlock()
		if refcount == 0 {
			w.log.DebugLog("oper", "DelWatchedPrefix", "prefix", path, "msg", "last watcher")
			q.Stop()
			w.trie.Delete(prefix)
		}
	}
	return nil
}

// Get returns all watcher Queues for the path, including all parents.
func (w *watchedPrefixes) Get(path string) []WatchEventQ {
	defer w.RUnlock()
	w.RLock()
	var ret []WatchEventQ
	collectfn := func(prefix patricia.Prefix, item patricia.Item) error {
		ret = append(ret, item.(*watchEventQ))
		return nil
	}
	w.trie.VisitPrefixes(patricia.Prefix(path), collectfn)
	return ret
}

// GetExact returns the watch queue for the exact path specified in the params
//  Returns nil if none exists
func (w *watchedPrefixes) GetExact(path string) WatchEventQ {
	defer w.RUnlock()
	w.RLock()
	ret := w.trie.Get(patricia.Prefix(path))
	if ret == nil {
		return nil
	}
	return ret.(*watchEventQ)
}

// Enqueue enqueues an watch event to the queue.
func (w *watchEventQ) Enqueue(evType kvstore.WatchEventType, obj runtime.Object) error {
	start := time.Now()
	objm := obj.(runtime.ObjectMetaAccessor)
	objmeta := objm.GetObjectMeta()
	v, err := strconv.ParseUint(objmeta.ResourceVersion, 10, 64)
	if err != nil {
		w.log.ErrorLog("oper", "EventQueueEnqueue", "type", evType, "msg", "parse version failed", "error", err)
		return err
	}
	w.log.DebugLog("oper", "EventQueueEnqueue", "type", evType)
	// XXXX-TODO(sanjayt): Use a pool here to reduce garbage collection work.
	i := &watchEvent{
		version:  v,
		evType:   evType,
		refCount: w.refCount,
		item:     obj,
		enqts:    time.Now(),
	}
	w.eventList.Insert(i)
	w.notify()
	w.stats.enqueues.Add(1)
	hdr.Record("watch.Enqueue", time.Since(start))
	return nil
}

// Dequeue dequeues elements from the event queue and calls the callback fn
// for each element. Dequeue() runs as a go routine and does callbacks in a tight loop
// till the end of the list is encountered.
func (w *watchEventQ) Dequeue(ctx context.Context, fromver uint64, cb eventHandlerFn, cleanupFn func()) {
	tracker := watcher{}
	tracker.ctx, tracker.cancel = context.WithCancel(ctx)
	tel := w.watcherList.Insert(&tracker)
	if cleanupFn != nil {
		defer cleanupFn()
	}
	defer w.watcherList.Remove(tel)
	var wg sync.WaitGroup
	sendCh := make(chan error)

	sendevent := func(e *list.Element) {
		obj := e.Value.(*watchEvent)
		defer tracker.Unlock()
		tracker.Lock()
		hdr.Record("watch.DequeueLatency", time.Since(obj.enqts))
		go func() {
			cb(obj.evType, obj.item)
			sendCh <- nil
		}()
		select {
		case <-sendCh:
		case <-tracker.ctx.Done():
		}
		tracker.version = obj.version
		tracker.lastUpd = time.Now()
		w.stats.dequeues.Add(1)
	}

	// Freeze the janitor at its current version
	w.janitorVerMu.Lock()
	tracker.version = w.janitorVer
	w.janitorVerMu.Unlock()

	var opts api.ListWatchOptions
	opts.ResourceVersion = fmt.Sprintf("%d", fromver)

	var prev, item *list.Element
	if fromver != 0 {
		item := w.eventList.Front()
		if item != nil {
			obj := item.Value.(*watchEvent)
			if obj.version > fromver {
				// We do not have enough history, error out.
				errmsg := api.Status{
					Result:  &api.StatusResultExpired,
					Message: fmt.Sprintf("version too old"),
					Code:    http.StatusGone,
				}
				cb(kvstore.WatcherError, &errmsg)
				return
			}
			for item != nil && obj != nil && obj.version < fromver {
				item = item.Next()
				obj = item.Value.(*watchEvent)
			}
			for item != nil {
				sendevent(item)
				item = item.Next()
			}
		}
	}

	condCh := make(chan error)
	stopCh := make(chan error)
	w.log.DebugLog("oper", "WatchEventQDequeue", "prefix", w.path, "msg", "starting dequeue instance")
	wg.Add(1)
	go func() {
		defer wg.Done()
		for {
			w.cond.L.Lock()
			w.cond.Wait()
			w.cond.L.Unlock()
			select {
			case condCh <- nil:
			case <-stopCh:
				return
			default:
			}
		}
	}()
	// Now we are ready to start dequeueing every event enqueued henceforth
	//  to the eventList. The Cond variable wakes this go-routine via the
	//  condCh when there are events enqueued.
	prev = w.eventList.Back()
	objs := w.store.List(w.path, opts)
	for _, obj := range objs {
		cb(kvstore.Created, obj)
	}
	for {
		select {
		case <-condCh:
			if prev == nil {
				item = w.eventList.Front()
			} else {
				item = prev.Next()
			}
			for item != nil {
				prev = item
				sendevent(item)
				item = item.Next()
			}
		case <-tracker.ctx.Done():
			close(stopCh)
			w.cond.Broadcast()
			wg.Wait()
			return
		}
	}
}

// janitorFn maintains the watchQ. It is responsible for the cleaning up the Queue of processed events.
//  XXX-TODO(sanjayt): This function is also responsible for evicting slow receivers to conserve system resources.
//  Eviction will be based on how much behind the watcher in terms of time. Not yet implemented.
func (w *watchEventQ) janitorFn() {
	var minVer uint64 = math.MaxUint64
	tail := w.eventList.Back()
	if tail == nil {
		return
	}
	obj := tail.Value.(*watchEvent)
	tailver := obj.version
	cmpfn := func(i interface{}) bool {
		v := i.(*watcher)
		ver, lastupd := v.getState()
		if ver < minVer {
			minVer = ver
		}
		if ver < tailver && time.Since(lastupd) > (w.config.EvictInterval) {
			w.log.ErrorLog("Watcher idle for %dmsecs, evicting", time.Since(lastupd).Nanoseconds()/int64(time.Millisecond))
			w.stats.clientEvictions.Add(1)
			v.cancel()
		}
		// continue to use minVer, next cycle will clean up after watcher has exited.
		return true
	}
	w.watcherList.Iterate(cmpfn)
	w.janitorVerMu.Lock()
	w.janitorVer = minVer
	w.janitorVerMu.Unlock()
	now := time.Now()
	cleanfn := func(len int, i interface{}) bool {
		v := i.(*watchEvent)
		// If the version is still being worked on by a receiver, let be.
		//  slow receivers will be cleaned up by the janitor anyway.
		if v.version >= minVer {
			return false
		}
		// If the event is outside the event window then remove.
		if now.Sub(v.enqts) > w.config.RetentionDuration {
			w.stats.ageoutEvictions.Add(1)
			return true
		}
		// If the event is in the window but we have exceeded our budget
		//  for number of messages, delete
		if len > w.config.RetentionDepthMax {
			w.stats.depthEvictions.Add(1)
			return true
		}
		return false
	}
	w.eventList.RemoveTill(cleanfn)
}

// janitor function maintains and garbage collects stale objects
// from the EventQ
func (w *watchEventQ) janitor() {
	ticker := time.NewTicker(w.config.SweepInterval)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			w.janitorFn()
		case <-w.stopCh:
			return
		}
	}
}

func (w *watchEventQ) notifier() {
	for {
		w.log.DebugLog("msg", "strarting notifier instance")
		select {
		case <-w.stopCh:
			return
		case <-w.notifyCh:
			w.log.DebugLog("msg", "notifier broadcasting ")
			w.cond.Broadcast()
		}
	}
}

func (w *watchEventQ) notify() {
	select {
	case w.notifyCh <- nil:
	default:
	}
}

func (w *watchEventQ) start() {
	w.stats.enqueues = expvar.NewInt(fmt.Sprintf("api.cache.watchq[%s].enqueue", w.path))
	w.stats.dequeues = expvar.NewInt(fmt.Sprintf("api.cache.watchq[%s].dequeue", w.path))
	w.stats.ageoutEvictions = expvar.NewInt(fmt.Sprintf("api.cache.watchq[%s].ageEvictions", w.path))
	w.stats.depthEvictions = expvar.NewInt(fmt.Sprintf("api.cache.watchq[%s].depthEvictions", w.path))
	w.stats.clientEvictions = expvar.NewInt(fmt.Sprintf("api.cache.watchq[%s].clientEvictions", w.path))
	go w.janitor()
	go w.notifier()
}

// Stop cleans up the WatchEventQ
func (w *watchEventQ) Stop() {
	w.Lock()
	defer w.Unlock()
	w.log.DebugLog("oper", "StopWatchEventQ", "msg", "stop called")
	w.refCount--
	if w.refCount != 0 {
		return
	}
	close(w.stopCh)
	w.log.DebugLog("oper", "StopWatchEventQ", "msg", "refCount reached 0, cleaning up")
	stopWatchers := func(i interface{}) bool {
		r := i.(*watcher)
		r.cancel()
		return true
	}
	w.watcherList.Iterate(stopWatchers)

	cleanFn := func(i interface{}) {}
	w.eventList.RemoveAll(cleanFn)
}
