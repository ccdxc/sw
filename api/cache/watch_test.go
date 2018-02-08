package cache

import (
	"context"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
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
	o1 := wp.Add("/testpath/one").(*watchEventQ)
	o2 := wp.Add("/testpath/two").(*watchEventQ)
	t1 := wp.Add("/testpath/one").(*watchEventQ)
	t2 := wp.Add("/testpath/two").(*watchEventQ)
	wp.Add("/testpath/three")
	if o1 != t1 || o2 != t2 {
		t.Errorf("Got different elements for same path")
	}
	if o1.refCount != 2 || o2.refCount != 2 {
		t.Errorf("expecting refcount 2 got %d/%d", o1.refCount, o2.refCount)
	}

	wp.Add("/testpath/one/two")
	r = wp.Get("/testpath/")
	if len(r) != 0 {
		t.Errorf("expecting 0 found %d", len(r))
	}
	r = wp.Get("/testpath/one")
	if len(r) != 1 {
		t.Errorf("expecting 1 found %d", len(r))
	}
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
	wp.Del("/testpath/one")
	wp.Del("/testpath/two")
	if r := wp.Get("/testpath/one"); len(r) != 1 {
		t.Errorf("expecting 1 found %d", len(r))
	}
	if r := wp.Get("/testpath/two"); len(r) != 1 {
		t.Errorf("expecting 1 found %d", len(r))
	}
	wp.Del("/testpath/one")
	wp.Del("/testpath/two")
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
		store:       &fakeStore{},
		watchConfig: wconfig,
	}
	wp.init()
	q := wp.Add("/testpath2/one").(*watchEventQ)
	if q == nil {
		t.Fatalf("Failed to create watchEventQ")
	}
	ctx := context.Background()
	rcvdEvents := 0
	rcvdErrors := 0
	cbfunc := func(evType kvstore.WatchEventType, obj runtime.Object) {
		t.Logf("received object %+v", obj)
		if evType == kvstore.WatcherError {
			rcvdErrors++
		} else {
			rcvdEvents++
		}
	}
	t.Logf(" --> Dequeue on an empty EventQ")
	d := time.Now().Add(50 * time.Millisecond)
	nctx, cancel := context.WithDeadline(ctx, d)
	q.Dequeue(nctx, 0, cbfunc, nil)
	cancel()
	if q.watcherList.Len() != 0 {
		t.Errorf("expecting number of active watchers to be 0 found %d", q.watcherList.Len())
	}
	t.Logf(" --> Enqueue without any active Watchers")
	b1 := testObj{}
	b1.ResourceVersion = "111"
	b2 := testObj{}
	b2.ResourceVersion = "112"
	b3 := testObj{}
	b3.ResourceVersion = "113"
	b4 := testObj{}
	b4.ResourceVersion = "114"
	b5 := testObj{}
	b5.ResourceVersion = "115"
	q.Enqueue(kvstore.Created, &b1)
	if q.eventList.Len() != 1 || q.watcherList.Len() != 0 {
		t.Errorf("expecing events: 1 and watcher: 0 , found %d/%d", q.eventList.Len(), q.watcherList.Len())
	}

	t.Logf(" --> dequeue instance without fromVer")
	nctx, cancel = context.WithCancel(ctx)
	go q.Dequeue(nctx, 0, cbfunc, nil)
	time.Sleep(100 * time.Millisecond)
	q.Enqueue(kvstore.Created, &b2)
	AssertEventually(t, func() (bool, interface{}) {
		return rcvdEvents == 1, nil
	}, "expecting 1 events", "10ms", "10000ms")

	cancel()
	AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == 0, nil
	}, "expecting watcher to exit", "10ms", "100ms")

	t.Logf(" --> dequeue instance with fromVer")
	q.Enqueue(kvstore.Created, &b3)
	rcvdEvents = 0
	nctx, cancel = context.WithCancel(ctx)

	// watch for old version.
	rcvdEvents = 0
	watchersCount := q.watcherList.Len()
	go q.Dequeue(nctx, 109, cbfunc, nil)
	AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount, nil
	}, "expecting new watcher", "10ms", "100ms")
	AssertEventually(t, func() (bool, interface{}) {
		t.Logf("erors is%d", rcvdErrors)
		return rcvdErrors == 1, nil
	}, "expecting 1 error", "10ms", "100ms")

	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 0, cbfunc, nil)
	AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is%d", rcvdEvents)
		return rcvdEvents == 0, nil
	}, "expecting 1 event", "10ms", "100ms")

	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 112, cbfunc, nil)
	AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is%d", rcvdEvents)
		return rcvdEvents == 2, nil
	}, "expecting 1 events", "10ms", "100ms")

	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 113, cbfunc, nil)
	AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is%d", rcvdEvents)
		return rcvdEvents == 1, nil
	}, "expecting 1 events", "10ms", "100ms")
	if q.watcherList.Len() != 3 {
		t.Errorf("incorrect number of watchers")
	}

	t.Logf(" --> Enqueue with multiple active watchers")
	rcvdEvents = 0
	q.Enqueue(kvstore.Created, &b4)
	q.Enqueue(kvstore.Created, &b5)
	AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is %d", rcvdEvents)
		return rcvdEvents == 6, nil
	}, "expecting 1 events", "10ms", "100ms")

	if q.eventList.Len() != 5 {
		t.Errorf("incorrect number of events")
	}

	t.Logf("let janitor cleanup events")
	q.janitorFn()
	if q.eventList.Len() != 1 {
		t.Errorf("incorrect number of events")
	}

	t.Logf(" --> janitor slow watchers")
	slowrcvd := 0
	slowBlockCount := 3
	slowcb := func(evType kvstore.WatchEventType, obj runtime.Object) {
		slowrcvd++
		t.Logf("slowCb called")
		if slowrcvd < slowBlockCount {
			return
		}
		t.Logf("slowCb blocking")
		select {
		case <-nctx.Done():
		}
	}
	rcvdEvents = 0
	watchersCount = q.watcherList.Len()
	go q.Dequeue(nctx, 0, slowcb, nil)
	AssertEventually(t, func() (bool, interface{}) {
		return q.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	b1.ResourceVersion = "116"
	b2.ResourceVersion = "117"
	b3.ResourceVersion = "118"
	b4.ResourceVersion = "119"
	q.Enqueue(kvstore.Created, &b1)
	q.Enqueue(kvstore.Created, &b2)
	q.Enqueue(kvstore.Created, &b3)
	q.Enqueue(kvstore.Created, &b4)

	if q.watcherList.Len() != 4 {
		t.Errorf("incorrect number of watchers")
	}
	AssertEventually(t, func() (bool, interface{}) {
		t.Logf("events is %d", rcvdEvents)
		return rcvdEvents == 12 && slowrcvd == 3, nil
	}, "expecting 1 events", "10ms", "100ms")
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
	AssertEventually(t, func() (bool, interface{}) {
		t.Logf("Lengths are  %d/%d", q.watcherList.Len(), q.eventList.Len())
		return q.watcherList.Len() == 0 && q.eventList.Len() == 0, nil
	}, "expecting 1 events", "10ms", "10s")

	t.Logf(" --> Test Eviction")
	wconfig.EvictInterval = time.Nanosecond
	wp.watchConfig = wconfig
	q3 := wp.Add("/testpath2/three").(*watchEventQ)
	nctx, cancel = context.WithCancel(ctx)
	slowrcvd = 0
	slowBlockCount = 2
	watchersCount = q3.watcherList.Len()
	go q3.Dequeue(nctx, 0, slowcb, nil)
	AssertEventually(t, func() (bool, interface{}) {
		return q3.watcherList.Len() == watchersCount+1, nil
	}, "expecting new watcher", "10ms", "100ms")
	b5.ResourceVersion = "120"
	q3.Enqueue(kvstore.Created, &b4)
	q3.Enqueue(kvstore.Created, &b5)
	AssertEventually(t, func() (bool, interface{}) {
		t.Logf("slow rcvd are  %d", slowrcvd)
		return slowrcvd == 2, nil
	}, "expecting to hit 2 slowrcvd", "10ms", "1000ms")
	q3.janitorFn()
	if q3.stats.clientEvictions.Value() != int64(1) {
		t.Errorf("expecting 1 eviction")
	}
	AssertEventually(t, func() (bool, interface{}) {
		return q3.watcherList.Len() == 0, nil
	}, "expecting watcher to exit", "10ms", "1000ms")
	q3.Stop()

	// Test timebased janitor cleanup
	t.Logf(" --> Test timed janitor cleanup")
	q4 := wp.Add("/testpath2/four").(*watchEventQ)
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
	cleanupCalled := 0
	cleanfunc := func() {
		cleanupCalled++
	}
	go q4.Dequeue(nctx, 0, cbfunc, cleanfunc)
	AssertEventually(t, func() (bool, interface{}) {
		return q4.watcherList.Len() == 1, nil
	}, "expecting to hit 1 watcher", "10ms", "1000ms")
	b3.ResourceVersion = "121"
	b4.ResourceVersion = "122"
	b5.ResourceVersion = "123"
	q4.Enqueue(kvstore.Created, &b3)
	q4.Enqueue(kvstore.Created, &b4)
	q4.Enqueue(kvstore.Created, &b5)
	AssertEventually(t, func() (bool, interface{}) {
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
	q4.Enqueue(kvstore.Created, &b3)
	q4.Enqueue(kvstore.Created, &b4)
	q4.Enqueue(kvstore.Created, &b5)
	AssertEventually(t, func() (bool, interface{}) {
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
	AssertEventually(t, func() (bool, interface{}) {

		return cleanupCalled == 1, nil
	}, "expecting to hit 3 dequeuesd", "10ms", "1000ms")
}
