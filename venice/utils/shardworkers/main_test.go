package shardworkers

import (
	"context"
	"strconv"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

type myWorkCtx struct {
	done bool
	key  string
}

func (w myWorkCtx) GetKey() string {
	return w.key
}

func (w myWorkCtx) GetKind() string {
	return "test"
}

func (w myWorkCtx) SetEvent(kvstore.WatchEventType) {
}

func (w myWorkCtx) SetNewObj(oldObj interface{}) {
}

func (w *myWorkCtx) WorkFunc(ctx context.Context) error {
	w.done = true
	return nil
}

func (w myWorkCtx) RuntimeObject() runtime.Object {
	return nil
}

func (w myWorkCtx) Lock() {
}

func (w myWorkCtx) Unlock() {
}

func TestWorkerStartStop(t *testing.T) {

	wp := NewWorkerPool(3)

	TestUtils.Assert(t, !wp.Running(), "Worker pool not running")
	wp.Start()
	TestUtils.Assert(t, wp.Running(), "Worker pool not running")

	myCtx := &myWorkCtx{key: "0"}
	wp.RunJob(myCtx)

	time.Sleep(100 * time.Millisecond)
	TestUtils.Assert(t, myCtx.done, "Work is done")

	cnt, err := wp.GetCompletedJobsCount(0)
	TestUtils.Assert(t, cnt == 1 && err == nil, "Jobs completed match on queue 0")

	cnt, err = wp.GetCompletedJobsCount(1)
	TestUtils.Assert(t, cnt == 0 && err == nil, "Jobs completed match on queue 1")

	cnt, err = wp.GetCompletedJobsCount(2)
	TestUtils.Assert(t, cnt == 0 && err == nil, "Jobs completed match on queue 2")

	myCtx = &myWorkCtx{key: "1"}
	wp.RunJob(myCtx)
	time.Sleep(100 * time.Millisecond)
	cnt, err = wp.GetCompletedJobsCount(1)
	TestUtils.Assert(t, cnt == 1 && err == nil, "Jobs completed match on queue 1")

	wp.Stop()
	TestUtils.Assert(t, !wp.Running(), "Worker pool not running")

}

func TestWorkerPeriodicJob(t *testing.T) {

	wp := NewWorkerPool(30)

	TestUtils.Assert(t, !wp.Running(), "Worker pool not running")
	wp.Start()
	TestUtils.Assert(t, wp.Running(), "Worker pool not running")

	for i := 0; i < 500; i++ {
		myCtx := &myWorkCtx{key: strconv.Itoa(i)}
		wp.RunJob(myCtx)
	}

	time.Sleep(2 * time.Second)

	total := 0
	for i := 0; i < 30; i++ {
		cnt, err := wp.GetCompletedJobsCount((uint32)(i))
		total += int(cnt)
		TestUtils.Assert(t, err == nil, "Got completed jobs")
	}

	TestUtils.Assert(t, total == 500, "Completed jobs matched")

	wp.Stop()
	TestUtils.Assert(t, !wp.Running(), "Worker pool not running")

}

func TestWorkerStartStopWithWorkFunc(t *testing.T) {

	wp := NewWorkerPool(3)

	TestUtils.Assert(t, !wp.Running(), "Worker pool not running")
	wp.Start()
	TestUtils.Assert(t, wp.Running(), "Worker pool not running")

	myWork := func(ctx context.Context, userCtx WorkObj) error {
		mywork := userCtx.(*myWorkCtx)
		mywork.done = true
		return nil
	}

	myCtx := &myWorkCtx{key: "0"}
	wp.RunFunction(myCtx, myWork)

	time.Sleep(100 * time.Millisecond)
	TestUtils.Assert(t, myCtx.done, "Work is done")

	cnt, err := wp.GetCompletedJobsCount(0)
	TestUtils.Assert(t, cnt == 1 && err == nil, "Jobs completed match on queue 0")

	cnt, err = wp.GetCompletedJobsCount(1)
	TestUtils.Assert(t, cnt == 0 && err == nil, "Jobs completed match on queue 1")

	cnt, err = wp.GetCompletedJobsCount(2)
	TestUtils.Assert(t, cnt == 0 && err == nil, "Jobs completed match on queue 2")

	myCtx = &myWorkCtx{key: "1"}
	wp.RunFunction(myCtx, myWork)
	time.Sleep(100 * time.Millisecond)
	cnt, err = wp.GetCompletedJobsCount(1)
	TestUtils.Assert(t, cnt == 1 && err == nil, "Jobs completed match on queue 1")

	wp.Stop()
	TestUtils.Assert(t, !wp.Running(), "Worker pool not running")

}
