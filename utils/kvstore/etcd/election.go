package etcd

import (
	"context"
	"fmt"
	"path"
	"sync"
	"time"

	"github.com/coreos/etcd/clientv3"
	"github.com/coreos/etcd/clientv3/concurrency"
	"github.com/pensando/sw/utils/log"

	"github.com/pensando/sw/utils/kvstore"
)

const (
	minTTL          = 5
	electionsPrefix = "/elections"
)

var (
	retryDelay = 100 * time.Millisecond
)

// election contains the context for running a leader election.
type election struct {
	sync.Mutex
	enabled  bool                        // Is the election enabled?
	store    *etcdStore                  // kv store
	name     string                      // name of the election
	id       string                      // identifier of the contender
	leader   string                      // winner of the election
	leaseID  clientv3.LeaseID            // lease id, if leader
	ttl      int                         // ttl for lease
	outCh    chan *kvstore.ElectionEvent // channel for election results
	session  *concurrency.Session        // Session corresponds to lease
	election *concurrency.Election       // Election structure
	ctx      context.Context
	cancel   context.CancelFunc
}

// newElection creates a new contender in an election.
func (e *etcdStore) newElection(ctx context.Context, name string, id string, ttl int) (*election, error) {
	if ttl < minTTL {
		return nil, fmt.Errorf("Invalid input: min ttl %v", ttl)
	}

	newCtx, cancel := context.WithTimeout(ctx, timeout)

	// Check if there is an existing lease.
	resp, err := e.client.KV.Get(newCtx, path.Join(electionsPrefix, name), clientv3.WithPrefix())
	cancel()
	if err != nil {
		return nil, err
	}

	leaseID := clientv3.NoLease
	for _, kv := range resp.Kvs {
		if string(kv.Value) == id {
			leaseID = clientv3.LeaseID(kv.Lease)
			log.Infof("Election(%v:%v): found existing lease %v", id, name, leaseID)
			break
		}
	}

	// Check if there is enough TTL left (minTTL/2). If we reclaim when TTL is
	// small, may lose the election soon after winning it, which will result
	// in an unnecessary flap.
	if leaseID != clientv3.NoLease {
		newCtx, cancel := context.WithTimeout(ctx, timeout)
		resp, err := e.client.TimeToLive(newCtx, leaseID)
		cancel()
		if err != nil {
			return nil, err
		}
		if resp.TTL < minTTL/2 {
			leaseID = clientv3.NoLease
		}
	}

	el := &election{
		enabled: true,
		store:   e,
		name:    name,
		id:      id,
		ttl:     ttl,
		outCh:   make(chan *kvstore.ElectionEvent, outCount),
	}

	go el.run(ctx, leaseID)

	return el, nil
}

// run starts the election and handles failures and restarts.
func (el *election) run(ctx context.Context, leaseID clientv3.LeaseID) {
	defer close(el.outCh)
	for {
		el.Lock()
		if !el.enabled {
			el.Unlock()
			return
		}

		// This code handles ctx.cancel() before creation of session.
		select {
		case <-ctx.Done():
			log.Infof("Election(%v:%v): canceled", el.id, el.name)
			el.Unlock()
			el.Stop()
			return
		default:
		}

		// Session code does not revoke the Lease on cancel(). Using a new context so that
		// we can call session.Close() on ctx.cancel().
		el.ctx, el.cancel = context.WithCancel(context.Background())

		opts := []concurrency.SessionOption{
			concurrency.WithTTL(el.ttl),
			concurrency.WithContext(el.ctx),
		}

		if leaseID != clientv3.NoLease {
			opts = append(opts, concurrency.WithLease(leaseID))
		}

		var err error
		el.session, err = concurrency.NewSession(el.store.client, opts...)
		if err != nil {
			log.Errorf("Election(%v:%v): failed to create session with error (retrying in 100ms): %v", el.id, el.name, err)
			el.cancel()
			el.Unlock()
			time.Sleep(time.Millisecond * 100)
			continue
		}

		el.leaseID = el.session.Lease()
		el.election = concurrency.NewElection(el.session, path.Join(electionsPrefix, el.name))

		el.Unlock()

		log.Infof("Election(%v:%v): starting campaign with lease %v", el.id, el.name, el.leaseID)

		errCh := make(chan struct{})
		doneCh := el.session.Done()
		obsCh := el.election.Observe(el.ctx)

		foundErr := false

		go el.attempt(errCh)

		for {
			select {
			case <-ctx.Done():
				// User called ctx.cancel().
				log.Infof("Election(%v:%v): canceled", el.id, el.name)
				// User invoked cancel on supplied context.
				el.Stop()
				return
			case <-errCh:
				// Campaign returned an error.
				foundErr = true
			case <-doneCh:
				// Session failed or was stopped.
				log.Errorf("Election(%v:%v): session done or failed", el.id, el.name)
				foundErr = true
			case resp, ok := <-obsCh:
				if !ok {
					log.Errorf("Election(%v:%v): observe with lease %v failed with error (restarting)", el.id, el.name, el.leaseID)
					foundErr = true
					el.Lock()
					if el.session != nil {
						el.session.Close()
						el.session = nil
					}
					el.Unlock()
					break
				}

				leader := string(resp.Kvs[0].Value)
				// If election is won with old lease, ignore it
				if leader == el.id && el.leaseID != clientv3.LeaseID(resp.Kvs[0].Lease) {
					log.Infof("Election(%v:%v): ignoring won election with old lease %v, current lease %v", el.id, el.name, resp.Kvs[0].Lease, el.leaseID)
					continue
				}

				el.Lock()
				wasLeader := el.leader == el.id
				el.leader = leader
				changed := el.leader != el.id
				el.Unlock()

				// If previously leader but lost election (unexpectedly), send a Lost event and start a new Campaign.
				if wasLeader && changed {
					el.sendEvent(kvstore.Lost, "")
					go el.attempt(errCh)
				}

				if changed {
					el.sendEvent(kvstore.Changed, leader)
				} else {
					el.sendEvent(kvstore.Elected, leader)
				}
			}
			if !el.enabled {
				// User invoked Stop on election. cleanup already happened in Stop.
				return
			}
			if foundErr {
				el.Lock()
				wasLeader := el.leader == el.id
				el.leader = ""
				el.Unlock()
				if wasLeader {
					el.sendEvent(kvstore.Lost, "")
				} else {
					el.sendEvent(kvstore.Changed, "")
				}
				break
			}
		}
		leaseID = clientv3.NoLease
	}
}

// attempt to become leader. Campaign returns on winning the election or on error.
func (el *election) attempt(errCh chan struct{}) {
	leaseID := el.leaseID
	err := el.election.Campaign(el.ctx, el.id)
	el.Lock()
	if !el.enabled {
		el.Unlock()
		return
	}
	el.Unlock()
	if err == nil {
		log.Infof("Election(%v:%v): won election with lease %v", el.id, el.name, leaseID)
	} else {
		log.Errorf("Election(%v:%v): campaign with lease %v failed with error: %v", el.id, el.name, leaseID, err)
		errCh <- struct{}{}
	}
}

// sendEvent sends out the event unless the election is stopped.
func (el *election) sendEvent(evType kvstore.ElectionEventType, leader string) {
	e := &kvstore.ElectionEvent{
		Type:   evType,
		Leader: leader,
	}

	log.Infof("Election(%v:%v): sending event %+v", el.id, el.name, e)

	if len(el.outCh) == outCount {
		log.Warnf("Election(%v:%v): number of buffered events hit max count of %v", el.id, el.name, outCount)
	}

	select {
	case el.outCh <- e:
	case <-el.ctx.Done():
	}
}

// EventChan returns the channel for election events.
func (el *election) EventChan() <-chan *kvstore.ElectionEvent {
	return el.outCh
}

// Stop stops the leader election.
func (el *election) Stop() {
	el.Lock()
	defer el.Unlock()
	log.Infof("Election(%v:%v): stop called", el.id, el.name)
	if el.session != nil {
		el.session.Close()
	}
	if el.cancel != nil {
		el.cancel()
	}
	el.session = nil
	el.cancel = nil
	el.enabled = false
}

// ID returns the id of this contender.
func (el *election) ID() string {
	return el.id
}

// Leader returns the current leader.
func (el *election) Leader() string {
	el.Lock()
	defer el.Unlock()
	return el.leader
}

// IsLeader returns if this contender is the leader.
func (el *election) IsLeader() bool {
	el.Lock()
	defer el.Unlock()
	return el.leader == el.id
}
