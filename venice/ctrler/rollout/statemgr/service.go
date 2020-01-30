// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sort"
	"sync"

	roproto "github.com/pensando/sw/api/generated/rollout"
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
func (sm *Statemgr) CreateServiceRolloutState(sro *protos.ServiceRollout, ros *RolloutState, svcStatus *roproto.RolloutPhase) error {
	_, err := sm.FindObject(kindServiceRollout, sro.Tenant, sro.Name)
	if err == nil {
		log.Errorf("ServiceRollout {+%v} exists", sro)
		return fmt.Errorf("serviceRollout already exists")
	}
	var phase = roproto.RolloutPhase_PROGRESSING
	sros := ServiceRolloutState{
		ServiceRollout: sro,
		Statemgr:       sm,
		ros:            ros,
		status:         make(map[protos.ServiceOp]protos.ServiceOpStatus),
	}

	if svcStatus != nil {
		log.Infof("Spec for building serviceRollout is %+v", ros.Spec)
		log.Infof("Service Status %+v", svcStatus)

		if svcStatus.Phase == roproto.RolloutPhase_COMPLETE.String() {
			st := protos.ServiceOpStatus{
				Op:       protos.ServiceOp_ServiceRunVersion,
				Version:  ros.Spec.Version,
				OpStatus: "success",
			}
			sros.status[protos.ServiceOp_ServiceRunVersion] = st
			phase = roproto.RolloutPhase_COMPLETE
			log.Infof("setting ServiceRollout to success with version %v", ros.Rollout.Spec.Version)
		}
	}

	sros.Mutex.Lock()
	// XXX validate parameters -
	if sros.GetObjectKind() != kindServiceRollout {
		sros.Mutex.Unlock()
		return fmt.Errorf("unexpected object kind %s", sros.GetObjectKind())
	}
	sros.Mutex.Unlock()

	sm.memDB.AddObject(&sros)
	ros.setServicePhase(sro.Name, "", "", phase)
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
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(sros)
}

// ServiceRolloutStateFromObj converts from memdb object to ServiceRollout state
func ServiceRolloutStateFromObj(obj memdb.Object) (*ServiceRolloutState, error) {
	switch nsobj := obj.(type) {
	case *ServiceRolloutState:
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// ListServiceRollouts lists all ServiceRollout objects
func (sm *Statemgr) ListServiceRollouts() ([]*ServiceRolloutState, error) {
	objs := sm.memDB.ListObjects(kindServiceRollout, nil)

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
	sros.ros.stopRolloutTimer()

	var message, reason string
	var phase roproto.RolloutPhase_Phases
	var updateStatus = false

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
		updateStatus = true
		if s.OpStatus == "success" {
			if s.Op == protos.ServiceOp_ServiceRunVersion {
				evt = fsmEvServiceUpgOK
				phase = roproto.RolloutPhase_COMPLETE
				sros.ros.completionSum += float32(sros.ros.completionDelta * 2)
				sros.ros.Status.CompletionPercentage = uint32(sros.ros.completionSum)
			}
		} else {
			if s.Op == protos.ServiceOp_ServiceRunVersion {
				evt = fsmEvServiceUpgFail
				phase = roproto.RolloutPhase_FAIL
			}
		}
		sros.status[s.Op] = s
		if evt != fsmEvInvalid {
			sros.ros.eventChan <- evt
		}
	}
	if updateStatus {
		sros.Statemgr.memDB.UpdateObject(sros)
	}
	sros.Mutex.Unlock()
	if updateStatus {
		sros.ros.setServicePhase(sros.Name, reason, message, phase)
	}

}
