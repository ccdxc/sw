package cache

import (
	"container/heap"
	"expvar"
	"fmt"
	"sort"
	"sync"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/tchap/go-patricia/patricia"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/interfaces"
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
	snapgGets *expvar.Int
	snapLists *expvar.Int
	stat      *expvar.Int
	statall   *expvar.Int
}

var storeID uint32

type mvccObj struct {
	deleted     bool
	obj         runtime.Object
	snapshotRev uint64
	objRev      uint64
}

type snapshot struct {
	keys    map[string]uint64
	deleted bool
}

// cacheObj is a wrapper around each object stored in the cache.
type cacheObj struct {
	revision     uint64
	key          string
	obj          runtime.Object
	lastUpd      time.Time
	deleted      bool
	inDelQ       bool
	delQId       int
	snapshots    map[uint64]mvccObj
	snapshotList []uint64
	lastSnapshot uint64
}

type delPendingHeap struct {
	l []*cacheObj
}

// store is an implementation of the Store interface
type store struct {
	sync.RWMutex
	objs         *patricia.Trie
	stats        StoreStats
	delPending   *delPendingHeap
	versioner    runtime.Versioner
	id           uint32
	lastVer      uint64
	lastSnapshot uint64
	snapshots    map[uint64]snapshot
}

// NewStore creates a new Local Object Store.
func NewStore() apiintf.Store {

	ret := &store{
		objs:       patricia.NewTrie(),
		id:         atomic.AddUint32(&storeID, 1),
		delPending: &delPendingHeap{},
		versioner:  runtime.NewObjectVersioner(),
		snapshots:  make(map[uint64]snapshot),
	}
	heap.Init(ret.delPending)
	ret.registerStats()
	return ret
}

type sortU64Intf struct {
	in []uint64
}

// Len is the number of elements in the collection.
func (in *sortU64Intf) Len() int {
	return len(in.in)
}

// Less reports whether the element with
// index i should sort before the element with index j.
func (in *sortU64Intf) Less(i, j int) bool {
	return in.in[i] < in.in[j]
}

// Swap swaps the elements with indexes i and j.
func (in *sortU64Intf) Swap(i, j int) {
	t := in.in[j]
	in.in[j] = in.in[i]
	in.in[i] = t
}

// addVersion adds a snapshot version to the cache object.
//  Conditions for taking recording a snapshot.
//    - Snapshot is requested. The current object does not yet have a snapshot for the lastSnapshotVer
//    - save object in the cache should be of a earlier time stamp than the snapshot request.
//  Snapshot once take is never to be updated since the rev never backpedals.
func (c *cacheObj) addVersion(rev, snapshotVer uint64, del bool, obj runtime.Object) bool {
	if snapshotVer == 0 {
		// nothing to snapshot
		return false
	}
	if c.lastSnapshot > snapshotVer {
		panic("snapshot version backpedals")
	}

	if c.lastSnapshot != snapshotVer {
		c.snapshots[snapshotVer] = mvccObj{deleted: del, obj: obj, snapshotRev: snapshotVer, objRev: rev}
		c.lastSnapshot = snapshotVer
		c.snapshotList = append(c.snapshotList, snapshotVer)
		return true
	}
	return false
}

func (c *cacheObj) delVersion(rev uint64) bool {
	if _, ok := c.snapshots[rev]; ok {
		delete(c.snapshots, rev)
		return true
	}
	return false
}

func (c *cacheObj) getVersion(rev uint64) (obj runtime.Object, deleted bool, err error) {
	if o, ok := c.snapshots[rev]; ok {
		return o.obj, o.deleted, nil
	}
	if len(c.snapshotList) == 0 {
		return nil, false, fmt.Errorf("snapshot not found")
	}
	// find the best match for the snapshot
	var pobj *mvccObj
	for i := range c.snapshotList {
		cobj := c.snapshots[c.snapshotList[i]]
		if c.snapshotList[i] > rev {
			if cobj.objRev <= rev {
				return c.snapshots[c.snapshotList[i]].obj, c.snapshots[c.snapshotList[i]].deleted, nil
			}
			if pobj != nil {
				return pobj.obj, pobj.deleted, nil
			}
			return nil, false, errorSnapshotNotFound
		}
		pobj = &cobj
	}
	return nil, false, errorSnapshotNotFound
}

// String returns a human readable string of the queue (not in heap order)
func (h *delPendingHeap) String() string {
	var keys []string
	for _, v := range h.l {
		keys = append(keys, v.key)
	}
	return fmt.Sprintf("Len: %d keys: [%v]", h.Len(), keys)
}

// Len returns the len of the pendingDelete heap
func (h *delPendingHeap) Len() int {
	return len(h.l)
}

// Less implements the sort.Interface
func (h *delPendingHeap) Less(i, j int) bool {
	return h.l[i].lastUpd.Before(h.l[j].lastUpd)
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
	heap.Fix(h, cobj.delQId)
}

// Pop pops item from the heap in accordance with the heap.Interface
func (h *delPendingHeap) Pop() interface{} {
	old := h.l
	l := len(h.l)
	ret := old[l-1]
	h.l = old[0 : l-1]
	ret.delQId = 0
	ret.inDelQ = false
	return ret
}

// Peek peeks at the top of the heap without popping it
func (h *delPendingHeap) Peek() *cacheObj {
	if len(h.l) == 0 {
		return nil
	}
	return h.l[0]
}

// Set updates the cache for the key with obj while making sure revision moves
//  only formward. Any updating with older version is rejected. the callback function
//  is called if the operation succeeds
func (s *store) Set(key string, rev uint64, obj runtime.Object, cb apiintf.SuccessCbFunc) (err error) {
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
			if !cobj.deleted {
				prev = cobj.obj
			}
			if s.lastSnapshot != 0 {
				if cobj.addVersion(cobj.revision, s.lastSnapshot, cobj.deleted, cobj.obj) {
					s.snapshots[s.lastSnapshot].keys[key] = cobj.revision
				}
			}
			cobj.obj = obj

			cobj.revision = rev
			cobj.deleted = false
			cobj.lastUpd = time.Now()
			if cobj.inDelQ {
				heap.Remove(s.delPending, cobj.delQId)
				cobj.delQId = 0
				cobj.inDelQ = false
			}
			s.lastVer = rev
			return nil
		}
		success = false
		return errorRevBackpedals
	}
	cobj := &cacheObj{
		revision:  rev,
		key:       key,
		obj:       obj,
		deleted:   false,
		inDelQ:    false,
		lastUpd:   time.Now(),
		snapshots: make(map[uint64]mvccObj),
	}
	s.lastVer = rev
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

func (s *store) getFromSnapshot(rev uint64, prefix patricia.Prefix) (runtime.Object, error) {
	if ss, ok := s.snapshots[rev]; !ok || ss.deleted {
		return nil, errorSnapshotNotFound
	}
	v := s.objs.Get(prefix)
	if v == nil {
		return nil, errorNotFound
	}
	c := v.(*cacheObj)
	if c.revision <= rev {
		if c.deleted {
			return nil, errorNotFound
		}
		return c.obj, nil
	}
	ret, del, err := c.getVersion(rev)
	if err != nil {
		return nil, err
	}
	if del {
		return nil, errorNotFound
	}
	return ret, nil
}

// Get retrieves a stored object from a snapshot in the cache.
func (s *store) GetFromSnapshot(rev uint64, key string) (runtime.Object, error) {
	start := time.Now()
	defer func() {
		hdr.Record("store.Get", time.Since(start))
	}()
	prefix := patricia.Prefix(key)
	s.RLock()
	defer s.RUnlock()
	ret, err := s.getFromSnapshot(rev, prefix)
	if err != nil {
		return nil, err
	}
	s.stats.snapgGets.Add(1)
	return ret, nil
}

// Delete deletes an object from the cache. If 0 is specified as rev, then
// the delete is unconditional, if rev is non-zero then entry is deleted only
// if the rev matches the current rev of the cache object.
func (s *store) Delete(key string, rev uint64, cb apiintf.SuccessCbFunc) (obj runtime.Object, err error) {
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
		s.lastVer = rev
	}

	if s.lastSnapshot != 0 {
		ccobj, err := obj.Clone(nil)
		if err != nil {
			panic(fmt.Sprintf("could not clone object (%s)", err))
		}
		if cobj.addVersion(cobj.revision, s.lastSnapshot, cobj.deleted, ccobj.(runtime.Object)) {
			s.snapshots[s.lastSnapshot].keys[key] = cobj.revision
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
func (s *store) List(key, kind string, opts api.ListWatchOptions) ([]runtime.Object, error) {
	start := time.Now()
	defer func() {
		hdr.Record("store.List", time.Since(start))
	}()
	var ret []runtime.Object
	prefix := patricia.Prefix(key)
	filters, err := getFilters(opts, kind)
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
		it, err := v.obj.Clone(nil)
		if err != nil {
			log.Errorf("Clone got error %s", err)
			return err
		}
		ret = append(ret, it.(runtime.Object))
		return nil
	}
	defer s.RUnlock()
	s.RLock()
	s.objs.VisitSubtree(prefix, visitfunc)
	s.stats.lists.Add(1)
	return ret, nil
}

// ListFromSnapshot lists all objects from the prefix from an existing snapshot after applying given filters
func (s *store) ListFromSnapshot(rev uint64, key, kind string, opts api.ListWatchOptions) ([]runtime.Object, error) {
	start := time.Now()
	defer func() {
		hdr.Record("store.ListSnapshot", time.Since(start))
	}()
	var ret []runtime.Object
	prefix := patricia.Prefix(key)
	filters, err := getFilters(opts, kind)
	if err != nil {
		return ret, err
	}
	visitfunc := func(prefix patricia.Prefix, item patricia.Item) error {
		v := item.(*cacheObj)
		if v.revision <= rev {
			if !v.deleted {
				it, err := v.obj.Clone(nil)
				if err == nil {
					ret = append(ret, it.(runtime.Object))
				} else {
					log.Errorf("could not clone (%s)", err)
				}
			}
			return nil
		}
		obj, deleted, err := v.getVersion(rev)
		if err != nil || deleted {
			return nil
		}
		for _, fn := range filters {
			if !fn(obj, nil) {
				return nil
			}
		}
		it, err := obj.Clone(nil)
		if err != nil {
			log.Errorf("Clone got error %s", err)
			return err
		}
		ret = append(ret, it.(runtime.Object))
		return nil
	}
	defer s.RUnlock()
	s.RLock()
	s.objs.VisitSubtree(prefix, visitfunc)
	s.stats.snapLists.Add(1)
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
func (s *store) Sweep(key string, cb apiintf.SuccessCbFunc) {
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
	// First look through snapshots and garbage collect snapshots. Adhere to the delayed delete quanta
	ss := []uint64{}
	for i := range s.snapshots {
		ss = append(ss, i)
	}
	sort.Sort(&sortU64Intf{ss})
	snapdels := 0

purgeSnapshot:
	for _, snap := range ss {
		sshot := s.snapshots[snap]
		if sshot.deleted {
			for k := range sshot.keys {
				prefix := patricia.Prefix(k)
				v := s.objs.Get(prefix)
				if v == nil {
					continue
				}
				cobj := v.(*cacheObj)
				if cobj.delVersion(snap) {
					snapdels++
				}
				// XXX-MUSTDO - promote object to the next snapshot if next snapshot is empty.
				delete(sshot.keys, k)
				if snapdels > delayedDelPurgeQuanta {
					break purgeSnapshot
				}
			}
			if len(sshot.keys) == 0 {
				delete(s.snapshots, snap)
			}
		}
	}

	for {
		cobj := s.delPending.Peek()
		if cobj == nil || time.Since(cobj.lastUpd) < past || deletes > delayedDelPurgeQuanta {
			return
		}
		if len(cobj.snapshots) != 0 {
			// This needs to be much more intelligent. For now this is okay since the snapshots are only
			//  supported for config save and is expected to shortlived, but when snapshots are used for
			//  other purposes like pagination etc. the janitor has to smarter to look past the ones with
			//  active snapshots and continue cleaning up.
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

// Stat returns stat for list of keys
func (s *store) Stat(keys []string) []apiintf.ObjectStat {
	s.RLock()
	defer s.RUnlock()
	var ret []apiintf.ObjectStat
	for i := range keys {
		prefix := patricia.Prefix(keys[i])
		v := s.objs.Get(prefix)
		if v == nil {
			ret = append(ret, apiintf.ObjectStat{Key: keys[i], Valid: false})
		} else {
			c := v.(*cacheObj)
			if c.deleted {
				ret = append(ret, apiintf.ObjectStat{Key: keys[i], Valid: false})
			} else {
				obj := apiintf.ObjectStat{Key: keys[i], Valid: true, Revision: c.revision, LastUpd: c.lastUpd}
				if c.obj != nil {
					if objm, err := runtime.GetObjectMeta(c.obj); err == nil {
						obj.ObjectMeta = *objm
						obj.TypeMeta.Kind = c.obj.GetObjectKind()
						obj.TypeMeta.APIVersion = c.obj.GetObjectAPIVersion()
					}
				}
				ret = append(ret, obj)
			}
		}
	}
	s.stats.stat.Add(1)
	return ret
}

// ListKeys returns all keys with the prefix
func (s *store) StatAll(prefix string) []apiintf.ObjectStat {
	var ret []apiintf.ObjectStat
	visitFunc := func(pfx patricia.Prefix, item patricia.Item) error {
		v := item.(*cacheObj)
		if v.deleted {
			ret = append(ret, apiintf.ObjectStat{Key: v.key, Valid: false})
		} else {
			obj := apiintf.ObjectStat{Key: v.key, Valid: true, Revision: v.revision, LastUpd: v.lastUpd}
			if v.obj != nil {
				if objm, err := runtime.GetObjectMeta(v.obj); err == nil {
					obj.ObjectMeta = *objm
					obj.TypeMeta.Kind = v.obj.GetObjectKind()
					obj.TypeMeta.APIVersion = v.obj.GetObjectAPIVersion()
				}
			}
			ret = append(ret, obj)
		}
		return nil
	}
	ppfx := patricia.Prefix(prefix)
	s.objs.VisitSubtree(ppfx, visitFunc)
	s.stats.statall.Add(1)
	return ret
}

// StartSnapshot marks the store to start a snapshot
func (s *store) StartSnapshot() uint64 {
	defer s.Unlock()
	s.Lock()
	ss, ok := s.snapshots[s.lastVer]
	if !ok {
		s.snapshots[s.lastVer] = snapshot{keys: make(map[string]uint64)}
		s.lastSnapshot = s.lastVer
	}
	if ss.deleted {
		ss.deleted = false
	}
	return s.lastSnapshot
}

func (s *store) DeleteSnapshot(rev uint64) error {
	defer s.Unlock()
	s.Lock()
	ss, ok := s.snapshots[rev]
	if !ok || ss.deleted {
		return errorSnapshotNotFound
	}
	ss.deleted = true
	s.snapshots[rev] = ss
	return nil
}

func (s *store) ListSnapshotWithCB(pfix string, rev uint64, cbfunc func(key string, cur, revObj runtime.Object, deleted bool) error) error {
	defer s.Unlock()
	s.Lock()
	ss, ok := s.snapshots[rev]
	if !ok || ss.deleted {
		return errorSnapshotNotFound
	}

	start := time.Now()
	defer func() {
		hdr.Record("store.ListSnapshotWithCB", time.Since(start))
	}()
	prefix := patricia.Prefix(pfix)
	visitFunc := func(prefix patricia.Prefix, item patricia.Item) error {
		v := item.(*cacheObj)
		sobj, err := s.getFromSnapshot(rev, prefix)
		if err == nil {
			return cbfunc(v.key, v.obj, sobj, v.deleted)
		}
		return cbfunc(v.key, v.obj, nil, v.deleted)
	}
	s.objs.VisitSubtree(prefix, visitFunc)
	s.stats.lists.Add(1)
	return nil
}

// Clear cleans up all objects in the store.
func (s *store) Clear() {
	defer s.Unlock()
	s.Lock()
	s.objs.DeleteSubtree(patricia.Prefix("/"))
}

// registerStats registers exported statistics vars for the store.
func (s *store) registerStats() {
	s.stats.sets = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.set", s.id))
	s.stats.deletes = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.delete", s.id))
	s.stats.gets = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.get", s.id))
	s.stats.snapgGets = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.snapGet", s.id))
	s.stats.lists = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.list", s.id))
	s.stats.snapLists = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.snapList", s.id))
	s.stats.purges = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.purges", s.id))
	s.stats.purgeRuns = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.purgeRuns", s.id))
	s.stats.errors = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.errors", s.id))
	s.stats.stat = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.stat", s.id))
	s.stats.statall = expvar.NewInt(fmt.Sprintf("api.cache.store[%d].ops.statall", s.id))
}
