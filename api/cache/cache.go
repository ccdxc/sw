package cache

import (
	"context"
	"expvar"
	"fmt"
	"reflect"
	"strconv"
	"sync"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/globals"
	hdr "github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/helper"
	kvs "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

var (
	errorNotFound                      = fmt.Errorf("object not found")
	errorNotCorrectRev                 = fmt.Errorf("object revision mismatch")
	errorRevBackpedals                 = fmt.Errorf("revision backpedals")
	errorCacheInactive                 = fmt.Errorf("cache is inactive")
	watchEvents, watchers, watchErrors expvar.Int
	createOps, updateOps, getOps       expvar.Int
	deleteOps, listOps                 expvar.Int
	markSweepInterval                  = time.Second * 10
	kvWatcherRetryInterval             = time.Millisecond * 500
)

// Interface is the cache interface exposed by the cache implementation. It provides a few additional operations
//   on top of standard KV Store operations.
type Interface interface {
	kvstore.Interface
	ListFiltered(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error
	WatchFiltered(ctx context.Context, key string, opts api.ListWatchOptions) kvstore.Watcher
	Start() error
	Clear()
}

// Config is the configuration passed in when initializing the cache.
type Config struct {
	// Config is the configuration for the KVStore backend for the cache.
	Config kvs.Config
	// NumKvClients is the number of KVStore connections that the cache is to use
	//   for better performance.
	NumKvClients int
	// Logger to log to
	Logger log.Logger
}

// filterFn returns true if the object passes the filter.
type filterFn func(obj runtime.Object) bool

// cache is an implementation of the cache.Interface
type cache struct {
	sync.RWMutex
	active bool
	store  Store
	pool   *connPool
	queues WatchedPrefixes
	argus  *backendWatcher
	ctx    context.Context
	cancel context.CancelFunc
	logger log.Logger
	config Config
}

// watchServer provides a kvstore.Watcher interface for watches established
//  on the cache.
type watchServer struct {
	ch     chan *kvstore.WatchEvent
	cancel context.CancelFunc
}

// EventChan returns the event channel
func (w *watchServer) EventChan() <-chan *kvstore.WatchEvent {
	return w.ch
}

// Stop stops the watcher
func (w *watchServer) Stop() {
	w.cancel()
}

// newWatchServer returns a new instance of a watcher.
func newWatchServer(cancel context.CancelFunc) *watchServer {
	return &watchServer{
		ch:     make(chan *kvstore.WatchEvent),
		cancel: cancel,
	}
}

// prefixWatcher is used to watch the backend for one prefix
type prefixWatcher struct {
	sync.Mutex
	path    string
	parent  *cache
	lastVer string
	running bool
	ctx     context.Context
	cancel  context.CancelFunc
}

// worker monitors the watch for a prefix and then updates the cache.
func (p *prefixWatcher) worker(ctx context.Context, wg *sync.WaitGroup) {
	var evtype kvstore.WatchEventType
	var w kvstore.Watcher
	var err error
	defer func() {
		p.Lock()
		p.running = false
		if p.cancel != nil {
			p.cancel()
			p.cancel = nil
		}
		p.Unlock()
		wg.Done()
	}()

	updatefn := func(key string, obj runtime.Object) {
		qs := p.parent.queues.Get(key)
		for i := range qs {
			qs[i].Enqueue(evtype, obj)
		}
	}

	var watchStart time.Time
	establishWatcher := func() {
		watchStart = time.Now()
		k := p.parent.pool.GetFromPool()
		if k == nil {
			p.parent.logger.Fatalf("cache could not find a KV conn to use [%s]", p.path)
		}
		if p.cancel != nil {
			p.cancel()
		}
		kv := k.(kvstore.Interface)

		p.ctx, p.cancel = context.WithCancel(ctx)
		for {
			if p.lastVer == "0" || p.lastVer == "" {
				into := struct {
					api.TypeMeta
					api.ListMeta
					Items []runtime.Object
				}{}
				if err = p.parent.listBackend(ctx, p.path, &into); err == nil {
					p.lastVer = into.ResourceVersion
					for _, v := range into.Items {
						meta, ver := mustGetObjectMetaVersion(v)
						if meta.SelfLink == "" {
							log.Fatalf("invalid Self link for object")
						}
						p.parent.store.Set(meta.SelfLink, ver, v, updatefn)
					}
				}
			}
			w, err = kv.PrefixWatch(p.ctx, p.path, p.lastVer)
			if err == nil {
				return
			}
			p.parent.logger.ErrorLog("func", "prefixWatcher.worker", "msg", "failed to establish watcher", "error", err)
			select {
			case <-time.After(kvWatcherRetryInterval):
				continue
			case <-p.ctx.Done():
				return
			}
		}
	}
	sweepFunc := func(ctx context.Context) {
		select {
		case <-time.After(markSweepInterval):
			p.parent.store.Sweep(p.path, updatefn)
		case <-ctx.Done():
		}
	}
	if ctx.Err() != nil {
		p.parent.logger.ErrorLog("func", "kvwatcher", "path", p.path, "msg", "Context was cancelled, exiting")
		return
	}
	establishWatcher()
	p.Lock()
	p.running = true
	p.Unlock()
	for {
		select {
		case <-ctx.Done():
			p.parent.logger.Infof("received exit for prefix watcher")
			return
		case ev, ok := <-w.EventChan():
			watchEvents.Add(1)
			if !ok {
				p.parent.logger.ErrorLog("func", "kvwatcher", "path", p.path, "msg", "watch error")
				watchErrors.Add(1)
				establishWatcher()
			} else {
				p.parent.logger.InfoLog("func", "kvwatcher", "path", p.path, "msg", "received event", "key", ev.Key)
				if evtype == kvstore.WatcherError {
					p.parent.logger.ErrorLog("func", "kvwatcher", "path", p.path, "msg", "watch error")
					watchErrors.Add(1)
					p.parent.store.Mark(p.path)

					p.lastVer = "0"
					establishWatcher()
					sweepFunc(p.ctx)
					continue
				}
				evtype = ev.Type
				meta, ver := mustGetObjectMetaVersion(ev.Object)
				upd, err := meta.ModTime.Time()
				if err == nil {
					if watchStart.Before(upd) {
						hdr.Record("kvstore.WatchLatency", time.Since(upd))
					}
				} else {
					p.parent.logger.ErrorLog("msg", "error converting timestamp", "error", err)
				}
				switch evtype {
				case kvstore.Created, kvstore.Updated:
					p.parent.store.Set(ev.Key, ver, ev.Object, updatefn)
				case kvstore.Deleted:
					p.parent.store.Delete(ev.Key, ver, updatefn)
				default:

				}
			}
		}
	}
}

// argus is responsible for tracking the backend KV store for updates and updating the cache.
type backendWatcher struct {
	sync.Mutex
	prefixes map[string]*prefixWatcher
	wg       sync.WaitGroup
	parent   *cache
}

// NewPrefixWatcher creates a new backend watcher on a prefix
func (a *backendWatcher) NewPrefixWatcher(ctx context.Context, path string) *prefixWatcher {
	a.parent.logger.DebugLog("oper", "NewPrefixWatcher", "msg", "called")
	a.Lock()
	defer a.Unlock()
	if v, ok := a.prefixes[path]; ok {
		return v
	}
	w := prefixWatcher{
		path:    path,
		parent:  a.parent,
		lastVer: "0",
	}
	a.prefixes[path] = &w
	a.wg.Add(1)
	a.parent.logger.DebugLog("oper", "NewPrefixWatcher", "path", path, "msg", "starting backend prefix watcher")
	go w.worker(ctx, &a.wg)
	return &w
}

func (a *backendWatcher) Stop() {
	defer a.Unlock()
	a.Lock()
	for _, v := range a.prefixes {
		v.cancel()
	}
	a.wg.Wait()
}

// txn is a wrapper for the kvstore.Txn object.
type txn struct {
	kvstore.Txn
	parent *cache
}

// Commit intercepts the commit response on a Txn to update the local cache.
func (t *txn) Commit(ctx context.Context) (kvstore.TxnResponse, error) {
	t.parent.logger.DebugLog("oper", "txnCommit", "msg", "called")
	var evtype kvstore.WatchEventType
	updatefn := func(key string, obj runtime.Object) {
		qs := t.parent.queues.Get(key)
		for i := range qs {
			qs[i].Enqueue(evtype, obj)
		}
	}
	resp, err := t.Txn.Commit(ctx)
	if err != nil {
		return resp, err
	}
	if resp.Succeeded {
		t.parent.logger.DebugLog("oper", "txnCommit", "msg", "txn success")
		for _, v := range resp.Responses {

			t.parent.logger.InfoLog("msg", "Got Txn response", "type", v.Oper)
			if v.Obj == nil {
				t.parent.logger.InfoLog("msg", "Got Txn response", "error", "Object is nil")
				continue
			}
			_, ver := mustGetObjectMetaVersion(v.Obj)
			switch v.Oper {
			case kvstore.OperUpdate:
				evtype = kvstore.Updated
				t.parent.logger.DebugLog("oper", "txnCommit", "msg", "kvstore success, updating cache")
				t.parent.store.Set(v.Key, ver, v.Obj, updatefn)
			case kvstore.OperDelete:
				evtype = kvstore.Deleted
				t.parent.logger.DebugLog("oper", "txnCommit", "msg", "kvstore success, deleting from cache")
				t.parent.store.Delete(v.Key, ver, updatefn)
			default:
				continue
			}
		}
	}
	return resp, err
}

// CreateNewCache creates a new cache instance with the config provided
func CreateNewCache(config Config) (Interface, error) {
	ret := cache{
		store:  NewStore(),
		pool:   &connPool{},
		logger: config.Logger,
	}
	wconfig := WatchEventQConfig{
		SweepInterval:     defSweepInterval,
		RetentionDuration: defRetentionDuration,
		RetentionDepthMax: defRetentionDepthMax,
		EvictInterval:     defEvictInterval,
	}
	ret.queues = NewWatchedPrefixes(config.Logger, ret.store, wconfig)
	argus := &backendWatcher{
		prefixes: make(map[string]*prefixWatcher),
		parent:   &ret,
	}
	ret.argus = argus
	ret.config = config
	return &ret, nil
}

func (c *cache) Start() error {
	defer c.Unlock()
	c.Lock()
	for i := 0; i < c.config.NumKvClients; i++ {
		k, err := kvs.New(c.config.Config)
		if err != nil {
			errors.Wrap(err, "could not create KV conn pool")
			for {
				conn := c.pool.GetFromPool()
				if conn == nil {
					return err
				}
				k := conn.(kvstore.Interface)
				k.Close()
				c.pool.DelFromPool(k)
			}
		}
		c.pool.AddToPool(k)
	}
	c.ctx, c.cancel = context.WithCancel(context.Background())
	// start monitor on the backend.
	// XXX-TODO(sanjayt): right now one watch on "/venice" but can be optimized to
	// have sharded watches one per object type using different connections.
	c.argus.NewPrefixWatcher(c.ctx, globals.RootPrefix)
	c.active = true
	return nil
}

// Clear clears all entries in the cache
func (c *cache) Clear() {
	// XXX-TODO(sanjayt)
}

// getCbFunc is a helper func used to generate SuccessCbFunc for use with cache
//  operations. The returned function handles generating Watch events on cache update.
func (c *cache) getCbFunc(evType kvstore.WatchEventType) SuccessCbFunc {
	return func(p string, obj runtime.Object) {
		qs := c.queues.Get(p)
		for _, q := range qs {
			q.Enqueue(evType, obj)
		}
	}
}

// Create creates a object in the backend KVStore. If the object creation is successful,
//   the Cache is updated and watch notifications are generated to watchers.
func (c *cache) Create(ctx context.Context, key string, obj runtime.Object) error {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	start := time.Now()
	c.logger.DebugLog("oper", "create", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	kvtime := time.Now()
	err := k.Create(ctx, key, obj)
	if err != nil {
		errors.Wrap(err, "kvstore Create Failed")
		return err
	}
	hdr.Record("kvstore.Create", time.Since(kvtime))
	_, v := mustGetObjectMetaVersion(obj)
	c.logger.DebugLog("oper", "create", "msg", "kvstore success, setting cache", "ver", v)
	c.store.Set(key, v, obj, c.getCbFunc(kvstore.Created))
	hdr.Record("cache.Create", time.Since(start))
	createOps.Add(1)
	return nil
}

// Delete deletes an object from the backend KVStore if it exists. The cached object is also deleted
//  from the cache and watch notifications are generated for all watchers.
func (c *cache) Delete(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	start := time.Now()
	c.logger.DebugLog("oper", "delete", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	kvtime := time.Now()
	err := k.Delete(ctx, key, into, cs...)
	hdr.Record("kvstore.Delete", time.Since(kvtime))
	if err == nil {
		c.logger.DebugLog("oper", "delete", "msg", "kvstore succcess. deleting from cache")
		c.store.Delete(key, 0, c.getCbFunc(kvstore.Deleted))
	}
	hdr.Record("cache.Delete", time.Since(start))
	deleteOps.Add(1)
	return err
}

// PrefixDelete is not used by the API hence is just a straight through wrapper on top of the
//   backend KVStore operation.
func (c *cache) PrefixDelete(ctx context.Context, prefix string) error {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	c.logger.DebugLog("oper", "prefixdelete", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	return k.PrefixDelete(ctx, prefix)
}

// Update updates an object in the backend KVStore if it already exists. If the operation is successful
//  the cache is updated and watch notifications are generated for established watches.
func (c *cache) Update(ctx context.Context, key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	// perform KV store Update.
	start := time.Now()
	c.logger.DebugLog("oper", "update", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	kvtime := time.Now()
	err := k.Update(ctx, key, obj, cs...)
	if err != nil {
		errors.Wrap(err, "kvstore update Failed")
		return err
	}
	hdr.Record("kvstore.Update", time.Since(kvtime))
	_, v := mustGetObjectMetaVersion(obj)
	c.logger.DebugLog("oper", "update", "msg", "kvstore success, updating cache")
	c.store.Set(key, v, obj, c.getCbFunc(kvstore.Updated))
	hdr.Record("cache.Update", time.Since(start))
	updateOps.Add(1)
	return nil
}

// ConsistentUpdate updates an object in the backend KVStore if it already exists. If the operation is successful
//  the cache is updated and watch notifications are generated for established watches.
func (c *cache) ConsistentUpdate(ctx context.Context, key string, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	c.logger.DebugLog("oper", "consistenupdate", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	err := k.ConsistentUpdate(ctx, key, into, updateFunc)
	if err != nil {
		errors.Wrap(err, "kvstore update Failed")
		return err
	}
	_, v := mustGetObjectMetaVersion(into)
	c.logger.DebugLog("oper", "consistenupdate", "msg", "kvstore success, updating cache")
	c.store.Set(key, v, into, c.getCbFunc(kvstore.Updated))
	return nil
}

// Get retrieves object from the cache if it exists.
func (c *cache) Get(ctx context.Context, key string, into runtime.Object) error {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	start := time.Now()
	c.logger.DebugLog("oper", "get", "msg", "called")
	obj, err := c.store.Get(key)
	if err != nil {
		return errors.Wrap(err, fmt.Sprintf("Get Failed for %s", key))
	}
	c.logger.DebugLog("oper", "get", "msg", "cache hit")
	obj.Clone(into)
	hdr.Record("cache.Get", time.Since(start))
	getOps.Add(1)
	return nil
}

// ListFiltered returns a list in into filtered as per the opts passed in
func (c *cache) ListFiltered(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	start := time.Now()
	c.logger.DebugLog("oper", "listfiltered", "msg", "called")
	v, err := helper.ValidListObjForDecode(into)
	if err != nil {
		return err
	}
	ptr := false
	elem := v.Type().Elem()
	if elem.Kind() == reflect.Ptr {
		ptr = true
		elem = elem.Elem()
	}
	items := c.store.List(prefix, opts)
	for _, kvo := range items {
		if ptr {
			v.Set(reflect.Append(v, reflect.ValueOf(kvo)))
		} else {
			v.Set(reflect.Append(v, reflect.ValueOf(kvo).Elem()))
		}
	}
	c.logger.DebugLog("oper", "listfiltered", "msg", "resp with %d entries", len(items))
	hdr.Record("cache.List", time.Since(start))
	listOps.Add(1)
	return nil
}

// List lists all the object of prefix. it is expected that all objects are of the same type and into is a List object.
func (c *cache) List(ctx context.Context, prefix string, into runtime.Object) error {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	c.logger.DebugLog("oper", "list", "msg", "called")
	opts := api.ListWatchOptions{}
	return c.ListFiltered(ctx, prefix, into, opts)
}

func (c *cache) listBackend(ctx context.Context, prefix string, into runtime.Object) error {
	start := time.Now()
	c.logger.DebugLog("oper", "listBackend", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	if k == nil {
		return fmt.Errorf("unable to get a kv store connection")
	}
	err := k.List(ctx, prefix, into)
	if err != nil {
		errors.Wrap(err, "kvstore list Failed")
		return err
	}
	hdr.Record("cache.listBackend", time.Since(start))
	return nil
}

// WatchFiltered returns a watcher. Watch events are filtered as per the opts passed in
func (c *cache) WatchFiltered(ctx context.Context, key string, opts api.ListWatchOptions) kvstore.Watcher {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return nil
	}
	c.logger.DebugLog("oper", "watchfiltered", "msg", "called")
	filters := getFilters(opts)
	nctx, cancel := context.WithCancel(ctx)
	ret := newWatchServer(cancel)
	watchHandler := func(evType kvstore.WatchEventType, item runtime.Object) {
		for _, fn := range filters {
			if !fn(item) {
				return
			}
		}
		ret.ch <- &kvstore.WatchEvent{
			Type:   evType,
			Object: item,
		}
	}
	wq := c.queues.Add(key)
	cleanupFn := func() {
		c.queues.Del(key)
	}
	var fromVer uint64
	var err error
	if opts.ResourceVersion != "" {
		fromVer, err = strconv.ParseUint(opts.ResourceVersion, 10, 64)
		if err != nil {
			fromVer = 0
		}
	}
	c.logger.DebugLog("oper", "watchfiltered", "msg", "starting watcher with version", fromVer)
	watchers.Add(1)
	go wq.Dequeue(nctx, fromVer, watchHandler, cleanupFn)
	return ret
}

// Watch returns a watcher.
func (c *cache) Watch(ctx context.Context, key string, fromVersion string) (kvstore.Watcher, error) {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return nil, errorCacheInactive
	}
	c.logger.DebugLog("oper", "watch", "msg", "called")
	opts := api.ListWatchOptions{}
	opts.ResourceVersion = fromVersion
	return c.WatchFiltered(ctx, key, opts), nil
}

// PrefixWatch returns a watcher waching on the prefix passed in.
func (c *cache) PrefixWatch(ctx context.Context, prefix string, fromVersion string) (kvstore.Watcher, error) {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return nil, errorCacheInactive
	}
	c.logger.DebugLog("oper", "prefixwatch", "msg", "called")
	opts := api.ListWatchOptions{}
	opts.ResourceVersion = fromVersion
	return c.WatchFiltered(ctx, prefix, opts), nil
}

// Contest is a wrapper around kvstore Context API.
func (c *cache) Contest(ctx context.Context, name string, id string, ttl uint64) (kvstore.Election, error) {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return nil, errorCacheInactive
	}
	c.logger.DebugLog("oper", "contest", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	return k.Contest(ctx, name, id, ttl)
}

// Lease is a wrapper around the kvstore Lease API
func (c *cache) Lease(ctx context.Context, key string, obj runtime.Object, ttl uint64) (chan kvstore.LeaseEvent, error) {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return nil, errorCacheInactive
	}
	c.logger.DebugLog("oper", "lease", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	return k.Lease(ctx, key, obj, ttl)
}

// NewTxn returns a wrapped kvstore.Txn object. We are only interested in the Commit operation on the Txn, so
//  the Commit method is overloaded so the response can be used to update the local cache.
func (c *cache) NewTxn() kvstore.Txn {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return nil
	}
	c.logger.DebugLog("oper", "newTxn", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	return &txn{
		Txn:    k.NewTxn(),
		parent: c,
	}
}

// Close closes the cache by shutting down backend watches and backend connections.
//   This probably needs clearing the cache itself, which will be done by a call to Clear.
func (c *cache) Close() {
	defer c.Unlock()
	c.Lock()
	c.active = false
	if c.cancel != nil {
		c.cancel()
	}
	c.Clear()
	for {
		i := c.pool.GetFromPool()
		if i == nil {
			return
		}
		k := i.(kvstore.Interface)
		k.Close()
		c.pool.DelFromPool(k)
	}
}

func mustGetObjectMetaVersion(obj runtime.Object) (*api.ObjectMeta, uint64) {
	objm := obj.(runtime.ObjectMetaAccessor)
	meta := objm.GetObjectMeta()
	v, err := strconv.ParseUint(meta.ResourceVersion, 10, 64)
	if err != nil {
		panic(fmt.Sprintf("unable to parse version string [%s](%s)", meta.ResourceVersion, err))
	}
	return meta, v
}

func init() {
	expvar.Publish("api.cache.watchers", &watchers)
	expvar.Publish("api.cache.kv.watchErrors", &watchErrors)
	expvar.Publish("api.cache.kv.watchEvents", &watchEvents)
	expvar.Publish("api.cache.ops.create", &createOps)
	expvar.Publish("api.cache.ops.update", &updateOps)
	expvar.Publish("api.cache.ops.get", &getOps)
	expvar.Publish("api.cache.ops.delete", &deleteOps)
	expvar.Publish("api.cache.ops.list", &listOps)
}
