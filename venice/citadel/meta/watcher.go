// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package meta

// this file implements the metadata watcher to watch cluster metadata

import (
	"context"
	"encoding/json"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Watcher watches the cluster state in kvstore
type Watcher struct {
	sync.Mutex                     // lock for the watcher state
	instID      string             // watcher instance identifier
	watchCancel context.CancelFunc // watch cancel
	kvsConfig   *store.Config      // kvstore config
	kvs         kvstore.Interface  // kvstore
	isStopped   bool               // is watcher stopped?
	cluster     map[string]*TscaleCluster
}

// NewWatcher creates a watcher
func NewWatcher(instID string, cfg *ClusterConfig) (*Watcher, error) {
	// kvstore schema
	s := runtime.NewScheme()
	s.AddKnownTypes(&TscaleCluster{}, &TscaleNodeInfo{})

	// kvstore config
	config := store.Config{
		Type:    cfg.MetastoreType,
		Servers: strings.Split(cfg.MetastoreURL, ","),
		Codec:   runtime.NewJSONCodec(s),
	}

	ctx, cancel := context.WithCancel(context.Background())

	// create watcher instance
	watcher := Watcher{
		instID:      instID,
		kvsConfig:   &config,
		watchCancel: cancel,
		cluster:     make(map[string]*TscaleCluster),
	}

	// start the watching thread
	go watcher.runWatcherLoop(ctx)

	return &watcher, nil
}

// GetCluster returns a copy of the cluster object
func (w *Watcher) GetCluster(clusterType string) *TscaleCluster {
	var cl TscaleCluster
	w.Lock()
	defer w.Unlock()
	wcl, ok := w.cluster[clusterType]
	if !ok {
		return &cl
	}
	cl.ShardMap = ref.DeepCopy(wcl.ShardMap).(*ShardMap)
	cl.NodeMap = ref.DeepCopy(wcl.NodeMap).(map[string]*NodeState)
	return &cl
}

// Stop stops the watcher
func (w *Watcher) Stop() error {
	w.isStopped = true
	w.watchCancel()
	if w.kvs != nil {
		w.kvs.Close()
		w.kvs = nil
	}

	return nil
}

// runWatcherLoop run watcher loop till stopped
func (w *Watcher) runWatcherLoop(ctx context.Context) {
	for {
		w.runWatcher(ctx)

		// check if watcher is stopped
		if w.isStopped {
			log.Infof("Stopping watcher thread")
			return
		}

		// wait for a second before running the loop again
		time.Sleep(time.Second)
	}
}

// runWatcher runs the watcher thread
func (w *Watcher) runWatcher(ctx context.Context) {
	// create a kvstore client
	kvs, err := store.New(*w.kvsConfig)
	if err != nil {
		log.Errorf("Error starting kvstore client. Err: %v", err)
		return
	}
	w.kvs = kvs

	// start a prefix watcher
	kvsWatcher, err := kvs.PrefixWatch(ctx, ClusterMetastoreURL, "")
	if err != nil {
		log.Errorf("Error watching kvstore. Err: %v", err)
		return
	}

	// read the current cluster state for tstore
	var tcl TscaleCluster
	err = kvs.Get(context.Background(), ClusterMetastoreURL+ClusterTypeTstore, &tcl)
	if err != nil && !kvstore.IsKeyNotFoundError(err) {
		log.Errorf("Error reading cluster state. Err: %v", err)
		return
	} else if kvstore.IsKeyNotFoundError(err) {
		log.Infof("Cluster key not found. Err: %v", err)
	}

	w.cluster[ClusterTypeTstore] = &tcl
	log.Infof("%s Got cluster state: %+v. Starting watch", ClusterTypeTstore, &tcl)

	// read current state of cluster for kstore
	var kcl TscaleCluster
	err = kvs.Get(context.Background(), ClusterMetastoreURL+ClusterTypeKstore, &kcl)
	if err != nil && !kvstore.IsKeyNotFoundError(err) {
		log.Errorf("Error reading cluster state. Err: %v", err)
		return
	} else if kvstore.IsKeyNotFoundError(err) {
		log.Infof("Cluster key not found. Err: %v", err)
	}

	w.cluster[ClusterTypeKstore] = &kcl
	log.Infof("%s Got cluster state: %+v. Starting watch", ClusterTypeKstore, &kcl)

	// loop till we get an error
	for {
		select {
		case evt, ok := <-kvsWatcher.EventChan():
			if !ok {
				log.Errorf("%s: Watcher channel closed exiting watcher", w.instID)
				return
			}

			switch evt.Type {
			case kvstore.WatcherError:
				log.Errorf("%s: Got watch error. closing watcher", w.instID)
				return
			case kvstore.Created:
				fallthrough
			case kvstore.Updated:
				cl := ref.DeepCopy(evt.Object).(*TscaleCluster)

				jstr, _ := json.Marshal(cl)
				log.Debugf("%s: Got %s event for cluster %s with: %s", w.instID, evt.Type, cl.Name, jstr)

				// save the cluster info
				w.Lock()
				w.cluster[cl.Name].ShardMap = cl.ShardMap
				w.cluster[cl.Name].NodeMap = cl.NodeMap
				w.Unlock()
			case kvstore.Deleted:
				// FIXME: handle deletes
			}
		}
	}
}
