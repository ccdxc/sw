// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cache

import (
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
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

// Statemgr is the object state manager
type Statemgr struct {
	memDB *memdb.Memdb // database of all objects

	// Implement APIClientGetter interface for ApiServer access
	clientGetter APIClientGetter

	// hostnameToSmartNICMap is a cache of known SmartNIC objects indexed by hostname
	hostnameToSmartNICMap     map[string]*cluster.SmartNIC
	hostnameToSmartNICMapLock sync.RWMutex
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
	return sm.memDB.ListObjects(kind)
}

// WatchObjects watches network state for changes
func (sm *Statemgr) WatchObjects(kind string, watchChan chan memdb.Event) error {
	// just add the channel to the list of watchers
	return sm.memDB.WatchObjects(kind, watchChan)
}

// StopWatchObjects Stops watches of network state
func (sm *Statemgr) StopWatchObjects(kind string, watchChan chan memdb.Event) error {
	// just remove the channel from the list of watchers
	return sm.memDB.StopWatchObjects(kind, watchChan)
}

// APIClient returns an APIServer client for the Cluster group
func (sm *Statemgr) APIClient() cluster.ClusterV1Interface {
	return sm.clientGetter.APIClient()
}

// NewStatemgr creates a new state manager object
func NewStatemgr(clientGetter APIClientGetter) *Statemgr {

	// create new statemgr instance
	statemgr := &Statemgr{
		memDB:                 memdb.NewMemdb(),
		clientGetter:          clientGetter,
		hostnameToSmartNICMap: make(map[string]*cluster.SmartNIC),
	}

	return statemgr
}
