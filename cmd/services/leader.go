package services

import (
	"sync"

	log "github.com/Sirupsen/logrus"

	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	leaderKey = "master"
	ttl       = 5
)

// LeaderService is the interface for leader election service.
type LeaderService interface {
	Start() error
	Stop()
	Leader() string
}

// leaderService contains the state for leader election service.
type leaderService struct {
	sync.Mutex
	id       string
	election kvstore.Election
}

// NewLeaderService creates a leader election service.
func NewLeaderService(id string) LeaderService {
	return &leaderService{
		id: id,
	}
}

// Start starts leader election on quorum nodes.
// FIXME: Start services on the leader.
func (l *leaderService) Start() error {
	log.Infof("Starting leader election with id: %v", l.id)

	election, err := env.KVStore.Contest(leaderKey, l.id, ttl)
	if err != nil {
		log.Errorf("Failed to start leader election with error: %v", err)
		return err
	}
	l.election = election

	for {
		select {
		case e, ok := <-election.EventChan():
			if !ok {
				return nil
			}
			log.Infof("Election event: %+v", e)
			env.Leader = e.Leader
		}
	}
}

// Stop stops the leader election.
func (l *leaderService) Stop() {
	l.Lock()
	defer l.Unlock()
	if l.election != nil {
		l.election.Stop()
		l.election = nil
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
