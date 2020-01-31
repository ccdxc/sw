// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package cache

import (
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/venice/utils/featureflags"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// LicenseState security policy state
type LicenseState struct {
	*sync.Mutex
	*cluster.License // License object
}

// LicenseStateFromObj converts from memdb object to License state
func LicenseStateFromObj(obj memdb.Object) (*LicenseState, error) {
	switch obj.(type) {
	case *LicenseState:
		ffobj := obj.(*LicenseState)
		return ffobj, nil

	default:
		return nil, errors.New("Incorrect object type")
	}
}

// NewLicenseState creates a new License state object
func NewLicenseState(sn *cluster.License) (*LicenseState, error) {
	// create License state object
	sns := LicenseState{
		License: sn,
		Mutex:   new(sync.Mutex),
	}
	return &sns, nil
}

// FindLicense finds License object by name
func (sm *Statemgr) FindLicense(name string) (*LicenseState, error) {
	// find the object
	obj, err := sm.FindObject("License", "", name)
	if err != nil {
		return nil, err
	}
	return LicenseStateFromObj(obj)
}

// ListLicense lists all License objects
func (sm *Statemgr) ListLicense() ([]*LicenseState, error) {
	var ffs []*LicenseState
	objs := sm.memDB.ListObjects("License", nil)
	for _, obj := range objs {
		ff, err := LicenseStateFromObj(obj)
		if err != nil {
			return ffs, err
		}

		ffs = append(ffs, ff)
	}
	return ffs, nil
}

// CreateLicense creates a License object
func (sm *Statemgr) CreateLicense(ff *cluster.License) (*LicenseState, error) {
	// see if we already have it
	ehs, err := sm.FindLicense(ff.ObjectMeta.Name)
	if err == nil {
		// Object exists in cache, but we got a watcher event with event-type:Created
		// and this can happen if there is a watcher error/reset and we need to update
		// the cache to handle it gracefully as an Update.
		log.Infof("Objects exists, updating Host OldState: {%+v}. New state: {%+v}", ehs, ff)
		ehs.Lock()
		defer ehs.Unlock()
		return ehs, sm.UpdateLicense(ff, false)
	}

	// create new Host state
	ffState, err := NewLicenseState(ff)
	if err != nil {
		log.Errorf("Error creating new License state. Err: %v", err)
		return nil, err
	}

	// store it in local DB
	err = sm.memDB.AddObject(ffState)
	if err != nil {
		log.Errorf("Error storing the Host state in memdb. Err: %v", err)
		return nil, err
	}

	// Update the local license library
	featureflags.Update(ff.Spec.Features)

	// License creates are never written back to ApiServer
	log.Infof("Created License state {%+v}", ff.ObjectMeta)
	return ffState, nil
}

// UpdateLicense updates a License object
// Caller is responsible for acquiring the lock before invocation and releasing it afterwards
func (sm *Statemgr) UpdateLicense(ff *cluster.License, writeback bool) error {
	obj, err := sm.FindObject("License", "", ff.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Can not find the License %s err: %v", ff.ObjectMeta.Name, err)
		return fmt.Errorf("License not found")
	}

	ffState, err := LicenseStateFromObj(obj)
	if err != nil {
		log.Errorf("Wrong object type in memdb! Expected License, got %T", obj)
		return err
	}
	ffState.License = ff

	// store it in local DB
	err = sm.memDB.UpdateObject(ffState)
	if err != nil {
		log.Errorf("Error storing License in memdb. Err: %v", err)
		return err
	}

	// Update the local license library
	featureflags.Update(ff.Spec.Features)

	// No writeback for License object to API Server/
	log.Debugf("Updated License state {%+v}", ff)
	return nil
}

// DeleteLicense deletes a Host state
func (sm *Statemgr) DeleteLicense(ff *cluster.License) error {
	// delete it from the DB
	err := sm.memDB.DeleteObject(ff)
	if err != nil {
		log.Errorf("Error deleting License state %+v: %v", ff.ObjectMeta, err)
		return err
	}

	// License deletes are never written back to ApiServer
	log.Infof("Deleted License state {%+v}", ff.ObjectMeta)
	return nil
}
