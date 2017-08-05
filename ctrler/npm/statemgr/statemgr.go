// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"github.com/pensando/sw/api"

	"github.com/pensando/sw/utils/memdb"
)

// Statemgr is the object state manager
type Statemgr struct {
	memDB *memdb.Memdb // database of all objects
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

// NewStatemgr creates a new state manager object
func NewStatemgr() (*Statemgr, error) {
	// create new statemgr instance
	statemgr := &Statemgr{
		memDB: memdb.NewMemdb(),
	}

	return statemgr, nil
}
