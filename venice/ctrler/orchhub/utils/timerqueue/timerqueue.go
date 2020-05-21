package timerqueue

import (
	"container/heap"
	"context"
	"sync"
	"time"
)

// Entry is the item on the queue
type Entry struct {
	workFn func() // function to run when the item's time is up
}

// Queue instance
type Queue struct {
	lock sync.RWMutex
	pq   *priorityQueue
	// Used to notify start routine that a job was added to the front of the pq
	addFrontCh chan bool
}

// NewQueue creates a new retry queue
func NewQueue() *Queue {
	pq := &priorityQueue{}
	heap.Init(pq)
	return &Queue{
		pq:         pq,
		addFrontCh: make(chan bool, 100),
	}
}

// Run starts the retry queue
func (q *Queue) Run(ctx context.Context) {
	for ctx.Err() == nil {
		q.lock.RLock()
		if q.pq.Len() == 0 {
			// Wait until an item comes
			q.lock.RUnlock()
			select {
			case <-ctx.Done():
				return
			case <-q.addFrontCh:
			}
			q.lock.RLock()
		}
		peekTime := (*q.pq)[0].time
		q.lock.RUnlock()
		select {
		case <-ctx.Done():
			return
		case <-q.addFrontCh:
			continue
		case <-time.After(time.Until(peekTime)):
			q.lock.Lock()
			item := heap.Pop(q.pq).(*pqItem)
			q.lock.Unlock()
			item.value.workFn()
		}
	}
}

// Add adds an item to the queue
func (q *Queue) Add(workFn func(), delay time.Duration) {
	q.lock.Lock()
	pqitem := &pqItem{
		value: Entry{
			workFn: workFn,
		},
		time: time.Now().Add(delay),
	}
	heap.Push(q.pq, pqitem)
	q.lock.Unlock()
	if pqitem.index == 0 {
		q.addFrontCh <- true
	}
}
