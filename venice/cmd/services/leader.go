package services

import (
	"context"
	"fmt"
	"regexp"
	"sync"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
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

	evtObjRef := &cluster.Node{}
	evtObjRef.Defaults("all")
	evtObjRef.Name = l.id
	recorder.Event(eventtypes.ELECTION_STARTED, "Leader election started", evtObjRef)

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
				log.Infof("Election event channel closed. restarting election")
				// Cant trust being a leader anymore.
				l.Stop()
				go l.Start()
			}
		case <-ctx.Done():
			log.Infof("Leader election cancelled")
			evtObjRef := &cluster.Node{}
			evtObjRef.Defaults("all")
			evtObjRef.Name = l.id
			recorder.Event(eventtypes.ELECTION_CANCELLED, "Leader election cancelled", evtObjRef)
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
	if l.cancel != nil {
		l.cancel()
		l.election.WaitForStop()
		l.cancel = nil
		l.election = nil
	}

	evtObjRef := &cluster.Node{}
	evtObjRef.Defaults("all")
	evtObjRef.Name = l.id
	recorder.Event(eventtypes.ELECTION_STOPPED, "Leader election stopped", evtObjRef)
}

// processEvent handles leader election events.
func (l *leaderService) processEvent(leader string) {
	evtObjRef := &cluster.Node{}
	evtObjRef.Defaults("all")
	evtObjRef.Name = l.id

	if l.id == leader {
		if l.IsLeader() {
			// Already leader, nothing to do.
			return
		}
		e := types.LeaderEvent{Evt: types.LeaderEventWon, Leader: leader}
		err := l.notify(e)
		recorder.Event(eventtypes.LEADER_ELECTED, fmt.Sprintf("Node %s elected as the leader", leader), evtObjRef)
		if err != nil {
			log.Errorf("Failed to notify %v with error: %v", e, err)
			l.notify(types.LeaderEvent{Evt: types.LeaderEventLost, Leader: leader})
			l.stop()
			go l.Start()
			recorder.Event(eventtypes.ELECTION_NOTIFICATION_FAILED, "Failed to send leader election notification", evtObjRef)
			return
		}

	} else if l.IsLeader() {
		l.notify(types.LeaderEvent{Evt: types.LeaderEventLost, Leader: leader})
		recorder.Event(eventtypes.LEADER_LOST, fmt.Sprintf("Node %s lost leadership", l.leader), evtObjRef)
	} else if l.leader != leader {
		l.notify(types.LeaderEvent{Evt: types.LeaderEventChange, Leader: leader})
		recorder.Event(eventtypes.LEADER_CHANGED, fmt.Sprintf("Leader changed from %s to %s",
			regexp.MustCompile("^$").ReplaceAllString(l.leader, "<none>"),
			regexp.MustCompile("^$").ReplaceAllString(leader, "<none>")), evtObjRef)
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

func (l *leaderService) ID() string {
	return l.id
}
