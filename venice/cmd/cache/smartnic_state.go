// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cache

import (
	"context"
	"errors"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/runtime"
)

// SmartNICState security policy state
type SmartNICState struct {
	*sync.RWMutex
	*cluster.DistributedServiceCard      // smartnic policy object
	dirty                           bool // dirty is true when there are uncommitted updates to apiserver
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
func NewSmartNICState(sn *cluster.DistributedServiceCard) (*SmartNICState, error) {
	// create smartnic state object
	sns := SmartNICState{
		DistributedServiceCard: sn,
		RWMutex:                new(sync.RWMutex),
	}

	return &sns, nil
}

// FindSmartNIC finds smartNIC object by name
func (sm *Statemgr) FindSmartNIC(name string) (*SmartNICState, error) {
	// find the object
	obj, err := sm.FindObject("DistributedServiceCard", "", name)
	if err != nil {
		return nil, err
	}

	return SmartNICStateFromObj(obj)
}

// ListSmartNICs lists all smartNIC objects
func (sm *Statemgr) ListSmartNICs() ([]*SmartNICState, error) {
	objs := sm.memDB.ListObjects("DistributedServiceCard", nil)

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
func (sm *Statemgr) CreateSmartNIC(sn *cluster.DistributedServiceCard, writeback bool) (*SmartNICState, error) {
	// see if we already have it
	esn, err := sm.FindSmartNIC(sn.ObjectMeta.Name)
	if err == nil {
		// Object exists in cache, but we got a watcher event with event-type:Created
		// and this can happen if there is a watcher error/reset and we need to update
		// the cache to handle it gracefully as an Update.
		esn.Lock()
		defer esn.Unlock()
		log.Infof("Objects exists, updating smartNIC OldState: {%+v}. New state: {%+v}", esn, sn)
		return esn, sm.UpdateSmartNIC(sn, writeback, true)
	}

	if sn.Spec.ID == "" {
		err = fmt.Errorf("Error creating new smartnic state: SmartNIC has empty hostname: %+v", sn)
		log.Errorf(err.Error())
		return nil, err
	}

	// create new smartnic state
	sns, err := NewSmartNICState(sn)
	if err != nil {
		log.Errorf("Error creating new smartnic state. Err: %v", err)
		return nil, err
	}

	sns.Lock()
	defer sns.Unlock()

	// Rejected SmartNIC objects can have duplicate hostnames.
	// In this case we don't update the map
	if sn.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_REJECTED.String() {
		sm.hostnameToSmartNICMapLock.Lock()
		nic, ok := sm.hostnameToSmartNICMap[sn.Spec.ID]
		if ok || nic != nil {
			log.Errorf("Error updating hostnameToSmartNICMap, key %s, exists: %v, value:%+v", sn.Spec.ID, ok, nic)
			// continue anyway
		}
		sm.hostnameToSmartNICMap[sn.Spec.ID] = sn
		sm.hostnameToSmartNICMapLock.Unlock()
	}

	// store it in local DB
	err = sm.memDB.AddObject(sns)
	if err != nil {
		log.Errorf("Error storing the smartnic in memdb. Err: %v", err)
		return nil, err
	}

	if writeback {
		// For creates we want to use a generous timeout and a couple of retries because
		// if we fail to create the SmartNIC object then we need to return an error to NMD
		// and have it retry
		f := func(ctx context.Context) (interface{}, error) {
			nic, err := sm.APIClient().DistributedServiceCard().Create(ctx, sn)
			return nic, err
		}
		_, err := utils.ExecuteWithRetry(f, apiServerRPCTimeout, maxAPIServerWriteRetries)
		if err != nil && !strings.Contains(strings.ToLower(err.Error()), "exists") {
			log.Errorf("Error creating SmartNIC object %+v: %v", sn.ObjectMeta, err)
			// if we didn't get to create the object in ApiServer, we need to remove it from local cache as well
			sm.memDB.DeleteObject(sn)
			sm.hostnameToSmartNICMapLock.Lock()
			delete(sm.hostnameToSmartNICMap, sn.Spec.ID)
			sm.hostnameToSmartNICMapLock.Unlock()
			return nil, fmt.Errorf("Error creating SmartNIC object")
		}
	}

	log.Infof("Created SmartNIC state {%+v}, writeback: %v", sns, writeback)
	return sns, nil
}

// UpdateSmartNIC updates a smartNIC object
// Caller is responsible for acquiring the lock before invocation and releasing it afterwards
func (sm *Statemgr) UpdateSmartNIC(updObj *cluster.DistributedServiceCard, writeback, forceSpec bool) error {
	obj, err := sm.FindObject("DistributedServiceCard", "", updObj.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Can not find the smartnic %s err: %v", updObj.ObjectMeta.Name, err)
		return fmt.Errorf("SmartNIC not found")
	}

	if updObj.Spec.ID == "" {
		err = fmt.Errorf("Error updating smartnic state: SmartNIC has empty hostname: %+v", updObj)
		log.Errorf(err.Error())
		return err
	}

	cachedState, err := SmartNICStateFromObj(obj)
	if err != nil {
		log.Errorf("Wrong object type in memdb! Expected SmartNIC, got %T", obj)
		return err
	}

	sm.hostnameToSmartNICMapLock.Lock()
	if cachedState.Spec.ID != updObj.Spec.ID &&
		cachedState.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_REJECTED.String() {
		_, ok := sm.hostnameToSmartNICMap[cachedState.Spec.ID]
		if !ok || cachedState.Spec.ID == "" {
			log.Errorf("Error updating hostnameToSmartNICMap, key %s does not exist or is empty", cachedState.Spec.ID)
			// continue anyway
		}
		delete(sm.hostnameToSmartNICMap, cachedState.Spec.ID)
	}
	// update hostnameToSmartNICMap entry even if there is no hostname change so the map
	// and the cache stay in sync and we don't keep old objects around
	if updObj.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_REJECTED.String() {
		sm.hostnameToSmartNICMap[updObj.Spec.ID] = updObj
	}
	sm.hostnameToSmartNICMapLock.Unlock()

	cachedState.DistributedServiceCard = updObj

	// store it in local DB
	err = sm.memDB.UpdateObject(cachedState)
	if err != nil {
		log.Errorf("Error storing smartnic in memdb. Err: %v", err)
		return err
	}

	if writeback || cachedState.dirty {
		ok := false
		// DSCs updates are periodic and can come in a big batch after leader update,
		// so we use lower retries number to avoid overloading ApiServer
		maxDSCUpdateAPIServerWriteRetries := 2
		for i := 0; i < maxDSCUpdateAPIServerWriteRetries; i++ {
			ctx, cancel := context.WithTimeout(context.Background(), apiServerRPCTimeout)
			if forceSpec {
				updObj.ResourceVersion = ""
				_, err = sm.APIClient().DistributedServiceCard().Update(ctx, updObj)
			} else {
				_, err = sm.APIClient().DistributedServiceCard().UpdateStatus(ctx, updObj)
			}
			if err == nil {
				ok = true
				cachedState.dirty = false
				cancel()
				log.Infof("Updated SmartNIC object in ApiServer: %+v", updObj)
				break
			}
			log.Errorf("Error updating SmartNIC object %+v: %v", updObj.ObjectMeta, err)
			cancel()
			time.Sleep(apiClientRetryInterval)
		}
		if !ok {
			cachedState.dirty = true
			log.Errorf("Error updating SmartNIC object %+v in ApiServer, retries exhausted", updObj.ObjectMeta)
		}
	}

	log.Debugf("Updated SmartNIC state {%+v}", cachedState)
	return nil
}

// DeleteSmartNIC deletes a smartNIC state
func (sm *Statemgr) DeleteSmartNIC(sn *cluster.DistributedServiceCard) error {
	if sn.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_REJECTED.String() {
		sm.hostnameToSmartNICMapLock.Lock()
		_, ok := sm.hostnameToSmartNICMap[sn.Spec.ID]
		if !ok || sn.Spec.ID == "" {
			log.Errorf("Error updating hostnameToSmartNICMap, key %s does not exist or is empty", sn.Spec.ID)
			// continue anyway
		}
		delete(sm.hostnameToSmartNICMap, sn.Spec.ID)
		sm.hostnameToSmartNICMapLock.Unlock()
	}

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

// GetSmartNICByID returns the SmartNIC object for a given hostname
func (sm *Statemgr) GetSmartNICByID(hostname string) *cluster.DistributedServiceCard {
	sm.hostnameToSmartNICMapLock.RLock()
	defer sm.hostnameToSmartNICMapLock.RUnlock()
	return sm.hostnameToSmartNICMap[hostname]
}

// MarkSmartNICsDirty retrieves SmartNIC objects from ApiServer and compares to local DB.
// If it finds a difference, it marks the SmartNIC as dirty, so that next update gets
// forcefully pushed to ApiServer
func (sm *Statemgr) MarkSmartNICsDirty() {
	var nics []*cluster.DistributedServiceCard
	var err error
	for i := 0; i < maxAPIServerWriteRetries; i++ {
		if sm.APIClient() == nil {
			log.Infof("APICLient not available, try %d of %d", i, maxAPIServerWriteRetries)
			time.Sleep(apiServerRPCTimeout)
			continue
		}
		ctx, cancel := context.WithTimeout(context.Background(), apiServerRPCTimeout)
		nics, err = sm.APIClient().DistributedServiceCard().List(ctx, &api.ListWatchOptions{})
		if err == nil {
			cancel()
			break
		}
		log.Errorf("Error listing DSCs from ApiServer, try %d of %d: %v", i, maxAPIServerWriteRetries, err)
		cancel()
		time.Sleep(apiClientRetryInterval)
	}
	if err != nil {
		log.Errorf("Error listing DSCs from ApiServer, retries exhausted")
		return
	}

	log.Infof("MarkSmartNICsDirty: got %d DSC objects", len(nics))
	for _, n := range nics {
		o, err := sm.FindObject("DistributedServiceCard", "", n.ObjectMeta.Name)
		if err == nil {
			s, _ := SmartNICStateFromObj(o)
			s.Lock()
			// mark dirty if it is out-of-sync
			if !runtime.FilterUpdate(n.Status, s.Status, []string{"LastTransitionTime"}, []string{"Conditions", "AdmissionPhase"}) {
				s.dirty = true
				log.Infof("Marked DSC %s dirty", n.ObjectMeta.Name)
			}
			s.Unlock()
		} else {
			log.Errorf("Error looking up DSC object %s in memdb", n.ObjectMeta.Name)
		}
	}
}

func (sm *Statemgr) retryDirtySmartNICs(done chan bool) {
	log.Infof("RetryDirtySmartNICs Start")
	ticker := time.NewTicker(time.Duration(2*maxAPIServerWriteRetries) * apiClientRetryInterval * time.Second)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			if sm.isLeader() {
				nics, err := sm.ListSmartNICs()
				if err != nil {
					log.Infof("Error listing SmartNICs: %v", err)
				}
				for _, s := range nics {
					s.Lock()
					if s.dirty {
						log.Infof("Pushing update for dirty DSC %s", s.DistributedServiceCard.Name)
						sm.UpdateSmartNIC(s.DistributedServiceCard, true, false)
					}
					s.Unlock()
				}
			}
		case <-done:
			log.Infof("RetryDirtySmartNICs Stop")
			return
		}
	}
}
