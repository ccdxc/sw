package watchstream

import (
	"context"
	"reflect"
	"sync/atomic"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	apitest "github.com/pensando/sw/api/api_test"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/testutils"
)

func TestWatchedPrefixes(t *testing.T) {
	wconfig := WatchEventQConfig{
		SweepInterval:     (120 * time.Second),
		RetentionDuration: time.Nanosecond,
		RetentionDepthMax: 0,
		EvictInterval:     10 * time.Second,
	}
	wp := watchedPrefixes{
		log:         log.GetNewLogger(log.GetDefaultConfig("cacheWatchTest")),
		store:       &mocks.FakeStore{},
		watchConfig: wconfig,
	}
	wp.init()
	r := wp.Get("/")
	if len(r) != 0 {
		t.Errorf("expecting 0 found %d", len(r))
	}
	re := wp.GetExact("/testpath")
	if re != nil {
		t.Errorf("expecting nil ")
	}
	o1 := wp.Add("/testpath/one", "peer1").(*watchEventQ)
	o2 := wp.Add("/testpath/two", "peer1").(*watchEventQ)
	t1 := wp.Add("/testpath/one", "peer1").(*watchEventQ)
	t2 := wp.Add("/testpath/two", "peer1").(*watchEventQ)
	wp.Add("/testpath/three", "peer1")
	if o1 != t1 || o2 != t2 {
		t.Errorf("Got different elements for same path")
	}
	if o1.refCount != 2 || o2.refCount != 2 {
		t.Errorf("expecting refcount 2 got %d/%d", o1.refCount, o2.refCount)
	}

	cbfunc := func(inctx context.Context, evType kvstore.WatchEventType, obj, prev runtime.Object, control *kvstore.WatchControl) {
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	t3 := wp.Add("/testpath/one/two", "peer1")
	r = wp.Get("/testpath/")
	if len(r) != 0 {
		t.Errorf("expecting 0 found %d", len(r))
	}
	r = wp.Get("/testpath/one")
	if len(r) != 0 {
		t.Errorf("expecting 1 found %d", len(r))
	}
	go o1.Dequeue(ctx, 0, false, cbfunc, func() {}, &api.ListWatchOptions{})
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return o1.watcherList.Len() == 1, nil
	}, "expecting 1 error", "5ms", "100ms")

	r = wp.Get("/testpath/one")
	if len(r) != 1 {
		t.Errorf("expecting 1 found %d", len(r))
	}
	r = wp.Get("/testpath/one/two")
	if len(r) != 1 {
		t.Errorf("expecting 2 found %d", len(r))
	}

	go t3.Dequeue(ctx, 0, false, cbfunc, func() {}, &api.ListWatchOptions{})
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return o1.watcherList.Len() == 1, nil
	}, "expecting 1 error", "5ms", "100ms")
	r = wp.Get("/testpath/one/two")
	if len(r) != 2 {
		t.Errorf("expecting 2 found %d", len(r))
	}

	re = wp.GetExact("/testpath/")
	if re != nil {
		t.Errorf("not expecting entry")
	}
	re = wp.GetExact("/testpath/junk")
	if re != nil {
		t.Errorf("one expecting entry")
	}
	re = wp.GetExact("/testpath/one")
	if re == nil {
		t.Errorf("expecting entry")
	}
	wp.Del("/testpath/one", "peer1")
	wp.Del("/testpath/two", "peer1")
	if r := wp.Get("/testpath/one"); len(r) != 1 {
		t.Errorf("expecting 1 found %d", len(r))
	}
	if r := wp.Get("/testpath/two"); len(r) != 0 {
		t.Errorf("expecting 0 found %d", len(r))
	}
	wp.Del("/testpath/one", "peer1")
	wp.Del("/testpath/two", "peer1")
	if r := wp.Get("/testpath/one"); len(r) != 0 {
		t.Errorf("expecting 1 found %d", len(r))
	}
	if r := wp.Get("/testpath/two"); len(r) != 0 {
		t.Errorf("expecting 1 found %d", len(r))
	}
}

func TestWatchEventQ(t *testing.T) {
	// Override the watch configuration
	wconfig := WatchEventQConfig{
		SweepInterval:     (120 * time.Second),
		RetentionDuration: time.Nanosecond,
		RetentionDepthMax: 0,
		EvictInterval:     10 * time.Second,
	}
	wp := watchedPrefixes{
		log:         log.GetNewLogger(log.GetDefaultConfig("cacheWatchTest")),
		store:       &mocks.FakeStore{},
		watchConfig: wconfig,
	}
	wp.init()
	q := wp.Add("/testpath2/one", "peer1").(*watchEventQ)
	if q == nil {
		t.Fatalf("Failed to create watchEventQ")
	}
	ctx := context.Background()
	rcvdEvents := int32(0)
	rcvdErrors := int32(0)
	cbfunc := func(id string) func(inctx context.Context, evType kvstore.WatchEventType, obj, prev runtime.Object, control *kvstore.WatchControl) {
		return func(inctx context.Context, evType kvstore.WatchEventType, obj, prev runtime.Object, control *kvstore.WatchControl) {
			t.Logf("Q [%s] received event %s object %+v", id, evType, obj)
			if evType == kvstore.WatcherError {
				atomic.AddInt32(&rcvdErrors, 1)
			} else {
				atomic.AddInt32(&rcvdEvents, 1)
			}
		}
	}
	t.Logf(" --> Dequeue on an empty EventQ")
	d := time.Now().Add(50 * time.Millisecond)
	nctx, cancel := context.WithDeadline(ctx, d)
	opts := api.ListWatchOptions{}
	q.Dequeue(nctx, 0, false, cbfunc("q1-0"), nil, &opts)
	cancel()
	if q.watcherList.Len() != 0 {
		t.Errorf("expecting number of active watchers to be 0 found %d", q.watcherList.Len())
	}

	t.Logf(" --> Dequeue with fromVer on an empty EventQ")
	d = time.Now().Add(50 * time.Millisecond)
	nctx, cancel = context.WithDeadline(ctx, d)
	q.Dequeue(nctx, 10, false, cbfunc("q1-0"), nil, &opts)
	cancel()
	if q.watcherList.Len() != 0 {
		t.Errorf("expecting number of active watchers to be 0 found %d", q.watcherList.Len())
	}
	testutils.Assert(t, rcvdErrors == 1, "expecting a received error")
	rcvdErrors = 0

	t.Logf(" --> Enqueue without any active Watchers")
	b1 := apitest.TestObj{}
	b1.ResourceVersion = "111"
	b2 := apitest.TestObj{}
	b2.ResourceVersion = "112"
	b3 := apitest.TestObj{}
	b3.ResourceVersion = "113"
	b4 := apitest.TestObj{}
	b4.ResourceVersion = "114"
	b5 := apitest.TestObj{}
	b5.ResourceVersion = "115"
	q.Enqueue(kvstore.Created, &b1, nil)
	if q.eventList.Len() != 1 || q.watcherList.Len() != 0 {
		t.Errorf("expecing events: 1 and watcher: 0 , found %d/%d", q.eventList.Len(), q.watcherList.Len())
	}

	t.Logf(" --> dequeue instance without fromVer")
	nctx, cancel = context.WithCancel(ctx)
	go q.Dequeue(nctx, 0, false, cbfunc("q2-0"), nil, &opts)
	time.Sleep(100 * time.Millisecond)
	q.Enqueue(kvstore.Created, &b2, nil)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return rcvdEvents == 1, nil
	}, "expecting 1 events", "10ms", "10000ms")

	cancel()
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == 0, nil
	}, "expecting watcher to exit", "10ms", "100ms")

	t.Logf(" --> dequeue instance with fromVer -1")
	rcvdEvents = 0
	nctx, cancel = context.WithCancel(ctx)

	watchersCount := q.watcherList.Len()
	go q.Dequeue(nctx, 0, true, cbfunc("q2-from-1"), nil, &opts)
	time.Sleep(100 * time.Millisecond)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is %d", rcvdEvents)
		return rcvdEvents == 0, nil
	}, "expecting 0 event", "10ms", "100ms")
	q.Enqueue(kvstore.Created, &b3, nil)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is %d", rcvdEvents)
		return rcvdEvents == 1, nil
	}, "expecting 0 event", "10ms", "100ms")
	cancel()
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount, nil
	}, "expecting watcher to exit", "10ms", "100ms")

	t.Logf(" --> dequeue instance with fromVer")

	rcvdEvents = 0
	nctx, cancel = context.WithCancel(ctx)

	// watch for old version.
	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 109, false, cbfunc("q3-109"), nil, &opts)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount, nil
	}, "expecting new watcher", "10ms", "100ms")
	testutils.AssertConsistently(t, func() (bool, interface{}) {
		t.Logf("erorrs is %d", rcvdErrors)
		return rcvdErrors == 1, nil
	}, "expecting 1 error", "10ms", "100ms")

	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 0, false, cbfunc("q4-0"), nil, &opts)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is %d", rcvdEvents)
		return rcvdEvents == 0, nil
	}, "expecting 0 event", "10ms", "100ms")

	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 112, false, cbfunc("q5-112"), nil, &opts)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is %d", rcvdEvents)
		return rcvdEvents == 2, nil
	}, "expecting 1 events", "10ms", "100ms")

	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 113, false, cbfunc("q6-113"), nil, &opts)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	testutils.AssertConsistently(t, func() (bool, interface{}) {
		t.Logf("events is%d", rcvdEvents)
		return rcvdEvents == 1, nil
	}, "expecting 1 events", "10ms", "100ms")
	if q.watcherList.Len() != 3 {
		t.Errorf("incorrect number of watchers")
	}

	t.Logf(" --> Enqueue with multiple active watchers")
	rcvdEvents = 0
	q.Enqueue(kvstore.Created, &b4, nil)
	q.Enqueue(kvstore.Created, &b5, nil)
	testutils.AssertConsistently(t, func() (bool, interface{}) {
		t.Logf("events is %d", rcvdEvents)
		return atomic.LoadInt32(&rcvdEvents) == 6, nil
	}, "expecting 6 events", "10ms", "200ms")

	if q.eventList.Len() != 5 {
		t.Errorf("incorrect number of events")
	}

	t.Logf("let janitor cleanup events")
	q.janitorFn()
	if q.eventList.Len() != 1 {
		t.Errorf("incorrect number of events")
	}

	t.Logf(" --> janitor slow watchers")
	slowrcvd := int32(0)
	slowBlockCount := int32(3)
	slowcb := func(id string) func(inctx context.Context, evType kvstore.WatchEventType, obj, prev runtime.Object, conteol *kvstore.WatchControl) {
		return func(inctx context.Context, evType kvstore.WatchEventType, obj, prev runtime.Object, control *kvstore.WatchControl) {
			atomic.AddInt32(&slowrcvd, 1)
			t.Logf("q[%s] slowCb called", id)
			if slowrcvd < slowBlockCount {
				return
			}
			t.Logf("q[%s] slowCb blocking", id)
			select {
			case <-nctx.Done():
			case <-inctx.Done():
			}
		}
	}
	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 116, false, slowcb("q1"), nil, &opts)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	b1.ResourceVersion = "116"
	b2.ResourceVersion = "117"
	b3.ResourceVersion = "118"
	b4.ResourceVersion = "119"
	q.Enqueue(kvstore.Created, &b1, nil)
	q.Enqueue(kvstore.Created, &b2, nil)
	q.Enqueue(kvstore.Created, &b3, nil)
	q.Enqueue(kvstore.Created, &b4, nil)

	if q.watcherList.Len() != 4 {
		t.Errorf("incorrect number of watchers")
	}
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is %d slowRcvd is %d", rcvdEvents, slowrcvd)
		return rcvdEvents == 12 && slowrcvd == 3, nil
	}, "expecting 12 and slowrcvd 3 events", "10ms", "100ms")
	if q.eventList.Len() != 5 {
		t.Errorf("incorrect number of events %d", q.eventList.Len())
	}
	q.janitorFn()
	if q.eventList.Len() != 3 {
		t.Errorf("incorrect number of events %d", q.eventList.Len())
	}

	t.Logf("Stop the Queue")
	q.Stop()
	cancel()
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("Lengths are  %d/%d", q.watcherList.Len(), q.eventList.Len())
		return q.watcherList.Len() == 0 && q.eventList.Len() == 0, nil
	}, "expecting 1 events", "10ms", "10s")

	t.Logf(" --> Test Eviction")
	wconfig.EvictInterval = time.Nanosecond
	wp.watchConfig = wconfig
	q3 := wp.Add("/testpath2/three", "peer1").(*watchEventQ)
	nctx, cancel = context.WithCancel(ctx)
	slowrcvd = 0
	slowBlockCount = 2
	watchersCount = q3.watcherList.Len()
	go q3.Dequeue(nctx, 0, false, slowcb("q3-0"), nil, &opts)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q3.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	b5.ResourceVersion = "120"
	q3.Enqueue(kvstore.Created, &b4, nil)
	q3.Enqueue(kvstore.Created, &b5, nil)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("slow rcvd are  %d", slowrcvd)
		return slowrcvd == 2, nil
	}, "expecting to hit 2 slowrcvd", "10ms", "1000ms")
	q3.janitorFn()
	if q3.stats.clientEvictions.Value() != int64(1) {
		t.Errorf("expecting 1 eviction")
	}
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q3.watcherList.Len() == 0, nil
	}, "expecting watcher to exit", "10ms", "1000ms")
	q3.Stop()
	cancel()

	// Test timebased janitor cleanup
	t.Logf(" --> Test timed janitor cleanup")
	q4 := wp.Add("/testpath2/four", "peer1").(*watchEventQ)
	q4.config.RetentionDuration = time.Nanosecond
	q4.config.RetentionDepthMax = 10
	ageouts := q4.stats.ageoutEvictions.Value()
	depthout := q4.stats.depthEvictions.Value()
	dequeues := q4.stats.dequeues.Value()
	t.Logf("got stats as %d/%d/%d", dequeues, ageouts, depthout)
	nctx, cancel = context.WithCancel(ctx)
	if q4.watcherList.Len() != 0 {
		t.Errorf("not expecing any watchers, got %d", q4.watcherList.Len())
	}
	cleanupCalled := int32(0)
	cleanfunc := func() {
		atomic.AddInt32(&cleanupCalled, 1)
	}
	go q4.Dequeue(nctx, 0, false, cbfunc("slow-q4-0"), cleanfunc, &opts)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return q4.watcherList.Len() == 1, nil
	}, "expecting to hit 1 watcher", "10ms", "1000ms")
	b3.ResourceVersion = "121"
	b4.ResourceVersion = "122"
	b5.ResourceVersion = "123"
	q4.Enqueue(kvstore.Created, &b3, nil)
	q4.Enqueue(kvstore.Created, &b4, nil)
	q4.Enqueue(kvstore.Created, &b5, nil)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("dequeues are %d/%d", dequeues, q4.stats.dequeues.Value())
		return (q4.stats.dequeues.Value() - dequeues) == 3, nil
	}, "expecting to hit 3 dequeuesd", "10ms", "1000ms")
	q4.janitorFn()
	if (q4.stats.ageoutEvictions.Value() - ageouts) != 2 {
		t.Errorf("expecting 2 ageouts got %d", q4.stats.ageoutEvictions.Value())
	}
	if (q4.stats.depthEvictions.Value() - depthout) != 0 {
		t.Errorf("expecting 0 depthouts got %d", q4.stats.depthEvictions.Value())
	}

	// Test depth based janitor cleanup
	q4.config.RetentionDuration = time.Second * 10
	q4.config.RetentionDepthMax = 2
	b3.ResourceVersion = "124"
	b4.ResourceVersion = "125"
	b5.ResourceVersion = "126"
	ageouts = q4.stats.ageoutEvictions.Value()
	depthout = q4.stats.depthEvictions.Value()
	dequeues = q4.stats.dequeues.Value()
	q4.Enqueue(kvstore.Created, &b3, nil)
	q4.Enqueue(kvstore.Created, &b4, nil)
	q4.Enqueue(kvstore.Created, &b5, nil)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("dequeues are %d/%d", dequeues, q4.stats.dequeues.Value())
		return (q4.stats.dequeues.Value() - dequeues) == 3, nil
	}, "expecting to hit 3 dequeuesd", "10ms", "1000ms")
	q4.janitorFn()
	if (q4.stats.ageoutEvictions.Value() - ageouts) != 0 {
		t.Errorf("expecting 2 ageouts got %d", q4.stats.ageoutEvictions.Value())
	}
	if (q4.stats.depthEvictions.Value() - depthout) != 2 {
		t.Errorf("expecting 2 depthouts got %d", q4.stats.depthEvictions.Value())
	}
	cancel()
	testutils.AssertEventually(t, func() (bool, interface{}) {

		return cleanupCalled == 1, nil
	}, "expecting to hit 3 dequeuesd", "10ms", "1000ms")
}

func TestAggregateWatchers(t *testing.T) {
	// Override the watch configuration
	wconfig := WatchEventQConfig{
		SweepInterval:     (120 * time.Second),
		RetentionDuration: time.Nanosecond,
		RetentionDepthMax: 0,
		EvictInterval:     10 * time.Second,
	}
	fakeStore := &mocks.FakeStore{}
	wp := watchedPrefixes{
		log:         log.GetNewLogger(log.GetDefaultConfig("cacheWatchTest")),
		store:       fakeStore,
		watchConfig: wconfig,
	}
	wp.init()

	// Test cases
	//  - Add multipath aggregate without a path watcher
	//  - Add path watcher while an aggregate watcher exists
	//  - Add multipath aggregatw with hybrid of existing and new paths
	//  - Ensure Get() returns all needed watch Queues
	//  - Ensure Dequeue does consistent list of events
	//  - Ensure Dequeue sends in order between kinds.
	//  - tear down Multipath before kind watcher.
	//  - tear down path before multipath and then kind watcher

	cb := func(inctx context.Context, evType kvstore.WatchEventType, obj, prev runtime.Object, control *kvstore.WatchControl) {
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	paths := []string{"/testpath/one", "/testpath2/one"}
	wp.AddAggregate(paths, "peer1")

	q1 := wp.Get("/testpath/one")
	q2 := wp.Get("/testpath2/one")
	q3 := wp.Get("/testpath2/two")
	testutils.Assert(t, q1 != nil, "q1 is nil")
	testutils.Assert(t, len(q1) == 1, "expecting len to be 2 got [%d]", len(q1))
	testutils.Assert(t, q2 != nil, "q2 is nil")
	testutils.Assert(t, len(q2) == 1, "expecting len to be 2 got [%d]", len(q2))
	testutils.Assert(t, q3 == nil, "q3 is not nil")

	// add a new path watchers
	w1 := wp.Add("/testpath2/one", "peer1")
	w1q := w1.(*watchEventQ)
	go w1.Dequeue(ctx, 0, false, cb, func() {}, &api.ListWatchOptions{})
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return w1q.watcherList.Len() == 1, nil
	}, "expecting 1 error", "5ms", "100ms")

	q1 = wp.Get("/testpath/one")
	q2 = wp.Get("/testpath2/one")
	q3 = wp.Get("/testpath2/two")
	testutils.Assert(t, q1 != nil, "q1 is nil")
	testutils.Assert(t, len(q1) == 1, "expecting len to be 2 got [%d]", len(q1))
	testutils.Assert(t, q2 != nil, "q2 is nil")
	testutils.Assert(t, len(q2) == 2, "expecting len to be 2 got [%d]", len(q2))
	testutils.Assert(t, q3 == nil, "q3 is not nil")

	w2 := wp.Add("/testpath2/two", "peer1")
	w2q := w2.(*watchEventQ)
	go w2.Dequeue(ctx, 0, false, cb, func() {}, &api.ListWatchOptions{})
	testutils.AssertEventually(t, func() (bool, interface{}) {
		return w2q.watcherList.Len() == 1, nil
	}, "expecting 1 error", "5ms", "100ms")
	q1 = wp.Get("/testpath/one")
	q2 = wp.Get("/testpath2/one")
	q3 = wp.Get("/testpath2/two")
	testutils.Assert(t, q1 != nil, "q1 is nil")
	testutils.Assert(t, len(q1) == 1, "expecting len to be 2 got [%d]", len(q1))
	testutils.Assert(t, q2 != nil, "q2 is nil")
	testutils.Assert(t, len(q2) == 2, "expecting len to be 2 got [%d]", len(q2))
	testutils.Assert(t, q3 != nil, "q3 is nil")
	testutils.Assert(t, len(q3) == 1, "expecting len to be 1 got [%d]", len(q3))

	paths = []string{"/testpath/one", "/testpath2/two"}
	wp.AddAggregate(paths, "peer1")
	q1 = wp.Get("/testpath/one")
	q2 = wp.Get("/testpath2/one")
	q3 = wp.Get("/testpath2/two")
	testutils.Assert(t, q1 != nil, "q1 is nil")
	testutils.Assert(t, len(q1) == 2, "expecting len to be 2 got [%d]", len(q1))
	testutils.Assert(t, q2 != nil, "q2 is nil")
	testutils.Assert(t, len(q2) == 2, "expecting len to be 2 got [%d]", len(q2))
	testutils.Assert(t, q3 != nil, "q3 is nil")
	testutils.Assert(t, len(q3) == 2, "expecting len to be 2 got [%d]", len(q3))

	// should reuse
	paths = []string{"/testpath/one", "/testpath2/two"}
	wp.AddAggregate(paths, "peer1")
	q1 = wp.Get("/testpath/one")
	q2 = wp.Get("/testpath2/one")
	q3 = wp.Get("/testpath2/two")
	testutils.Assert(t, q1 != nil, "q1 is nil")
	testutils.Assert(t, len(q1) == 2, "expecting len to be 2 got [%d]", len(q1))
	testutils.Assert(t, q2 != nil, "q2 is nil")
	testutils.Assert(t, len(q2) == 2, "expecting len to be 2 got [%d]", len(q2))
	testutils.Assert(t, q3 != nil, "q3 is nil")
	testutils.Assert(t, len(q3) == 2, "expecting len to be 2 got [%d]", len(q3))

	// Test Dequeue
	rcvdEvents := int32(0)
	rcvdErrors := int32(0)
	recvdCtrls := int32(0)
	recvdObjs := []string{}
	cbfunc := func(id string) func(inctx context.Context, evType kvstore.WatchEventType, obj, prev runtime.Object, control *kvstore.WatchControl) {
		return func(inctx context.Context, evType kvstore.WatchEventType, obj, prev runtime.Object, control *kvstore.WatchControl) {
			t.Logf("Q [%s] received event %s object %+v", id, evType, obj)
			if evType == kvstore.WatcherError {
				atomic.AddInt32(&rcvdErrors, 1)
			} else {
				atomic.AddInt32(&rcvdEvents, 1)
			}
			if evType == kvstore.WatcherControl {
				atomic.AddInt32(&recvdCtrls, 1)
			} else {
				robj := obj.(*apitest.TestObj)
				recvdObjs = append(recvdObjs, robj.Name)
			}

		}
	}

	b1 := apitest.TestObj{}
	b1.ResourceVersion = "110"
	b1.Name = "obj1"
	b2 := apitest.TestObj{}
	b2.ResourceVersion = "111"
	b2.Name = "obj2"
	b3 := apitest.TestObj{}
	b3.ResourceVersion = "112"
	b3.Name = "obj3"
	b4 := apitest.TestObj{}
	b4.ResourceVersion = "113"
	b4.Name = "obj4"
	b5 := apitest.TestObj{}
	b5.ResourceVersion = "114"
	b5.Name = "obj5"
	b6 := apitest.TestObj{}
	b6.ResourceVersion = "115"
	b6.Name = "obj6"

	listFromSnapshotFn := func(rev uint64, key, kind string, opts api.ListWatchOptions) ([]runtime.Object, error) {
		t.Logf("got getSnapshot for [%v]", key)
		var ret []runtime.Object
		if key == "/testpath/one" {
			ret = []runtime.Object{&b2, &b3}
		}
		if key == "/testpath2/one" {
			ret = []runtime.Object{&b4, &b5}
		}
		return ret, nil
	}

	fakeStore.SnapShotRev = uint64(112)
	fakeStore.ListFromSnapshotFn = listFromSnapshotFn

	qs := wp.Get("/testpath2/one")
	testutils.Assert(t, len(qs) == 2, "expecting 2 entries got [%d]", len(qs))
	var q *watchEventQ
	for _, i := range qs {
		if i.(*watchEventQ).multiPath {
			q = i.(*watchEventQ)
			break
		}
	}
	testutils.Assert(t, q != nil, "did not find multipath queue")
	q.start()
	q.Enqueue(kvstore.Created, &b1, nil)
	q.Enqueue(kvstore.Created, &b6, nil)
	d := time.Now().Add(50000 * time.Millisecond)
	nctx, cancel := context.WithDeadline(ctx, d)
	go q.Dequeue(nctx, 0, false, cbfunc("multipath"), nil, nil)
	testutils.AssertEventually(t, func() (bool, interface{}) {
		t.Logf("dequeues are %d", q.stats.dequeues.Value())
		return (q.stats.dequeues.Value()) == 2, nil
	}, "expecting to hit 2 dequeuesd", "10ms", "1000ms")
	cancel()
	exp := []string{"obj2", "obj3", "obj4", "obj5", "obj6"}
	testutils.Assert(t, reflect.DeepEqual(exp, recvdObjs), "recvd objects does not match [%v][%v]", recvdObjs, exp)
}
