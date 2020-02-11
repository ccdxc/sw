// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package meta

import (
	"context"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// this file contains code specific to node state

// Node is a node instance
type Node struct {
	sync.Mutex                                // lock for the watcher state
	waitGrp      sync.WaitGroup               // wait group to wait on all go routines to exit
	nodeUUID     string                       // leader instance identifier
	nodeURL      string                       // node URL
	leaderCancel context.CancelFunc           // leader thread cancel
	elecCancel   context.CancelFunc           // election thread cancel
	kvsConfig    *store.Config                // kvstore config
	kvs          kvstore.Interface            // kvstore client
	mdmgr        map[ClusterType]*MetadataMgr // metadata mgr
	isLeader     bool                         // is this the leader?
	isStopped    bool                         // is leader stopped?
	clusterCfg   *ClusterConfig               // cluster config
}

// NewNode returns a new leader instance
func NewNode(cfg *ClusterConfig, nodeUUID, nodeURL string) (*Node, error) {
	// kvstore schema
	s := runtime.NewScheme()
	s.AddKnownTypes(&TscaleCluster{}, &TscaleNodeInfo{})

	// kvstore config
	config := store.Config{
		Type:        cfg.MetastoreType,
		Credentials: cfg.MetaStoreTLSConfig,
		Codec:       runtime.NewJSONCodec(s),
	}

	ctx, cancel := context.WithCancel(context.Background())

	// create node instance
	l := Node{
		kvsConfig:  &config,
		elecCancel: cancel,
		nodeUUID:   nodeUUID,
		nodeURL:    nodeURL,
		clusterCfg: cfg,
		mdmgr:      make(map[ClusterType]*MetadataMgr),
	}

	// run leader thread
	l.waitGrp.Add(1)
	go l.runElectionLoop(ctx)

	return &l, nil
}

// IsLeader returns true if current node is the leader
func (l *Node) IsLeader() bool {
	return l.isLeader
}

// Stop stops the leader thread
func (l *Node) Stop() error {
	l.isLeader = false
	l.isStopped = true
	l.elecCancel()
	if l.leaderCancel != nil {
		l.leaderCancel()
	}

	// wait for all goroutines to exit
	l.waitGrp.Wait()

	// stop any metadata mgr if its still running
	// Note that this step should be done after leader loop is stopped to make sure
	// we dont miss a metadata mgr that is being created by leader loop
	for k, md := range l.mdmgr {
		md.Stop()
		delete(l.mdmgr, k)
	}

	l.unregisterNode()

	// close kvstore client
	if l.kvs != nil {
		l.kvs.Close()
		l.kvs = nil
	}

	return nil
}

// unregisterNode unregisters the node from kvstore
func (l *Node) unregisterNode() error {
	kvstorePath := NodesMetastoreURL + l.nodeUUID
	log.Infof("Unregistering node: %s at Path %s", l.nodeUUID, kvstorePath)
	if l.kvs != nil {
		l.kvs.Delete(context.Background(), kvstorePath, nil)
	}

	return nil
}

// runElection runs the election thread
func (l *Node) runElection(ctx context.Context) {
	// create a kvstore client
	kvs, err := store.New(*l.kvsConfig)
	if err != nil {
		log.Errorf("Error connecting to kvstore %+v. Err: %v", l.kvsConfig, err)
		return
	}
	l.kvs = kvs

	// node instance
	node := TscaleNodeInfo{
		TypeMeta:   api.TypeMeta{Kind: "TscaleNodeInfo"},
		ObjectMeta: api.ObjectMeta{Name: l.nodeUUID},
		NodeUUID:   l.nodeUUID,
		NodeURL:    l.nodeURL,
	}

	kvstorePath := NodesMetastoreURL + l.nodeUUID
	log.Infof("Registering node: %+v at Path %s", node, kvstorePath)

	// register the node
	leaseCh, err := kvs.Lease(context.Background(), kvstorePath, &node, l.clusterCfg.NodeTTL)
	if err != nil && !kvstore.IsKeyExistsError(err) {
		log.Errorf("Error registering node %s. Err: %v", l.nodeUUID, err)
		return
	}

	// start the leader contest
	elec, err := kvs.Contest(ctx, "citadel-meta-leader", l.nodeUUID, l.clusterCfg.NodeTTL)
	if err != nil {
		log.Errorf("Error starting leader election. Err: %v", err)
		return
	}

	// loop forever waiting on the event channel
	for {
		select {
		case <-ctx.Done():
			log.Infof("%s Stopping election loop", l.nodeUUID)
			return
		case leaseEvt, ok := <-leaseCh:
			if !ok {
				log.Infof("Lease channel closing for node lease. Retrying to acquire lease")
				leaseCh, err = kvs.Lease(context.Background(), kvstorePath, &node, l.clusterCfg.NodeTTL)
				if err != nil {
					log.Errorf("Error registering node %s. Err: %v", l.nodeUUID, err)
					return
				}
			} else {
				log.Infof("Got lease event: %+v", leaseEvt)
			}
		case evt, ok := <-elec.EventChan():
			if !ok {
				log.Errorf("Error reading from event channel.")
				return
			}

			log.Infof("%s Received leader election event: %+v isLeader: %v ", l.nodeUUID, evt, l.IsLeader())

			// run the leader FSM
			switch evt.Type {
			case kvstore.Elected:
				if !l.IsLeader() {
					l.becomeLeader()
				}
			case kvstore.Lost:
				if l.IsLeader() {
					l.unbecomeLeader()
				}
			case kvstore.Changed:
				// verify we didnt loose the leader state
				if l.IsLeader() {
					if evt.Leader != l.nodeUUID {
						l.unbecomeLeader()
					}
				}
			case kvstore.ElectionError:
				log.Errorf("Election error")
				return
			}
		}
	}
}

// runElectionLoop runs election in a loop
func (l *Node) runElectionLoop(ctx context.Context) {
	defer l.waitGrp.Done()
	// run election loop forever
	for {
		l.kvsConfig.Servers = GetMetastoreURLs(ctx, l.clusterCfg)
		l.runElection(ctx)

		// check if we are stopped
		if l.isStopped {
			log.Infof("%s Leader election is stopped", l.nodeUUID)
			return
		}
		// wait for second before running next loop
		time.Sleep(time.Second)
	}
}

// becomeLeader starts the leader loop
func (l *Node) becomeLeader() {
	// create a context for the leader
	ctx, cancel := context.WithCancel(context.Background())
	l.leaderCancel = cancel

	log.Infof("%s is becoming leader", l.nodeUUID)

	l.isLeader = true

	// start a prefix watcher
	nodeWatcher, err := l.kvs.PrefixWatch(ctx, NodesMetastoreURL, "")
	if err != nil {
		log.Errorf("Error watching kvstore. Err: %v", err)
		return
	}

	// create metadata managers if requested
	if l.clusterCfg.EnableTstoreMeta {
		// create a metadata mgr for tstore
		mdm, err := NewMetadataMgr(ClusterTypeTstore, l.nodeUUID, l.kvs, l.clusterCfg)
		if err != nil {
			log.Errorf("Error creating metadata mgr. Err: %v", err)
			// FIXME: release leadership
			return
		}

		l.mdmgr[ClusterTypeTstore] = mdm
	}
	if l.clusterCfg.EnableKstoreMeta {
		// create a metadata mgr for kstore
		mdm, err := NewMetadataMgr(ClusterTypeKstore, l.nodeUUID, l.kvs, l.clusterCfg)
		if err != nil {
			log.Errorf("Error creating metadata mgr. Err: %v", err)
			// FIXME: release leadership
			return
		}

		l.mdmgr[ClusterTypeKstore] = mdm
	}

	// run leader loop
	l.waitGrp.Add(1)
	go l.runLeader(ctx, nodeWatcher)
}

// unbecomeLeader stop being a leader
func (l *Node) unbecomeLeader() {
	l.isLeader = false
	if l.leaderCancel != nil {
		l.leaderCancel()
		l.leaderCancel = nil
	}
	for k, md := range l.mdmgr {
		md.Stop()
		delete(l.mdmgr, k)
	}

	log.Infof("%s lost leadership", l.nodeUUID)
}

// runLeader runs the main leader loop
func (l *Node) runLeader(ctx context.Context, nodeWatcher kvstore.Watcher) {
	defer l.waitGrp.Done()

	log.Infof("%s leader started watching node events", l.nodeUUID)

	for {
		select {
		case <-ctx.Done():
			log.Warnf("%s Stopping leader loop", l.nodeUUID)
			return
		case evt, ok := <-nodeWatcher.EventChan():
			if !ok {
				var err error

			innerLoop:
				for i := 0; i < l.clusterCfg.MetaStoreRetry; i++ {
					// return if node is stopped
					if l.isStopped || ctx.Err() != nil {
						return
					}

					log.Warnf("Node watcher channel closed. Retrying")
					time.Sleep(time.Second)

					// restart the prefix watcher
					nodeWatcher, err = l.kvs.PrefixWatch(ctx, NodesMetastoreURL, "")
					if err == nil {
						break innerLoop
					}

					log.Errorf("Error watching kvstore. Err: %v", err)
				}

				// continue to select loop
				continue
			}

			// return if node is stopped
			if l.isStopped {
				return
			}

			// if this node is not a leader, skip the event & wait for ctx.Done
			if !l.IsLeader() || l.mdmgr == nil {
				log.Infof("Non leader node is running leader loop: %+v, skip event: %+v", l, evt)
				continue
			}

			// handle event type
			switch evt.Type {
			case kvstore.WatcherError:
				log.Errorf("Got node watch error %+v. Retrying", evt)

			retryLoop:
				for i := 0; i < l.clusterCfg.MetaStoreRetry; i++ {
					var err error

					// return if node is stopped
					if l.isStopped || ctx.Err() != nil {
						return
					}

					log.Warnf("Node watcher channel closed. Retrying")
					time.Sleep(time.Second)

					// restart the prefix watcher
					nodeWatcher, err = l.kvs.PrefixWatch(ctx, NodesMetastoreURL, "")
					if err == nil {
						break retryLoop
					}

					log.Errorf("Error(attempt %d) watching kvstore. Err: %v", i+1, err)
				}

				continue
			case kvstore.Created:
				fallthrough
			case kvstore.Updated:
				node := evt.Object.(*TscaleNodeInfo)

				log.Infof("%s Got %s event for node: %+v", l.nodeUUID, evt.Type, node)

				for _, md := range l.mdmgr {
					md.addNode(node)
				}

			case kvstore.Deleted:
				node := evt.Object.(*TscaleNodeInfo)

				log.Infof("%s Got %s event for node: %+v.", l.nodeUUID, evt.Type, node)

				for _, md := range l.mdmgr {
					md.delNode(node)
				}
			}
		}
	}
}
