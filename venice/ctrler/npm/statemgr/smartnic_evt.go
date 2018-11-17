// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// SmartNICReactor is the event reactor for smartNic events
type SmartNICReactor struct {
	stateMgr *Statemgr // state manager
}

// SmartNICHandler smartNic event handler interface
type SmartNICHandler interface {
	CreateSmartNIC(smartNic cluster.SmartNIC) error
	DeleteSmartNIC(smartNic cluster.SmartNIC) error
}

// SmartNICState is a wrapper for smartNic object
type SmartNICState struct {
	sync.Mutex                 // lock the smartNic object
	cluster.SmartNIC           // smartNic object
	stateMgr         *Statemgr // pointer to state manager
}

// SmartNICStateFromObj conerts from memdb object to smartNic state
func SmartNICStateFromObj(obj memdb.Object) (*SmartNICState, error) {
	switch obj.(type) {
	case *SmartNICState:
		nsobj := obj.(*SmartNICState)
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewSmartNICState creates new smartNic state object
func NewSmartNICState(smartNic cluster.SmartNIC, stateMgr *Statemgr) (*SmartNICState, error) {
	hs := &SmartNICState{
		SmartNIC: smartNic,
		stateMgr: stateMgr,
	}

	// Notify statemgr about the smartnic
	stateMgr.smartNICCreated(hs)

	return hs, nil
}

// CreateSmartNIC handles smartNic creation
func (hr *SmartNICReactor) CreateSmartNIC(smartNic cluster.SmartNIC) error {
	// see if we already have the smartNic
	hs, err := hr.stateMgr.FindSmartNIC(smartNic.Tenant, smartNic.Name)
	if err == nil {
		hs.SmartNIC = smartNic
		return nil
	}

	log.Infof("Creating smart nic: %+v", smartNic)

	// create new smartNic object
	hs, err = NewSmartNICState(smartNic, hr.stateMgr)
	if err != nil {
		log.Errorf("Error creating smartNic %+v. Err: %v", smartNic, err)
		return err
	}

	return nil
}

// DeleteSmartNIC handles smartNic deletion
func (hr *SmartNICReactor) DeleteSmartNIC(smartNic cluster.SmartNIC) error {
	// see if we have the smartNic
	hs, err := hr.stateMgr.FindSmartNIC(smartNic.Tenant, smartNic.Name)
	if err != nil {
		log.Errorf("Could not find the smartNic %v. Err: %v", smartNic, err)
		return err
	}

	log.Infof("Deleting smart nic: %+v", smartNic)

	// notify statemgr
	return hr.stateMgr.smartNICDeleted(hs)
}

// NewSmartNICReactor creates new smartNic event reactor
func NewSmartNICReactor(sm *Statemgr) (*SmartNICReactor, error) {
	smartNic := SmartNICReactor{
		stateMgr: sm,
	}

	return &smartNic, nil
}

// FindSmartNIC finds a smartNic
func (sm *Statemgr) FindSmartNIC(tenant, name string) (*SmartNICState, error) {
	// find the object
	obj, err := sm.FindObject("SmartNIC", tenant, name)
	if err != nil {
		return nil, err
	}

	return SmartNICStateFromObj(obj)
}

// FindSmartNICByMacAddr finds the smart nic by mac addr
func (sm *Statemgr) FindSmartNICByMacAddr(macAddr string) (*SmartNICState, error) {
	objs := sm.ListObjects("SmartNIC")

	for _, obj := range objs {
		snic, err := SmartNICStateFromObj(obj)
		if err != nil {
			return nil, err
		}

		if snic.Status.PrimaryMAC == macAddr {
			return snic, nil
		}
	}

	return nil, fmt.Errorf("Smartnic not found for mac addr %v", macAddr)
}
