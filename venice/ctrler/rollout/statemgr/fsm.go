package statemgr

import (
	"strconv"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/api/generated/rollout"

	"github.com/pensando/sw/venice/utils/log"
)

const defaultNumParallel = 2 // if user has not specified parallelism in Spec, we do do many SmartNICs in parallel. We can change this logic in future as needed..

var preUpgradeTimeout = 480 * time.Second
var veniceUpgradeTimeout = 15 * time.Minute

type rofsmEvent uint
type rofsmState uint
type rofsmActionFunc func(ros *RolloutState)

const (
	fsmstInvalid rofsmState = iota
	fsmstStart
	fsmstPreCheckingVenice
	fsmstPreCheckingSmartNIC
	fsmstWaitForSchedule
	fsmstRollingOutVenice
	fsmstRollingOutService
	fsmstRollingoutOutSmartNIC
	fsmstRolloutSuccess
	fsmstRolloutPausing
	fsmstRolloutFail
	fsmstRolloutSuspend
)

func (x rofsmState) String() string {
	switch x {
	case fsmstInvalid:
		return "fsmstInvalid"
	case fsmstStart:
		return "fsmstStart"
	case fsmstPreCheckingVenice:
		return "fsmstPreCheckingVenice"
	case fsmstPreCheckingSmartNIC:
		return "fsmstPreCheckingSmartNIC"
	case fsmstWaitForSchedule:
		return "fsmstWaitForSchedule"
	case fsmstRollingOutVenice:
		return "fsmstRollingOutVenice"
	case fsmstRollingOutService:
		return "fsmstRollingOutService"
	case fsmstRollingoutOutSmartNIC:
		return "fsmstRollingoutOutSmartNIC"
	case fsmstRolloutSuccess:
		return "fsmstRolloutSuccess"
	case fsmstRolloutPausing:
		return "fsmstRolloutPausing"
	case fsmstRolloutFail:
		return "fsmstRolloutFail"
	case fsmstRolloutSuspend:
		return "fsmstRolloutSuspend"
	}
	return "unknownState " + strconv.Itoa(int(x))
}

const (
	fsmEvInvalid rofsmEvent = iota
	fsmEvROCreated
	fsmEvVeniceBypass // bypass venice rollout on user request
	fsmEvOneVenicePreUpgSuccess
	fsmEvOneVenicePreUpgFail
	fsmEvAllVenicePreUpgOK
	fsmEvOneSmartNICPreupgSuccess
	fsmEvOneSmartNICPreupgFail
	fsmEvAllSmartNICPreUpgOK
	fsmEvSomeSmartNICPreUpgFail
	fsmEvScheduleNow
	fsmEvOneVeniceUpgSuccess
	fsmEvOneVeniceUpgFail
	fsmEvAllVeniceUpgOK
	fsmEvServiceUpgFail
	fsmEvServiceUpgOK
	fsmEvOneSmartNICUpgSuccess
	fsmEvOneSmartNICUpgFail
	fsmEvFailThresholdReached
	fsmEvSuspend
	fsmEvFail
	fsmEvSuccess
)

func (x rofsmEvent) String() string {
	switch x {
	case fsmEvInvalid:
		return "fsmEvInvalid"
	case fsmEvROCreated:
		return "fsmEvROCreated"
	case fsmEvVeniceBypass:
		return "fsmEvVeniceBypass"
	case fsmEvOneVenicePreUpgSuccess:
		return "fsmEvOneVenicePreUpgSuccess"
	case fsmEvOneVenicePreUpgFail:
		return "fsmEvOneVenicePreUpgFail"
	case fsmEvAllVenicePreUpgOK:
		return "fsmEvAllVenicePreUpgOK"
	case fsmEvOneSmartNICPreupgSuccess:
		return "fsmEvOneSmartNICPreupgSuccess"
	case fsmEvOneSmartNICPreupgFail:
		return "fsmEvOneSmartNICPreupgFail"
	case fsmEvAllSmartNICPreUpgOK:
		return "fsmEvAllSmartNICPreUpgOK"
	case fsmEvSomeSmartNICPreUpgFail:
		return "fsmEvSomeSmartNICPreUpgFail"
	case fsmEvScheduleNow:
		return "fsmEvScheduleNow"
	case fsmEvOneVeniceUpgSuccess:
		return "fsmEvOneVeniceUpgSuccess"
	case fsmEvOneVeniceUpgFail:
		return "fsmEvOneVeniceUpgFail"
	case fsmEvAllVeniceUpgOK:
		return "fsmEvAllVeniceUpgOK"
	case fsmEvServiceUpgFail:
		return "fsmEvServiceUpgFail"
	case fsmEvServiceUpgOK:
		return "fsmEvServiceUpgOK"
	case fsmEvOneSmartNICUpgSuccess:
		return "fsmEvOneSmartNICUpgSuccess"
	case fsmEvOneSmartNICUpgFail:
		return "fsmEvOneSmartNICUpgFail"
	case fsmEvFailThresholdReached:
		return "fsmEvFailThresholdReached"
	case fsmEvSuspend:
		return "fsmEvSuspend"
	case fsmEvFail:
		return "fsmEvFail"
	case fsmEvSuccess:
		return "fsmEvSuccess"
	}
	return "unknownEvent " + strconv.Itoa(int(x))

}

type fsmNode struct {
	nextSt rofsmState
	actFn  rofsmActionFunc
}

var roFSM = [][]fsmNode{
	fsmstStart: {
		fsmEvROCreated: {nextSt: fsmstPreCheckingVenice, actFn: fsmAcCreated},
		fsmEvSuspend:   {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},

	fsmstPreCheckingVenice: {
		fsmEvOneVenicePreUpgSuccess: {nextSt: fsmstPreCheckingVenice, actFn: fsmAcOneVenicePreupgSuccess},
		fsmEvOneVenicePreUpgFail:    {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvAllVenicePreUpgOK:      {nextSt: fsmstPreCheckingSmartNIC, actFn: fsmAcPreUpgSmartNIC},
		fsmEvVeniceBypass:           {nextSt: fsmstPreCheckingSmartNIC, actFn: fsmAcPreUpgSmartNIC},
		fsmEvSuspend:                {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},
	fsmstPreCheckingSmartNIC: {
		fsmEvAllSmartNICPreUpgOK:      {nextSt: fsmstWaitForSchedule, actFn: fsmAcWaitForSchedule},
		fsmEvSomeSmartNICPreUpgFail:   {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvOneSmartNICPreupgSuccess: {nextSt: fsmstPreCheckingSmartNIC}, // TODO
		fsmEvOneSmartNICPreupgFail:    {nextSt: fsmstPreCheckingSmartNIC}, // TODO
		fsmEvSuspend:                  {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},
	fsmstWaitForSchedule: {
		fsmEvScheduleNow: {nextSt: fsmstRollingOutVenice, actFn: fsmAcIssueNextVeniceRollout},
		fsmEvSuspend:     {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},
	fsmstRollingOutVenice: {
		fsmEvOneVeniceUpgSuccess: {nextSt: fsmstRollingOutVenice, actFn: fsmAcIssueNextVeniceRollout},
		fsmEvAllVeniceUpgOK:      {nextSt: fsmstRollingOutService, actFn: fsmAcIssueServiceRollout},
		fsmEvVeniceBypass:        {nextSt: fsmstRollingoutOutSmartNIC, actFn: fsmAcRolloutSmartNICs},
		fsmEvOneVeniceUpgFail:    {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvSuspend:             {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},
	fsmstRollingOutService: {
		fsmEvServiceUpgOK: {nextSt: fsmstRollingoutOutSmartNIC, actFn: fsmAcRolloutSmartNICs},
		fsmEvSuspend:      {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},
	fsmstRollingoutOutSmartNIC: {
		fsmEvSuccess:               {nextSt: fsmstRolloutSuccess, actFn: fsmAcRolloutSuccess},
		fsmEvOneSmartNICUpgFail:    {nextSt: fsmstRollingoutOutSmartNIC}, // TODO
		fsmEvOneSmartNICUpgSuccess: {nextSt: fsmstRollingoutOutSmartNIC}, // TODO
		fsmEvFail:                  {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvSuspend:               {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},
}

func (ros *RolloutState) runFSM() {
	defer ros.Done()
	for {
		select {
		case <-ros.stopChan:
			return
		case evt := <-ros.eventChan:
			log.Infof("In State %s got Event %s", ros.currentState, evt)
			nstate := ros.fsm[ros.currentState][evt].nextSt
			action := ros.fsm[ros.currentState][evt].actFn
			if action != nil {
				log.Infof("calling action %v", action)
				action(ros)
			}
			ros.currentState = nstate
		}
	}
}

func fsmAcCreated(ros *RolloutState) {
	ros.Status.OperationalState = rollout.RolloutStatus_PRECHECK_IN_PROGRESS.String()
	ros.setPreviousVersion(ros.writer.GetClusterVersion())
	ros.Status.CompletionPercentage = 0
	ros.computeProgressDelta()

	if ros.Spec.GetSuspend() {
		log.Infof("Rollout object created with state SUSPENDED.")
		ros.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUSPENDED)]
		ros.eventChan <- fsmEvSuspend
		return
	}

	if ros.Spec.ScheduledStartTime == nil {
		ros.setStartTime()
	}

	if ros.Spec.SmartNICsOnly {
		ros.eventChan <- fsmEvVeniceBypass
	} else {
		ros.startRolloutTimer()
		numPendingPrecheck, err := ros.preCheckNextVeniceNode()
		if err == nil && numPendingPrecheck == 0 {
			ros.eventChan <- fsmEvAllVenicePreUpgOK
		}
	}
}
func fsmAcOneVenicePreupgSuccess(ros *RolloutState) {
	ros.stopRolloutTimer()
	ros.startRolloutTimer()
	numPendingPrecheck, err := ros.preCheckNextVeniceNode()
	if err != nil {
		log.Errorf("Error %s issuing precheck to next venice", err)
		return
	}
	if numPendingPrecheck == 0 { // all venice have been issued pre-check
		if !ros.allVenicePreCheckSuccess() {
			ros.eventChan <- fsmEvFail
		} else {
			ros.eventChan <- fsmEvAllVenicePreUpgOK
		}
	}
}

func fsmAcPreUpgSmartNIC(ros *RolloutState) {

	ros.stopRolloutTimer()
	ros.writer.WriteRollout(ros.Rollout)
	ros.Add(1)

	go func() {
		defer ros.Done()
		ros.preUpgradeSmartNICs()
		if ros.numPreUpgradeFailures == 0 {
			ros.eventChan <- fsmEvAllSmartNICPreUpgOK
		} else {
			ros.eventChan <- fsmEvSomeSmartNICPreUpgFail
		}
	}()
}

func fsmAcWaitForSchedule(ros *RolloutState) {
	if ros.Spec.ScheduledStartTime == nil {
		ros.eventChan <- fsmEvScheduleNow
		ros.raiseRolloutEvent(rollout.RolloutStatus_PROGRESSING)
		ros.Status.OperationalState = rollout.RolloutStatus_PROGRESSING.String()
		ros.saveStatus()
		return
	}
	t, err := ros.Spec.ScheduledStartTime.Time()
	now := time.Now()
	if err != nil || now.After(t) { // specified time is in the past
		ros.eventChan <- fsmEvScheduleNow
		ros.raiseRolloutEvent(rollout.RolloutStatus_PROGRESSING)
		ros.Status.OperationalState = rollout.RolloutStatus_PROGRESSING.String()
		ros.saveStatus()
		return
	}
	ros.Status.OperationalState = rollout.RolloutStatus_SCHEDULED.String()
	ros.saveStatus()

	for d := t.Sub(now); d.Seconds() > 0; d = t.Sub(time.Now()) {
		if d.Seconds() > 30 {
			time.Sleep(30 * time.Second)
		} else {
			time.Sleep(d)
		}
		if ros.Spec.GetSuspend() {
			ros.eventChan <- fsmEvSuspend
			return
		}
	}
	ros.Status.OperationalState = rollout.RolloutStatus_PROGRESSING.String()
	ros.saveStatus()
	ros.eventChan <- fsmEvScheduleNow
	ros.raiseRolloutEvent(rollout.RolloutStatus_PROGRESSING)
	return
}

func fsmAcIssueNextVeniceRollout(ros *RolloutState) {

	if ros.Spec.GetSuspend() {
		log.Infof("Rollout is SUSPENDED. Returning without further controller node Rollout.")
		ros.Status.OperationalState = rollout.RolloutStatus_SUSPENDED.String()
		ros.eventChan <- fsmEvSuspend
		return
	}
	if ros.Status.StartTime == nil {
		ros.setStartTime()
	}
	if ros.Spec.SmartNICsOnly {
		ros.eventChan <- fsmEvVeniceBypass
		return
	}
	//set the in progress rollout version
	err := ros.writer.SetRolloutBuildVersion(ros.Spec.Version)
	if err != nil {
		log.Errorf("Failed to set cluster.Version %v", err)
	}
	ros.stopRolloutTimer()
	ros.startRolloutTimer()
	numPendingRollout, err := ros.startNextVeniceRollout()
	if err != nil {
		log.Errorf("Error %s issuing rollout to next venice", err)
		return
	}

	if numPendingRollout == 0 {
		if !ros.allVeniceRolloutSuccess() { // some venice rollout failed
			ros.eventChan <- fsmEvFail
		} else {
			ros.eventChan <- fsmEvAllVeniceUpgOK
		}
	}
}
func fsmAcIssueServiceRollout(ros *RolloutState) {
	log.Infof("fsmAcIssueServiceRollout..")
	ros.stopRolloutTimer()
	serviceRolloutPending, err := ros.issueServiceRollout()
	if err != nil {
		log.Errorf("Error %s issuing service rollout", err)
		return
	}

	log.Infof("fsmAcIssueServiceRollout.. checking pending service Rollout")
	if !serviceRolloutPending {
		log.Infof("issue fsmEvServiceUpgOK")
		ros.eventChan <- fsmEvServiceUpgOK
	}
	log.Infof("fsmAcIssueServiceRollout.. returning from fsmAcIssueServiceRollout")
}
func fsmAcRolloutSmartNICs(ros *RolloutState) {
	ros.stopRolloutTimer()
	if ros.Spec.GetSuspend() {
		log.Infof("Rollout is SUSPENDED. Returning without smartNIC Rollout.")
		ros.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUSPENDED)]
		ros.eventChan <- fsmEvSuspend
		return
	}
	ros.writer.WriteRollout(ros.Rollout)
	ros.Add(1)
	go func() {
		defer ros.Done()
		ros.doUpdateSmartNICs()

		numFailures := atomic.LoadUint32(&ros.numFailuresSeen)
		if numFailures <= ros.Spec.MaxNICFailuresBeforeAbort {
			ros.eventChan <- fsmEvSuccess
		} else {
			ros.eventChan <- fsmEvFail
		}
	}()
}
func fsmAcRolloutSuccess(ros *RolloutState) {
	ros.stopRolloutTimer()
	ros.setEndTime()
	ros.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUCCESS)]
	ros.Status.CompletionPercentage = 100
	ros.saveStatus()
	ros.updateRolloutAction()
	ros.raiseRolloutEvent(rollout.RolloutStatus_SUCCESS)
	ros.Statemgr.deleteRollouts()
	ros.currentState = fsmstRolloutSuccess
	err := ros.writer.SetRolloutBuildVersion("")
	if err != nil {
		log.Errorf("Failed to set cluster.Version %s", err)
	}
	ros.stop()
}
func fsmAcRolloutFail(ros *RolloutState) {
	if ros.stateTimer != nil {
		ros.stopRolloutTimer()
	}
	ros.setEndTime()
	ros.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_FAILURE)]
	ros.saveStatus()
	ros.updateRolloutAction()
	ros.raiseRolloutEvent(rollout.RolloutStatus_FAILURE)
	ros.Statemgr.deleteRollouts()
	ros.currentState = fsmstRolloutFail
	err := ros.writer.SetRolloutBuildVersion("")
	if err != nil {
		log.Errorf("Failed to set cluster.Version %s", err)
	}
	ros.stop()
}
func fsmAcRolloutSuspend(ros *RolloutState) {
	ros.stopRolloutTimer()
	ros.setEndTime()
	ros.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUSPENDED)]
	ros.saveStatus()
	ros.updateRolloutAction()
	ros.raiseRolloutEvent(rollout.RolloutStatus_SUSPENDED)
	ros.Statemgr.deleteRollouts()
	ros.currentState = fsmstRolloutSuspend
	err := ros.writer.SetRolloutBuildVersion("")
	if err != nil {
		log.Errorf("Failed to set cluster.Version %s", err)
	}
	ros.stop()
}
