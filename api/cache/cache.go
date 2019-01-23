package cache

import (
	"context"
	"expvar"
	"fmt"
	"reflect"
	"sort"
	"strconv"
	"sync"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/utils"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/ctxutils"
	hdr "github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/helper"
	"github.com/pensando/sw/venice/utils/kvstore/memkv"
	kvs "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

var (
	errorNotFound          = errors.New("object not found")
	errorNotCorrectRev     = errors.New("object revision mismatch")
	errorRevBackpedals     = errors.New("revision backpedals")
	errorCacheInactive     = errors.New("cache is inactive")
	errorKVStoreConnection = errors.New("unable to get a kv store connection")

	watchEvents, watchers, watchErrors expvar.Int
	createOps, updateOps, getOps       expvar.Int
	deleteOps, listOps                 expvar.Int
	maxStartRetryInterval              = time.Second * 15
	markSweepInterval                  = time.Second * 10
	kvWatcherRetryInterval             = time.Millisecond * 500
	delayedDelPurgeQuanta              = 100
)

type apiOper string

const (
	operCreate apiOper = "create"
	operUpdate apiOper = "update"
	operDelete apiOper = "delete"
)

// Config is the configuration passed in when initializing the cache.
type Config struct {
	// Config is the configuration for the KVStore backend for the cache.
	Config kvs.Config
	// NumKvClients is the number of KVStore connections that the cache is to use
	//   for better performance.
	NumKvClients int
	// Logger to log to
	Logger log.Logger
	// APIServer that the cache will be attached to
	APIServer apiserver.Server
}

// filterFn returns true if the object passes the filter.
type filterFn func(obj, prev runtime.Object) bool

// cache is an implementation of the cache.Interface
type cache struct {
	sync.RWMutex
	active    bool
	store     Store
	pool      *connPool
	queues    WatchedPrefixes
	argus     *backendWatcher
	ctx       context.Context
	cancel    context.CancelFunc
	logger    log.Logger
	config    Config
	apiserver apiserver.Server
	versioner runtime.Versioner
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
func (p *prefixWatcher) worker(ctx context.Context, wg *sync.WaitGroup, startCh chan error) {
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

	updatefn := func(key string, obj, prev runtime.Object) {
		qs := p.parent.queues.Get(key)
		for i := range qs {
			qs[i].Enqueue(evtype, obj, prev)
		}
	}

	var watchStart time.Time
	establishWatcher := func() {
		watchStart = time.Now()

		if p.cancel != nil {
			p.cancel()
		}

		p.ctx, p.cancel = context.WithCancel(ctx)
		for {
			k := p.parent.pool.GetFromPool()
			if k == nil {
				p.parent.logger.Fatalf("cache could not find a KV conn to use [%s]", p.path)
			}
			kv := k.(kvstore.Interface)
			if p.lastVer == "0" || p.lastVer == "" {
				into := struct {
					api.TypeMeta
					api.ListMeta
					Items []runtime.Object
				}{}
				if err = p.parent.listBackend(ctx, p.path, &into); err == nil {
					p.lastVer = into.ResourceVersion
					evtype = kvstore.Created
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
				p.parent.logger.Infof("Context is cancelled returning")
				// w is a typed nil, explicitly set to nil so check can pass in the outer for loop.
				w = nil
				return
			}
		}
	}
	sweepFunc := func(ctx context.Context) {
		select {
		case <-time.After(markSweepInterval):
			p.parent.store.Sweep(p.path, updatefn)
		case <-p.ctx.Done():
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
	close(startCh)
	for {
		if w == nil { // check this before entering the select loop to prevent panic
			return
		}
		select {
		case <-p.ctx.Done():
			p.parent.logger.Infof("received exit for prefix watcher")
			return
		case ev, ok := <-w.EventChan():
			watchEvents.Add(1)
			if !ok {
				p.parent.logger.ErrorLog("func", "kvwatcher", "path", p.path, "msg", "watch error")
				watchErrors.Add(1)
				establishWatcher()
			} else {
				p.parent.logger.InfoLog("func", "kvwatcher", "path", p.path, "msg", "received event", "key", ev.Key, "oper", ev.Type)
				evtype = ev.Type
				if evtype == kvstore.WatcherError {
					p.parent.logger.ErrorLog("func", "kvwatcher", "path", p.path, "msg", "received error event", "key", ev.Key, "error", ev.Object)
					watchErrors.Add(1)
					p.parent.store.Mark(p.path)

					p.lastVer = "0"
					establishWatcher()
					sweepFunc(p.ctx)
					restoreOverlays(p.ctx, globals.StagingBasePath, p.parent, p.parent.apiserver)
					continue
				}
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
	startCh := make(chan error)
	go w.worker(ctx, &a.wg, startCh)
	<-startCh
	return &w
}

func (a *backendWatcher) Stop() {
	defer a.Unlock()
	a.Lock()
	for _, v := range a.prefixes {
		if v.cancel != nil {
			v.cancel()
		}
	}
	a.wg.Wait()
}

// txn is a wrapper for the kvstore.Txn object.

type txnOp struct {
	key  string
	oper apiOper
	obj  runtime.Object
}
type cacheTxn struct {
	kvstore.Txn
	sync.Mutex
	parent *cache
	ops    []txnOp
}

// Commit intercepts the commit response on a Txn to update the local cache.
func (t *cacheTxn) Commit(ctx context.Context) (kvstore.TxnResponse, error) {
	// No known path of updating transaction from a non-txn cache operation.
	//  It is safe to acquire the cache first without risking a deadlock.
	defer t.parent.Unlock()
	t.parent.Lock()
	return t.commit(ctx)
}

func (t *cacheTxn) commit(ctx context.Context) (kvstore.TxnResponse, error) {
	t.parent.logger.DebugLog("oper", "txnCommit", "msg", "called")
	var evtype kvstore.WatchEventType
	updatefn := func(key string, obj, prev runtime.Object) {
		qs := t.parent.queues.Get(key)
		for i := range qs {
			qs[i].Enqueue(evtype, obj, prev)
		}
	}
	versioner := runtime.NewObjectVersioner()
	defer t.Unlock()
	t.Lock()
	resp, err := t.Txn.Commit(ctx)
	if err != nil {
		// txn cannot be reused now
		t.ops = nil
		return resp, err
	}
	// the kvstore returns the current object only in the case of delete. for create and update,
	//  rely on the cache for the current object.
	respOps := []kvstore.TxnOpResponse{}
	if resp.Succeeded {
		for _, v := range resp.Responses {
			if v.Oper == kvstore.OperDelete {
				respOps = append(respOps, v)
			}
		}
	}
	if resp.Succeeded {
		ver := uint64(resp.Revision)
		t.parent.logger.DebugLog("oper", "txnCommit", "msg", "txn success")
		for _, v := range t.ops {
			switch v.oper {
			case operCreate:
				evtype = kvstore.Created
				t.parent.logger.DebugLog("oper", "txnCommit", "msg", "kvstore success, updating cache")
				versioner.SetVersion(v.obj, ver)
				t.parent.store.Set(v.key, ver, v.obj, updatefn)
				respOps = append(respOps, kvstore.TxnOpResponse{Oper: kvstore.OperUpdate, Key: v.key, Obj: v.obj})
			case operUpdate:
				evtype = kvstore.Updated
				t.parent.logger.DebugLog("oper", "txnCommit", "msg", "kvstore success, updating cache")
				versioner.SetVersion(v.obj, ver)
				t.parent.store.Set(v.key, ver, v.obj, updatefn)
				respOps = append(respOps, kvstore.TxnOpResponse{Oper: kvstore.OperUpdate, Key: v.key, Obj: v.obj})
			case operDelete:
				evtype = kvstore.Deleted
				t.parent.logger.DebugLog("oper", "txnCommit", "msg", "kvstore success, deleting from cache")
				versioner.SetVersion(v.obj, ver)
				t.parent.store.Delete(v.key, ver, updatefn)
			default:
				continue
			}
		}
		resp.Responses = respOps
	}
	// txn cannot be reused now
	t.ops = nil
	return resp, err
}

// Create stages an object creation in a transaction.
func (t *cacheTxn) Create(key string, obj runtime.Object) error {
	defer t.Unlock()
	t.Lock()
	t.ops = append(t.ops, txnOp{key: key, obj: obj, oper: operCreate})
	t.Txn.Create(key, obj)
	return nil
}

// Delete stages an object deletion in a transaction.
func (t *cacheTxn) Delete(key string, cs ...kvstore.Cmp) error {
	defer t.Unlock()
	t.Lock()
	t.ops = append(t.ops, txnOp{key: key, obj: nil, oper: operDelete})
	t.Txn.Delete(key, cs...)
	return nil
}

// Update stages an object update in a transaction.
func (t *cacheTxn) Update(key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	defer t.Unlock()
	t.Lock()
	t.ops = append(t.ops, txnOp{key: key, obj: obj, oper: operUpdate})
	t.Txn.Update(key, obj, cs...)
	return nil
}

// CreateNewCache creates a new cache instance with the config provided
func CreateNewCache(config Config) (apiintf.CacheInterface, error) {
	ret := cache{
		store:     NewStore(),
		pool:      &connPool{},
		logger:    config.Logger,
		versioner: runtime.NewObjectVersioner(),
	}
	wconfig := WatchEventQConfig{
		SweepInterval:     defSweepInterval,
		RetentionDuration: defRetentionDuration,
		RetentionDepthMax: defRetentionDepthMax,
		EvictInterval:     defEvictInterval,
		PurgeInterval:     defPurgeInterval,
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
	c.Lock()
	end := time.Now().Add(maxStartRetryInterval)
	for i := 0; i < c.config.NumKvClients; i++ {
		k, err := kvs.New(c.config.Config)
		if err != nil {
			if time.Now().Before(end) {
				// Still in grace period, retry connecting to KV store
				i = i - 1
				continue
			}
			// Exceed max retry time, error out
			for {
				conn := c.pool.GetFromPool()
				if conn == nil {
					err = errors.Wrap(err, "could not create KV conn pool")
					c.Unlock()
					return err
				}
				k1 := conn.(kvstore.Interface)
				k1.Close()
				c.pool.DelFromPool(k1)
			}
		}
		if c.config.Config.Type == kvs.KVStoreTypeMemkv {
			// We have to set the revision mode to cluster mode.
			mkv := k.(*memkv.MemKv)
			mkv.SetRevMode(memkv.ClusterRevision)
		}
		c.pool.AddToPool(k)
	}
	c.ctx, c.cancel = context.WithCancel(context.Background())
	c.Unlock()
	// start monitor on the backend.
	// XXX-TODO(sanjayt): right now one watch on "/venice" but can be optimized to
	// have sharded watches one per object type using different connections.
	c.argus.NewPrefixWatcher(c.ctx, globals.ConfigRootPrefix)
	c.active = true
	c.logger.InfoLog("msg", "Started cache. Set to active")
	return nil
}

// Restores any persisted state.
func (c *cache) Restore() error {
	// First populate cache from the backend
	into := struct {
		api.TypeMeta
		api.ListMeta
		Items []runtime.Object
	}{}

	if err := c.listBackend(context.Background(), globals.ConfigRootPrefix+"/", &into); err == nil {
		c.logger.Infof("got [%d] objects from backend while restoring cache", len(into.Items))
		c.Lock()
		for _, v := range into.Items {
			meta, ver := mustGetObjectMetaVersion(v)
			if meta.SelfLink == "" {
				log.Fatalf("invalid Self link for object [%+v]", v)
			}
			err := c.store.Set(meta.SelfLink, ver, v, nil)
			c.logger.Infof("restoring object [%v] from backend while restoring cache(%v)", meta.SelfLink, err)
		}
		c.Unlock()
	}
	// restore any persisted overlays
	err := restoreOverlays(context.Background(), globals.StagingBasePath, c, c.config.APIServer)
	if err != nil {
		c.logger.Errorf("restoring logger returned error (%s)", err)
		return err
	}
	c.logger.Infof("restoring overlays complete")
	return nil
}

func (c *cache) clear() {
	c.store.Clear()
}

// Clear clears all entries in the cache
func (c *cache) Clear() {
	defer c.Unlock()
	c.Lock()
	c.clear()
}

// getCbFunc is a helper func used to generate SuccessCbFunc for use with cache
//  operations. The returned function handles generating Watch events on cache update.
func (c *cache) getCbFunc(evType kvstore.WatchEventType) apiintf.SuccessCbFunc {
	return func(p string, obj, prev runtime.Object) {
		qs := c.queues.Get(p)
		for _, q := range qs {
			q.Enqueue(evType, obj, prev)
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
		return fmt.Errorf("Object create failed: %s", kvstore.ErrorDesc(err))
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
		v := uint64(0)
		if into != nil {
			_, v = mustGetObjectMetaVersion(into)
		}
		c.store.Delete(key, v, c.getCbFunc(kvstore.Deleted))
	} else {
		err = fmt.Errorf("Object delete failed: %s", kvstore.ErrorDesc(err))
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
	err := k.PrefixDelete(ctx, prefix)
	if err != nil {
		return fmt.Errorf("Object delete failed: %s", kvstore.ErrorDesc(err))
	}
	return nil
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
		return fmt.Errorf("Object update failed: %s", kvstore.ErrorDesc(err))
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
		return fmt.Errorf("Object update failed: %s", kvstore.ErrorDesc(err))
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
		return fmt.Errorf("Object get failed: %s", err.Error())
	}
	c.logger.DebugLog("oper", "get", "msg", "cache hit")
	obj.Clone(into)
	hdr.Record("cache.Get", time.Since(start))
	getOps.Add(1)
	return nil
}

func (c *cache) sortByVersion(items []runtime.Object, dir bool) func(i, j int) bool {
	return func(i, j int) bool {
		v1, err := c.versioner.GetVersion(items[i])
		if err != nil {
			// This should never happen. Recovery is undefined, hence panic.
			panic("could not retrieve object version")
		}
		v2, err := c.versioner.GetVersion(items[j])
		if err != nil {
			// This should never happen. Recovery is undefined, hence panic.
			panic("could not retrieve object version")
		}
		if dir {
			return v1 < v2
		}
		return v2 < v1
	}
}

func (c *cache) sortByName(items []runtime.Object, dir bool) func(i, j int) bool {
	return func(i, j int) bool {
		oma, ok := items[i].(runtime.ObjectMetaAccessor)
		if !ok {
			panic("object of wrong type")
		}
		om1 := oma.GetObjectMeta()
		if om1 == nil {
			// This should never happen. Recovery is undefined, hence panic.
			panic("could not retrieve object version")
		}
		oma, ok = items[j].(runtime.ObjectMetaAccessor)
		if !ok {
			panic("object of wrong type")
		}
		om2 := oma.GetObjectMeta()
		if om2 == nil {
			// This should never happen. Recovery is undefined, hence panic.
			panic("could not retrieve object version")
		}
		if dir {
			return om1.Name < om2.Name
		}
		return om2.Name < om1.Name
	}
}

func (c *cache) sortByCreationTime(items []runtime.Object, dir bool) func(i, j int) bool {
	return func(i, j int) bool {
		oma, ok := items[i].(runtime.ObjectMetaAccessor)
		if !ok {
			panic("object of wrong type")
		}
		om1 := oma.GetObjectMeta()
		if om1 == nil {
			// This should never happen. Recovery is undefined, hence panic.
			panic("could not retrieve object version")
		}
		oma, ok = items[j].(runtime.ObjectMetaAccessor)
		if !ok {
			panic("object of wrong type")
		}
		om2 := oma.GetObjectMeta()
		if om2 == nil {
			// This should never happen. Recovery is undefined, hence panic.
			panic("could not retrieve object version")
		}
		ts1, err := om1.CreationTime.Time()
		if err != nil {
			log.Errorf("Failed to get creation time [%v]", om1.CreationTime)
			return true
		}
		ts2, err := om2.CreationTime.Time()
		if err != nil {
			log.Errorf("Failed to get creation time [%v]", om2.CreationTime)
			return false
		}
		before := ts1.Before(ts2)
		if dir {
			return before
		}
		return !before
	}
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
		return fmt.Errorf("Object list failed: %s", err.Error())
	}
	ptr := false
	elem := v.Type().Elem()
	if elem.Kind() == reflect.Ptr {
		ptr = true
	}
	kind := ""
	k, ok := apiutils.GetVar(ctx, apiutils.CtxKeyObjKind)
	if ok {
		kind = k.(string)
	}
	items, err := c.store.List(prefix, kind, opts)
	if err != nil {
		return fmt.Errorf("Object list failed: %s", err.Error())
	}
	if opts.SortOrder != api.ListWatchOptions_None.String() && opts.SortOrder != "" {
		switch opts.SortOrder {
		case api.ListWatchOptions_ByName.String():
			sort.Slice(items, c.sortByName(items, true))
		case api.ListWatchOptions_ByNameReverse.String():
			sort.Slice(items, c.sortByName(items, false))
		case api.ListWatchOptions_ByVersion.String(), api.ListWatchOptions_ByModTime.String():
			sort.Slice(items, c.sortByVersion(items, true))
		case api.ListWatchOptions_ByVersionReverse.String(), api.ListWatchOptions_ByModTimeReverse.String():
			sort.Slice(items, c.sortByVersion(items, false))
		case api.ListWatchOptions_ByCreationTime.String():
			sort.Slice(items, c.sortByCreationTime(items, true))
		case api.ListWatchOptions_ByCreationTimeReverse.String():
			sort.Slice(items, c.sortByCreationTime(items, false))
		default:
			return fmt.Errorf("unknown sort order [%s]", opts.SortOrder)
		}
	}
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
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return errorCacheInactive
	}
	start := time.Now()
	c.logger.DebugLog("oper", "listBackend", "msg", "called")
	k := c.pool.GetFromPool().(kvstore.Interface)
	if k == nil {
		return errorKVStoreConnection
	}
	err := k.List(ctx, prefix, into)
	if err != nil {
		return fmt.Errorf("object list Failed: %s", kvstore.ErrorDesc(err))
	}
	hdr.Record("cache.listBackend", time.Since(start))
	return nil
}

// WatchFiltered returns a watcher. Watch events are filtered as per the opts passed in
func (c *cache) WatchFiltered(ctx context.Context, key string, opts api.ListWatchOptions) (kvstore.Watcher, error) {
	defer c.RUnlock()
	c.RLock()
	if !c.active {
		return nil, errorCacheInactive
	}
	kind := ""
	k, ok := apiutils.GetVar(ctx, apiutils.CtxKeyObjKind)
	if ok {
		kind = k.(string)
	}
	c.logger.DebugLog("oper", "watchfiltered", "msg", "called", "kind", kind)
	filters, err := getFilters(opts, kind)
	if err != nil {
		return nil, fmt.Errorf("Establishing watch failed: %s", err.Error())
	}
	nctx, cancel := context.WithCancel(ctx)
	ret := newWatchServer(cancel)
	peer := ctxutils.GetContextID(nctx)
	watchHandler := func(evType kvstore.WatchEventType, item, prev runtime.Object) {
		for _, fn := range filters {
			if !fn(item, prev) {
				return
			}
		}
		ret.ch <- &kvstore.WatchEvent{
			Type:   evType,
			Object: item,
		}
	}
	wq := c.queues.Add(key, peer)
	cleanupFn := func() {
		c.queues.Del(key, peer)
	}
	var fromVer uint64
	if opts.ResourceVersion != "" {
		fromVer, err = strconv.ParseUint(opts.ResourceVersion, 10, 64)
		if err != nil {
			fromVer = 0
		}
	}
	c.logger.DebugLog("oper", "watchfiltered", "msg", "starting watcher with version", fromVer)
	watchers.Add(1)
	go func() {
		wq.Dequeue(nctx, fromVer, watchHandler, cleanupFn)
		watchers.Add(-1)
	}()
	return ret, nil
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
	return c.WatchFiltered(ctx, key, opts)
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
	return c.WatchFiltered(ctx, prefix, opts)
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
	return &cacheTxn{
		Txn:    k.NewTxn(),
		parent: c,
	}
}

// Close closes the cache by shutting down backend watches and backend connections.
//   This probably needs clearing the cache itself, which will be done by a call to Clear.
func (c *cache) Close() {
	c.Lock()
	c.active = false
	c.Unlock()
	if c.cancel != nil {
		c.cancel()
	}
	if c.argus != nil {
		c.argus.Stop()
	}

	defer c.Unlock()
	c.Lock()
	qs := c.queues.Get("/")
	for _, q := range qs {
		q.Stop()
	}
	c.clear()
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

func (c *cache) GetKvConn() kvstore.Interface {
	r := c.pool.GetFromPool()
	if r != nil {
		return r.(kvstore.Interface)
	}
	return nil
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
