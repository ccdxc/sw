// Package workfarm provides a worker farm to perform a workload of repeated work, typically for performance testing.
// the actualy work performed is opaque to the package. Stats such are round-trip-times, failures, timeouts etc. are
// kept for the work done.
package workfarm

import (
	"context"
	"fmt"
	"sync"
	"sync/atomic"
	"time"

	rl "github.com/juju/ratelimit"

	"github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// RTTHistogram is name of histogram for round trip time
	RTTHistogram = "work-rtt"
)

// WorkFunc is the function performing the unit of work
type WorkFunc func(ctx context.Context, id, iter int, userCtx interface{}) error

// WorkerFarm creates a farm of workers performing the given work at the specified rate while keeping stats on the work done.
type WorkerFarm struct {
	sync.RWMutex
	doneWg  sync.WaitGroup
	startWg sync.WaitGroup
	workQ   chan int
	results chan *Stats
	count   int
	bucket  *rl.Bucket
	Timeout time.Duration
	fn      WorkFunc
	ctx     context.Context
	cancel  context.CancelFunc
	stats   Stats
	running bool
}

// Stats for the workfarm, returned as results.
type Stats struct {
	Result         error
	Iters          int64
	WorkerErrors   int64
	WorkerTimeouts int64
	Histogram      histogram.HistMap
	Duration       time.Duration
}

func (s *Stats) String() string {
	ret := fmt.Sprintf("Error                : %v\n", s.Result)
	ret = ret + fmt.Sprintf("Total Duration       : %v\n", s.Duration)
	ret = ret + fmt.Sprintf("Total Iterations     : %d\n", s.Iters)
	ret = ret + fmt.Sprintf("Error Iterations     : %d\n", s.WorkerErrors)
	ret = ret + fmt.Sprintf("Timeout Iterations   : %d\n", s.WorkerTimeouts)
	ret = ret + fmt.Sprintf("Histogram            :\n %s\n", s.Histogram.SprintOne(RTTHistogram))
	return ret
}

func (w *WorkerFarm) worker(ctx context.Context, id int, userCtx interface{}) {
	w.startWg.Done()
	defer w.doneWg.Done()
	reCh := make(chan error)
	var now time.Time
	runFn := func(nctx context.Context, iter int) {
		now = time.Now()
		reCh <- w.fn(nctx, id, iter, userCtx)
	}
	for {
		select {
		case iter, ok := <-w.workQ:
			if !ok {
				return
			}
			fctx, cancel := context.WithCancel(ctx)
			go runFn(fctx, iter)
			select {
			case err := <-reCh:
				if err != nil {
					atomic.AddInt64(&w.stats.WorkerErrors, 1)
				} else {
					elapsed := time.Since(now)
					w.stats.Histogram.Record(RTTHistogram, elapsed)
				}
			case <-time.After(w.Timeout):
				atomic.AddInt64(&w.stats.WorkerTimeouts, 1)
			}
			cancel()
			atomic.AddInt64(&w.stats.Iters, 1)
		case <-ctx.Done():
			return
		}
	}
}

func (w *WorkerFarm) activateWorkers(ctx context.Context, userCtx interface{}) error {
	for i := 0; i < w.count; i++ {
		w.startWg.Add(1)
		w.doneWg.Add(1)
		go w.worker(ctx, i, userCtx)
	}
	w.startWg.Wait()
	return nil
}

func (w *WorkerFarm) deactivateWorkers() {
	log.Infof("deactivating workers")
	w.cancel()
	w.doneWg.Wait()
}

func (w *WorkerFarm) producer(iters int, maxtime time.Duration) {
	startTime := time.Now()
	doneCh := make(chan bool)
	go func() {
		for i := 0; i < iters; i++ {
			if w.bucket != nil {
				w.bucket.Wait(1)
			}
			w.RLock()
			if !w.running {
				close(doneCh)
				w.RUnlock()
				return
			}
			w.RUnlock()
			w.workQ <- i
		}
		close(doneCh)
	}()
	timer := time.NewTimer(maxtime)
	select {
	case <-doneCh:
		log.Infof("Work completed")
		w.stats.Result = nil
	case <-w.ctx.Done():
		log.Errorf("Context was cancelled, exiting (%s)", w.ctx.Err())
		w.stats.Result = fmt.Errorf("Context was cancelled")
	case <-timer.C:
		log.Errorf("exceeded max time allowed")
		w.stats.Result = fmt.Errorf("exceed max time allowed")
	}
	elapsed := time.Since(startTime)
	w.stats.Duration = elapsed

	<-doneCh
	close(w.workQ)
	w.doneWg.Wait()
	w.Lock()
	w.running = false
	w.workQ = nil
	w.Unlock()
	w.deactivateWorkers()
	w.results <- &w.stats
}

// New creates a new workfarm
//  count  : number of workers
//  timeout: timeout for each unit of work
//  work   : unit of work.
func New(count int, timeout time.Duration, work WorkFunc) *WorkerFarm {
	return &WorkerFarm{
		count:   count,
		Timeout: timeout,
		fn:      work,
		workQ:   make(chan int, count),
		results: make(chan *Stats),
	}
}

// Run starts work on the farm.
//  iters      : number of iteration to run
//  rate       : iterations per second to attempt. 0 is max possible.
//  maxtime    : total allowed time for the complete run.
//  userCtx    : opaque context passed as a parameter to each unit of work.
func (w *WorkerFarm) Run(ctx context.Context, iters int, rate int, maxtime time.Duration, userCtx interface{}) (ret chan *Stats, err error) {
	w.Lock()
	if w.running {
		w.Unlock()
		return w.results, fmt.Errorf("already running")
	}
	w.running = true
	w.Unlock()

	defer func() {
		if err != nil {
			w.Lock()
			w.running = false
			w.workQ = make(chan int, w.count)
			w.Unlock()
		}
	}()
	w.ctx, w.cancel = context.WithCancel(ctx)
	err = w.activateWorkers(w.ctx, userCtx)
	if err != nil {
		log.Errorf("failed to the activate workers (%s)", err)
		return w.results, err
	}

	if rate > 0 {
		w.bucket = rl.NewBucketWithRate(float64(rate), int64(w.count))
	}
	go w.producer(iters, maxtime)
	return w.results, nil
}

// Stop stops a running farm
func (w *WorkerFarm) Stop() {
	w.Lock()
	if !w.running {
		w.Unlock()
		return
	}
	w.running = false
	w.Unlock()
	w.deactivateWorkers()
	w.results <- &w.stats
}
