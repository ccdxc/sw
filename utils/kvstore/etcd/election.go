package etcd

import (
	"context"
	"fmt"
	"path"
	"sync"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/coreos/etcd/clientv3"

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
	enabled     bool                        // Is the election enabled?
	store       *etcdStore                  // kv store
	name        string                      // name of the election
	id          string                      // identifier of the contender
	leader      string                      // winner of the election
	leaseID     clientv3.LeaseID            // lease id, if leader
	ttl         uint64                      // ttl for lease
	modRevision int64                       // modified version (only for debugging)
	outCh       chan *kvstore.ElectionEvent // channel for election results
	ctx         context.Context
	cancel      context.CancelFunc
}

// newElection creates a new election interface for running an election.
func (e *etcdStore) newElection(name string, id string, ttl uint64) (*election, error) {
	if ttl < minTTL {
		return nil, fmt.Errorf("Invalid input: min ttl %v", ttl)
	}

	ctx, cancel := context.WithCancel(context.Background())
	el := &election{
		enabled: true,
		store:   e,
		name:    name,
		id:      id,
		ttl:     ttl,
		outCh:   make(chan *kvstore.ElectionEvent, outCount),
		ctx:     ctx,
		cancel:  cancel,
	}

	log.Infof("Election(%v:%v): starting election", id, name)

	go func() {
		for {
			el.Lock()
			if !el.enabled {
				el.Unlock()
				return
			}
			el.Unlock()

			// If start returns error, need to restart. When Stop is
			// called, start returns with no error and this routine
			// can exit.
			err := el.start()
			if err == nil {
				break
			}
			log.Errorf("Election(%v:%v): failed with error %v, retrying in %v", id, name, err, retryDelay)
			time.Sleep(retryDelay)
		}
	}()

	return el, nil
}

// attempt to become leader by atomically creating the key. On success, the
// lease is kept alive forever.
func (el *election) attempt(key string, version int64) error {

	// Check if this contender is the leader, this code handles process
	// restart when holding leadership.
	getResp, err := el.store.client.KV.Get(el.ctx, key)
	if err != nil {
		return err
	}

	if len(getResp.Kvs) != 0 {
		// Check if this contender is the leader.
		if string(getResp.Kvs[0].Value) == el.id {
			el.leaseID = clientv3.LeaseID(getResp.Kvs[0].Lease)
			log.Infof("Election(%v:%v): had won with lease id %v, restarting keepalive", el.id, el.name, el.leaseID)
			// Restart the keepalive.
			el.store.client.Lease.KeepAlive(el.ctx, el.leaseID)
			// Success event generation is handled by watcher in start().
			// Failure can be ignored, will result in an event on watcher
			// when lease expires.
		}
		return nil
	}

	leaseID, opts, err := el.store.ttlOpts(el.ctx, int64(el.ttl))
	if err != nil {
		return err
	}

	txnResp, err := el.store.client.KV.Txn(el.ctx).If(
		clientv3.Compare(clientv3.ModRevision(key), "=", version),
	).Then(
		clientv3.OpPut(key, el.id, opts...),
	).Commit()

	if txnResp.Succeeded {
		log.Infof("Election(%v:%v): won with lease id %v", el.id, el.name, leaseID)
		el.leaseID = leaseID
		// Start the keepalive.
		el.store.client.Lease.KeepAlive(el.ctx, leaseID)
		// Success event generation is handled by watcher in start().
		// Failure can be ignored, will result in an event on watcher
		// when lease expires.
	} else {
		// Revoke the lease if not successful in winning the election.
		el.store.client.Lease.Revoke(el.ctx, leaseID)
	}

	return err
}

// reset clears all state and stops the watch, refresh routines.
func (el *election) reset() {
	el.Lock()
	defer el.Unlock()
	if el.enabled {
		// Stop watch and refresh routines.
		el.cancel()
		el.ctx, el.cancel = context.WithCancel(context.Background())
		el.leader = ""
		el.modRevision = 0
		el.leaseID = clientv3.LeaseID(0)
	}
}

// start begins the leader election.
func (el *election) start() error {
	defer el.reset()

	key := path.Join(electionsPrefix, el.name)

	wc := el.store.client.Watch(el.ctx, key, clientv3.WithRev(0))

	// Start an attempt after setting up watch, and after every delete event.
	if err := el.attempt(key, 0); err != nil {
		return err
	}

	for wr := range wc {
		if wr.Err() != nil {
			return wr.Err()
		}
		for _, e := range wr.Events {
			if e.IsModify() && string(e.Kv.Value) == el.leader {
				// This is a ttl refresh of leader, ignore.
				// TODO: Is this event possible?
				continue
			}
			evType := kvstore.Changed
			value := string(e.Kv.Value)
			el.modRevision = e.Kv.ModRevision

			if e.IsModify() {
				el.sendEvent(evType, value)
			} else if e.IsCreate() {
				if value == el.id {
					// Won the election.
					evType = kvstore.Elected
				}
				el.sendEvent(evType, value)
			} else if e.Type == clientv3.EventTypeDelete {
				if e.PrevKv != nil && string(e.PrevKv.Value) == el.id {
					// Unexpected loss of leadership.
					log.Infof("Election(%v:%v): lost leadership, revoking lease %v", el.id, el.name, el.leaseID)
					el.reset()
					el.sendEvent(kvstore.Lost, "")
					el.store.client.Lease.Revoke(el.ctx, el.leaseID)
				} else {
					el.sendEvent(kvstore.Changed, "")
				}
				// attempt to become leader. If there is a subsequent
				// Create event in wr.Events, this attempt will fail.
				// Dont need to optimize for that.
				if err := el.attempt(key, 0); err != nil {
					return err
				}
			} else {
				return fmt.Errorf("Unexpected type %v", e.Type)
			}

		}
	}

	// Stop was called.
	if el.leader == el.id {
		// Revoke the lease. This will delete the key and give someone else a
		// chance to win the election right away. Need to use a new context as
		// the old one is cancelled.
		newCtx, cancel := context.WithTimeout(context.Background(), time.Second)

		log.Infof("Election(%v:%v): revoking lease %v", el.id, el.name, el.leaseID)
		_, err := el.store.client.Lease.Revoke(newCtx, el.leaseID)
		if err != nil {
			log.Errorf("Election(%v:%v): failed to revoke lease %v with error: %v", el.id, el.name, el.leaseID, err)
		}
		cancel()
	}
	return nil
}

// sendEvent sends out the event unless the election is stopped.
func (el *election) sendEvent(evType kvstore.ElectionEventType, leader string) {
	log.Infof("Election(%v:%v): setting leader to %v", el.id, el.name, leader)
	el.leader = leader

	e := &kvstore.ElectionEvent{
		Type:   evType,
		Leader: leader,
	}

	log.Infof("Election(%v:%v): sending event %+v", el.id, el.name, e)

	if len(el.outCh) == outCount {
		log.Warningf("Election(%v:%v): number of buffered events hit max count of %v", el.id, el.name, outCount)
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
	el.cancel()
	el.enabled = false
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
