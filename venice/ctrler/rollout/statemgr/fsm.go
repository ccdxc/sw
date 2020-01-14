package statemgr

import (
	"strconv"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/api"

	"github.com/pensando/sw/api/generated/rollout"

	"github.com/pensando/sw/venice/utils/log"
)

const defaultNumParallel = 2           // if user has not specified parallelism in Spec, we do do many SmartNICs in parallel. We can change this logic in future as needed..
const dSCTimeoutSeconds = 480          // 8 mins of timeout for DSC preUpgrade..
const rolloutPhasesTimeoutSeconds = 10 //timeout between rollout phases

var preUpgradeTimeout = dSCTimeoutSeconds * time.Second
var rolloutPhasesTimeout = rolloutPhasesTimeoutSeconds * time.Second
var veniceUpgradeTimeout = 15 * time.Minute
var rolloutRetryTimeout = 5 * time.Minute
var maxRetriesBeforeAbort uint32 = 5

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
	fsmstRetry
)

func (x rofsmState) String() string {
	switch x {
	case fsmstInvalid:
		return "fsmstInvalid"
	case fsmstStart:
		return "fsmstStart"
	case fsmstRetry:
		return "fsmstRetry"
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
	fsmEvRetry
	fsmEvRetryStart
)

func (x rofsmEvent) String() string {
	switch x {
	case fsmEvInvalid:
		return "fsmEvInvalid"
	case fsmEvROCreated:
		return "fsmEvROCreated"
	case fsmEvRetry:
		return "fsmEvRetry"
	case fsmEvRetryStart:
		return "fsmEvRetryStart"
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
		fsmEvFail:      {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
	},
	fsmstRetry: {
		fsmEvRetryStart: {nextSt: fsmstPreCheckingSmartNIC, actFn: fsmAcPreUpgSmartNIC},
		fsmEvSuspend:    {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
		fsmEvFail:       {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
	},

	fsmstPreCheckingVenice: {
		fsmEvOneVenicePreUpgSuccess: {nextSt: fsmstPreCheckingVenice, actFn: fsmAcOneVenicePreupgSuccess},
		fsmEvOneVenicePreUpgFail:    {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvAllVenicePreUpgOK:      {nextSt: fsmstPreCheckingSmartNIC, actFn: fsmAcPreUpgSmartNIC},
		fsmEvVeniceBypass:           {nextSt: fsmstPreCheckingSmartNIC, actFn: fsmAcPreUpgSmartNIC},
		fsmEvSuspend:                {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
		fsmEvFail:                   {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
	},
	fsmstPreCheckingSmartNIC: {
		fsmEvAllSmartNICPreUpgOK:      {nextSt: fsmstWaitForSchedule, actFn: fsmAcWaitForSchedule},
		fsmEvSomeSmartNICPreUpgFail:   {nextSt: fsmstWaitForSchedule, actFn: fsmAcWaitForSchedule},
		fsmEvOneSmartNICPreupgSuccess: {nextSt: fsmstPreCheckingSmartNIC},
		fsmEvOneSmartNICPreupgFail:    {nextSt: fsmstPreCheckingSmartNIC},
		fsmEvSuspend:                  {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},
	fsmstWaitForSchedule: {
		fsmEvScheduleNow:              {nextSt: fsmstRollingOutVenice, actFn: fsmAcIssueNextVeniceRollout},
		fsmEvSuspend:                  {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
		fsmEvOneSmartNICPreupgSuccess: {nextSt: fsmstWaitForSchedule},
		fsmEvOneSmartNICPreupgFail:    {nextSt: fsmstWaitForSchedule},
	},
	fsmstRollingOutVenice: {
		fsmEvOneVeniceUpgSuccess:      {nextSt: fsmstRollingOutVenice, actFn: fsmAcIssueNextVeniceRollout},
		fsmEvAllVeniceUpgOK:           {nextSt: fsmstRollingOutService, actFn: fsmAcIssueServiceRollout},
		fsmEvVeniceBypass:             {nextSt: fsmstRollingoutOutSmartNIC, actFn: fsmAcRolloutSmartNICs},
		fsmEvOneVeniceUpgFail:         {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvSuspend:                  {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
		fsmEvOneSmartNICPreupgSuccess: {nextSt: fsmstRollingOutVenice},
		fsmEvOneSmartNICPreupgFail:    {nextSt: fsmstRollingOutVenice},
	},
	fsmstRollingOutService: {
		fsmEvServiceUpgOK:             {nextSt: fsmstRollingoutOutSmartNIC, actFn: fsmAcRolloutSmartNICs},
		fsmEvSuspend:                  {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
		fsmEvOneSmartNICPreupgSuccess: {nextSt: fsmstRollingOutService},
		fsmEvFail:                     {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvOneSmartNICPreupgFail:    {nextSt: fsmstRollingOutService},
	},
	fsmstRollingoutOutSmartNIC: {
		fsmEvSuccess:               {nextSt: fsmstRolloutSuccess, actFn: fsmAcRolloutSuccess},
		fsmEvOneSmartNICUpgFail:    {nextSt: fsmstRollingoutOutSmartNIC}, // TODO
		fsmEvOneSmartNICUpgSuccess: {nextSt: fsmstRollingoutOutSmartNIC}, // TODO
		fsmEvFail:                  {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvSuspend:               {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
	},
	fsmstRolloutFail: {
		fsmEvRetry:   {nextSt: fsmstRetry, actFn: fsmAcRetry},
		fsmEvSuspend: {nextSt: fsmstRolloutSuspend, actFn: fsmAcRolloutSuspend},
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
			if ros.fsm[ros.currentState][evt].nextSt == fsmstInvalid {
				log.Infof("Unexpected Event %s in State %s . No action taken", evt, ros.currentState)
				continue
			}
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
	log.Infof("Completion Percentage %v", ros.Rollout.Status.CompletionPercentage)
	ros.computeProgressDelta()

	if ros.Spec.GetSuspend() {
		if ros.restart == false {
			log.Infof("Rollout object created with state SUSPENDED.")
			ros.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUSPENDED)]
			ros.eventChan <- fsmEvSuspend
			return
		}
		//controller node status found suspend_in_progress
		ros.Status.OperationalState = rollout.RolloutStatus_SUSPEND_IN_PROGRESS.String()
	}

	if ros.Spec.ScheduledStartTime == nil {
		ros.setStartTime()
	}

	if ros.Spec.DSCsOnly {
		ros.eventChan <- fsmEvVeniceBypass
	} else {
		ros.startRolloutTimer()

		// Check the venice health only for nodes for which the PreCheckIssue is pending
		veniceRollouts := ros.getVenicePendingPreCheckIssue()
		name, msg := ros.checkVeniceHealth(veniceRollouts)
		if msg != "" {
			log.Errorf("Precheck failed: %v", msg)
			ros.setVenicePhase(name, "", msg, rollout.RolloutPhase_FAIL)
			ros.eventChan <- fsmEvFail
			return
		}

		numPendingPrecheck, n, err := ros.preCheckNextVeniceNode()
		if err != nil {
			log.Errorf("Precheck failed: %v", err)
			ros.setVenicePhase(n, "", err.Error(), rollout.RolloutPhase_FAIL)
			ros.eventChan <- fsmEvFail
			return
		}
		if numPendingPrecheck == 0 {
			ros.eventChan <- fsmEvAllVenicePreUpgOK
		}
	}
}
func fsmAcOneVenicePreupgSuccess(ros *RolloutState) {
	ros.stopRolloutTimer()
	ros.startRolloutTimer()
	numPendingPrecheck, n, err := ros.preCheckNextVeniceNode()
	if err != nil {
		log.Errorf("Error %s issuing precheck to next venice", err)
		ros.setVenicePhase(n, "", err.Error(), rollout.RolloutPhase_FAIL)
		ros.eventChan <- fsmEvFail
		return
	}
	if numPendingPrecheck == 0 { // all venice have been issued pre-check
		if !ros.allVenicePreCheckSuccess() {
			ros.veniceRolloutFailed = true
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
	ros.Mutex.Lock()

	if ros.Spec.GetSuspend() && ros.restart == false {
		log.Infof("Rollout is SUSPENDED. Returning without further controller node Rollout.")
		ros.eventChan <- fsmEvSuspend
		ros.Mutex.Unlock()
		return
	}

	if ros.Spec.ScheduledStartTime == nil {
		ros.Spec.ScheduledStartTime = &api.Timestamp{}
		ros.Spec.ScheduledStartTime.Seconds = int64(time.Now().Second())
		ros.eventChan <- fsmEvScheduleNow
		ros.raiseRolloutEvent(rollout.RolloutStatus_PROGRESSING)
		if ros.Spec.GetSuspend() {
			ros.Status.OperationalState = rollout.RolloutStatus_SUSPEND_IN_PROGRESS.String()
		} else {
			ros.Status.OperationalState = rollout.RolloutStatus_PROGRESSING.String()
		}
		ros.saveStatus()
		ros.Mutex.Unlock()
		ros.updateRolloutAction()
		return
	}
	t, err := ros.Spec.ScheduledStartTime.Time()
	now := time.Now()
	if err != nil || now.After(t) { // specified time is in the past
		ros.eventChan <- fsmEvScheduleNow
		if ros.Spec.GetSuspend() {
			ros.Status.OperationalState = rollout.RolloutStatus_SUSPEND_IN_PROGRESS.String()
		} else {
			ros.raiseRolloutEvent(rollout.RolloutStatus_PROGRESSING)
			ros.Status.OperationalState = rollout.RolloutStatus_PROGRESSING.String()
		}
		ros.saveStatus()
		ros.Mutex.Unlock()
		ros.updateRolloutAction()

		return
	}
	ros.Status.OperationalState = rollout.RolloutStatus_SCHEDULED.String()
	ros.saveStatus()
	ros.Mutex.Unlock()

	for d := t.Sub(now); d.Seconds() > 0; d = time.Until(t) {
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
	ros.Mutex.Lock()
	ros.Status.OperationalState = rollout.RolloutStatus_PROGRESSING.String()
	ros.saveStatus()
	ros.Mutex.Unlock()
	ros.updateRolloutAction()

	ros.eventChan <- fsmEvScheduleNow
	ros.raiseRolloutEvent(rollout.RolloutStatus_PROGRESSING)
}

func fsmAcIssueNextVeniceRollout(ros *RolloutState) {

	if ros.Status.StartTime == nil {
		ros.setStartTime()
	}
	if ros.Spec.DSCsOnly {
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
			ros.veniceRolloutFailed = true
			ros.eventChan <- fsmEvFail
		} else {
			ros.eventChan <- fsmEvAllVeniceUpgOK
		}
	}
}
func fsmAcIssueServiceRollout(ros *RolloutState) {
	log.Infof("fsmAcIssueServiceRollout..")
	ros.stopRolloutTimer()
	//Add a small delay between rollout phases
	time.Sleep(rolloutPhasesTimeout * time.Second)
	ros.startRolloutTimer()
	serviceRolloutPending, err := ros.issueServiceRollout()
	if err != nil {
		log.Errorf("Error %s issuing service rollout", err)
		return
	}

	log.Infof("fsmAcIssueServiceRollout.. checking pending service Rollout")
	if !serviceRolloutPending {
		log.Infof("issue fsmEvServiceUpgOK")
		ros.stopRolloutTimer()
		ros.eventChan <- fsmEvServiceUpgOK
	}
	log.Infof("fsmAcIssueServiceRollout.. returning from fsmAcIssueServiceRollout")
}
func fsmAcRolloutSmartNICs(ros *RolloutState) {
	err := ros.writer.SetRolloutBuildVersion("")
	if err != nil {
		log.Errorf("Failed to set cluster.Version %s", err)
	}
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
		numSkipped := atomic.LoadUint32(&ros.numSkipped)
		log.Infof("Rollout numFailures %d numSkipped %d numPreUpgradeFailures %d", numFailures, numSkipped, ros.numPreUpgradeFailures)
		if numFailures == 0 && numSkipped == 0 && ros.numPreUpgradeFailures == 0 {
			ros.eventChan <- fsmEvSuccess
		} else {
			ros.eventChan <- fsmEvFail
		}
	}()
}
func fsmAcRetry(ros *RolloutState) {
	ros.retryTimer = time.AfterFunc(rolloutRetryTimeout, func() {
		log.Infof("Rollout retry begin")
		ros.numPreUpgradeFailures = 0
		ros.numSkipped = 0
		ros.numFailuresSeen = 0
		atomic.AddUint32(&ros.numRetries, 1)
		smartNICROs, err := ros.Statemgr.ListDSCRollouts()
		if err != nil {
			log.Errorf("Error %v listing DSCRollouts", err)
		} else {
			for _, v := range smartNICROs {
				ros.Statemgr.DeleteDSCRolloutState(v.DSCRollout)
			}
		}
		ros.Status.OperationalState = rollout.RolloutStatus_PRECHECK_IN_PROGRESS.String()
		ros.eventChan <- fsmEvRetryStart
	})
}
func fsmAcRolloutSuccess(ros *RolloutState) {
	ros.stopRolloutTimer()
	ros.stopRetryTimer()
	if ros.Status.StartTime != nil {
		ros.setEndTime()
	}
	ros.Mutex.Lock()
	ros.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUCCESS)]
	ros.Status.CompletionPercentage = 100
	ros.saveStatus()
	ros.currentState = fsmstRolloutSuccess
	ros.Mutex.Unlock()
	ros.updateRolloutAction()
	ros.raiseRolloutEvent(rollout.RolloutStatus_SUCCESS)
	ros.Statemgr.deleteRollouts()
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
	ros.stopRetryTimer()
	if ros.Status.StartTime != nil {
		ros.setEndTime()
	}

	if ros.checkIntendForRetry() {
		log.Infof("scheduled for retry..")
		ros.Mutex.Lock()
		ros.Status.OperationalState = rollout.RolloutStatus_SCHEDULED_FOR_RETRY.String()
		ros.saveStatus()
		ros.Mutex.Unlock()
		return
	}
	ros.Mutex.Lock()
	ros.Status.OperationalState = rollout.RolloutStatus_FAILURE.String()
	if ros.rolloutTimedout {
		ros.Status.Reason = "Rollout failed due to short maintenance window"
	}
	ros.saveStatus()
	ros.currentState = fsmstRolloutFail
	ros.Mutex.Unlock()
	ros.updateRolloutAction()
	ros.raiseRolloutEvent(rollout.RolloutStatus_FAILURE)
	ros.Statemgr.deleteRollouts()
	err := ros.writer.SetRolloutBuildVersion("")
	if err != nil {
		log.Errorf("Failed to set cluster.Version %s", err)
	}
	ros.stop()
}
func fsmAcRolloutSuspend(ros *RolloutState) {
	ros.stopRolloutTimer()
	ros.stopRetryTimer()
	if ros.Status.StartTime != nil {
		ros.setEndTime()
	}
	ros.Mutex.Lock()
	ros.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUSPENDED)]
	ros.saveStatus()
	ros.currentState = fsmstRolloutSuspend
	ros.Mutex.Unlock()
	ros.updateRolloutAction()
	ros.raiseRolloutEvent(rollout.RolloutStatus_SUSPENDED)
	ros.Statemgr.deleteRollouts()
	err := ros.writer.SetRolloutBuildVersion("")
	if err != nil {
		log.Errorf("Failed to set cluster.Version %s", err)
	}
	ros.stop()
}
