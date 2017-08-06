package services

import (
	"context"
	"sync"

	"github.com/pensando/sw/utils/log"

	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	ttl = 5
)

// leaderService contains the state for leader election service.
type leaderService struct {
	sync.Mutex
	leaderKey string
	id        string
	leader    string
	stopped   bool
	store     kvstore.Interface
	election  kvstore.Election
	cancel    context.CancelFunc
	observers []types.LeadershipObserver
}

// NewLeaderService creates a leader election service.
func NewLeaderService(store kvstore.Interface, leaderKey, id string) types.LeaderService {
	return &leaderService{
		id:        id,
		store:     store,
		leaderKey: leaderKey,
	}
}

// Start starts leader election on quorum nodes.
func (l *leaderService) Start() error {
	l.Lock()
	defer l.Unlock()
	return l.start()
}

func (l *leaderService) Register(o types.LeadershipObserver) {
	l.Lock()
	defer l.Unlock()
	l.observers = append(l.observers, o)
}

func (l *leaderService) UnRegister(o types.LeadershipObserver) {
	l.Lock()
	defer l.Unlock()
	var i int
	for i = range l.observers {
		if l.observers[i] == o {
			break
		}
	}
	l.observers = append(l.observers[:i], l.observers[i+1:]...)
}

// return first encountered err of the observers.
// All the observers are notified of the event even if someone fails
func (l *leaderService) notify(e types.LeaderEvent) error {
	var err error
	for _, o := range l.observers {
		log.Infof("Calling observer: %+v  with leaderevent: %v", o, e)
		er := o.OnNotifyLeaderEvent(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

// start is a helper function to start election.
func (l *leaderService) start() error {
	if l.election != nil {
		return nil
	}
	log.Infof("Starting leader election with id: %v", l.id)
	ctx, cancel := context.WithCancel(context.Background())
	l.cancel = cancel

	election, err := l.store.Contest(ctx, l.leaderKey, l.id, ttl)
	if err != nil {
		log.Errorf("Failed to start leader election with error: %v", err)
		return err
	}
	l.election = election
	l.stopped = false
	go l.waitForEventsOrCancel(ctx)
	return nil
}

// waitForEventsOrCancel waits for events until ctx is canceled.
func (l *leaderService) waitForEventsOrCancel(ctx context.Context) {
	evCh := l.election.EventChan()
	for {
		select {
		case e, ok := <-evCh:
			if l.stopped {
				return
			} else if ok {
				log.Infof("Election event: %+v", e)
				l.Lock()
				l.processEvent(e.Leader)
				l.Unlock()
			} else {
				// Cant trust being a leader anymore.
				l.Stop()
				go l.Start()
			}
		case <-ctx.Done():
			log.Infof("Leader election cancelled")
			return
		}
	}
}

// Stop stops the leader election.
func (l *leaderService) Stop() {
	l.Lock()
	defer l.Unlock()
	l.stop()
	l.processEvent("")
}

// stop is a helper function to stop the leader election.
func (l *leaderService) stop() {
	l.stopped = true
	if l.election != nil {
		l.election.Stop()
		l.election = nil
	}
	if l.cancel != nil {
		l.cancel()
		l.cancel = nil
	}
}

// processEvent handles leader election events.
func (l *leaderService) processEvent(leader string) {
	if l.id == leader {
		if l.IsLeader() {
			// Already leader, nothing to do.
			return
		}
		e := types.LeaderEvent{Evt: types.LeaderEventWon, Leader: leader}
		if err := l.notify(e); err != nil {
			log.Errorf("Failed to notify %v with error: %v", e, err)
			l.notify(types.LeaderEvent{Evt: types.LeaderEventLost, Leader: leader})
			l.stop()
			go l.Start()
			return
		}

	} else if l.IsLeader() {
		l.notify(types.LeaderEvent{Evt: types.LeaderEventLost, Leader: leader})
	} else if l.leader != leader {
		l.notify(types.LeaderEvent{Evt: types.LeaderEventChange, Leader: leader})
	}
	log.Infof("Setting leader to %v", leader)
	l.leader = leader
}

// IsLeader checks if this instance is the leader.
func (l *leaderService) IsLeader() bool {
	return l.id == l.leader
}

// Leader returns the current leader.
func (l *leaderService) Leader() string {
	l.Lock()
	defer l.Unlock()
	return l.leader
}
