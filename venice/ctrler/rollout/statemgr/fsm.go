package statemgr

import (
	"strconv"
	"time"

	"github.com/pensando/sw/venice/utils/log"
)

const defaultNumParallel = 2 // if user has not specified parallelism in Spec, we do do many SmartNICs in parallel. We can change this logic in future as needed..

var preUpgradeTimeout = 45 * time.Second

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
	},

	fsmstPreCheckingVenice: {
		fsmEvOneVenicePreUpgSuccess: {nextSt: fsmstPreCheckingVenice, actFn: fsmAcOneVenicePreupgSuccess},
		fsmEvOneVenicePreUpgFail:    {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvAllVenicePreUpgOK:      {nextSt: fsmstPreCheckingSmartNIC, actFn: fsmAcPreUpgSmartNIC},
		fsmEvVeniceBypass:           {nextSt: fsmstPreCheckingSmartNIC, actFn: fsmAcPreUpgSmartNIC},
	},
	fsmstPreCheckingSmartNIC: {
		fsmEvAllSmartNICPreUpgOK:      {nextSt: fsmstWaitForSchedule, actFn: fsmAcWaitForSchedule},
		fsmEvSomeSmartNICPreUpgFail:   {nextSt: fsmstRolloutFail, actFn: fsmAcRolloutFail},
		fsmEvOneSmartNICPreupgSuccess: {nextSt: fsmstPreCheckingSmartNIC}, // TODO
	},
	fsmstWaitForSchedule: {
		fsmEvScheduleNow: {nextSt: fsmstRollingOutVenice, actFn: fsmAcIssueNextVeniceRollout},
	},
	fsmstRollingOutVenice: {
		fsmEvOneVeniceUpgSuccess: {nextSt: fsmstRollingOutVenice, actFn: fsmAcIssueNextVeniceRollout},
		fsmEvAllVeniceUpgOK:      {nextSt: fsmstRollingOutService, actFn: fsmAcIssueServiceRollout},
		fsmEvVeniceBypass:        {nextSt: fsmstRollingoutOutSmartNIC, actFn: fsmAcRolloutSmartNICs},
	},
	fsmstRollingOutService: {
		fsmEvServiceUpgOK: {nextSt: fsmstRollingoutOutSmartNIC, actFn: fsmAcRolloutSmartNICs},
	},
	fsmstRollingoutOutSmartNIC: {
		fsmEvSuccess:               {nextSt: fsmstRolloutSuccess, actFn: fsmAcRolloutSuccess},
		fsmEvOneSmartNICUpgFail:    {nextSt: fsmstRollingoutOutSmartNIC}, // TODO
		fsmEvOneSmartNICUpgSuccess: {nextSt: fsmstRollingoutOutSmartNIC}, // TODO

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
				action(ros)
			}
			ros.currentState = nstate
		}
	}
}

func fsmAcCreated(ros *RolloutState) {
	if ros.Spec.SmartNICsOnly {
		ros.eventChan <- fsmEvVeniceBypass
	} else {
		ros.preCheckNextVeniceNode()
	}
}
func fsmAcOneVenicePreupgSuccess(ros *RolloutState) {
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
		return
	}
	t, err := ros.Spec.ScheduledStartTime.Time()
	if err != nil {
		ros.eventChan <- fsmEvScheduleNow
		return
	}
	now := time.Now()
	if now.After(t) { // specified time is in the past
		ros.eventChan <- fsmEvScheduleNow
		return
	}
	d := t.Sub(now)
	time.Sleep(d)
	// TODO: Provide a way to cancel this when user Stops (or Deletes) Rollout
	ros.eventChan <- fsmEvScheduleNow
	return
}

func fsmAcIssueNextVeniceRollout(ros *RolloutState) {
	if ros.Spec.SmartNICsOnly {
		ros.eventChan <- fsmEvVeniceBypass
		return
	}
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
	serviceRolloutPending, err := ros.issueServiceRollout()
	if err != nil {
		log.Errorf("Error %s issuing service rollout", err)
		return
	}
	if !serviceRolloutPending {
		ros.eventChan <- fsmEvServiceUpgOK
	}
}
func fsmAcRolloutSmartNICs(ros *RolloutState) {
	ros.writer.WriteRollout(ros.Rollout)
	ros.Add(1)
	go func() {
		defer ros.Done()
		ros.doUpdateSmartNICs()
		// TODO: Handle Failures in smartNIC
		ros.eventChan <- fsmEvSuccess
	}()
}
func fsmAcRolloutSuccess(ros *RolloutState) {
	ros.writer.WriteRollout(ros.Rollout)
}
func fsmAcRolloutFail(ros *RolloutState) {
	ros.writer.WriteRollout(ros.Rollout)
}
