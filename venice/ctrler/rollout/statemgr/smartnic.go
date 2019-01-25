// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sort"
	"sync"
	"sync/atomic"

	"github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// SmartNICRolloutState - Internal state for Rollout
type SmartNICRolloutState struct {
	Mutex sync.Mutex
	*protos.SmartNICRollout
	*Statemgr
	ros    *RolloutState
	status map[protos.SmartNICOp]protos.SmartNICOpStatus
}

// CreateSmartNICRolloutState to create a SmartNICRollout Object in statemgr
func (sm *Statemgr) CreateSmartNICRolloutState(ro *protos.SmartNICRollout, ros *RolloutState) error {
	_, err := sm.FindObject(kindSmartNICRollout, ro.Tenant, ro.Name)
	if err == nil {
		log.Errorf("SmartNICRollout {+%v} exists", ro)
		return fmt.Errorf("SmartNICRollout already exists")
	}

	sros := SmartNICRolloutState{
		SmartNICRollout: ro,
		Statemgr:        sm,
		ros:             ros,
		status:          make(map[protos.SmartNICOp]protos.SmartNICOpStatus),
	}

	sros.Mutex.Lock()
	// XXX validate parameters -
	if sros.GetObjectKind() != kindSmartNICRollout {
		sros.Mutex.Unlock()
		return fmt.Errorf("Unexpected object kind %s", sros.GetObjectKind())
	}
	sros.Mutex.Unlock()

	sm.memDB.AddObject(&sros)
	return nil
}

// GetSmartNICRolloutState : Get the specified rollout state
func (sm *Statemgr) GetSmartNICRolloutState(tenant, name string) (*SmartNICRolloutState, error) {
	dbMs, err := sm.FindObject(kindSmartNICRollout, tenant, name)
	if err != nil {
		return nil, err
	}
	return dbMs.(*SmartNICRolloutState), nil
}

// DeleteSmartNICRolloutState - delete rollout
func (sm *Statemgr) DeleteSmartNICRolloutState(ro *protos.SmartNICRollout) {
	ros, err := sm.GetSmartNICRolloutState(ro.Tenant, ro.Name)
	if err != nil {
		log.Debugf("non-existent SmartNICRollout {%+v}. Err: %v", ro, err)
		return
	}

	log.Infof("Deleting SmartNICRollout %v", ros.SmartNICRollout.Name)
	ros.Mutex.Lock()
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state
	ros.Mutex.Unlock()

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(ros)
}

// SmartNICRolloutStateFromObj converts from memdb object to SmartNICRollout state
func SmartNICRolloutStateFromObj(obj memdb.Object) (*SmartNICRolloutState, error) {
	switch obj.(type) {
	case *SmartNICRolloutState:
		nsobj := obj.(*SmartNICRolloutState)
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// ListSmartNICRollouts lists all SmartNICRollout objects
func (sm *Statemgr) ListSmartNICRollouts() ([]*SmartNICRolloutState, error) {
	objs := sm.memDB.ListObjects(kindSmartNICRollout)

	var smartNICUpdateStates []*SmartNICRolloutState
	for _, obj := range objs {
		nso, err := SmartNICRolloutStateFromObj(obj)
		if err != nil {
			return smartNICUpdateStates, err
		}

		smartNICUpdateStates = append(smartNICUpdateStates, nso)
	}
	sort.Sort(bySNICROSName(smartNICUpdateStates))

	return smartNICUpdateStates, nil
}

// UpdateSmartNICRolloutStatus - update status
func (snicState *SmartNICRolloutState) UpdateSmartNICRolloutStatus(newStatus *protos.SmartNICRolloutStatus) {

	log.Infof("Updating status of SmartNICRollout %v", snicState.SmartNICRollout.Name)
	version := snicState.ros.Rollout.Spec.Version

	var phase rollout.RolloutPhase_Phases
	var reason, message string
	snicState.Mutex.Lock()
	for _, s := range newStatus.OpStatus {
		if s.Version != version {
			continue
		}
		existingStatus := snicState.status[s.Op]
		if existingStatus.OpStatus == s.OpStatus {
			continue
		}
		evt := fsmEvInvalid
		if s.OpStatus == "success" {
			switch s.Op {
			case protos.SmartNICOp_SmartNICPreCheckForDisruptive:
				evt = fsmEvOneSmartNICPreupgSuccess
				phase = rollout.RolloutPhase_WAITING_FOR_TURN
			case protos.SmartNICOp_SmartNICPreCheckForUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICPreupgSuccess
				phase = rollout.RolloutPhase_WAITING_FOR_TURN
			case protos.SmartNICOp_SmartNICUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICUpgSuccess
				phase = rollout.RolloutPhase_COMPLETE
			case protos.SmartNICOp_SmartNICDisruptiveUpgrade:
				evt = fsmEvOneSmartNICUpgSuccess
				phase = rollout.RolloutPhase_COMPLETE
			}
		} else {
			switch s.Op {
			case protos.SmartNICOp_SmartNICPreCheckForDisruptive:
				evt = fsmEvOneSmartNICPreupgFail
				atomic.AddInt32(&snicState.ros.numPreUpgradeFailures, 1)
				phase = rollout.RolloutPhase_FAIL
			case protos.SmartNICOp_SmartNICPreCheckForUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICPreupgFail
				atomic.AddInt32(&snicState.ros.numPreUpgradeFailures, 1)
				phase = rollout.RolloutPhase_FAIL
			case protos.SmartNICOp_SmartNICUpgOnNextHostReboot:
				atomic.AddUint32(&snicState.ros.numFailuresSeen, 1)
				evt = fsmEvOneSmartNICUpgFail
				phase = rollout.RolloutPhase_FAIL
			case protos.SmartNICOp_SmartNICDisruptiveUpgrade:
				atomic.AddUint32(&snicState.ros.numFailuresSeen, 1)
				evt = fsmEvOneSmartNICUpgFail
				phase = rollout.RolloutPhase_FAIL
			}
		}
		snicState.status[s.Op] = s
		if evt != fsmEvInvalid {
			snicState.ros.eventChan <- evt
			message = s.Message
			reason = s.OpStatus
		}
	}
	snicState.Statemgr.memDB.UpdateObject(snicState)
	snicState.Mutex.Unlock()
	snicState.ros.setSmartNICPhase(snicState.Name, reason, message, phase)

}

func (snicState *SmartNICRolloutState) anyPendingOp() bool {
	snicState.Mutex.Lock()
	defer snicState.Mutex.Unlock()

	for _, s := range snicState.Spec.Ops {
		if st := snicState.status[s.Op]; st.OpStatus == "" {
			return true
		}
	}
	return false
}

func (snicState *SmartNICRolloutState) addSpecOp(version string, op protos.SmartNICOp) {

	snicState.Mutex.Lock()
	for _, ops := range snicState.Spec.Ops {
		if ops.Op == op && ops.Version == version {
			log.Debugf("version %s and op %s already exist for %v", version, op, snicState.ObjectMeta)
			snicState.Mutex.Unlock()
			return // version and op already exist
		}
	}
	snicState.Spec.Ops = append(snicState.Spec.Ops, &protos.SmartNICOpSpec{Op: op, Version: version})
	snicState.Mutex.Unlock()

	snicState.Statemgr.memDB.UpdateObject(snicState)
}
