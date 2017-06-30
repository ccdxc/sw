package memkv

import (
	"context"
	"errors"
	"fmt"
	"math/rand"
	"sync"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	electionsPrefix = "/elections"
	minTTL          = 5
)

// election contains the context for running a leader election.
type election struct {
	sync.Mutex
	enabled bool                        // Is the election enabled?
	f       *MemKv                      // kv store
	name    string                      // name of the election
	id      string                      // identifier of the contender
	leader  string                      // winner of the election
	termID  int                         // termID for current election
	ttl     int                         // ttl for lease
	outCh   chan *kvstore.ElectionEvent // channel for election results
}

type memkvElection struct {
	leader     string
	termID     int
	contenders []*election
}

type memkvCluster struct {
	sync.Mutex
	elections map[string]*memkvElection // current elections
	clientID  int                       // current id of the store
	stores    map[string]*MemKv         // all client stores
}

// newElection creates a new contender in an election.
// memkv election is really a fake election because there is no real communication across
// multiple nodes, therefore:
// - there are no leases obtained and delays involved
// - some randomness is introduced when selecting a leader to mimic the nature
func (f *MemKv) newElection(ctx context.Context, name string, id string, ttl int) (*election, error) {
	if ttl < minTTL {
		return nil, fmt.Errorf("Invalid input: min ttl %v", ttl)
	}

	el := &election{
		enabled: true,
		f:       f,
		name:    name,
		id:      id,
		ttl:     ttl,
		outCh:   make(chan *kvstore.ElectionEvent, outCount),
	}

	// update cluster's contender's list
	f.Lock()

	c, ok := f.cluster.(*memkvCluster)
	if !ok {
		log.Fatalf("invalid cluster")
		f.Unlock()
		return nil, errors.New("invalid cluster")
	}

	c.Lock()
	if elec, ok := c.elections[name]; ok {
		elec.contenders = append(elec.contenders, el)
	} else {
		elec := &memkvElection{contenders: []*election{el}, leader: ""}
		c.elections[name] = elec
	}
	c.Unlock()
	f.Unlock()

	// run election, select leader
	go el.run(ctx)

	return el, nil
}

// run starts the election and handles failures and restarts
// this routine is run by multiple clients simultaneously to know about
// election win, lose or changes
func (el *election) run(ctx context.Context) {
	// sleep for random period to let arbitrary selection of winner before entering election
	time.Sleep((time.Duration)(rand.Intn(minTTL)) * time.Millisecond)

	// loop until disabled elect a leader
	for {
		el.Lock()

		// withdraw from the contest
		if !el.enabled {
			el.Unlock()
			return
		}

		// handle ctx.cancel()
		select {
		case <-ctx.Done():
			log.Infof("Election(%v:%v): canceled", el.id, el.name)
			el.stop()
			el.Unlock()
			return
		default:
		}

		f := el.f
		f.Lock()

		c, ok := f.cluster.(*memkvCluster)
		if !ok {
			log.Fatalf("invalid cluster")
			f.Unlock()
			el.Unlock()
			return
		}

		c.Lock()
		elec := c.elections[el.name]

		// pick a leader, upon leader going away or first time
		if elec.leader == "" {
			// pick a leader
			elec.leader = el.id
			elec.termID++

			// notify all contenders
			for _, contender := range elec.contenders {
				if contender.id == elec.leader {
					contender.sendEvent(kvstore.Elected, elec.leader)
				}
			}
		} else if elec.termID != el.termID {
			el.sendEvent(kvstore.Changed, elec.leader)
		}

		el.leader = elec.leader
		el.termID = elec.termID

		// TBD: no need to adjust ttl and auto-renew the lease for memkv

		f.Unlock()
		c.Unlock()
		el.Unlock()

		time.Sleep(10 * time.Millisecond)
	}
}

// sendEvent sends out the event unless the election is stopped.
func (el *election) sendEvent(evType kvstore.ElectionEventType, leader string) {
	e := &kvstore.ElectionEvent{
		Type:   evType,
		Leader: leader,
	}

	if len(el.outCh) == outCount {
		log.Warningf("Election(%v:%v): number of buffered events hit max count of %v", el.id, el.name, outCount)
	}

	el.outCh <- e
}

// EventChan returns the channel for election events.
func (el *election) EventChan() <-chan *kvstore.ElectionEvent {
	return el.outCh
}

// Stop stops the leader election.
func (el *election) Stop() {
	el.Lock()
	defer el.Unlock()
	el.stop()
}

// Helper to stop the election, called under lock.
func (el *election) stop() {
	el.enabled = false

	f := el.f
	f.Lock()
	defer f.Unlock()

	c, ok := f.cluster.(*memkvCluster)
	if !ok {
		log.Fatalf("invalid cluster")
		return
	}

	c.Lock()
	defer c.Unlock()

	elec := c.elections[el.name]

	// if you are the leader, mark leader to be undecided
	if elec.leader == el.id {
		elec.leader = ""
	}

	// remove election from contenders list
	for idx, contender := range elec.contenders {
		if contender == el {
			elec.contenders = append(elec.contenders[:idx], elec.contenders[idx+1:]...)
			break
		}
	}
}

// ID returns the id of this contender.
func (el *election) ID() string {
	return el.id
}

// Leader returns the current leader.
func (el *election) Leader() string {
	return el.leader
}

// IsLeader returns if this contender is the leader.
func (el *election) IsLeader() bool {
	return el.leader == el.id
}
