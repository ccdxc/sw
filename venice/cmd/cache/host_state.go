// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cache

import (
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// HostState security policy state
type HostState struct {
	*sync.Mutex
	*cluster.Host           // host object
	stateMgr      *Statemgr // pointer to state manager
}

// HostStateFromObj converts from memdb object to Host state
func HostStateFromObj(obj memdb.Object) (*HostState, error) {
	switch obj.(type) {
	case *HostState:
		snobj := obj.(*HostState)
		return snobj, nil

	default:
		return nil, errors.New("Incorrect object type")
	}
}

// NewHostState creates a new security policy state object
func NewHostState(sn *cluster.Host, stateMgr *Statemgr) (*HostState, error) {
	// create host state object
	sns := HostState{
		Host:     sn,
		stateMgr: stateMgr,
		Mutex:    new(sync.Mutex),
	}
	return &sns, nil
}

// FindHost finds Host object by name
func (sm *Statemgr) FindHost(tenant, name string) (*HostState, error) {
	// find the object
	obj, err := sm.FindObject("Host", tenant, name)
	if err != nil {
		return nil, err
	}
	return HostStateFromObj(obj)
}

// ListHosts lists all Host objects
func (sm *Statemgr) ListHosts() ([]*HostState, error) {
	var sgs []*HostState
	objs := sm.memDB.ListObjects("Host")
	for _, obj := range objs {
		sg, err := HostStateFromObj(obj)
		if err != nil {
			return sgs, err
		}

		sgs = append(sgs, sg)
	}
	return sgs, nil
}

// CreateHost creates a Host object
func (sm *Statemgr) CreateHost(sn *cluster.Host) error {
	// see if we already have it
	esn, err := sm.FindObject("Host", sn.ObjectMeta.Tenant, sn.ObjectMeta.Name)
	if err == nil {
		// Object exists in cache, but we got a watcher event with event-type:Created
		// and this can happen if there is a watcher error/reset and we need to update
		// the cache to handle it gracefully as an Update.
		log.Infof("Objects exists, updating Host OldState: {%+v}. New state: {%+v}", esn, sn)
		return sm.UpdateHost(sn)
	}

	// create new Host state
	sns, err := NewHostState(sn, sm)
	if err != nil {
		log.Errorf("Error creating new Host state. Err: %v", err)
		return err
	}

	// store it in local DB
	err = sm.memDB.AddObject(sns)
	if err != nil {
		log.Errorf("Error storing the Host state in memdb. Err: %v", err)
		return err
	}

	log.Infof("Created Host state {%+v}", sns)
	return nil
}

// UpdateHost updates a Host object
func (sm *Statemgr) UpdateHost(sn *cluster.Host) error {
	// see if we already have it
	obj, err := sm.FindObject("Host", sn.ObjectMeta.Tenant, sn.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Can not find the Host %s|%s err: %v", sn.ObjectMeta.Tenant, sn.ObjectMeta.Name, err)
		return fmt.Errorf("Host not found")
	}

	sns, err := HostStateFromObj(obj)
	if err != nil {
		log.Errorf("Wrong object type in memdb! Expected Host, got %T", obj)
	}
	sns.Lock()
	defer sns.Unlock()
	sns.Host = sn

	// store it in local DB
	err = sm.memDB.UpdateObject(sns)
	if err != nil {
		log.Errorf("Error storing Host in memdb. Err: %v", err)
		return err
	}

	log.Debugf("Updated Host state {%+v}", sns)
	return nil
}

// DeleteHost deletes a Host state
func (sm *Statemgr) DeleteHost(tenant, name string) error {
	// see if we already have it
	sn, err := sm.FindObject("Host", tenant, name)
	if err != nil {
		log.Errorf("Can not find the Host %s|%s", tenant, name)
		return fmt.Errorf("Host not found")
	}

	// convert it to Host state
	sn, err = HostStateFromObj(sn)
	if err != nil {
		return err
	}

	meta := sn.GetObjectMeta()
	if meta != nil {
		log.Infof("Deleting Host state {%+v}", meta)
	}
	// delete it from the DB
	return sm.memDB.DeleteObject(sn)
}
