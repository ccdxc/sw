// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
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
	numFailuresSeen       uint32
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

// === Status updaters ===
func (ros *RolloutState) saveStatus() {
	if ros != nil && ros.writer != nil {
		ros.writer.WriteRollout(ros.Rollout)
	}
}

func (ros *RolloutState) setPreviousVersion(v string) {
	if ros.Status.PreviousVersion == "" {
		ros.Status.PreviousVersion = v
		ros.saveStatus()
	}
}
func (ros *RolloutState) setStartTime() {
	if ros.Status.StartTime == nil {
		t := api.Timestamp{}
		t.SetTime(time.Now())
		ros.Status.StartTime = &t
		ros.saveStatus()
	}
}

func (ros *RolloutState) setEndTime() {
	if ros.Status.EndTime == nil {
		t := api.Timestamp{}
		t.SetTime(time.Now())
		ros.Status.EndTime = &t
		ros.saveStatus()
	}
}

func (ros *RolloutState) setStateAndPercentage(state string, percent uint32) {
	ros.Status.CompletionPercentage = percent
	ros.Status.OperationalState = state
}

func (ros *RolloutState) setVenicePhase(name, reason, message string, phase roproto.RolloutPhase_Phases) {
	index := -1
	for i, curStatus := range ros.Status.ControllerNodesStatus {
		if curStatus.Name == name {
			index = i
		}
	}

	if index == -1 {
		rp := roproto.RolloutPhase{
			Name: name,
		}
		ros.Status.ControllerNodesStatus = append(ros.Status.ControllerNodesStatus, &rp)
		index = len(ros.Status.ControllerNodesStatus) - 1
	}

	switch phase {
	case roproto.RolloutPhase_PROGRESSING:
		startTime := api.Timestamp{}
		startTime.SetTime(time.Now())
		ros.Status.ControllerNodesStatus[index].StartTime = &startTime

	case roproto.RolloutPhase_COMPLETE, roproto.RolloutPhase_FAIL:
		endTime := api.Timestamp{}
		endTime.SetTime(time.Now())
		ros.Status.ControllerNodesStatus[index].EndTime = &endTime

	case roproto.RolloutPhase_PRE_CHECK:
	case roproto.RolloutPhase_DEPENDENCIES_CHECK:
	case roproto.RolloutPhase_WAITING_FOR_TURN:

	}
	ros.Status.ControllerNodesStatus[index].Reason = reason
	ros.Status.ControllerNodesStatus[index].Message = message
	ros.Status.ControllerNodesStatus[index].Phase = phase.String()
	ros.saveStatus()
}

func (ros *RolloutState) setServicePhase(name, reason, message string, phase roproto.RolloutPhase_Phases) {
	index := -1
	for i, curStatus := range ros.Status.ControllerServicesStatus {
		if curStatus.Name == name {
			index = i
		}
	}

	if index == -1 {
		rp := roproto.RolloutPhase{
			Name: name,
		}
		ros.Status.ControllerServicesStatus = append(ros.Status.ControllerServicesStatus, &rp)
		index = len(ros.Status.ControllerServicesStatus) - 1
	}

	switch phase {
	case roproto.RolloutPhase_PROGRESSING:
		startTime := api.Timestamp{}
		startTime.SetTime(time.Now())
		ros.Status.ControllerServicesStatus[index].StartTime = &startTime

	case roproto.RolloutPhase_COMPLETE, roproto.RolloutPhase_FAIL:
		endTime := api.Timestamp{}
		endTime.SetTime(time.Now())
		ros.Status.ControllerServicesStatus[index].EndTime = &endTime

	case roproto.RolloutPhase_PRE_CHECK:
	case roproto.RolloutPhase_DEPENDENCIES_CHECK:
	case roproto.RolloutPhase_WAITING_FOR_TURN:

	}
	ros.Status.ControllerServicesStatus[index].Reason = reason
	ros.Status.ControllerServicesStatus[index].Message = message
	ros.Status.ControllerServicesStatus[index].Phase = phase.String()
	ros.saveStatus()
}
func (ros *RolloutState) setSmartNICPhase(name, reason, message string, phase roproto.RolloutPhase_Phases) {
	index := -1
	for i, curStatus := range ros.Status.SmartNICsStatus {
		if curStatus.Name == name {
			index = i
		}
	}

	if index == -1 {
		rp := roproto.RolloutPhase{
			Name: name,
		}
		ros.Status.SmartNICsStatus = append(ros.Status.SmartNICsStatus, &rp)
		index = len(ros.Status.SmartNICsStatus) - 1
	}

	switch phase {
	case roproto.RolloutPhase_PROGRESSING:
		startTime := api.Timestamp{}
		startTime.SetTime(time.Now())
		ros.Status.SmartNICsStatus[index].StartTime = &startTime

	case roproto.RolloutPhase_COMPLETE, roproto.RolloutPhase_FAIL:
		endTime := api.Timestamp{}
		endTime.SetTime(time.Now())
		ros.Status.SmartNICsStatus[index].EndTime = &endTime

	case roproto.RolloutPhase_PRE_CHECK:
	case roproto.RolloutPhase_DEPENDENCIES_CHECK:
	case roproto.RolloutPhase_WAITING_FOR_TURN:

	}
	ros.Status.SmartNICsStatus[index].Reason = reason
	ros.Status.SmartNICsStatus[index].Message = message
	ros.Status.SmartNICsStatus[index].Phase = phase.String()
	ros.saveStatus()
}
