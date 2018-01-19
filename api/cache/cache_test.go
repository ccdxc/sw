package cache

import (
	"context"
	"reflect"
	"sync"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	memkv "github.com/pensando/sw/venice/utils/kvstore/memkv"
	kvs "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type testObj struct {
	api.TypeMeta
	api.ObjectMeta
	Spec   string
	Status string
}

type testObjList struct {
	api.TypeMeta
	api.ListMeta
	Items []*testObj
}

type fakeKvStore struct {
	creates, deletes               uint64
	prefixdeletes, updates         uint64
	consupdates, gets, lists       uint64
	closes, watches, prefixwatches uint64
	contests, leases, newtxns      uint64
	watchfiltered, listfiltered    uint64
	createfn                       func(ctx context.Context, key string, obj runtime.Object) error
	deletefn                       func(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error
	updatefn                       func(ctx context.Context, key string, obj runtime.Object) error
	watchfn                        func(ctx context.Context, key string, fromVersion string) (kvstore.Watcher, error)
	fwatchfn                       func(ctx context.Context, key string, opts api.ListWatchOptions) kvstore.Watcher
	getfn                          func(ctx context.Context, key string, into runtime.Object) error
	listfn                         func(ctx context.Context, prefix string, into runtime.Object) error
}

func (f *fakeKvStore) Create(ctx context.Context, key string, obj runtime.Object) error {
	f.creates++
	if f.createfn != nil {
		return f.createfn(ctx, key, obj)
	}
	return nil
}
func (f *fakeKvStore) Delete(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error {
	f.deletes++
	if f.deletefn != nil {
		return f.deletefn(ctx, key, into, cs...)
	}
	return nil
}

func (f *fakeKvStore) PrefixDelete(ctx context.Context, prefix string) error {
	f.prefixdeletes++
	return nil
}

func (f *fakeKvStore) Update(ctx context.Context, key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	f.updates++
	if f.updatefn != nil {
		return f.updatefn(ctx, key, obj)
	}
	return nil
}

func (f *fakeKvStore) ConsistentUpdate(ctx context.Context, key string, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	f.consupdates++
	if f.updatefn != nil {
		return f.updatefn(ctx, key, into)
	}
	return nil
}

func (f *fakeKvStore) Get(ctx context.Context, key string, into runtime.Object) error {
	f.gets++
	if f.getfn != nil {
		return f.getfn(ctx, key, into)
	}
	return nil
}

func (f *fakeKvStore) List(ctx context.Context, prefix string, into runtime.Object) error {
	f.lists++
	if f.listfn != nil {
		return f.listfn(ctx, prefix, into)
	}
	return nil
}

func (f *fakeKvStore) Watch(ctx context.Context, key string, fromVersion string) (kvstore.Watcher, error) {
	f.watches++
	if f.watchfn != nil {
		return f.watchfn(ctx, key, fromVersion)
	}
	return nil, nil
}

func (f *fakeKvStore) WatchFiltered(ctx context.Context, key string, opts api.ListWatchOptions) kvstore.Watcher {
	f.watchfiltered++
	if f.fwatchfn != nil {
		return f.fwatchfn(ctx, key, opts)
	}
	return nil
}

func (f *fakeKvStore) PrefixWatch(ctx context.Context, prefix string, fromVersion string) (kvstore.Watcher, error) {
	f.prefixwatches++
	if f.watchfn != nil {
		return f.watchfn(ctx, prefix, fromVersion)
	}
	return nil, nil
}

func (f *fakeKvStore) Contest(ctx context.Context, name string, id string, ttl uint64) (kvstore.Election, error) {
	f.contests++
	return nil, nil
}

func (f *fakeKvStore) Lease(ctx context.Context, key string, obj runtime.Object, ttl uint64) (chan kvstore.LeaseEvent, error) {
	f.leases++
	ch := make(chan kvstore.LeaseEvent)
	return ch, nil
}
func (f *fakeKvStore) NewTxn() kvstore.Txn {
	f.newtxns++
	return nil
}
func (f *fakeKvStore) Close() {
	f.closes++
}

func (f *fakeKvStore) reset() {
	f.creates = 0
	f.deletes = 0
	f.prefixdeletes = 0
	f.updates = 0
	f.consupdates = 0
	f.gets = 0
	f.lists = 0
	f.watches = 0
	f.prefixwatches = 0
	f.contests = 0
	f.leases = 0
	f.newtxns = 0
	f.closes = 0
}

type fakeStore struct {
	sets, gets, deletes uint64
	lists, flushes      uint64
	marks, sweeps       uint64
	setfn               func(key string, rev uint64, obj runtime.Object, cb SuccessCbFunc) error
	getfn               func(key string) (runtime.Object, error)
	deletefn            func(key string, rev uint64, cb SuccessCbFunc) (runtime.Object, error)
	listfn              func(key string, opts api.ListWatchOptions) []runtime.Object
}

func (f *fakeStore) Set(key string, rev uint64, obj runtime.Object, cb SuccessCbFunc) error {
	f.sets++
	if f.setfn != nil {
		return f.setfn(key, rev, obj, cb)
	}
	return nil
}
func (f *fakeStore) Get(key string) (runtime.Object, error) {
	f.gets++
	if f.getfn != nil {
		return f.getfn(key)
	}
	return nil, nil
}

func (f *fakeStore) Delete(key string, rev uint64, cb SuccessCbFunc) (runtime.Object, error) {
	f.deletes++
	if f.deletefn != nil {
		f.deletefn(key, rev, cb)
	}
	return nil, nil
}

func (f *fakeStore) List(key string, opts api.ListWatchOptions) []runtime.Object {
	f.lists++
	if f.listfn != nil {
		return f.listfn(key, opts)
	}
	var ret []runtime.Object
	return ret
}

func (f *fakeStore) Mark(key string) {
	f.marks++
}
func (f *fakeStore) Sweep(key string, cb SuccessCbFunc) {
	f.sweeps++
}

func (f *fakeStore) Clear() {
	f.flushes++
}

func (f *fakeStore) reset() {
	f.sets = 0
	f.gets = 0
	f.deletes = 0
	f.lists = 0
	f.flushes = 0
	f.marks = 0
	f.sweeps = 0
}

type fakeWatchPrefixes struct {
	adds, dels, gets, getexs uint64
	addfn                    func(path string) WatchEventQ
	delfn                    func(path string) WatchEventQ
	getfn                    func(path string) []WatchEventQ
	getexfn                  func(path string) WatchEventQ
}

func (f *fakeWatchPrefixes) Add(path string) WatchEventQ {
	f.adds++
	if f.addfn != nil {
		return f.addfn(path)
	}
	return nil
}

func (f *fakeWatchPrefixes) Del(path string) WatchEventQ {
	f.dels++
	if f.delfn != nil {
		return f.delfn(path)
	}
	return nil
}

func (f *fakeWatchPrefixes) Get(path string) []WatchEventQ {
	f.gets++
	if f.getfn != nil {
		return f.getfn(path)
	}
	return nil
}

func (f *fakeWatchPrefixes) GetExact(path string) WatchEventQ {
	f.getexs++
	if f.getexfn != nil {
		return f.getexfn(path)
	}
	return nil
}

type fakeWatchEventQ struct {
	enqueues, dequeues, stops uint64
	dqCh                      chan error
}

func (f *fakeWatchEventQ) Enqueue(evType kvstore.WatchEventType, obj runtime.Object) error {
	f.enqueues++
	return nil
}

func (f *fakeWatchEventQ) Dequeue(ctx context.Context, fromver uint64, cb eventHandlerFn, cleanupfn func()) {
	close(f.dqCh)
	f.dequeues++
}

func (f *fakeWatchEventQ) Stop() {
	f.stops++
}

type fakeTxn struct {
	commitfn func(ctx context.Context) (kvstore.TxnResponse, error)
}

func (f *fakeTxn) Create(key string, obj runtime.Object) error {
	return nil
}

func (f *fakeTxn) Delete(key string, cs ...kvstore.Cmp) error {
	return nil
}

func (f *fakeTxn) Update(key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	return nil
}
func (f *fakeTxn) IsEmpty() bool {
	return false
}

func (f *fakeTxn) AddComparator(cs ...kvstore.Cmp) {}

func (f *fakeTxn) Commit(ctx context.Context) (kvstore.TxnResponse, error) {
	if f.commitfn != nil {
		return f.commitfn(ctx)
	}
	return kvstore.TxnResponse{}, nil
}

func TestCreateCache(t *testing.T) {
	cluster := []string{
		"abc",
	}
	kcfg := kvs.Config{
		Type:    kvs.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: cluster,
	}
	config := Config{
		Config:       kcfg,
		NumKvClients: 20,
		Logger:       log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
	}
	c, err := CreateNewCache(config)
	if err != nil {
		t.Fatalf("Cache create failed")
	}
	err = c.Start()
	if err != nil {
		t.Fatalf("failed to start cache")
	}
	c1 := c.(*cache)
	if c1.pool == nil || len(c1.pool.pool) != 20 {
		t.Errorf("pool not initialzed")
	}
	c.Close()
	if len(c1.pool.pool) != 0 {
		t.Errorf("pool not cleaned")
	}
}

func TestCacheOper(t *testing.T) {
	b := &testObj{}
	scheme := runtime.NewScheme()
	scheme.AddKnownTypes(b)
	kstr, err := memkv.NewMemKv([]string{"test-cluster"}, runtime.NewJSONCodec(scheme))
	kvstr := kstr.(*memkv.MemKv)
	if err != nil {
		t.Fatalf("unable to create memkv")
	}
	str := &fakeStore{}
	c := cache{
		store:  str,
		pool:   &connPool{},
		queues: &fakeWatchPrefixes{},
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(kstr)

	b.ResourceVersion = "10"
	key := "/testkey"
	ctx := context.TODO()

	t.Logf("  -> test create (success)")

	err = c.Create(ctx, key, b)
	if err != nil {
		t.Errorf("expecting to succeed")
	}
	ret := &testObj{}
	if err = kstr.Get(ctx, key, ret); err != nil {
		t.Errorf("object not found in backend (%s)", err)
	}
	if str.sets != 1 {
		t.Errorf("invalid create counts %d", str.sets)
	}
	str.reset()

	t.Logf("  -> test create (failure)")
	kvstr.SetErrorState(true)
	err = c.Create(ctx, key, b)
	if err == nil {
		t.Errorf("expecting to fail")
	}
	if str.sets != 0 {
		t.Errorf("invalid create counts %d", str.sets)
	}
	str.reset()

	t.Logf("  -> test update (success)")
	kvstr.SetErrorState(false)
	err = c.Update(ctx, key, b)
	if err != nil {
		t.Errorf("expecting success")
	}
	if str.sets != 1 {
		t.Errorf("invalid update counts %d", str.sets)
	}

	t.Logf("  -> test update (failure)")
	kvstr.SetErrorState(true)
	err = c.Update(ctx, key, b)
	if err == nil {
		t.Errorf("expecting failure")
	}
	if str.sets != 1 {
		t.Errorf("invalid updates counts %d", str.sets)
	}
	str.reset()

	t.Logf("  -> test consistent update (success)")
	consistenfn := func(in runtime.Object) (runtime.Object, error) {
		return in, nil
	}
	kvstr.SetErrorState(false)
	err = c.ConsistentUpdate(ctx, key, b, consistenfn)
	if err != nil {
		t.Errorf("expecting success")
	}
	if str.sets != 1 {
		t.Errorf("invalid consistent update counts %d", str.sets)
	}

	t.Logf("  -> test consistent update (failure)")
	kvstr.SetErrorState(true)
	err = c.ConsistentUpdate(ctx, key, b, consistenfn)
	if err == nil {
		t.Errorf("expecting failure")
	}
	if str.sets != 1 {
		t.Errorf("invalid consistent update counts %d", str.sets)
	}
	str.reset()

	t.Logf("  -> test contest")
	kvstr.SetErrorState(false)
	_, err = c.Contest(ctx, "testName", "testId", 10)
	if err != nil {
		t.Errorf("expecting success got (%s)", err)
	}

	t.Logf("  -> test lease")
	_, err = c.Lease(ctx, "/test/lease1", b, 0)
	if err != nil {
		t.Errorf("expecting success got (%s)", err)
	}
	str.reset()

	t.Logf("  -> test delete (failure)")
	kvstr.SetErrorState(true)
	err = c.Delete(ctx, key, b)
	if err == nil {
		t.Errorf("expecting failure")
	}
	if str.deletes != 0 {
		t.Errorf("invalid delete counts %d", str.deletes)
	}
	str.reset()

	t.Logf("  -> test delete (success)")
	kvstr.SetErrorState(false)

	err = c.Delete(ctx, key, b)
	if err != nil {
		t.Errorf("expecting success")
	}
	if err = kstr.Get(ctx, key, ret); err == nil {
		t.Errorf("object not expected to be found in backend")
	}
	if str.deletes != 1 {
		t.Errorf("invalid delete counts %d", str.deletes)
	}
	str.reset()
	c.Close()

}

func TestCacheGet(t *testing.T) {
	b := &testObj{}
	scheme := runtime.NewScheme()
	scheme.AddKnownTypes(b)
	kstr, err := memkv.NewMemKv([]string{"test-cluster"}, runtime.NewJSONCodec(scheme))
	if err != nil {
		t.Fatalf("unable to create memkv")
	}
	str := &fakeStore{}
	c := cache{
		store:  str,
		pool:   &connPool{},
		queues: &fakeWatchPrefixes{},
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(kstr)
	b.ResourceVersion = "10"
	key := "/testkey"
	ctx := context.TODO()
	var err1 error

	getfn := func(key string) (runtime.Object, error) {
		return b, err1
	}

	t.Logf("  -> test get (success)")
	err1 = nil
	str.getfn = getfn
	err = c.Get(ctx, key, b)
	if err != nil {
		t.Errorf("expecting success")
	}
	if str.gets != 1 {
		t.Errorf("invalid get counts %d", str.gets)
	}

	t.Logf("  -> test get (failure)")
	err1 = errorNotFound
	err = c.Get(ctx, key, b)
	if err == nil {
		t.Errorf("expecting failure")
	}
	c.Close()
}

func TestCacheList(t *testing.T) {
	b := &testObj{}
	scheme := runtime.NewScheme()
	scheme.AddKnownTypes(b)
	kstr, err := memkv.NewMemKv([]string{"test-cluster"}, runtime.NewJSONCodec(scheme))
	if err != nil {
		t.Fatalf("unable to create memkv")
	}
	str := &fakeStore{}
	c := cache{
		store:  str,
		pool:   &connPool{},
		queues: &fakeWatchPrefixes{},
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(kstr)
	key := "/testkey"
	ctx := context.TODO()
	b1 := &testObj{}
	b1.ResourceVersion = "10"
	b1.Name = "book 1"
	b2 := &testObj{}
	b2.ResourceVersion = "100"
	b2.Name = "book 2"
	b3 := &testObj{}
	b3.ResourceVersion = "231"
	b3.Name = "book 3"
	expected := []runtime.Object{b1, b2, b3}

	t.Logf("  -> test list (plain and filtered)")
	listfn := func(key string, opts api.ListWatchOptions) []runtime.Object {
		return expected
	}
	str.listfn = listfn
	into := &testObjList{}
	c.List(ctx, key, into)
	if len(into.Items) != 3 {
		t.Errorf("expecting 3 objects, got %d", len(into.Items))
	}
	t.Logf("got %+v", into.Items)
	for _, v := range into.Items {
		for i, cmp := range expected {
			if reflect.DeepEqual(v, cmp) {
				expected[i] = nil
			}
		}
	}
	for _, cmp := range expected {
		if cmp != nil {
			t.Errorf("Found an object that was not matched in list %+v", cmp)
		}
	}
	c.Close()
}

func TestCacheWatch(t *testing.T) {
	b := &testObj{}
	scheme := runtime.NewScheme()
	scheme.AddKnownTypes(b)
	kstr, err := memkv.NewMemKv([]string{"test-cluster"}, runtime.NewJSONCodec(scheme))
	if err != nil {
		t.Fatalf("unable to create memkv")
	}
	str := &fakeStore{}
	fakeqs := &fakeWatchPrefixes{}
	c := cache{
		store:  str,
		pool:   &connPool{},
		queues: fakeqs,
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(kstr)
	key := "/testkey"
	ctx := context.TODO()

	fakeq := fakeWatchEventQ{
		dqCh: make(chan error),
	}
	addfn := func(path string) WatchEventQ {
		return &fakeq
	}
	fakeqs.addfn = addfn

	c.Watch(ctx, key, "0")
	<-fakeq.dqCh
	if fakeqs.adds != 1 || fakeq.dequeues != 1 {
		t.Errorf("wrong counts %d/%d", fakeqs.adds, fakeq.dequeues)
	}
	c.Close()
}

func TestPrefixWatcher(t *testing.T) {
	kstr := &fakeKvStore{}
	str := &fakeStore{}
	fakeqs := &fakeWatchPrefixes{}
	c := cache{
		store:  str,
		pool:   &connPool{},
		queues: fakeqs,
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(kstr)
	ctx, cancel := context.WithCancel(context.TODO())
	b1 := &testObj{}
	b1.ResourceVersion = "10"
	b1.Name = "book 1"

	pw := prefixWatcher{
		path:    "/test",
		parent:  &c,
		lastVer: "0",
	}
	ws := &watchServer{
		ch: make(chan *kvstore.WatchEvent),
	}

	watchfn := func(ctx context.Context, key string, fromVersion string) (kvstore.Watcher, error) {
		return ws, nil
	}
	kstr.watchfn = watchfn
	var wg sync.WaitGroup
	wg.Add(1)
	go pw.worker(ctx, &wg)
	t.Logf("  -> test watcher create")
	AssertEventually(t, func() (bool, []interface{}) {
		defer pw.Unlock()
		pw.Lock()
		return pw.running == true, nil
	}, "running not set")
	if kstr.prefixwatches != 1 {
		t.Fatalf("Watch not established")
	}
	t.Logf("  -> test update event")
	ev := kvstore.WatchEvent{
		Type:   kvstore.Updated,
		Object: b1,
	}
	ws.ch <- &ev
	AssertEventually(t, func() (bool, []interface{}) {
		return str.sets == 1, nil
	}, "did not see cache set on Update event")

	t.Logf("  -> test create event")
	ev.Type = kvstore.Created
	ws.ch <- &ev
	AssertEventually(t, func() (bool, []interface{}) {
		return str.sets == 2, nil
	}, "did not see cache set on create event")

	t.Logf("  -> test delete event")
	ev.Type = kvstore.Deleted
	ws.ch <- &ev
	AssertEventually(t, func() (bool, []interface{}) {
		return str.deletes == 1, nil
	}, "did not see cache delete on delete event")
	t.Logf("  -> test WatcherError event")
	ev.Type = kvstore.WatcherError
	ws.ch <- &ev
	t.Logf("  -> test exit")
	cancel()
	AssertEventually(t, func() (bool, []interface{}) {
		return !pw.running, nil
	}, "did not prefix watcher exit on close")
}

func TestBackendWatcher(t *testing.T) {
	b := &testObj{}
	scheme := runtime.NewScheme()
	scheme.AddKnownTypes(b)
	kstr, err := memkv.NewMemKv([]string{"test-cluster"}, runtime.NewJSONCodec(scheme))
	if err != nil {
		t.Fatalf("unable to create memkv")
	}
	str := &fakeStore{}
	fakeqs := &fakeWatchPrefixes{}
	c := cache{
		store:  str,
		pool:   &connPool{},
		queues: fakeqs,
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(kstr)
	ctx, cancel := context.WithCancel(context.TODO())
	bw := &backendWatcher{
		prefixes: make(map[string]*prefixWatcher),
		parent:   &c,
	}
	t.Logf("  -> test double add same prefix")
	w1 := bw.NewPrefixWatcher(ctx, "/test/path1")
	w2 := bw.NewPrefixWatcher(ctx, "/test/path1")
	if w1 != w2 {
		t.Errorf("created new prefix watcher for same prefix")
	}
	if len(bw.prefixes) != 1 {
		t.Errorf("expecting 1 prefixWatchers, found %d", len(bw.prefixes))
	}

	t.Logf("  -> test add new prefix")
	w3 := bw.NewPrefixWatcher(ctx, "/test/path2")
	if w1 == w3 {
		t.Errorf("did not create new prefix watcher for different prefix")
	}
	if len(bw.prefixes) != 2 {
		t.Errorf("expecting 2 prefixWatchers, found %d", len(bw.prefixes))
	}

	t.Logf("  -> test stop")
	cancel()
	AssertEventually(t, func() (bool, []interface{}) {
		for _, v := range bw.prefixes {
			if v.running {
				return false, nil
			}
		}
		return true, nil
	}, "did not prefix watcher exit on close")
}

func TestTxnCommit(t *testing.T) {
	b := &testObj{}
	scheme := runtime.NewScheme()
	scheme.AddKnownTypes(b)
	kstr, err := memkv.NewMemKv([]string{"test-cluster"}, runtime.NewJSONCodec(scheme))
	if err != nil {
		t.Fatalf("unable to create memkv")
	}
	str := &fakeStore{}
	fakeqs := &fakeWatchPrefixes{}
	c := cache{
		store:  str,
		pool:   &connPool{},
		queues: fakeqs,
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(kstr)
	ctx, cancel := context.WithCancel(context.TODO())
	fakeq1 := fakeWatchEventQ{}
	fakeq2 := fakeWatchEventQ{}
	wqs := []WatchEventQ{&fakeq1, &fakeq2}
	getfn := func(path string) []WatchEventQ {
		t.Logf("get called")
		return wqs
	}
	fakeqs.getfn = getfn
	f := fakeTxn{}
	b1 := &testObj{}
	b1.ResourceVersion = "10"
	b1.Name = "book 1"
	resps := []kvstore.TxnOpResponse{
		{
			Oper: kvstore.OperUpdate,
			Obj:  b1,
		},
		{
			Oper: kvstore.OperUpdate,
			Obj:  b1,
		},
		{
			Oper: kvstore.OperDelete,
			Obj:  b1,
		},
	}
	resp := kvstore.TxnResponse{
		Succeeded: true,
		Responses: resps,
	}
	setfn := func(key string, rev uint64, obj runtime.Object, cb SuccessCbFunc) error {
		cb(key, b1)
		return nil
	}
	str.setfn = setfn
	delfn := func(key string, rev uint64, cb SuccessCbFunc) (runtime.Object, error) {
		cb(key, b1)
		return b1, nil
	}
	str.deletefn = delfn
	commitfn := func(ctx context.Context) (kvstore.TxnResponse, error) {
		t.Logf("commit called")
		return resp, nil
	}
	f.commitfn = commitfn
	tx := txn{
		Txn:    &f,
		parent: &c,
	}
	tx.Commit(ctx)
	if fakeq1.enqueues != 3 || fakeq2.enqueues != 3 {
		t.Errorf("expecting 3 enqueues in 2 queueus got %d/%d", fakeq1.enqueues, fakeq2.enqueues)
	}
	cancel()
}
