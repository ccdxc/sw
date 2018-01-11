package cache

import (
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
	sets    *expvar.Int
	deletes *expvar.Int
	gets    *expvar.Int
	lists   *expvar.Int
}

var storeID uint32

// SuccessCbFunc is called when a Store operation succeeds. The call happens in
// the critical path hence the function should the min needed.
type SuccessCbFunc func(key string, obj runtime.Object)

// Store is the interface for the Local Object Store
type Store interface {
	Set(key string, rev uint64, obj runtime.Object, cb SuccessCbFunc) error
	Get(key string) (runtime.Object, error)
	Delete(key string, rev uint64, cb SuccessCbFunc) (runtime.Object, error)
	List(key string, opts api.ListWatchOptions) []runtime.Object
	Mark(key string)
	Sweep(key string, cb SuccessCbFunc)
	Clear()
}

// cacheObj is a wrapper around each object stored in the cache.
type cacheObj struct {
	revision uint64
	obj      runtime.Object
	deleted  bool
}

// store is an implementation of the Store interface
type store struct {
	sync.RWMutex
	objs  *patricia.Trie
	stats StoreStats
	id    uint32
}

// NewStore creates a new Local Object Store.
func NewStore() Store {

	ret := &store{
		objs: patricia.NewTrie(),
		id:   atomic.AddUint32(&storeID, 1),
	}
	ret.registerStats()
	return ret
}

// Set updates the cache for the key with obj while making sure revision moves
//  only formward. Any updating with older version is rejected. the callback function
//  is called if the operation succeeds
func (s *store) Set(key string, rev uint64, obj runtime.Object, cb SuccessCbFunc) (err error) {
	start := time.Now()
	success := true
	defer func() {
		hdr.Record("store.Set", time.Since(start))
	}()
	prefix := patricia.Prefix(key)
	if cb != nil {
		defer func() {
			if success == true {
				cb(key, obj)
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
			cobj.obj = obj
			cobj.revision = rev
			cobj.deleted = false
			return nil
		}
		success = false
		return errorRevBackpedals
	}
	cobj := &cacheObj{
		revision: rev,
		obj:      obj,
		deleted:  false,
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
				cb(key, obj)
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
		if cobj.revision != rev {
			return nil, errorNotCorrectRev
		}
	}
	ok := s.objs.Delete(prefix)
	if !ok {
		return nil, errorNotFound
	}
	s.stats.deletes.Add(1)
	return
}

// List lists all the onbjects for the prefix specified in key after applying a filter
//  as per the opts parameter.
func (s *store) List(key string, opts api.ListWatchOptions) []runtime.Object {
	start := time.Now()
	defer func() {
		hdr.Record("store.List", time.Since(start))
	}()
	var ret []runtime.Object
	prefix := patricia.Prefix(key)
	filters := getFilters(opts)
	visitfunc := func(prefix patricia.Prefix, item patricia.Item) error {
		v := item.(*cacheObj)
		for _, fn := range filters {
			if !fn(v.obj) {
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
	return ret
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
			cb(string(prefix), v.obj)
		}
		return nil
	}
	prefx := patricia.Prefix(key)
	defer s.Unlock()
	s.Lock()
	s.objs.VisitSubtree(prefx, visitfunc)
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
}
