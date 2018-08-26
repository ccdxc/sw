// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"sync"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// HostReactor is the event reactor for host events
type HostReactor struct {
	stateMgr *Statemgr // state manager
}

// HostHandler host event handler interface
type HostHandler interface {
	CreateHost(host cluster.Host) error
	DeleteHost(host cluster.Host) error
}

// HostState is a wrapper for host object
type HostState struct {
	sync.Mutex             // lock the host object
	cluster.Host           // host object
	stateMgr     *Statemgr // pointer to state manager
}

// HostStateFromObj conerts from memdb object to host state
func HostStateFromObj(obj memdb.Object) (*HostState, error) {
	switch obj.(type) {
	case *HostState:
		nsobj := obj.(*HostState)
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewHostState creates new host state object
func NewHostState(host cluster.Host, stateMgr *Statemgr) (*HostState, error) {
	hs := &HostState{
		Host:     host,
		stateMgr: stateMgr,
	}

	// store it in local DB
	stateMgr.memDB.AddObject(hs)

	return hs, nil
}

// CreateHost handles host creation
func (hr *HostReactor) CreateHost(host cluster.Host) error {
	// see if we already have the host
	hs, err := hr.stateMgr.FindHost(host.Tenant, host.Name)
	if err == nil {
		hs.Host = host
		return nil
	}

	log.Infof("Creating host: %+v", host)

	// create new host object
	hs, err = NewHostState(host, hr.stateMgr)
	if err != nil {
		log.Errorf("Error creating host %+v. Err: %v", host, err)
		return err
	}

	return nil
}

// DeleteHost handles host deletion
func (hr *HostReactor) DeleteHost(host cluster.Host) error {
	// see if we have the host
	hs, err := hr.stateMgr.FindHost(host.Tenant, host.Name)
	if err != nil {
		log.Errorf("Could not find the host %v. Err: %v", host, err)
		return err
	}

	log.Infof("Deleting host: %+v", host)

	// delete the object
	return hr.stateMgr.memDB.DeleteObject(hs)
}

// NewHostReactor creates new host event reactor
func NewHostReactor(sm *Statemgr) (*HostReactor, error) {
	host := HostReactor{
		stateMgr: sm,
	}

	return &host, nil
}

// FindHost finds a host
func (sm *Statemgr) FindHost(tenant, name string) (*HostState, error) {
	// find the object
	obj, err := sm.FindObject("Host", tenant, name)
	if err != nil {
		return nil, err
	}

	return HostStateFromObj(obj)
}
