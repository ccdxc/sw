package services

import (
	"context"
	"sync"

	log "github.com/Sirupsen/logrus"

	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	leaderKey = "master"
	ttl       = 5
)

// leaderService contains the state for leader election service.
type leaderService struct {
	sync.Mutex
	id        string
	isLeader  bool
	virtualIP string
	store     kvstore.Interface
	election  kvstore.Election
	ipSvc     types.IPService
	sysSvc    types.SystemdService
	cancel    context.CancelFunc
}

// NewLeaderService creates a leader election service.
func NewLeaderService(store kvstore.Interface, ipSvc types.IPService, sysSvc types.SystemdService, id, virtualIP string) types.LeaderService {
	return &leaderService{
		id:        id,
		store:     store,
		ipSvc:     ipSvc,
		sysSvc:    sysSvc,
		virtualIP: virtualIP,
	}
}

// Start starts leader election on quorum nodes.
func (l *leaderService) Start() error {
	l.Lock()
	defer l.Unlock()
	return l.start()
}

// start is a helper function to start election.
func (l *leaderService) start() error {
	log.Infof("Starting leader election with id: %v", l.id)
	ctx, cancel := context.WithCancel(context.Background())
	l.cancel = cancel

	election, err := l.store.Contest(ctx, leaderKey, l.id, ttl)
	if err != nil {
		log.Errorf("Failed to start leader election with error: %v", err)
		return err
	}
	l.election = election
	go l.waitForEventsOrCancel(ctx)
	return nil
}

// waitForEventsOrCancel waits for events until ctx is canceled.
func (l *leaderService) waitForEventsOrCancel(ctx context.Context) {
	for {
		select {
		case e, ok := <-l.election.EventChan():
			if !ok {
				// Cant trust being a leader anymore.
				l.processEvent("")
				return
			}
			log.Infof("Election event: %+v", e)
			l.processEvent(e.Leader)
		case <-ctx.Done():
			log.Infof("Leader election cancelled")
			l.cancel = nil
			l.processEvent("")
			return
		}
	}
}

// Stop stops the leader election.
func (l *leaderService) Stop() {
	l.Lock()
	defer l.Unlock()
	l.stop()
}

// stop is a helper function to stop the leader election.
func (l *leaderService) stop() {
	if l.cancel != nil {
		l.cancel()
		l.cancel = nil
	}
}

// restartElection restarts the election, needs to be called in case of failures
// when starting master services. This will give another node a chance to become
// leader in case of persistent failures.
func (l *leaderService) restartElection() {
	l.stop()
	l.start()
}

// processEvent handles leader election events.
func (l *leaderService) processEvent(leader string) {
	l.Lock()
	defer l.Unlock()
	if l.id == leader {
		if l.isLeader {
			// Already leader, nothing to do.
			return
		}
		if err := l.startLeaderServices(); err != nil {
			log.Errorf("Failed to start leader services with error: %v", err)
			l.stopLeaderServices()
			l.restartElection()
			return
		}
		l.isLeader = true
	} else if l.isLeader {
		l.isLeader = false
		l.stopLeaderServices()
	}
}

// startLeaderServices configures Virtual IP and starts the leader services.
func (l *leaderService) startLeaderServices() error {
	if found, err := l.ipSvc.HasIP(l.virtualIP); err != nil {
		return err
	} else if !found {
		if err = l.ipSvc.AddSecondaryIP(l.virtualIP); err != nil {
			return err
		}
	}

	if err := l.sysSvc.StartLeaderServices(l.virtualIP); err != nil {
		return err
	}

	return nil
}

// stopLeaderServices stops the leader services and removes Virtual IP.
func (l *leaderService) stopLeaderServices() {
	l.sysSvc.StopLeaderServices()

	if err := l.ipSvc.DeleteIP(l.virtualIP); err != nil {
		log.Errorf("Failed to delete virtual IP with error: %v", err)
	}
}

// Leader returns the current leader.
func (l *leaderService) Leader() string {
	l.Lock()
	defer l.Unlock()
	if l.election != nil {
		return l.election.Leader()
	}
	return ""
}
