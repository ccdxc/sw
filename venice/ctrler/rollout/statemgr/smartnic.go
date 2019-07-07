// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sort"
	"sync"
	"sync/atomic"

	roproto "github.com/pensando/sw/api/generated/rollout"
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
func (sm *Statemgr) CreateSmartNICRolloutState(ro *protos.SmartNICRollout, ros *RolloutState, snicStatus *roproto.RolloutPhase) error {
	_, err := sm.FindObject(kindSmartNICRollout, ro.Tenant, ro.Name)
	if err == nil {
		log.Errorf("SmartNICRollout {+%v} exists", ro)
		return fmt.Errorf("smartNICRollout already exists")
	}

	sros := SmartNICRolloutState{
		SmartNICRollout: ro,
		Statemgr:        sm,
		ros:             ros,
		status:          make(map[protos.SmartNICOp]protos.SmartNICOpStatus),
	}

	if snicStatus != nil {

		log.Infof("Spec for building is %+v", ros.Spec)
		log.Infof("SmartNIC Status %+v", snicStatus)

		var op, nextOp protos.SmartNICOp
		switch ros.Spec.UpgradeType {
		case roproto.RolloutSpec_Disruptive.String():
			op = protos.SmartNICOp_SmartNICPreCheckForDisruptive
			nextOp = protos.SmartNICOp_SmartNICDisruptiveUpgrade
		case roproto.RolloutSpec_OnNextHostReboot.String():
			op = protos.SmartNICOp_SmartNICPreCheckForUpgOnNextHostReboot
			nextOp = protos.SmartNICOp_SmartNICUpgOnNextHostReboot
		default:
			op = protos.SmartNICOp_SmartNICPreCheckForDisruptive
			nextOp = protos.SmartNICOp_SmartNICDisruptiveUpgrade
		}

		st := protos.SmartNICOpStatus{
			Op:       op,
			Version:  ros.Rollout.Spec.Version,
			OpStatus: "success",
		}
		sros.status[op] = st

		if snicStatus.Phase == roproto.RolloutPhase_PROGRESSING.String() {
			sros.Spec.Ops = append(sros.Spec.Ops, protos.SmartNICOpSpec{Op: nextOp, Version: ros.Rollout.Spec.Version})
		}
		if snicStatus.Phase == roproto.RolloutPhase_COMPLETE.String() {
			sros.Spec.Ops = append(sros.Spec.Ops, protos.SmartNICOpSpec{Op: nextOp, Version: ros.Rollout.Spec.Version})
			stNext := protos.SmartNICOpStatus{
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
	if sros.GetObjectKind() != kindSmartNICRollout {
		sros.Mutex.Unlock()
		return fmt.Errorf("unexpected object kind %s", sros.GetObjectKind())
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
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(ros)
}

// SmartNICRolloutStateFromObj converts from memdb object to SmartNICRollout state
func SmartNICRolloutStateFromObj(obj memdb.Object) (*SmartNICRolloutState, error) {
	switch nsobj := obj.(type) {
	case *SmartNICRolloutState:
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

	log.Infof("Updating status %+v of SmartNICRollout %v", newStatus, snicState.SmartNICRollout.Name)
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
			continue
		}

		existingStatus := snicState.status[s.Op]
		if existingStatus.OpStatus == s.OpStatus {
			continue
		}
		updateStatus = true
		if s.OpStatus == "success" {
			switch s.Op {
			case protos.SmartNICOp_SmartNICPreCheckForDisruptive:
				evt = fsmEvOneSmartNICPreupgSuccess
				phase = roproto.RolloutPhase_WAITING_FOR_TURN
			case protos.SmartNICOp_SmartNICPreCheckForUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICPreupgSuccess
				phase = roproto.RolloutPhase_WAITING_FOR_TURN
			case protos.SmartNICOp_SmartNICUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICUpgSuccess
				phase = roproto.RolloutPhase_COMPLETE
				snicState.ros.Status.CompletionPercentage += uint32(snicState.ros.completionDelta)
			case protos.SmartNICOp_SmartNICDisruptiveUpgrade:
				evt = fsmEvOneSmartNICUpgSuccess
				phase = roproto.RolloutPhase_COMPLETE
				snicState.ros.Status.CompletionPercentage += uint32(snicState.ros.completionDelta)
			default:
				log.Errorf("Success for unknown Op %d from %s ", s.Op, snicState.SmartNICRollout.Name)
				return
			}
		} else {
			switch s.Op {
			case protos.SmartNICOp_SmartNICPreCheckForDisruptive:
				evt = fsmEvOneSmartNICPreupgFail
				atomic.AddInt32(&snicState.ros.numPreUpgradeFailures, 1)
				phase = roproto.RolloutPhase_FAIL
			case protos.SmartNICOp_SmartNICPreCheckForUpgOnNextHostReboot:
				evt = fsmEvOneSmartNICPreupgFail
				atomic.AddInt32(&snicState.ros.numPreUpgradeFailures, 1)
				phase = roproto.RolloutPhase_FAIL
			case protos.SmartNICOp_SmartNICUpgOnNextHostReboot:
				atomic.AddUint32(&snicState.ros.numFailuresSeen, 1)
				evt = fsmEvOneSmartNICUpgFail
				phase = roproto.RolloutPhase_FAIL
			case protos.SmartNICOp_SmartNICDisruptiveUpgrade:
				atomic.AddUint32(&snicState.ros.numFailuresSeen, 1)
				evt = fsmEvOneSmartNICUpgFail
				phase = roproto.RolloutPhase_FAIL
			default:
				log.Errorf("Failure for unknown Op %d from %s ", s.Op, snicState.SmartNICRollout.Name)
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
	log.Infof("version %s and op %s update %+v", version, op, snicState.Spec.Ops)
	for _, ops := range snicState.Spec.Ops {
		if ops.Op == op && ops.Version == version {
			log.Infof("version %s and op %s already exist for %v", version, op, snicState.ObjectMeta)
			snicState.Mutex.Unlock()
			return // version and op already exist
		}
	}
	snicState.Spec.Ops = []protos.SmartNICOpSpec{{Op: op, Version: version}}
	log.Infof("version %s and op %s update %+v", version, op, snicState.Spec.Ops)
	snicState.Mutex.Unlock()

	if err := snicState.Statemgr.memDB.UpdateObject(snicState); err != nil {
		log.Errorf("smartNICUpdate Object for %s returned %s", snicState.Name, err.Error())
	}
}
