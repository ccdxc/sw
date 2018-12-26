// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"

	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// SmartNICState is a wrapper for smartNic object
type SmartNICState struct {
	SmartNIC *ctkit.SmartNIC `json:"-"` // smartNic object
	stateMgr *Statemgr       // pointer to state manager
}

// SmartNICStateFromObj conerts from memdb object to smartNic state
func SmartNICStateFromObj(obj runtime.Object) (*SmartNICState, error) {
	switch obj.(type) {
	case *ctkit.SmartNIC:
		sobj := obj.(*ctkit.SmartNIC)
		switch sobj.HandlerCtx.(type) {
		case *SmartNICState:
			nsobj := sobj.HandlerCtx.(*SmartNICState)
			return nsobj, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewSmartNICState creates new smartNic state object
func NewSmartNICState(smartNic *ctkit.SmartNIC, stateMgr *Statemgr) (*SmartNICState, error) {
	hs := &SmartNICState{
		SmartNIC: smartNic,
		stateMgr: stateMgr,
	}
	smartNic.HandlerCtx = hs

	// Notify statemgr about the smartnic
	stateMgr.smartNICCreated(hs)

	return hs, nil
}

// OnSmartNICCreate handles smartNic creation
func (sm *Statemgr) OnSmartNICCreate(smartNic *ctkit.SmartNIC) error {
	// see if we already have the smartNic
	hs, err := sm.FindSmartNIC(smartNic.Tenant, smartNic.Name)
	if err == nil {
		hs.SmartNIC = smartNic
		return nil
	}

	log.Infof("Creating smart nic: %+v", smartNic)

	// create new smartNic object
	hs, err = NewSmartNICState(smartNic, sm)
	if err != nil {
		log.Errorf("Error creating smartNic %+v. Err: %v", smartNic, err)
		return err
	}

	return nil
}

// OnSmartNICUpdate handles update event on smartnic
func (sm *Statemgr) OnSmartNICUpdate(smartNic *ctkit.SmartNIC) error {
	return nil
}

// OnSmartNICDelete handles smartNic deletion
func (sm *Statemgr) OnSmartNICDelete(smartNic *ctkit.SmartNIC) error {
	// see if we have the smartNic
	hs, err := sm.FindSmartNIC(smartNic.Tenant, smartNic.Name)
	if err != nil {
		log.Errorf("Could not find the smartNic %v. Err: %v", smartNic, err)
		return err
	}

	log.Infof("Deleting smart nic: %+v", smartNic)

	// notify statemgr
	return sm.smartNICDeleted(hs)
}

// FindSmartNIC finds a smartNic
func (sm *Statemgr) FindSmartNIC(tenant, name string) (*SmartNICState, error) {
	// find the object
	obj, err := sm.FindObject("SmartNIC", "", "", name)
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

		if snic.SmartNIC.Status.PrimaryMAC == macAddr {
			return snic, nil
		}
	}

	return nil, fmt.Errorf("Smartnic not found for mac addr %v", macAddr)
}
