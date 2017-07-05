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
	leader    string
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
	evCh := l.election.EventChan()
	for {
		select {
		case e, ok := <-evCh:
			if ok {
				log.Infof("Election event: %+v", e)
				l.Lock()
				l.processEvent(e.Leader)
				l.Unlock()
			} else {
				// Cant trust being a leader anymore.
				l.Stop()
				go l.start()
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
	if l.election != nil {
		l.election.Stop()
		l.election = nil
	}
	l.leader = ""
	if l.cancel != nil {
		l.cancel()
		l.cancel = nil
	}
}

// processEvent handles leader election events.
func (l *leaderService) processEvent(leader string) {
	l.leader = leader
	if l.id == leader {
		if l.isLeader {
			// Already leader, nothing to do.
			return
		}
		if err := l.startLeaderServices(); err != nil {
			log.Errorf("Failed to start leader services with error: %v", err)
			l.stopLeaderServices()
			l.stop()
			go l.start()
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
	return l.leader
}
