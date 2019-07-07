// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"

	"github.com/pensando/sw/events/generated/eventtypes"

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
	stateTimer   *time.Timer

	fsm [][]fsmNode

	numPreUpgradeFailures int32 // number of failures seen so far (computed from status)
	numFailuresSeen       uint32
	completionDelta       float32
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
		sm.updateRolloutState(ro)

	case kvstore.Deleted:
		log.Infof("deleteRolloutState - %s\n", ro.Name)

		sm.deleteRolloutState(ro)
	}
}

// updateRolloutState to create a Rollout Object in statemgr
func (sm *Statemgr) updateRolloutState(ro *roproto.Rollout) error {
	// All parameters are validated (using apiserver hooks) by the time we get here
	ros, err := sm.GetRolloutState(ro.Tenant, ro.Name)
	if err != nil {
		log.Debugf("Error updating non-existent rollout {%+v}. Err: %v", ro, err)
		return err
	}

	log.Infof("Updating Rollout %v", ros.Rollout.Name)

	ros.Mutex.Lock()
	// XXX validate parameters -
	if ros.GetObjectKind() != kindRollout {
		ros.Mutex.Unlock()
		return fmt.Errorf("unexpected object kind %s", ros.GetObjectKind())
	}
	ros.Spec.Suspend = ro.Spec.Suspend
	ros.Status.OperationalState = ro.Status.OperationalState
	ros.Mutex.Unlock()
	sm.memDB.AddObject(ros)

	return nil
}

// isTerminalState check the status of rollout
func isTerminalState(ro *roproto.Rollout) bool {
	opState := ro.Status.GetOperationalState()
	if opState == roproto.RolloutStatus_FAILURE.String() ||
		opState == roproto.RolloutStatus_SUCCESS.String() ||
		opState == roproto.RolloutStatus_SUSPENDED.String() {
		return true
	}
	return false
}

// createRolloutState to create a Rollout Object in statemgr
func (sm *Statemgr) createRolloutState(ro *roproto.Rollout) error {
	// All parameters are validated (using apiserver hooks) by the time we get here
	_, err := sm.FindObject(kindRollout, ro.Tenant, ro.Name)
	if err == nil {
		log.Errorf("Rollout {+%v} exists", ro)
		return fmt.Errorf("rollout already exists")
	}
	//On process restart we get create events for rollouts
	//For the historical rollout obejcts we do nothing
	if isTerminalState(ro) {
		log.Infof("Rollout {+%v} in terminal state. Nothing more to do.", ro)
		return nil
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
		return fmt.Errorf("unexpected object kind %s", ros.GetObjectKind())
	}
	ros.Mutex.Unlock()

	for _, nodeStatus := range ro.Status.ControllerNodesStatus {
		veniceRollouts := ros.getVenicePendingPreCheckIssue()

		for _, n := range veniceRollouts {
			if n != nodeStatus.Name {
				log.Infof("Status %s doesnt match rollout Name %s", nodeStatus.Name, n)
				continue
			}
			log.Infof("Creating veniceRollout for %s", n)

			veniceRollout := protos.VeniceRollout{
				TypeMeta: api.TypeMeta{
					Kind: "VeniceRollout",
				},
				ObjectMeta: api.ObjectMeta{
					Name: n,
				},
				Spec: protos.VeniceRolloutSpec{
					Ops: []protos.VeniceOpSpec{
						{
							Op:      protos.VeniceOp_VenicePreCheck,
							Version: ro.Spec.Version,
						},
					},
				},
			}
			err := sm.CreateVeniceRolloutState(&veniceRollout, &ros, nodeStatus)
			if err != nil {
				log.Errorf("Error %v creating venice rollout state", err)
				return err
			}
		}
	}

	for _, snicStatus := range ro.Status.SmartNICsStatus {

		var op protos.SmartNICOp
		switch ros.Spec.UpgradeType {
		case roproto.RolloutSpec_Disruptive.String():
			op = protos.SmartNICOp_SmartNICPreCheckForDisruptive
		case roproto.RolloutSpec_OnNextHostReboot.String():
			op = protos.SmartNICOp_SmartNICPreCheckForUpgOnNextHostReboot
		default:
			op = protos.SmartNICOp_SmartNICPreCheckForDisruptive
		}

		snStates, err := sm.ListSmartNICs()
		if err != nil {
			log.Errorf("Error %v listing smartNICs", err)
			return err
		}
		sn := orderSmartNICs(ros.Rollout.Spec.OrderConstraints, ros.Rollout.Spec.SmartNICMustMatchConstraint, snStates)

		for _, s := range sn {
			for _, snicState := range s {
				// smartNICRollout Create
				if snicState.Name != snicStatus.Name {
					log.Infof("mismatch in names stateName %s statusName %s", snicState.Name, snicStatus.Name)
					continue
				}

				log.Infof("Creating smartNICRollout State for %s snicState %+v Tenant %s", snicStatus.Name, snicState, snicState.Tenant)
				snicRollout := protos.SmartNICRollout{
					TypeMeta: api.TypeMeta{
						Kind: kindSmartNICRollout,
					},
					ObjectMeta: api.ObjectMeta{
						Name:   snicState.Name,
						Tenant: snicState.Tenant,
					},
					Spec: protos.SmartNICRolloutSpec{
						Ops: []protos.SmartNICOpSpec{
							{
								Op:      op,
								Version: ros.Rollout.Spec.Version,
							},
						},
					},
				}

				log.Infof("Creating smartNICRolloutState %#v", snicRollout)

				err = sm.CreateSmartNICRolloutState(&snicRollout, &ros, snicStatus)
				if err != nil {
					log.Errorf("Error %v creating smartnic rollout state", err)
					return err
				}
			}
		}

	}
	for _, svcStatus := range ro.Status.ControllerServicesStatus {

		log.Infof("Creating serviceRollout for %s", svcStatus.Name)

		serviceRollout := protos.ServiceRollout{
			TypeMeta: api.TypeMeta{
				Kind: kindServiceRollout,
			},
			ObjectMeta: api.ObjectMeta{
				Name: "serviceRollout",
			},
			Spec: protos.ServiceRolloutSpec{
				Ops: []protos.ServiceOpSpec{
					{
						Op:      protos.ServiceOp_ServiceRunVersion,
						Version: ros.Rollout.Spec.Version,
					},
				},
			},
		}
		log.Infof("Creating serviceRollout")
		err = sm.CreateServiceRolloutState(&serviceRollout, &ros, svcStatus)
		if err != nil {
			log.Errorf("Error %v creating service rollout state", err)
			return err
		}
	}

	ros.Status.CompletionPercentage = ro.Status.CompletionPercentage
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
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state

	// delete rollout state from DB
	_ = sm.memDB.DeleteObject(ros)
	sm.deleteRollouts()
}

func (ros *RolloutState) startRolloutTimer() {

	ros.stateTimer = time.AfterFunc(veniceUpgradeTimeout, func() {
		log.Errorf("Timeout during venice rollout \n")
		if ros.currentState == fsmstRollingOutVenice {
			phase := roproto.RolloutPhase_FAIL
			for _, curStatus := range ros.Status.ControllerNodesStatus {
				if curStatus.Phase == roproto.RolloutPhase_PROGRESSING.String() {
					ros.setVenicePhase(curStatus.Name, "", "Timeout waiting for status from Venice", phase)
				}
			}
			ros.eventChan <- fsmEvOneVeniceUpgFail
		}
		if ros.currentState == fsmstPreCheckingVenice {
			phase := roproto.RolloutPhase_FAIL
			for _, curStatus := range ros.Status.ControllerNodesStatus {
				if curStatus.Phase == roproto.RolloutPhase_PRE_CHECK.String() {
					ros.setVenicePhase(curStatus.Name, "", "Timeout waiting for status from Venice", phase)
				}
			}
			ros.eventChan <- fsmEvOneVenicePreUpgFail
		}
	})
}

func (ros *RolloutState) stopRolloutTimer() {
	if ros.stateTimer != nil {
		ros.stateTimer.Stop()
		ros.stateTimer = nil
	}
}

func (ros *RolloutState) start() {
	ros.currentState = fsmstStart
	ros.Add(1)
	go ros.runFSM()
	ros.eventChan <- fsmEvROCreated
}

func (ros *RolloutState) stop() {
	if ros.stopped {
		return
	}
	ros.stopped = true
	close(ros.stopChan)
	ros.Wait()
}

// === Status updaters ===
func (ros *RolloutState) raiseRolloutEvent(status roproto.RolloutStatus_RolloutOperationalState) {

	if ros.Statemgr.evtsRecorder == nil {
		log.Infof("Event recorder not found")
		return
	}
	switch status {
	case roproto.RolloutStatus_SUCCESS:
		ros.Statemgr.evtsRecorder.Event(eventtypes.ROLLOUT_SUCCESS, fmt.Sprintf("Rollout(%s) to version(%s) completed successfully", ros.Rollout.Name, ros.Rollout.Spec.Version), ros.Rollout)
	case roproto.RolloutStatus_SUSPENDED:
		ros.Statemgr.evtsRecorder.Event(eventtypes.ROLLOUT_SUSPENDED, fmt.Sprintf("Rollout(%s) to version(%s) suspended", ros.Rollout.Name, ros.Rollout.Spec.Version), ros.Rollout)
	case roproto.RolloutStatus_FAILURE:
		ros.Statemgr.evtsRecorder.Event(eventtypes.ROLLOUT_FAILED, fmt.Sprintf("Rollout(%s) to version(%s) failed", ros.Rollout.Name, ros.Rollout.Spec.Version), ros.Rollout)
	case roproto.RolloutStatus_PROGRESSING:
		ros.Statemgr.evtsRecorder.Event(eventtypes.ROLLOUT_STARTED, fmt.Sprintf("Rollout(%s) to version(%s) started", ros.Rollout.Name, ros.Rollout.Spec.Version), ros.Rollout)
	}
}
func (ros *RolloutState) saveStatus() {
	if ros != nil && ros.writer != nil {
		ros.writer.WriteRollout(ros.Rollout)
	}
}
func (ros *RolloutState) updateRolloutAction() {
	if ros != nil && ros.writer != nil {
		ros.writer.WriteRolloutAction(ros.Rollout)
	}
}
func (ros *RolloutState) setPreviousVersion(v string) {
	if ros.Status.PreviousVersion == "" {
		ros.Status.PreviousVersion = v
		ros.saveStatus()
	}
}
func (ros *RolloutState) getRolloutTimeStamps(nodeName string) (startTime, endTime *api.Timestamp) {
	apicl, err := ros.writer.GetAPIClient()
	if apicl == nil || err != nil {
		log.Errorf("Failed to get API Client %v", err)
		return nil, nil
	}
	obj := api.ObjectMeta{
		Name: ros.Name,
	}

	for ii := 0; ii < 30; ii++ {
		roObj, err := apicl.RolloutV1().Rollout().Get(context.Background(), &obj)
		if err != nil {
			log.Infof("Rollout Get Failed %v", err)
			time.Sleep(time.Second)
			continue
		}
		for _, obj := range roObj.Status.ControllerNodesStatus {
			if obj.Name == nodeName {
				log.Infof("Found Status: StartTime (%v) EndTime (%v)", obj.StartTime, obj.EndTime)
				return obj.StartTime, obj.EndTime
			}
		}
	}
	log.Infof("No rollot status for %s", nodeName)
	return nil, nil
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
		stTime, _ := ros.getRolloutTimeStamps(ros.Status.ControllerNodesStatus[index].Name)
		if stTime == nil {
			ros.Status.ControllerNodesStatus[index].StartTime = &startTime
		}

	case roproto.RolloutPhase_COMPLETE, roproto.RolloutPhase_FAIL:
		endTime := api.Timestamp{}
		endTime.SetTime(time.Now())
		_, eTime := ros.getRolloutTimeStamps(ros.Status.ControllerNodesStatus[index].Name)
		if eTime == nil {
			ros.Status.ControllerNodesStatus[index].EndTime = &endTime
		}

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
	ros.Mutex.Lock()
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
		if ros.Status.SmartNICsStatus[index].StartTime != nil {
			endTime := api.Timestamp{}
			endTime.SetTime(time.Now())
			ros.Status.SmartNICsStatus[index].EndTime = &endTime
		}

	case roproto.RolloutPhase_PRE_CHECK:
	case roproto.RolloutPhase_DEPENDENCIES_CHECK:
	case roproto.RolloutPhase_WAITING_FOR_TURN:

	}
	ros.Status.SmartNICsStatus[index].Reason = reason
	ros.Status.SmartNICsStatus[index].Message = message
	ros.Status.SmartNICsStatus[index].Phase = phase.String()
	ros.saveStatus()
	ros.Mutex.Unlock()
}
