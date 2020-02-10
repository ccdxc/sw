// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cache

import (
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/memdb"
)

var (
	// parameters for pushing updates to ApiServer
	maxAPIServerWriteRetries = 10
	apiServerRPCTimeout      = 10 * time.Second
	apiClientRetryInterval   = 3 * time.Second
)

// APIClientGetter is an interface that returns an API Client.
type APIClientGetter interface {
	APIClient() cluster.ClusterV1Interface
}

// LeaderServiceGetter is an interface that returns a getter for leader services
type LeaderServiceGetter func() types.LeaderService

// Statemgr is the object state manager
type Statemgr struct {
	memDB *memdb.Memdb // database of all objects

	// Implement APIClientGetter interface for ApiServer access
	clientGetter APIClientGetter

	// Implement LeaderServiceGetter
	leaderService LeaderServiceGetter

	// hostnameToSmartNICMap is a cache of known DistributedServiceCard objects indexed by hostname
	hostnameToSmartNICMap     map[string]*cluster.DistributedServiceCard
	hostnameToSmartNICMapLock sync.RWMutex

	// channel to stop spawned goroutines
	done chan bool
}

// FindObject looks up an object in local db
func (sm *Statemgr) FindObject(kind, tenant, name string) (memdb.Object, error) {

	// form network key
	ometa := api.ObjectMeta{
		Tenant: tenant,
		Name:   name,
	}

	// find it in db
	return sm.memDB.FindObject(kind, &ometa)
}

// ListObjects list all objects of a kind
func (sm *Statemgr) ListObjects(kind string) []memdb.Object {
	return sm.memDB.ListObjects(kind, nil)
}

// WatchObjects watches network state for changes
func (sm *Statemgr) WatchObjects(kind string, watcher *memdb.Watcher) error {
	// just add the channel to the list of watchers
	return sm.memDB.WatchObjects(kind, watcher)
}

// StopWatchObjects Stops watches of network state
func (sm *Statemgr) StopWatchObjects(kind string, watcher *memdb.Watcher) error {
	// just remove the channel from the list of watchers
	return sm.memDB.StopWatchObjects(kind, watcher)
}

// APIClient returns an APIServer client for the Cluster group
func (sm *Statemgr) APIClient() cluster.ClusterV1Interface {
	return sm.clientGetter.APIClient()
}

func (sm *Statemgr) isLeader() bool {
	return sm.leaderService() != nil && sm.leaderService().IsLeader()
}

// NewStatemgr creates a new state manager object
func NewStatemgr(clientGetter APIClientGetter, leaderSvc LeaderServiceGetter) *Statemgr {

	// create new statemgr instance
	statemgr := &Statemgr{
		memDB:                 memdb.NewMemdb(),
		clientGetter:          clientGetter,
		leaderService:         leaderSvc,
		hostnameToSmartNICMap: make(map[string]*cluster.DistributedServiceCard),
		done:                  make(chan bool),
	}

	// Host objects are not updated periodically, so to make sure that updates
	// are eventually pushed to ApiServer, we need to track dirty objects and
	// re-trigger updates from a dedicated goroutine
	go statemgr.retryDirtyHosts(statemgr.done)
	// SmartNIC objects are updated periodically, but only if the DSC is sending
	// heartbeat so we still need a background job to update
	go statemgr.retryDirtySmartNICs(statemgr.done)

	return statemgr
}
