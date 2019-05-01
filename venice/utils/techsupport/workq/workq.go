package workq

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/utils/log"
)

// WorkQ is the TechSupportRequest queue
type WorkQ struct {
	lock    sync.RWMutex
	Q       []*tsproto.TechSupportRequest
	max     int
	cur     int
	tarball string
}

// NewWorkQ creates new TechsupportRequest queue
func NewWorkQ(maxLen int) *WorkQ {
	if maxLen <= 0 {
		log.Errorf("Invalid queue size passed : %v", maxLen)
		return nil
	}

	return &WorkQ{
		Q:   []*tsproto.TechSupportRequest{},
		max: maxLen,
		cur: 0,
	}
}

// Put adds one techsupport request in the pending queue
func (q *WorkQ) Put(work *tsproto.TechSupportRequest) error {
	if q == nil {
		log.Error("Queue is nil. Cannot perform a put.")
		return fmt.Errorf("queue is nil")
	}

	q.lock.Lock()
	defer q.lock.Unlock()

	log.Infof("WorkQ PUT : %v", work)
	if q.max == q.cur {
		log.Errorf("Queue Full. Cannot add more work requests.")
		return fmt.Errorf("queue full")
	}

	q.Q = append(q.Q, work)

	q.cur++
	return nil
}

// Get gets the next available techsupport request from pending queue
func (q *WorkQ) Get() *tsproto.TechSupportRequest {
	if q == nil {
		log.Error("Queue is nil. Cannot perform a get.")
		return nil
	}

	q.lock.Lock()
	defer q.lock.Unlock()

	log.Infof("WorkQ Get")
	if q.cur > 0 {
		work := q.Q[0]
		q.Q = q.Q[1:len(q.Q)]
		q.cur--
		return work
	}

	log.Errorf("Work Q is empty.")
	return nil
}

// IsEmpty checks if the current queue is empty or not
func (q *WorkQ) IsEmpty() bool {
	return q.cur == 0
}
