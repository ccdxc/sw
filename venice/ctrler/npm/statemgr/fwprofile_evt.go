// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"sync"

	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// FirewallProfileReactor is the event reactor for fwProfile events
type FirewallProfileReactor struct {
	stateMgr *Statemgr // state manager
}

// FirewallProfileHandler fwProfile event handler interface
type FirewallProfileHandler interface {
	CreateFirewallProfile(fwProfile security.FirewallProfile) error
	DeleteFirewallProfile(fwProfile security.FirewallProfile) error
}

// FirewallProfileState is a wrapper for fwProfile object
type FirewallProfileState struct {
	sync.Mutex                         // lock the fwProfile object
	security.FirewallProfile           // fwProfile object
	stateMgr                 *Statemgr // pointer to state manager
}

// FirewallProfileStateFromObj conerts from memdb object to fwProfile state
func FirewallProfileStateFromObj(obj memdb.Object) (*FirewallProfileState, error) {
	switch obj.(type) {
	case *FirewallProfileState:
		nsobj := obj.(*FirewallProfileState)
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewFirewallProfileState creates new fwProfile state object
func NewFirewallProfileState(fwProfile security.FirewallProfile, stateMgr *Statemgr) (*FirewallProfileState, error) {
	hs := &FirewallProfileState{
		FirewallProfile: fwProfile,
		stateMgr:        stateMgr,
	}

	// store it in local DB
	stateMgr.memDB.AddObject(hs)

	return hs, nil
}

// CreateFirewallProfile handles fwProfile creation
func (hr *FirewallProfileReactor) CreateFirewallProfile(fwProfile security.FirewallProfile) error {
	// see if we already have the fwProfile
	hs, err := hr.stateMgr.FindFirewallProfile(fwProfile.Tenant, fwProfile.Name)
	if err == nil {
		hs.FirewallProfile = fwProfile
		hr.stateMgr.memDB.UpdateObject(hs)
		log.Infof("Updated fwProfile: %+v", fwProfile)
		return nil
	}

	log.Infof("Creating fwProfile: %+v", fwProfile)

	// create new fwProfile object
	hs, err = NewFirewallProfileState(fwProfile, hr.stateMgr)
	if err != nil {
		log.Errorf("Error creating fwProfile %+v. Err: %v", fwProfile, err)
		return err
	}

	return nil
}

// DeleteFirewallProfile handles fwProfile deletion
func (hr *FirewallProfileReactor) DeleteFirewallProfile(fwProfile security.FirewallProfile) error {
	// see if we have the fwProfile
	hs, err := hr.stateMgr.FindFirewallProfile(fwProfile.Tenant, fwProfile.Name)
	if err != nil {
		log.Errorf("Could not find the fwProfile %v. Err: %v", fwProfile, err)
		return err
	}

	log.Infof("Deleting fwProfile: %+v", fwProfile)

	// delete the object
	return hr.stateMgr.memDB.DeleteObject(hs)
}

// NewFirewallProfileReactor creates new fwProfile event reactor
func NewFirewallProfileReactor(sm *Statemgr) (*FirewallProfileReactor, error) {
	fwProfile := FirewallProfileReactor{
		stateMgr: sm,
	}

	return &fwProfile, nil
}

// FindFirewallProfile finds a fwProfile
func (sm *Statemgr) FindFirewallProfile(tenant, name string) (*FirewallProfileState, error) {
	// find the object
	obj, err := sm.FindObject("FirewallProfile", tenant, name)
	if err != nil {
		return nil, err
	}

	return FirewallProfileStateFromObj(obj)
}

// ListFirewallProfiles lists all apps
func (sm *Statemgr) ListFirewallProfiles() ([]*FirewallProfileState, error) {
	objs := sm.memDB.ListObjects("FirewallProfile")

	var fwps []*FirewallProfileState
	for _, obj := range objs {
		fwp, err := FirewallProfileStateFromObj(obj)
		if err != nil {
			return fwps, err
		}

		fwps = append(fwps, fwp)
	}

	return fwps, nil
}
