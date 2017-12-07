// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cache

import (
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// SmartNICState security policy state
type SmartNICState struct {
	cmd.SmartNIC           // smartnic policy object
	stateMgr     *Statemgr // pointer to state manager
}

// SmartNICStateFromObj converts from memdb object to SmartNIC state
func SmartNICStateFromObj(obj memdb.Object) (*SmartNICState, error) {
	switch obj.(type) {
	case *SmartNICState:
		snobj := obj.(*SmartNICState)
		return snobj, nil
	default:
		return nil, errors.New("Incorrect object type")
	}
}

// NewSmartNICState creates a new security policy state object
func NewSmartNICState(sn *cmd.SmartNIC, stateMgr *Statemgr) (*SmartNICState, error) {
	// create smartnic state object
	sns := SmartNICState{
		SmartNIC: *sn,
		stateMgr: stateMgr,
	}

	return &sns, nil
}

// FindSmartNIC finds smartNIC object by name
func (sm *Statemgr) FindSmartNIC(tenant, name string) (*SmartNICState, error) {
	// find the object
	obj, err := sm.FindObject("SmartNIC", tenant, name)
	if err != nil {
		return nil, err
	}

	return SmartNICStateFromObj(obj)
}

// ListSmartNICs lists all smartNIC objects
func (sm *Statemgr) ListSmartNICs() ([]*SmartNICState, error) {
	objs := sm.memDB.ListObjects("SmartNIC")

	var sgs []*SmartNICState
	for _, obj := range objs {
		sg, err := SmartNICStateFromObj(obj)
		if err != nil {
			return sgs, err
		}

		sgs = append(sgs, sg)
	}

	return sgs, nil
}

// CreateSmartNIC creates a smartNIC object
func (sm *Statemgr) CreateSmartNIC(sn *cmd.SmartNIC) error {
	// see if we already have it
	esn, err := sm.FindObject("SmartNIC", sn.ObjectMeta.Tenant, sn.ObjectMeta.Name)
	if err == nil {
		log.Errorf("Can not change existing smartNIC {%+v}. New state: {%+v}", esn, sn)
		return fmt.Errorf("Can not change smartNIC after its created")
	}

	// create new sg state
	sns, err := NewSmartNICState(sn, sm)
	if err != nil {
		log.Errorf("Error creating new smartnic state. Err: %v", err)
		return err
	}

	// store it in local DB
	err = sm.memDB.AddObject(sns)
	if err != nil {
		log.Errorf("Error storing the smartnic in memdb. Err: %v", err)
		return err
	}

	log.Infof("Created SmartNIC state {%+v}", sns)
	return nil
}

// UpdateSmartNIC updates a smartNIC object
func (sm *Statemgr) UpdateSmartNIC(sn *cmd.SmartNIC) error {

	// see if we already have it
	_, err := sm.FindObject("SmartNIC", sn.ObjectMeta.Tenant, sn.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Can not find the smartnic %s|%s", sn.ObjectMeta.Tenant, sn.ObjectMeta.Name)
		return fmt.Errorf("SmartNIC not found")
	}

	// create new smartnic state
	sns, err := NewSmartNICState(sn, sm)
	if err != nil {
		log.Errorf("Error creating new smartnic state. Err: %v", err)
		return err
	}

	// store it in local DB
	err = sm.memDB.UpdateObject(sns)
	if err != nil {
		log.Errorf("Error storing smartnic in memdb. Err: %v", err)
		return err
	}

	log.Infof("Created SmartNIC state {%+v}", sns)
	return nil
}

// DeleteSmartNIC deletes a smartNIC state
func (sm *Statemgr) DeleteSmartNIC(tenant, name string) error {

	// see if we already have it
	sn, err := sm.FindObject("SmartNIC", tenant, name)
	if err != nil {
		log.Errorf("Can not find the smartnic %s|%s", tenant, name)
		return fmt.Errorf("SmartNIC not found")
	}

	// convert it to smartNIC state
	sn, err = SmartNICStateFromObj(sn)
	if err != nil {
		return err
	}

	// delete it from the DB
	return sm.memDB.DeleteObject(sn)
}
