package watchstream

import (
	"container/list"
	"context"
	"expvar"
	"fmt"
	"math"
	"net/http"
	"sort"
	"strconv"
	"sync"
	"time"

	"github.com/tchap/go-patricia/patricia"

	"github.com/pensando/sw/api"
	apiintf "github.com/pensando/sw/api/interfaces"
	apiutils "github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/safelist"
)

// Default timers
var (
	defSweepInterval     = (3 * time.Second)
	defRetentionDuration = (30 * time.Second)
	defRetentionDepthMax = 1024 * 1024
	defEvictInterval     = (30 * time.Second)
	defPurgeInterval     = (10 * time.Second)
)

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
	// PurgeInterval is the time for which deleted objects are kept around before
	//  purging them.
	PurgeInterval time.Duration
}

// WatchEventQ is a interface for a Watch Q which is used to mux events to watchers.
type WatchEventQ interface {
	Enqueue(evType kvstore.WatchEventType, obj, prev runtime.Object) error
	Dequeue(ctx context.Context, fromver uint64, cb apiintf.EventHandlerFn, cleanupfn func(), opts *api.ListWatchOptions)
	// Stop signals a watcher exiting. Returns true when the last
	//  watcher has returned
	Stop() bool
}

// WatchQueueStat gives visibility to the WatchQueue for debugging
type WatchQueueStat struct {
	PeerID        string
	QLength       int
	LastRev       uint64
	LastTimeStamp time.Time
}

// WatchedPrefixes is an interface for managing WatchEventQueues
type WatchedPrefixes interface {
	Add(path, peer string) WatchEventQ
	Del(path, peer string) WatchEventQ
	Get(path string) []WatchEventQ
	GetExact(path string) WatchEventQ
	Stats() map[string][]WatchQueueStat
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
	// peerID is the name of the peer
	peerID string
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
	version uint64
	evType  kvstore.WatchEventType
	enqts   time.Time
	item    runtime.Object
	prev    runtime.Object
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
	// TBD Pending Heap

	// Mutex protects all mutable unsafe members that dont have a more granular lock.
	//  currently only refCount
	sync.Mutex
	// path is the KV path pertaining to the eventQueue
	path string
	// Store for the cache
	store apiintf.Store
	// eventList is the queue of events
	eventList *safelist.SafeList
	// watcherList is the list of watchers currently active
	watcherList *safelist.SafeList
	// refCount is the number of references to this watchEventQ
	refCount int
	// cond is the condition variable that will be signaled on enqueue
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
	// versioner provides utilities to get/set versions on objects.
	versioner runtime.Versioner
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
	store       apiintf.Store
	watchConfig WatchEventQConfig
}

func (w *watchedPrefixes) init() {
	w.trie = patricia.NewTrie()
}

// NewWatchedPrefixes returns an implementation of WatchedPrefixes
func NewWatchedPrefixes(logger log.Logger, store apiintf.Store, config WatchEventQConfig) WatchedPrefixes {
	wconfig := config
	if wconfig.EvictInterval == 0 {
		wconfig.EvictInterval = defEvictInterval
	}
	if wconfig.PurgeInterval == 0 {
		wconfig.PurgeInterval = defPurgeInterval
	}
	if wconfig.RetentionDepthMax == 0 {
		wconfig.RetentionDepthMax = defRetentionDepthMax
	}
	if wconfig.RetentionDuration == 0 {
		wconfig.RetentionDuration = defRetentionDuration
	}
	if wconfig.SweepInterval == 0 {
		wconfig.SweepInterval = defSweepInterval
	}
	ret := &watchedPrefixes{
		log:         logger,
		store:       store,
		watchConfig: wconfig,
	}

	ret.init()
	return ret
}

// Add adds a new Watcher to the cache.
func (w *watchedPrefixes) Add(path, peer string) WatchEventQ {
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
	ret.notifyCh = make(chan error, 1)
	ret.path = path
	ret.store = w.store
	ret.config = w.watchConfig
	ret.versioner = runtime.NewObjectVersioner()
	w.trie.Insert(prefix, ret)

	w.log.InfoLog("oper", "AddWatchedPrefix", "prefix", path, "peer", peer, "msg", "starting watcher")
	ret.start()
	return ret
}

// Del removes a Watcher from the cache. This will free up any events pending in the queue
// and cleanup any watchers using the Queue.
func (w *watchedPrefixes) Del(path, peer string) WatchEventQ {
	w.log.DebugLog("oper", "DelWatchedPrefix", "prefix", path)
	defer w.Unlock()
	w.Lock()
	prefix := patricia.Prefix(path)
	i := w.trie.Get(prefix)
	if i != nil {
		w.log.InfoLog("oper", "DelWatchedPrefix", "prefix", path, "peer", peer, "msg", "del watcher")
		q := i.(*watchEventQ)
		last := q.Stop()
		if last {
			w.log.InfoLog("oper", "DelWatchedPrefix", "prefix", path, "peer", peer, "msg", "last watcher")
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

func (w *watchedPrefixes) Stats() map[string][]WatchQueueStat {
	defer w.RUnlock()
	w.RLock()
	ret := make(map[string][]WatchQueueStat)
	log.Infof("got call to Stat WatchedPrefixes")
	collectFn := func(prefix patricia.Prefix, item patricia.Item) error {
		if i, ok := item.(*watchEventQ); ok {
			path := i.path
			statWatcher := func(i interface{}) bool {
				r := i.(*watcher)
				rev, ts := r.GetState()
				ret[path] = append(ret[path], WatchQueueStat{
					PeerID:        r.peerID,
					LastRev:       rev,
					LastTimeStamp: ts,
				})
				return true
			}
			i.watcherList.Iterate(statWatcher)
		} else {
			log.Infof("no requested Type [%+v]", i)
		}
		return nil
	}
	w.trie.VisitSubtree(patricia.Prefix("/"), collectFn)
	return ret
}

// Enqueue enqueues an watch event to the queue.
func (w *watchEventQ) Enqueue(evType kvstore.WatchEventType, obj, prev runtime.Object) error {
	start := time.Now()
	objm := obj.(runtime.ObjectMetaAccessor)
	objmeta := objm.GetObjectMeta()
	v, err := strconv.ParseUint(objmeta.ResourceVersion, 10, 64)
	if err != nil {
		w.log.ErrorLog("oper", "WatchEventQEnqueue", "type", evType, "msg", "parse version failed", "err", err, "path", w.path)
		return err
	}
	w.log.DebugLog("oper", "WatchEventQEnqueue", "type", evType, "path", w.path, "version", v)
	// XXXX-TODO(sanjayt): Use a pool here to reduce garbage collection work.
	// XXX-TODO(sanjayt): TODO: The API server is done with the object enqueued here.
	//  so although it is safe to enqueue this object to the eventQ, to keep race detector happy
	//  we could clone the object here and enqueue the cloned object.
	i := &watchEvent{
		version: v,
		evType:  evType,
		item:    obj,
		prev:    prev,
		enqts:   time.Now(),
	}
	w.eventList.Insert(i)
	w.notify()
	w.stats.enqueues.Add(1)
	histogram.Record("watch.Enqueue", time.Since(start))
	return nil
}

// Dequeue dequeues elements from the event queue and calls the callback fn
// for each element. Dequeue() runs as a go routine and does callbacks in a tight loop
// till the end of the list is encountered.
func (w *watchEventQ) Dequeue(ctx context.Context,
	fromver uint64, cb apiintf.EventHandlerFn, cleanupFn func(),
	opts *api.ListWatchOptions) {
	peer := ctxutils.GetContextID(ctx)
	tracker := watcher{peerID: peer}
	tracker.ctx, tracker.cancel = context.WithCancel(ctx)
	w.wg.Add(1)
	tel := w.watcherList.Insert(&tracker)
	if cleanupFn != nil {
		defer cleanupFn()
	}
	defer func() {
		w.watcherList.Remove(tel)
		w.wg.Done()
	}()
	var wg sync.WaitGroup
	var startVer uint64

	sendevent := func(e *list.Element) {
		sendCh := make(chan error)
		obj := e.Value.(*watchEvent)
		if obj.version != 0 && obj.version < startVer && startVer != math.MaxUint64 {
			w.log.InfoLog("oper", "WatchEventQDequeue", "msg", "SendDrop", "type", obj.evType, "path", w.path, "startVer", startVer, "ResVersion", obj.version, "peer", peer)
			return
		}

		histogram.Record("watch.DequeueLatency", time.Since(obj.enqts))
		go func() {
			w.log.DebugLog("oper", "WatchEventQDequeue", "msg", "Send", "type", obj.evType, "path", w.path, "ResVersion", obj.version, "peer", peer)
			cb(tracker.ctx, obj.evType, obj.item, obj.prev)
			close(sendCh)
		}()
		select {
		case <-sendCh:
		}
		tracker.Lock()
		tracker.version = obj.version
		tracker.lastUpd = time.Now()
		tracker.Unlock()
		w.stats.dequeues.Add(1)
	}

	// Freeze the janitor at its current version
	tracker.Lock()
	w.janitorVerMu.Lock()
	tracker.version = w.janitorVer
	w.janitorVerMu.Unlock()
	tracker.lastUpd = time.Now()
	tracker.Unlock()
	w.log.InfoLog("oper", "WatchEventQDequeue", "msg", "Start", "path", w.path, "fromVer", fromver, "peer", peer)
	var lopts api.ListWatchOptions
	if opts != nil {
		lopts = *opts
	}
	lopts.ResourceVersion = fmt.Sprintf("%d", fromver)
	maxver := uint64(0)

	// List from store if fromver is not specified
	if fromver == 0 {
		// Peek at the top of the queue. The latest object in the store may not be an accurate marker
		// if there have been recent deletes.
		var item *list.Element
		var qver uint64
		item = w.eventList.Back()
		if item != nil {
			obj := item.Value.(*watchEvent)
			qver = obj.version
		}
		kind := ""
		k, ok := apiutils.GetVar(ctx, apiutils.CtxKeyObjKind)
		if ok {
			kind = k.(string)
		}
		// List all objects
		objs, err := w.store.List(w.path, kind, lopts)
		if err == nil {
			sort.Slice(objs, func(i int, j int) bool {
				v1, err := w.versioner.GetVersion(objs[i])
				if err != nil {
					// This should never happen. Recovery is undefined, hence panic.
					panic("could not retrieve object version")
				}
				v2, err := w.versioner.GetVersion(objs[j])
				if err != nil {
					// This should never happen. Recovery is undefined, hence panic.
					panic("could not retrieve object version")
				}
				return v1 < v2
			})
			if len(objs) > 0 {
				maxver, err = w.versioner.GetVersion(objs[len(objs)-1])
				if err != nil {
					// This should never happen. Recovery is undefined, hence panic.
					panic("could not retrieve object version")
				}
			}
			for _, obj := range objs {
				ver, err := w.versioner.GetVersion(obj)
				if err != nil {
					// This should never happen. Recovery is undefined, hence panic.
					panic("could not retrieve object version")
				}
				w.log.InfoLog("oper", "WatchEventQDequeue", "msg", "Send", "reason", "list", "type", kvstore.Created, "path", w.path, "peer", peer, "ResVersion", ver)
				cb(tracker.ctx, kvstore.Created, obj, nil)
				tracker.Lock()
				tracker.version = ver
				tracker.lastUpd = time.Now()
				tracker.Unlock()
			}
		}
		startVer = maxver + 1
		if qver+1 > startVer {
			startVer = qver + 1
		}
		w.log.InfoLog("oper", "WatchEventQDequeue", "startVer", startVer, "path", w.path, "fromVer", fromver, "peer", peer)
	} else {
		startVer = fromver
	}

	// Scan the current eventList
	w.log.InfoLog("oper", "WatchEventQDequeue", "msg", "Catchup", "path", w.path, "fromVer", fromver, "peer", peer)
	var prev, item *list.Element
	item = w.eventList.Front()
	prev = item
	if item != nil {
		obj := item.Value.(*watchEvent)
		if fromver != 0 && obj.version > fromver {
			// fromver was specified and we do not have enough history, error out.
			errmsg := api.Status{
				Result:  api.StatusResultExpired,
				Message: []string{fmt.Sprintf("version too old")},
				Code:    http.StatusGone,
			}
			w.log.InfoLog("oper", "WatchEventQDequeueSend", "type", kvstore.WatcherError, "path", w.path, "reason", "catch up", "peer", peer)
			cb(tracker.ctx, kvstore.WatcherError, &errmsg, nil)
			return
		}
		// ignore events older than startVer
		for item != nil && obj != nil && startVer != math.MaxUint64 && obj.version < startVer {
			item = item.Next()
			if item != nil {
				obj = item.Value.(*watchEvent)
				prev = item
			}
		}
		for item != nil {
			select {
			case <-tracker.ctx.Done():
				// bail out here instead of iterating through items and then exiting.
				return
			default:
				sendevent(item)
				prev = item
				item = item.Next()
			}
		}
	} else {
		if fromver != 0 && fromver != math.MaxUint64 {
			// fromVer specified but there is nothing in the queue.
			errmsg := api.Status{
				Result:  api.StatusResultExpired,
				Message: []string{fmt.Sprintf("version not in cache history, retry without specifying version")},
				Code:    http.StatusGone,
			}
			w.log.InfoLog("oper", "WatchEventQDequeueSend", "type", kvstore.WatcherError, "path", w.path, "reason", "catch up", "peer", peer)
			cb(tracker.ctx, kvstore.WatcherError, &errmsg, nil)
			return
		}
	}

	condCh := make(chan error, 1)
	wg.Add(1)
	deferCh := make(chan bool)
	go func() {
		defer wg.Done()
		once := true
		for {
			w.cond.L.Lock()
			if once {
				close(deferCh)
				once = false
			}
			w.cond.Wait()
			w.cond.L.Unlock()
			select {
			case <-tracker.ctx.Done():
				return
			default:
			}
			select {
			case condCh <- nil:
			default:
			}
		}
	}()
	<-deferCh
	// Kickstart the dequeue monitor
	w.notify()
	w.log.InfoLog("oper", "WatchEventQDequeue", "prefix", w.path, "msg", "starting dequeue monitor", "peer", peer)
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
		ver, lastupd := v.GetState()
		if ver < minVer {
			minVer = ver
		}
		peer := ctxutils.GetContextID(v.ctx)
		if ver != 0 && ver < tailver && time.Since(lastupd) > (w.config.EvictInterval) {
			w.log.Errorf("Watcher [%s] idle for %v, evicting", peer, time.Since(lastupd))
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
	// Run purge of delete pending items in store
	w.store.PurgeDeleted(w.config.PurgeInterval)
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
	w.log.DebugLog("msg", "starting watch queue", "queue", w.path)
	k := fmt.Sprintf("api.cache.watchq[%s].enqueue", w.path)
	if v := expvar.Get(k); v == nil {
		w.stats.enqueues = expvar.NewInt(k)
	} else {
		w.stats.enqueues = v.(*expvar.Int)
	}
	k = fmt.Sprintf("api.cache.watchq[%s].dequeue", w.path)
	if v := expvar.Get(k); v == nil {
		w.stats.dequeues = expvar.NewInt(k)
	} else {
		w.stats.dequeues = v.(*expvar.Int)
	}
	k = fmt.Sprintf("api.cache.watchq[%s].ageEvictions", w.path)
	if v := expvar.Get(k); v == nil {
		w.stats.ageoutEvictions = expvar.NewInt(k)
	} else {
		w.stats.ageoutEvictions = v.(*expvar.Int)
	}
	k = fmt.Sprintf("api.cache.watchq[%s].depthEvictions", w.path)
	if v := expvar.Get(k); v == nil {
		w.stats.depthEvictions = expvar.NewInt(k)
	} else {
		w.stats.depthEvictions = v.(*expvar.Int)
	}
	k = fmt.Sprintf("api.cache.watchq[%s].clientEvictions", w.path)
	if v := expvar.Get(k); v == nil {
		w.stats.clientEvictions = expvar.NewInt(k)
	} else {
		w.stats.clientEvictions = v.(*expvar.Int)
	}
	go w.janitor()
	go w.notifier()
}

// Stop cleans up the WatchEventQ
func (w *watchEventQ) Stop() bool {
	defer w.Unlock()
	w.Lock()
	w.log.DebugLog("oper", "StopWatchEventQ", "msg", "stop called")
	w.refCount--
	if w.refCount != 0 {
		return false
	}
	close(w.stopCh)
	w.log.DebugLog("oper", "StopWatchEventQ", "msg", "refCount reached 0, cleaning up")
	stopWatchers := func(i interface{}) bool {
		r := i.(*watcher)
		r.cancel()
		return true
	}
	w.watcherList.Iterate(stopWatchers)
	w.wg.Wait()
	cleanFn := func(i interface{}) {}
	w.eventList.RemoveAll(cleanFn)
	return true
}
