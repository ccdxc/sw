// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sort"
	"sync"

	"github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// VeniceRolloutState - Internal state for Rollout
type VeniceRolloutState struct {
	Mutex sync.Mutex
	*protos.VeniceRollout
	*Statemgr
	ros    *RolloutState
	status map[protos.VeniceOp]protos.VeniceOpStatus
}

// CreateVeniceRolloutState to create a VeniceRollout Object in statemgr
func (sm *Statemgr) CreateVeniceRolloutState(ro *protos.VeniceRollout, ros *RolloutState, nodeStatus *rollout.RolloutPhase) error {
	_, err := sm.FindObject(kindVeniceRollout, ro.Tenant, ro.Name)
	if err == nil {
		log.Errorf("VeniceRollout {+%v} exists", ro)
		return fmt.Errorf("veniceRollout already exists")
	}

	vros := VeniceRolloutState{
		VeniceRollout: ro,
		Statemgr:      sm,
		ros:           ros,
		status:        make(map[protos.VeniceOp]protos.VeniceOpStatus),
	}
	if nodeStatus != nil {
		log.Infof("Spec for building veniceRollout is %+v", ros.Spec)
		log.Infof("Venice Status %+v", nodeStatus)
		st := protos.VeniceOpStatus{
			Op:       protos.VeniceOp_VenicePreCheck,
			Version:  ros.Rollout.Spec.Version,
			OpStatus: "success",
		}
		vros.status[protos.VeniceOp_VenicePreCheck] = st

		if nodeStatus.Phase == rollout.RolloutPhase_PROGRESSING.String() {
			vros.Spec.Ops = append(vros.Spec.Ops, protos.VeniceOpSpec{Op: protos.VeniceOp_VeniceRunVersion, Version: ros.Rollout.Spec.Version})
		}
		if nodeStatus.Phase == rollout.RolloutPhase_COMPLETE.String() {
			vros.Spec.Ops = append(vros.Spec.Ops, protos.VeniceOpSpec{Op: protos.VeniceOp_VeniceRunVersion, Version: ros.Rollout.Spec.Version})
			stNext := protos.VeniceOpStatus{
				Op:       protos.VeniceOp_VeniceRunVersion,
				Version:  ros.Spec.Version,
				OpStatus: "success",
			}
			vros.status[protos.VeniceOp_VeniceRunVersion] = stNext
			log.Infof("setting VeniceRollout for %v with version %v", ro.Name, ros.Rollout.Spec.Version)
		}
	}
	log.Infof("Status for Venice %v Spec %+v Status %+v", vros.Name, vros.Spec.Ops, vros.status)
	// XXX validate parameters -
	ros.Mutex.Lock()
	if vros.GetObjectKind() != kindVeniceRollout {
		ros.Mutex.Unlock()
		return fmt.Errorf("unexpected object kind %s", vros.GetObjectKind())
	}
	ros.Mutex.Unlock()

	return sm.memDB.AddObject(&vros)
}

// GetVeniceRolloutState : Get the specified rollout state
func (sm *Statemgr) GetVeniceRolloutState(tenant, name string) (*VeniceRolloutState, error) {
	dbMs, err := sm.FindObject(kindVeniceRollout, tenant, name)
	if err != nil {
		return nil, err
	}
	return dbMs.(*VeniceRolloutState), nil
}

// DeleteVeniceRolloutState - delete rollout
func (sm *Statemgr) DeleteVeniceRolloutState(ro *protos.VeniceRollout) {
	ros, err := sm.GetVeniceRolloutState(ro.Tenant, ro.Name)
	if err != nil {
		log.Debugf("non-existent VeniceRollout {%+v}. Err: %v", ro, err)
		return
	}

	log.Infof("Deleting VeniceRollout %v", ros.VeniceRollout.Name)
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(ros)
}

// VeniceRolloutStateFromObj converts from memdb object to VeniceRollout state
func VeniceRolloutStateFromObj(obj memdb.Object) (*VeniceRolloutState, error) {
	switch nsobj := obj.(type) {
	case *VeniceRolloutState:
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// ListVeniceRollouts lists all VeniceRollout objects
func (sm *Statemgr) ListVeniceRollouts() ([]*VeniceRolloutState, error) {
	objs := sm.memDB.ListObjects(kindVeniceRollout, nil)

	var nodeUpdateStates []*VeniceRolloutState
	for _, obj := range objs {
		nso, err := VeniceRolloutStateFromObj(obj)
		if err != nil {
			return nodeUpdateStates, err
		}

		nodeUpdateStates = append(nodeUpdateStates, nso)
	}

	sort.Sort(byVROSName(nodeUpdateStates))
	return nodeUpdateStates, nil
}

// UpdateVeniceRolloutStatus - update status
func (vros *VeniceRolloutState) UpdateVeniceRolloutStatus(newStatus *protos.VeniceRolloutStatus) error {

	version := vros.ros.Rollout.Spec.Version

	log.Infof("Updating status of VeniceRollout %v newStatus %+v", vros.VeniceRollout.Name, newStatus)

	var message, reason string
	var phase rollout.RolloutPhase_Phases
	var updateStatus = false

	vros.Mutex.Lock()
	for _, s := range newStatus.OpStatus {
		if s.Version != version {
			continue
		}
		existingStatus := vros.status[s.Op]
		if existingStatus.OpStatus == s.OpStatus {
			continue
		}
		evt := fsmEvInvalid
		updateStatus = true
		if s.OpStatus == "success" {
			switch s.Op {
			case protos.VeniceOp_VenicePreCheck:
				evt = fsmEvOneVenicePreUpgSuccess
				phase = rollout.RolloutPhase_WAITING_FOR_TURN
				vros.ros.completionSum += vros.ros.completionDelta
				vros.ros.Status.CompletionPercentage = uint32(vros.ros.completionSum)
			case protos.VeniceOp_VeniceRunVersion:
				evt = fsmEvOneVeniceUpgSuccess
				phase = rollout.RolloutPhase_COMPLETE
				vros.ros.completionSum += vros.ros.completionDelta
				vros.ros.Status.CompletionPercentage = uint32(vros.ros.completionSum)

			}
		} else {
			switch s.Op {
			case protos.VeniceOp_VenicePreCheck:
				evt = fsmEvOneVenicePreUpgFail
				phase = rollout.RolloutPhase_FAIL
			case protos.VeniceOp_VeniceRunVersion:
				evt = fsmEvOneVeniceUpgFail
				phase = rollout.RolloutPhase_FAIL
			}
		}
		vros.status[s.Op] = s
		if evt != fsmEvInvalid {
			vros.ros.eventChan <- evt
			message = s.Message
			reason = s.OpStatus
		}
	}
	if updateStatus {
		vros.Statemgr.memDB.UpdateObject(vros)
	}
	vros.Mutex.Unlock()
	if updateStatus {
		vros.ros.setVenicePhase(vros.Name, reason, message, phase)
	}

	return nil
}
