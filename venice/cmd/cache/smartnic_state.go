// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cache

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

const (
	maxAPIServerWriteRetries = 10
	apiServerRetryInterval   = 250 * time.Millisecond
	apiServerRPCTimeout      = 4 * time.Second
)

// SmartNICState security policy state
type SmartNICState struct {
	*sync.Mutex
	*cluster.SmartNIC // smartnic policy object
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
func NewSmartNICState(sn *cluster.SmartNIC) (*SmartNICState, error) {
	// create smartnic state object
	sns := SmartNICState{
		SmartNIC: sn,
		Mutex:    new(sync.Mutex),
	}

	return &sns, nil
}

// FindSmartNIC finds smartNIC object by name
func (sm *Statemgr) FindSmartNIC(name string) (*SmartNICState, error) {
	// find the object
	obj, err := sm.FindObject("SmartNIC", "", name)
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
func (sm *Statemgr) CreateSmartNIC(sn *cluster.SmartNIC, writeback bool) (*SmartNICState, error) {
	// see if we already have it
	esn, err := sm.FindSmartNIC(sn.ObjectMeta.Name)
	if err == nil {
		// Object exists in cache, but we got a watcher event with event-type:Created
		// and this can happen if there is a watcher error/reset and we need to update
		// the cache to handle it gracefully as an Update.
		esn.Lock()
		defer esn.Unlock()
		log.Infof("Objects exists, updating smartNIC OldState: {%+v}. New state: {%+v}", esn, sn)
		return esn, sm.UpdateSmartNIC(sn, writeback)
	}

	// create new smartnic state
	sns, err := NewSmartNICState(sn)
	if err != nil {
		log.Errorf("Error creating new smartnic state. Err: %v", err)
		return nil, err
	}

	sns.Lock()
	defer sns.Unlock()

	// store it in local DB
	err = sm.memDB.AddObject(sns)
	if err != nil {
		log.Errorf("Error storing the smartnic in memdb. Err: %v", err)
		return nil, err
	}

	if writeback {
		f := func() (interface{}, error) {
			return sm.APIClient().SmartNIC().Create(context.Background(), sn)
		}
		_, err := utils.ExecuteWithRetry(f, apiServerRetryInterval, maxAPIServerWriteRetries)
		if err != nil {
			log.Errorf("Error creating SmartNIC object %+v: %v", sn.ObjectMeta, err)
		}
	}

	log.Infof("Created SmartNIC state {%+v}, writeback: %v", sns, writeback)
	return sns, nil
}

// UpdateSmartNIC updates a smartNIC object
// Caller is responsible for acquiring the lock before invocation and releasing it afterwards
func (sm *Statemgr) UpdateSmartNIC(sn *cluster.SmartNIC, writeback bool) error {
	obj, err := sm.FindObject("SmartNIC", "", sn.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Can not find the smartnic %s err: %v", sn.ObjectMeta.Name, err)
		return fmt.Errorf("SmartNIC not found")
	}

	sns, err := SmartNICStateFromObj(obj)
	if err != nil {
		log.Errorf("Wrong object type in memdb! Expected SmartNIC, got %T", obj)
		return err
	}
	sns.SmartNIC = sn

	// store it in local DB
	err = sm.memDB.UpdateObject(sns)
	if err != nil {
		log.Errorf("Error storing smartnic in memdb. Err: %v", err)
		return err
	}

	if writeback {
		nicObj := sn
		ok := false
		for i := 0; i < maxAPIServerWriteRetries; i++ {
			ctx, cancel := context.WithTimeout(context.Background(), apiServerRPCTimeout)
			defer cancel()
			_, err = sm.APIClient().SmartNIC().Update(ctx, nicObj)
			if err == nil {
				ok = true
				log.Infof("Updated SmartNIC object in ApiServer: %+v", nicObj)
				break
			}
			log.Errorf("Error updating SmartNIC object %+v: %v", nicObj.ObjectMeta, err)
			// Write error -- fetch updated Spec + Meta and retry
			updObj, err := sm.APIClient().SmartNIC().Get(ctx, &nicObj.ObjectMeta)
			if err == nil {
				updObj.Status = nicObj.Status
				nicObj = updObj
				// retain Status as that's what we are trying to update
			}
		}
		if !ok {
			log.Errorf("Error updating SmartNIC object %+v in ApiServer, retries exhausted", nicObj.ObjectMeta)
		}
	}

	log.Debugf("Updated SmartNIC state {%+v}", sns)
	return nil
}

// DeleteSmartNIC deletes a smartNIC state
func (sm *Statemgr) DeleteSmartNIC(sn *cluster.SmartNIC) error {
	// delete it from the DB
	err := sm.memDB.DeleteObject(sn)
	if err != nil {
		log.Errorf("Error deleting SmartNIC state %+v: %v", sn.ObjectMeta, err)
		return err
	}

	// Deletes are never written back to ApiServer
	log.Infof("Deleted SmartNIC state {%+v}", sn.ObjectMeta)
	return nil
}
