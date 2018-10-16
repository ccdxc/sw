// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sync"

	roproto "github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// RolloutState - Internal state for Rollout
type RolloutState struct {
	Mutex sync.Mutex
	sync.WaitGroup
	*roproto.Rollout
	*Statemgr

	// Local information
	eventChan    chan rofsmEvent
	stopChan     chan bool
	currentState rofsmState
	stopped      bool

	fsm [][]fsmNode

	numPreUpgradeFailures int32 // number of failures seen so far (computed from status)
}

func (sm *Statemgr) handleRolloutEvent(et kvstore.WatchEventType, ro *roproto.Rollout) {
	switch et {
	case kvstore.Created:
		log.Infof("createRolloutState - %s\n", ro.Name)
		err := sm.createRolloutState(ro)
		if err != nil {
			log.Errorf("Error creating Rollout {%+v}. Err: %v", ro, err)
			return
		}
	case kvstore.Updated:
		log.Infof("UpdateRollout - %s\n", ro.Name)

	case kvstore.Deleted:
		log.Infof("deleteRolloutState - %s\n", ro.Name)

		sm.deleteRolloutState(ro)
	}
}

// createRolloutState to create a Rollout Object in statemgr
func (sm *Statemgr) createRolloutState(ro *roproto.Rollout) error {
	// All parameters are validated (using apiserver hooks) by the time we get here
	_, err := sm.FindObject(kindRollout, ro.Tenant, ro.Name)
	if err == nil {
		log.Errorf("Rollout {+%v} exists", ro)
		return fmt.Errorf("Rollout already exists")
	}

	ros := RolloutState{
		Rollout:   ro,
		Statemgr:  sm,
		eventChan: make(chan rofsmEvent, 100),
		stopChan:  make(chan bool),
		fsm:       roFSM,
	}

	ros.Mutex.Lock()
	// XXX validate parameters -
	if ros.GetObjectKind() != kindRollout {
		ros.Mutex.Unlock()
		return fmt.Errorf("Unexpected object kind %s", ros.GetObjectKind())
	}
	ros.Mutex.Unlock()
	sm.memDB.AddObject(&ros)

	// TODO: Ensure there is only one Rollout object that is running the state machine at any point of time
	ros.start()

	return nil
}

// GetRolloutState : Get the specified rollout state
func (sm *Statemgr) GetRolloutState(tenant, name string) (*RolloutState, error) {
	dbMs, err := sm.FindObject(kindRollout, tenant, name)
	if err != nil {
		return nil, err
	}
	return dbMs.(*RolloutState), nil
}

// deleteRolloutState - delete rollout
func (sm *Statemgr) deleteRolloutState(ro *roproto.Rollout) {
	ros, err := sm.GetRolloutState(ro.Tenant, ro.Name)
	if err != nil {
		log.Debugf("Error deleting non-existent rollout {%+v}. Err: %v", ro, err)
		return
	}

	ros.stop()

	log.Infof("Deleting Rollout %v", ros.Rollout.Name)
	ros.Mutex.Lock()
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state
	ros.Mutex.Unlock()

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(ros)
	sm.deleteRollouts()
}

func (ros *RolloutState) start() {
	ros.currentState = fsmstStart
	ros.Add(1)
	go ros.runFSM()
	ros.eventChan <- fsmEvROCreated
}

func (ros *RolloutState) stop() {
	ros.stopped = true
	close(ros.stopChan)
	ros.Wait()
	ros.currentState = fsmstInvalid

}
