package cache

import (
	"context"
	"reflect"
	"sync"
	"testing"

	"github.com/pensando/sw/api"
	cachemocks "github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/interfaces"
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

func (t *testObj) MakeKey(prefix string) string {
	return "/test/" + prefix + "/" + t.Name
}

func (t *testObj) Clone(into interface{}) (interface{}, error) {
	var out *testObj
	if into == nil {
		out = &testObj{}
	} else {
		out = into.(*testObj)
	}
	*out = *t
	return out, nil
}

type testObj2 struct {
	api.TypeMeta
	api.ObjectMeta
	Spec   string
	Status int
}

func (t *testObj2) MakeKey(prefix string) string {
	return "/test2/" + prefix + "/" + t.Name
}

func (t *testObj2) Clone(into interface{}) (interface{}, error) {
	var out *testObj2
	if into == nil {
		out = &testObj2{}
	} else {
		out = into.(*testObj2)
	}
	*out = *t
	return out, nil
}

type testObjList struct {
	api.TypeMeta
	api.ListMeta
	Items []*testObj
}

func (t *testObjList) Clone(into interface{}) (interface{}, error) {
	var out *testObjList
	if into == nil {
		out = &testObjList{}
	} else {
		out = into.(*testObjList)
	}
	*out = *t
	return out, nil
}

type fakeWatchPrefixes struct {
	sync.Mutex
	qmap                     map[string]*fakeWatchEventQ
	adds, dels, Gets, getexs uint64
	addfn                    func(path string) WatchEventQ
	delfn                    func(path string) WatchEventQ
	Getfn                    func(path string) []WatchEventQ
	getexfn                  func(path string) WatchEventQ
}

func (f *fakeWatchPrefixes) Add(path, peer string) WatchEventQ {
	defer f.Unlock()
	f.Lock()
	f.adds++
	if f.addfn != nil {
		return f.addfn(path)
	}
	return nil
}

func (f *fakeWatchPrefixes) Del(path, peer string) WatchEventQ {
	defer f.Unlock()
	f.Lock()
	f.dels++
	if f.delfn != nil {
		return f.delfn(path)
	}
	return nil
}

func (f *fakeWatchPrefixes) Get(path string) []WatchEventQ {
	defer f.Unlock()
	f.Lock()
	f.Gets++
	if f.Getfn != nil {
		return f.Getfn(path)
	}
	return nil
}

func (f *fakeWatchPrefixes) GetExact(path string) WatchEventQ {
	defer f.Unlock()
	f.Lock()
	f.getexs++
	if f.getexfn != nil {
		return f.getexfn(path)
	}
	return nil
}

type fakeWatchEventQ struct {
	sync.Mutex
	enqueues, dequeues, stops uint64
	dqCh                      chan error
}

func (f *fakeWatchEventQ) Enqueue(evType kvstore.WatchEventType, obj, prev runtime.Object) error {
	defer f.Unlock()
	f.Lock()
	f.enqueues++
	return nil
}

func (f *fakeWatchEventQ) Dequeue(ctx context.Context, fromver uint64, cb eventHandlerFn, cleanupfn func()) {
	defer f.Unlock()
	f.Lock()
	close(f.dqCh)
	f.dequeues++
}

func (f *fakeWatchEventQ) Stop() bool {
	defer f.Unlock()
	f.Lock()
	f.stops++
	return false
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
	str := &cachemocks.FakeStore{}
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
	if str.Sets != 1 {
		t.Errorf("invalid create counts %d", str.Sets)
	}
	str.Reset()

	t.Logf("  -> test create (failure)")
	kvstr.SetErrorState(true)
	err = c.Create(ctx, key, b)
	if err == nil {
		t.Errorf("expecting to fail")
	}
	if str.Sets != 0 {
		t.Errorf("invalid create counts %d", str.Sets)
	}
	str.Reset()

	t.Logf("  -> test update (success)")
	kvstr.SetErrorState(false)
	err = c.Update(ctx, key, b)
	if err != nil {
		t.Errorf("expecting success")
	}
	if str.Sets != 1 {
		t.Errorf("invalid update counts %d", str.Sets)
	}

	t.Logf("  -> test update (failure)")
	kvstr.SetErrorState(true)
	err = c.Update(ctx, key, b)
	if err == nil {
		t.Errorf("expecting failure")
	}
	if str.Sets != 1 {
		t.Errorf("invalid updates counts %d", str.Sets)
	}
	str.Reset()

	t.Logf("  -> test consistent update (success)")
	consistenfn := func(in runtime.Object) (runtime.Object, error) {
		return in, nil
	}
	kvstr.SetErrorState(false)
	err = c.ConsistentUpdate(ctx, key, b, consistenfn)
	if err != nil {
		t.Errorf("expecting success")
	}
	if str.Sets != 1 {
		t.Errorf("invalid consistent update counts %d", str.Sets)
	}

	t.Logf("  -> test consistent update (failure)")
	kvstr.SetErrorState(true)
	err = c.ConsistentUpdate(ctx, key, b, consistenfn)
	if err == nil {
		t.Errorf("expecting failure")
	}
	if str.Sets != 1 {
		t.Errorf("invalid consistent update counts %d", str.Sets)
	}
	str.Reset()

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
	str.Reset()

	t.Logf("  -> test delete (failure)")
	kvstr.SetErrorState(true)
	err = c.Delete(ctx, key, b)
	if err == nil {
		t.Errorf("expecting failure")
	}
	if str.Deletes != 0 {
		t.Errorf("invalid delete counts %d", str.Deletes)
	}
	str.Reset()

	t.Logf("  -> test delete (success)")
	kvstr.SetErrorState(false)

	err = c.Delete(ctx, key, b)
	if err != nil {
		t.Errorf("expecting success")
	}
	if err = kstr.Get(ctx, key, ret); err == nil {
		t.Errorf("object not expected to be found in backend")
	}
	if str.Deletes != 1 {
		t.Errorf("invalid delete counts %d", str.Deletes)
	}
	str.Reset()
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
	str := &cachemocks.FakeStore{}
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
	str.Getfn = getfn
	err = c.Get(ctx, key, b)
	if err != nil {
		t.Errorf("expecting success")
	}
	if str.Gets != 1 {
		t.Errorf("invalid get counts %d", str.Gets)
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
	str := &cachemocks.FakeStore{}
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
	str.Listfn = listfn
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
	str := &cachemocks.FakeStore{}
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
	defer fakeqs.Unlock()
	fakeqs.Lock()
	if fakeqs.adds != 1 || fakeq.dequeues != 1 {
		t.Errorf("wrong counts %d/%d", fakeqs.adds, fakeq.dequeues)
	}
	c.Close()
}

func TestPrefixWatcher(t *testing.T) {
	kstr := &cachemocks.FakeKvStore{}
	str := &cachemocks.FakeStore{}
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
	kstr.Watchfn = watchfn
	var wg sync.WaitGroup
	wg.Add(1)
	startCh := make(chan error)
	go pw.worker(ctx, &wg, startCh)
	t.Logf("  -> test watcher create")
	<-startCh
	pw.Lock()
	running := pw.running
	pw.Unlock()
	if !running {
		t.Fatalf("running not set for prefix watcher")
	}
	if kstr.Prefixwatches != 1 {
		t.Fatalf("Watch not established")
	}
	t.Logf("  -> test update event")
	ev := kvstore.WatchEvent{
		Type:   kvstore.Updated,
		Object: b1,
	}
	ws.ch <- &ev
	AssertEventually(t, func() (bool, interface{}) {
		return str.Sets == 1, nil
	}, "did not see cache set on Update event")

	t.Logf("  -> test create event")
	ev.Type = kvstore.Created
	ws.ch <- &ev
	AssertEventually(t, func() (bool, interface{}) {
		return str.Sets == 2, nil
	}, "did not see cache set on create event")

	t.Logf("  -> test delete event")
	ev.Type = kvstore.Deleted
	ws.ch <- &ev
	AssertEventually(t, func() (bool, interface{}) {
		return str.Deletes == 1, nil
	}, "did not see cache delete on delete event")
	t.Logf("  -> test WatcherError event")
	ev.Type = kvstore.WatcherError
	ws.ch <- &ev
	t.Logf("  -> test exit")
	cancel()
	AssertEventually(t, func() (bool, interface{}) {
		defer pw.Unlock()
		pw.Lock()
		return !pw.running, nil
	}, "did not prefix watcher exit on close")
}

func TestCacheDelayedDelete(t *testing.T) {
	kstr := &cachemocks.FakeKvStore{}
	str := NewStore()
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
	b1.Name = "book1"
	key := "/test/book1"
	fakeq := fakeWatchEventQ{
		dqCh: make(chan error),
	}
	addfn := func(path string) WatchEventQ {
		return &fakeq
	}
	fakeqs.addfn = addfn

	getfn := func(path string) []WatchEventQ {
		return []WatchEventQ{&fakeq}
	}
	fakeqs.Getfn = getfn
	_, err := c.Watch(ctx, "/test/", "0")
	if err != nil {
		t.Fatalf("Watch  on cache failed (%s)", err)
	}

	err = c.Create(ctx, key, b1)
	if err != nil {
		t.Errorf("expecting to succeed")
	}
	AssertEventually(t, func() (bool, interface{}) {
		return fakeq.enqueues == 1, nil
	}, "did not receive create watch event")

	err = c.Delete(ctx, key, nil)
	if err != nil {
		t.Errorf("expecting to succeed")
	}
	AssertEventually(t, func() (bool, interface{}) {
		return fakeq.enqueues == 2, nil
	}, "did not receive delete watch event")

	// purge delete Q and ensure there is no new events raised.
	str.PurgeDeleted(0)
	AssertConsistently(t, func() (bool, interface{}) {
		return fakeq.enqueues == 2, nil
	}, "received new watch event", "10ms", "200ms")
	cancel()
}

func TestBackendWatcher(t *testing.T) {
	b := &testObj{}
	scheme := runtime.NewScheme()
	scheme.AddKnownTypes(b)
	kstr, err := memkv.NewMemKv([]string{"test-cluster"}, runtime.NewJSONCodec(scheme))
	if err != nil {
		t.Fatalf("unable to create memkv")
	}
	str := &cachemocks.FakeStore{}
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
	AssertEventually(t, func() (bool, interface{}) {
		for _, v := range bw.prefixes {
			v.Lock()
			if v.running {
				v.Unlock()
				return false, nil
			}
			v.Unlock()
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
	kstr.(*memkv.MemKv).SetRevMode(memkv.ClusterRevision)
	str := &cachemocks.FakeStore{}
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
	fakeqs.Getfn = getfn
	f := cachemocks.FakeTxn{}
	b1 := &testObj{}
	b1.ResourceVersion = "10"
	b1.Name = "book 1"

	tx := cacheTxn{
		Txn:    &f,
		parent: &c,
	}
	tx.Update("/key1", b1)
	tx.Update("/key2", b1)
	tx.Delete("/key3")
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
	setfn := func(key string, rev uint64, obj runtime.Object, cb apiintf.SuccessCbFunc) error {
		cb(key, b1, nil)
		return nil
	}
	str.Setfn = setfn
	delfn := func(key string, rev uint64, cb apiintf.SuccessCbFunc) (runtime.Object, error) {
		cb(key, b1, nil)
		return b1, nil
	}
	str.Deletefn = delfn
	commitfn := func(ctx context.Context) (kvstore.TxnResponse, error) {
		return resp, nil
	}
	f.Commitfn = commitfn

	tx.Commit(ctx)
	if fakeq1.enqueues != 3 || fakeq2.enqueues != 3 {
		t.Errorf("expecting 3 enqueues in 2 queueus got %d/%d", fakeq1.enqueues, fakeq2.enqueues)
	}
	cancel()
}
