// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// HostState is a wrapper for host object
type HostState struct {
	Host     *ctkit.Host `json:"-"` // host object
	stateMgr *Statemgr   // pointer to state manager
}

// HostStateFromObj conerts from memdb object to host state
func HostStateFromObj(obj runtime.Object) (*HostState, error) {
	switch obj.(type) {
	case *ctkit.Host:
		hobj := obj.(*ctkit.Host)
		switch hobj.HandlerCtx.(type) {
		case *HostState:
			nsobj := hobj.HandlerCtx.(*HostState)
			return nsobj, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewHostState creates new host state object
func NewHostState(host *ctkit.Host, stateMgr *Statemgr) (*HostState, error) {
	hs := &HostState{
		Host:     host,
		stateMgr: stateMgr,
	}
	host.HandlerCtx = hs

	return hs, nil
}

// OnHostCreate handles host creation
func (sm *Statemgr) OnHostCreate(host *ctkit.Host) error {
	// see if we already have the host
	hs, err := sm.FindHost(host.Tenant, host.Name)
	if err == nil {
		hs.Host = host
		return nil
	}

	log.Infof("Creating host: %+v", host)

	// create new host object
	hs, err = NewHostState(host, sm)
	if err != nil {
		log.Errorf("Error creating host %+v. Err: %v", host, err)
		return err
	}

	return nil
}

// OnHostUpdate handles host object update
func (sm *Statemgr) OnHostUpdate(host *ctkit.Host) error {
	return nil
}

// OnHostDelete handles host deletion
func (sm *Statemgr) OnHostDelete(host *ctkit.Host) error {
	// nothing to do
	return nil
}

// FindHost finds a host
func (sm *Statemgr) FindHost(tenant, name string) (*HostState, error) {
	// find the object
	obj, err := sm.FindObject("Host", "", "", name)
	if err != nil {
		return nil, err
	}

	return HostStateFromObj(obj)
}
