// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sort"
	"sync"

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
func (sm *Statemgr) CreateVeniceRolloutState(ro *protos.VeniceRollout, ros *RolloutState) error {
	_, err := sm.FindObject(kindVeniceRollout, ro.Tenant, ro.Name)
	if err == nil {
		log.Errorf("VeniceRollout {+%v} exists", ro)
		return fmt.Errorf("VeniceRollout already exists")
	}

	vros := VeniceRolloutState{
		VeniceRollout: ro,
		Statemgr:      sm,
		ros:           ros,
		status:        make(map[protos.VeniceOp]protos.VeniceOpStatus),
	}

	// XXX validate parameters -
	ros.Mutex.Lock()
	if vros.GetObjectKind() != kindVeniceRollout {
		ros.Mutex.Unlock()
		return fmt.Errorf("Unexpected object kind %s", vros.GetObjectKind())
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
	ros.Mutex.Lock()
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state
	ros.Mutex.Unlock()

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(ros)
}

// VeniceRolloutStateFromObj converts from memdb object to VeniceRollout state
func VeniceRolloutStateFromObj(obj memdb.Object) (*VeniceRolloutState, error) {
	switch obj.(type) {
	case *VeniceRolloutState:
		nsobj := obj.(*VeniceRolloutState)
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// ListVeniceRollouts lists all VeniceRollout objects
func (sm *Statemgr) ListVeniceRollouts() ([]*VeniceRolloutState, error) {
	objs := sm.memDB.ListObjects(kindVeniceRollout)

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

	log.Infof("Updating status of VeniceRollout %v", vros.VeniceRollout.Name)

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
		if s.OpStatus == "success" {
			switch s.Op {
			case protos.VeniceOp_VenicePreCheck:
				evt = fsmEvOneVenicePreUpgSuccess
			case protos.VeniceOp_VeniceRunVersion:
				evt = fsmEvOneVeniceUpgSuccess
			}

		} else {
			switch s.Op {
			case protos.VeniceOp_VenicePreCheck:
				evt = fsmEvOneVenicePreUpgFail
			case protos.VeniceOp_VeniceRunVersion:
				evt = fsmEvOneVeniceUpgFail
			}
		}
		vros.status[s.Op] = s
		if evt != fsmEvInvalid {
			vros.ros.eventChan <- evt
		}
	}
	vros.Statemgr.memDB.UpdateObject(vros)
	vros.Mutex.Unlock()

	return nil
}
