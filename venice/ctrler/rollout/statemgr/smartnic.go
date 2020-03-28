// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sort"
	"sync"
	"sync/atomic"

	roproto "github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// DSCRolloutState - Internal state for Rollout
type DSCRolloutState struct {
	Mutex sync.Mutex
	*protos.DSCRollout
	*Statemgr
	ros    *RolloutState
	status map[protos.DSCOp]protos.DSCOpStatus
}

// CreateDSCRolloutState to create a DSCRollout Object in statemgr
func (sm *Statemgr) CreateDSCRolloutState(ro *protos.DSCRollout, ros *RolloutState, snicStatus *roproto.RolloutPhase) error {
	_, err := sm.FindObject(kindDSCRollout, ro.Tenant, ro.Name)
	if err == nil {
		log.Errorf("DSCRollout {+%v} exists", ro)
		return fmt.Errorf("smartNICRollout already exists")
	}

	sros := DSCRolloutState{
		DSCRollout: ro,
		Statemgr:   sm,
		ros:        ros,
		status:     make(map[protos.DSCOp]protos.DSCOpStatus),
	}

	if snicStatus != nil {

		log.Infof("Spec for building is %+v", ros.Spec)
		log.Infof("SmartNIC Status %+v", snicStatus)

		var op, nextOp protos.DSCOp
		opStatus := "success"
		switch ros.Spec.UpgradeType {
		case roproto.RolloutSpec_Disruptive.String():
			op = protos.DSCOp_DSCPreCheckForDisruptive
			nextOp = protos.DSCOp_DSCDisruptiveUpgrade
		case roproto.RolloutSpec_OnNextHostReboot.String():
			op = protos.DSCOp_DSCPreCheckForUpgOnNextHostReboot
			nextOp = protos.DSCOp_DSCUpgOnNextHostReboot
		default:
			op = protos.DSCOp_DSCPreCheckForDisruptive
			nextOp = protos.DSCOp_DSCDisruptiveUpgrade
		}
		if snicStatus.Phase == roproto.RolloutPhase_FAIL.String() {
			opStatus = "failure"
			atomic.AddInt32(&ros.numPreUpgradeFailures, 1)
		}
		st := protos.DSCOpStatus{
			Op:       op,
			Version:  ros.Rollout.Spec.Version,
			OpStatus: opStatus,
		}
		sros.status[op] = st

		if snicStatus.Phase == roproto.RolloutPhase_PROGRESSING.String() {
			sros.Spec.Ops = append(sros.Spec.Ops, protos.DSCOpSpec{Op: nextOp, Version: ros.Rollout.Spec.Version})
		}
		if snicStatus.Phase == roproto.RolloutPhase_COMPLETE.String() {
			sros.Spec.Ops = append(sros.Spec.Ops, protos.DSCOpSpec{Op: nextOp, Version: ros.Rollout.Spec.Version})
			stNext := protos.DSCOpStatus{
				Op:       nextOp,
				Version:  ros.Rollout.Spec.Version,
				OpStatus: "success",
			}
			sros.status[nextOp] = stNext
			log.Infof("setting SmartNIC for %v with version %v", ro.Name, ros.Rollout.Spec.Version)
		}

	}

	sros.Mutex.Lock()
	// XXX validate parameters -
	if sros.GetObjectKind() != kindDSCRollout {
		sros.Mutex.Unlock()
		return fmt.Errorf("unexpected object kind %s", sros.GetObjectKind())
	}
	sros.Mutex.Unlock()

	sm.memDB.AddObject(&sros)
	return nil
}

// GetDSCRolloutState : Get the specified rollout state
func (sm *Statemgr) GetDSCRolloutState(tenant, name string) (*DSCRolloutState, error) {
	dbMs, err := sm.FindObject(kindDSCRollout, tenant, name)
	if err != nil {
		return nil, err
	}
	return dbMs.(*DSCRolloutState), nil
}

// DeleteDSCRolloutState - delete rollout
func (sm *Statemgr) DeleteDSCRolloutState(ro *protos.DSCRollout) {
	ros, err := sm.GetDSCRolloutState(ro.Tenant, ro.Name)
	if err != nil {
		log.Debugf("non-existent DSCRollout {%+v}. Err: %v", ro, err)
		return
	}

	log.Infof("Deleting DSCRollout %v", ros.DSCRollout.Name)
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(ros)
}

// DSCRolloutStateFromObj converts from memdb object to DSCRollout state
func DSCRolloutStateFromObj(obj memdb.Object) (*DSCRolloutState, error) {
	switch nsobj := obj.(type) {
	case *DSCRolloutState:
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// ListDSCRollouts lists all DSCRollout objects
func (sm *Statemgr) ListDSCRollouts() ([]*DSCRolloutState, error) {
	objs := sm.memDB.ListObjects(kindDSCRollout, nil)

	var smartNICUpdateStates []*DSCRolloutState
	for _, obj := range objs {
		nso, err := DSCRolloutStateFromObj(obj)
		if err != nil {
			return smartNICUpdateStates, err
		}

		smartNICUpdateStates = append(smartNICUpdateStates, nso)
	}
	sort.Sort(bySNICROSName(smartNICUpdateStates))

	return smartNICUpdateStates, nil
}

// UpdateDSCRolloutStatus - update status
func (snicState *DSCRolloutState) UpdateDSCRolloutStatus(newStatus *protos.DSCRolloutStatus) {

	log.Infof("Updating status %+v of DSCRollout %v", newStatus, snicState.DSCRollout.Name)
	if snicState.ros == nil {
		log.Infof("ROS State is null")
		for _, s := range newStatus.OpStatus {
			smartnicState, err := snicState.GetSmartNICState(snicState.Tenant, snicState.Name)
			if err != nil {
				log.Errorf("Error GetSmartNICState for {%+v}. Err: %v", snicState.Name, err)
				continue
			}
			if s.OpStatus == opStatusSuccess && s.Op == protos.DSCOp_DSCDisruptiveUpgrade {
				snicState.Statemgr.evtsRecorder.Event(eventtypes.ROLLOUT_SUCCESS, fmt.Sprintf("Force Rollout to version(%s) successful", s.Version), smartnicState.DistributedServiceCard)
			}
			if s.OpStatus != opStatusSuccess {
				snicState.Statemgr.evtsRecorder.Event(eventtypes.ROLLOUT_FAILED, fmt.Sprintf("Force Rollout to version(%s) failed", s.Version), smartnicState.DistributedServiceCard)
			}
			if s.Op == protos.DSCOp_DSCDisruptiveUpgrade {
				log.Infof("Received status disruptive upgrade. Delete DSCRollout Object")
				snicState.Statemgr.DeleteDSCRolloutState(snicState.DSCRollout)
			}
		}
		return
	}
	version := snicState.ros.Rollout.Spec.Version

	var phase roproto.RolloutPhase_Phases
	var reason, message string
	var updateStatus = false
	evt := fsmEvInvalid

	snicState.Mutex.Lock()
	for _, s := range newStatus.OpStatus {
		log.Infof("Updating OpStatus %+v", s)
		if s.Version != version {
			continue
		}

		found := false
		for _, specops := range snicState.Spec.Ops {
			if specops.Op == s.Op && s.Version == specops.Version {
				found = true
			}
		}
		if !found { // got status for some operation that we didnt even request for. ignore..
			log.Infof("Ignoring DSCstatus of %v", snicState.DSCRollout.Name)
			continue
		}

		existingStatus := snicState.status[s.Op]
		if existingStatus.OpStatus == s.OpStatus {
			log.Infof("Existing Status. so ignoring Status of %v", snicState.DSCRollout.Name)
			continue
		}
		if s.OpStatus == opStatusSkipped {
			atomic.AddUint32(&snicState.ros.numSkipped, 1)
		}
		updateStatus = true
		if s.OpStatus == opStatusSuccess || s.OpStatus == opStatusSkipped {
			switch s.Op {
			case protos.DSCOp_DSCPreCheckForDisruptive:
				evt = fsmEvOneSmartNICPreupgSuccess
				phase = roproto.RolloutPhase_WAITING_FOR_TURN
			case protos.DSCOp_DSCPreCheckForUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICPreupgSuccess
				phase = roproto.RolloutPhase_WAITING_FOR_TURN
			case protos.DSCOp_DSCUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICUpgSuccess
				phase = roproto.RolloutPhase_COMPLETE
				snicState.ros.completionSum += snicState.ros.completionDelta
				snicState.ros.Status.CompletionPercentage = uint32(snicState.ros.completionSum)
			case protos.DSCOp_DSCDisruptiveUpgrade:
				evt = fsmEvOneSmartNICUpgSuccess
				phase = roproto.RolloutPhase_COMPLETE
				snicState.ros.completionSum += snicState.ros.completionDelta
				snicState.ros.Status.CompletionPercentage = uint32(snicState.ros.completionSum)
			default:
				log.Errorf("Success for unknown Op %d from %s ", s.Op, snicState.DSCRollout.Name)
				return
			}
		} else {
			switch s.Op {
			case protos.DSCOp_DSCPreCheckForDisruptive:
				evt = fsmEvOneSmartNICPreupgFail
				atomic.AddInt32(&snicState.ros.numPreUpgradeFailures, 1)
				phase = roproto.RolloutPhase_FAIL
			case protos.DSCOp_DSCPreCheckForUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICPreupgFail
				atomic.AddInt32(&snicState.ros.numPreUpgradeFailures, 1)
				phase = roproto.RolloutPhase_FAIL
			case protos.DSCOp_DSCUpgOnNextHostReboot:
				atomic.AddUint32(&snicState.ros.numFailuresSeen, 1)
				evt = fsmEvOneSmartNICUpgFail
				phase = roproto.RolloutPhase_FAIL
			case protos.DSCOp_DSCDisruptiveUpgrade:
				atomic.AddUint32(&snicState.ros.numFailuresSeen, 1)
				evt = fsmEvOneSmartNICUpgFail
				phase = roproto.RolloutPhase_FAIL
			default:
				log.Errorf("Failure for unknown Op %d from %s ", s.Op, snicState.DSCRollout.Name)
				return
			}
		}
		snicState.status[s.Op] = s
		message = s.Message
		reason = s.OpStatus

		break
	}
	if updateStatus {
		if err := snicState.Statemgr.memDB.UpdateObject(snicState); err != nil {
			log.Errorf("smartNICUpdate Object for %s returned %s", snicState.Name, err.Error())
		}
	}
	snicState.Mutex.Unlock()
	if updateStatus {
		snicState.ros.setSmartNICPhase(snicState.Name, reason, message, phase)
		snicState.ros.eventChan <- evt
	}
}

func (snicState *DSCRolloutState) anyPendingOp() bool {
	snicState.Mutex.Lock()
	defer snicState.Mutex.Unlock()

	for _, s := range snicState.Spec.Ops {
		if st := snicState.status[s.Op]; st.OpStatus == "" {
			return true
		}
	}
	return false
}

func (snicState *DSCRolloutState) addSpecOp(version string, op protos.DSCOp) {

	snicState.Mutex.Lock()
	log.Infof("version %s and op %s update %+v of %+v", version, op, snicState.Spec.Ops, snicState.Name)
	for _, ops := range snicState.Spec.Ops {
		if ops.Op == op && ops.Version == version {
			log.Infof("version %s and op %s already exist for %v", version, op, snicState.ObjectMeta)
			snicState.Mutex.Unlock()
			return // version and op already exist
		}
	}
	snicState.Spec.Ops = []protos.DSCOpSpec{{Op: op, Version: version}}
	log.Infof("version %s and op %s update %+v", version, op, snicState.Spec.Ops)
	snicState.Mutex.Unlock()

	if err := snicState.Statemgr.memDB.UpdateObject(snicState); err != nil {
		log.Errorf("smartNICUpdate Object for %s returned %s", snicState.Name, err.Error())
	}
}
