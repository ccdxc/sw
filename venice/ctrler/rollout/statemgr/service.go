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

// ServiceRolloutState - Internal state for Rollout
type ServiceRolloutState struct {
	Mutex sync.Mutex
	*protos.ServiceRollout
	*Statemgr
	ros    *RolloutState
	status map[protos.ServiceOp]protos.ServiceOpStatus
}

// CreateServiceRolloutState to create a ServiceRollout Object in statemgr
func (sm *Statemgr) CreateServiceRolloutState(sro *protos.ServiceRollout, ros *RolloutState) error {
	_, err := sm.FindObject(kindServiceRollout, sro.Tenant, sro.Name)
	if err == nil {
		log.Errorf("ServiceRollout {+%v} exists", sro)
		return fmt.Errorf("ServiceRollout already exists")
	}

	sros := ServiceRolloutState{
		ServiceRollout: sro,
		Statemgr:       sm,
		ros:            ros,
		status:         make(map[protos.ServiceOp]protos.ServiceOpStatus),
	}

	sros.Mutex.Lock()
	// XXX validate parameters -
	if sros.GetObjectKind() != kindServiceRollout {
		sros.Mutex.Unlock()
		return fmt.Errorf("Unexpected object kind %s", sros.GetObjectKind())
	}
	sros.Mutex.Unlock()

	sm.memDB.AddObject(&sros)
	return nil
}

// GetServiceRolloutState : Get the specified rollout state
func (sm *Statemgr) GetServiceRolloutState(tenant, name string) (*ServiceRolloutState, error) {
	dbMs, err := sm.FindObject(kindServiceRollout, tenant, name)
	if err != nil {
		return nil, err
	}
	return dbMs.(*ServiceRolloutState), nil
}

// DeleteServiceRolloutState - delete rollout
func (sm *Statemgr) DeleteServiceRolloutState(ro *protos.ServiceRollout) {
	sros, err := sm.GetServiceRolloutState(ro.Tenant, ro.Name)
	if err != nil {
		log.Debugf("non-existent ServiceRollout {%+v}. Err: %v", ro, err)
		return
	}

	log.Infof("Deleting ServiceRollout %v", sros.ServiceRollout.Name)
	sros.Mutex.Lock()
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state
	sros.Mutex.Unlock()

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(sros)
}

// ServiceRolloutStateFromObj converts from memdb object to ServiceRollout state
func ServiceRolloutStateFromObj(obj memdb.Object) (*ServiceRolloutState, error) {
	switch obj.(type) {
	case *ServiceRolloutState:
		nsobj := obj.(*ServiceRolloutState)
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// ListServiceRollouts lists all ServiceRollout objects
func (sm *Statemgr) ListServiceRollouts() ([]*ServiceRolloutState, error) {
	objs := sm.memDB.ListObjects(kindServiceRollout)

	var nodeUpdateStates []*ServiceRolloutState
	for _, obj := range objs {
		nso, err := ServiceRolloutStateFromObj(obj)
		if err != nil {
			return nodeUpdateStates, err
		}

		nodeUpdateStates = append(nodeUpdateStates, nso)
	}
	sort.Sort(bySROSName(nodeUpdateStates))
	return nodeUpdateStates, nil
}

// UpdateServiceRolloutStatus - update status
func (sros *ServiceRolloutState) UpdateServiceRolloutStatus(newStatus *protos.ServiceRolloutStatus) {
	version := sros.ros.Rollout.Spec.Version

	log.Infof("Updating status of ServiceRollout %v", sros.ServiceRollout.Name)
	sros.Mutex.Lock()
	for _, s := range newStatus.OpStatus {
		if s.Version != version {
			continue
		}
		existingStatus := sros.status[s.Op]
		if existingStatus.OpStatus == s.OpStatus {
			continue
		}
		evt := fsmEvInvalid
		if s.OpStatus == "success" {
			switch s.Op {
			case protos.ServiceOp_ServiceRunVersion:
				evt = fsmEvServiceUpgOK
			}
		} else {
			switch s.Op {
			case protos.ServiceOp_ServiceRunVersion:
				evt = fsmEvServiceUpgFail
			}
		}
		sros.status[s.Op] = s
		if evt != fsmEvInvalid {
			sros.ros.eventChan <- evt
		}
	}
	sros.Statemgr.memDB.UpdateObject(sros)
	sros.Mutex.Unlock()
}
