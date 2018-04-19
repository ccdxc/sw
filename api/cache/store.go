package cache

import (
	"container/heap"
	"expvar"
	"fmt"
	"sync"
	"sync/atomic"
	"time"

	"github.com/tchap/go-patricia/patricia"

	"github.com/pensando/sw/api"
	hdr "github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/runtime"
)

// StoreStats contains stats for the store
type StoreStats struct {
	sets      *expvar.Int
	deletes   *expvar.Int
	gets      *expvar.Int
	lists     *expvar.Int
	purges    *expvar.Int
	purgeRuns *expvar.Int
	errors    *expvar.Int
}

var storeID uint32

// SuccessCbFunc is called when a Store operation succeeds. The call happens in
// the critical path hence the function should the min needed.
type SuccessCbFunc func(key string, obj, prev runtime.Object)

// Store is the interface for the Local Object Store
type Store interface {
	Set(key string, rev uint64, obj runtime.Object, cb SuccessCbFunc) error
	Get(key string) (runtime.Object, error)
	Delete(key string, rev uint64, cb SuccessCbFunc) (runtime.Object, error)
	List(key string, opts api.ListWatchOptions) ([]runtime.Object, error)
	Mark(key string)
	Sweep(key string, cb SuccessCbFunc)
	PurgeDeleted(past time.Duration)
	Clear()
}

// cacheObj is a wrapper around each object stored in the cache.
type cacheObj struct {
	revision uint64
	key      string
	obj      runtime.Object
	lastUpd  time.Time
	deleted  bool
	inDelQ   bool
	delQId   int
}

type delPendingHeap struct {
	l []*cacheObj
}

// store is an implementation of the Store interface
type store struct {
	sync.RWMutex
	objs       *patricia.Trie
	stats      StoreStats
	delPending *delPendingHeap
	versioner  runtime.Versioner
	id         uint32
}

// NewStore creates a new Local Object Store.
func NewStore() Store {

	ret := &store{
		objs:       patricia.NewTrie(),
		id:         atomic.AddUint32(&storeID, 1),
		delPending: &delPendingHeap{},
		versioner:  runtime.NewObjectVersioner(),
	}
	heap.Init(ret.delPending)
	ret.registerStats()
	return ret
}

// Len returns the len of the pendingDelete heap
func (h *delPendingHeap) Len() int {
	return len(h.l)
}

// Less implements the sort.Interface
func (h *delPendingHeap) Less(i, j int) bool {
	return h.l[j].lastUpd.Before(h.l[i].lastUpd)
}

// Swap implements the sort.Interface
func (h *delPendingHeap) Swap(i, j int) {
	cobj1 := h.l[i]
	cobj2 := h.l[j]
	cobj1.delQId = j
	cobj2.delQId = i
	h.l[i], h.l[j] = cobj2, cobj1
}

// Push pushes an item to the heap in accordance with the heap.Interface
func (h *delPendingHeap) Push(i interface{}) {
	cobj := i.(*cacheObj)
	cobj.delQId = len(h.l)
	h.l = append(h.l, i.(*cacheObj))
}

// Pop pops item from the heap in accordance with the heap.Interface
func (h *delPendingHeap) Pop() interface{} {
	old := h.l
	l := len(h.l)
	ret := old[l-1]
	h.l = old[0 : l-1]
	return ret
}

// Peek peeks at the top of the heap without popping it
func (h *delPendingHeap) Peek() *cacheObj {
	l := len(h.l)
	if l == 0 {
		return nil
	}
	return h.l[l-1]
}

// Set updates the cache for the key with obj while making sure revision moves
//  only formward. Any updating with older version is rejected. the callback function
//  is called if the operation succeeds
func (s *store) Set(key string, rev uint64, obj runtime.Object, cb SuccessCbFunc) (err error) {
	start := time.Now()
	success := true
	var prev runtime.Object
	defer func() {
		hdr.Record("store.Set", time.Since(start))
	}()
	prefix := patricia.Prefix(key)
	if cb != nil {
		defer func() {
			if success == true {
				cb(key, obj, prev)
			}
		}()
	}
	defer s.Unlock()
	s.Lock()
	v := s.objs.Get(prefix)
	if v != nil {
		cobj := v.(*cacheObj)
		if cobj.revision <= rev {
			success = cobj.revision < rev
			prev = cobj.obj
			cobj.obj = obj
			cobj.revision = rev
			cobj.deleted = false
			cobj.lastUpd = time.Now()
			if cobj.inDelQ {
				heap.Remove(s.delPending, cobj.delQId)
				cobj.delQId = 0
				cobj.inDelQ = false
			}
			return nil
		}
		success = false
		return errorRevBackpedals
	}
	cobj := &cacheObj{
		revision: rev,
		key:      key,
		obj:      obj,
		deleted:  false,
		inDelQ:   false,
		lastUpd:  time.Now(),
	}
	s.objs.Set(prefix, cobj)
	s.stats.sets.Add(1)
	return nil
}

// Get retrieves a stored object from the cache.
func (s *store) Get(key string) (runtime.Object, error) {
	start := time.Now()
	defer func() {
		hdr.Record("store.Get", time.Since(start))
	}()
	prefix := patricia.Prefix(key)
	s.RLock()
	defer s.RUnlock()
	v := s.objs.Get(prefix)
	if v == nil {
		return nil, errorNotFound
	}
	c := v.(*cacheObj)
	if c.deleted {
		return nil, errorNotFound
	}
	s.stats.gets.Add(1)
	return c.obj, nil
}

// Delete deletes an object from the cache. If 0 is specified as rev, then
// the delete is unconditional, if rev is non-zero then entry is deleted only
// if the rev matches the current rev of the cache object.
func (s *store) Delete(key string, rev uint64, cb SuccessCbFunc) (obj runtime.Object, err error) {
	prefix := patricia.Prefix(key)
	start := time.Now()
	defer func() {
		hdr.Record("store.Delete", time.Since(start))
	}()
	defer s.Unlock()
	s.Lock()
	if cb != nil {
		defer func() {
			if err == nil {
				cb(key, obj, nil)
			}
		}()
	}
	v := s.objs.Get(prefix)
	if v == nil {
		return nil, errorNotFound
	}
	cobj := v.(*cacheObj)
	obj = cobj.obj
	if rev != 0 {
		if rev <= cobj.revision {
			return nil, errorNotCorrectRev
		}
	}

	if cobj.deleted {
		return nil, errorNotFound
	}
	if rev != 0 {
		cobj.revision = rev
	}
	s.versioner.SetVersion(obj, rev)
	cobj.lastUpd = time.Now()
	cobj.deleted = true
	if !cobj.inDelQ {
		cobj.inDelQ = true
		heap.Push(s.delPending, cobj)
	} else {
		heap.Fix(s.delPending, cobj.delQId)
	}
	s.stats.deletes.Add(1)
	return
}

// List lists all the onbjects for the prefix specified in key after applying a filter
//  as per the opts parameter.
func (s *store) List(key string, opts api.ListWatchOptions) ([]runtime.Object, error) {
	start := time.Now()
	defer func() {
		hdr.Record("store.List", time.Since(start))
	}()
	var ret []runtime.Object
	prefix := patricia.Prefix(key)
	filters, err := getFilters(opts)
	if err != nil {
		return ret, err
	}
	visitfunc := func(prefix patricia.Prefix, item patricia.Item) error {
		v := item.(*cacheObj)
		if v.deleted {
			return nil
		}
		for _, fn := range filters {
			if !fn(v.obj, nil) {
				return nil
			}
		}
		ret = append(ret, v.obj)
		return nil
	}
	defer s.RUnlock()
	s.RLock()
	s.objs.VisitSubtree(prefix, visitfunc)
	s.stats.lists.Add(1)
	return ret, nil
}

// Mark marks all objects in the cache for deletion. This is used in conjunction with
//  the Sweep function below to clean up stale objects from the cache on watch channel
//  failures. When a watch channel error is seen and watch cannot be resumed due to
//  compaction, the cache has to rebuilt. Completing blowing away the cache and
//  reconstructing the cache is disruptive to the clients of the cache. So the following
//  strategy is used to get the store back in sync with the backend. Before establishing
//  the watch all objects in the cache are marked for deletion. A new watch is established.
//  with fromrev as 0. Incoming KV objects reset the mark. Once all the current objects
//  are received on the watch, Sweep is called and deletes all object still having the
//  mark and hence cleaning up stale objects from the cache.
func (s *store) Mark(key string) {
	visitfunc := func(prefix patricia.Prefix, item patricia.Item) error {
		v := item.(*cacheObj)
		v.deleted = true
		return nil
	}
	prefx := patricia.Prefix(key)
	defer s.Unlock()
	s.Lock()
	s.objs.VisitSubtree(prefx, visitfunc)
}

// Sweep clears the cache of the objects marked for deletion by Mark.
func (s *store) Sweep(key string, cb SuccessCbFunc) {
	visitfunc := func(prefix patricia.Prefix, item patricia.Item) error {
		v := item.(*cacheObj)
		if v.deleted {
			s.objs.Delete(prefix)
			cb(string(prefix), v.obj, nil)
		}
		return nil
	}
	prefx := patricia.Prefix(key)
	defer s.Unlock()
	s.Lock()
	s.objs.VisitSubtree(prefx, visitfunc)
}

// PurgeDeleted permenently deletes objects in the delete queue
func (s *store) PurgeDeleted(past time.Duration) {
	s.stats.purgeRuns.Add(1)
	defer s.Unlock()
	s.Lock()
	deletes := 0
	for {
		cobj := s.delPending.Peek()
		if cobj == nil || time.Since(cobj.lastUpd) < past || deletes > delayedDelPurgeQuanta {
			return
		}
		deletes++
		heap.Pop(s.delPending)
		ok := s.objs.Delete(patricia.Prefix(cobj.key))
		if !ok {
			s.stats.errors.Add(1)
		} else {
			s.stats.purges.Add(1)
		}
		cobj = s.delPending.Peek()
	}
}

// Clear cleans up all objects in the store.
func (s *store) Clear() {
	s.objs.DeleteSubtree(patricia.Prefix("/"))
}

// registerStats registers exported statistics vars for the store.
func (s *store) registerStats() {
	s.stats.sets = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.set", s.id))
	s.stats.deletes = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.delete", s.id))
	s.stats.gets = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.get", s.id))
	s.stats.lists = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.list", s.id))
	s.stats.purges = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.purges", s.id))
	s.stats.purgeRuns = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.purgeRuns", s.id))
	s.stats.errors = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.errors", s.id))
}
