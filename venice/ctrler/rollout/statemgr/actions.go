package statemgr

import (
	"fmt"
	"sync"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/api/generated/cluster"

	"github.com/davecgh/go-spew/spew"

	"github.com/pensando/sw/api"
	roproto "github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

const (
	opStatusSkipped = "skipped"
	opStatusSuccess = "success"
)

// ==== helper routines

func getVenicePendingRolloutIssue(version string, veniceRollouts []*VeniceRolloutState) []string {
	var pendingVenice []string
	for _, v := range veniceRollouts {
		found := false
		for _, ops := range v.Spec.Ops {
			if ops.Op == protos.VeniceOp_VeniceRunVersion && ops.Version == version {
				found = true
			}
		}
		if !found {
			pendingVenice = append(pendingVenice, v.ObjectMeta.Name)
		}
	}
	return pendingVenice
}

func isPrecheckInProgress(op protos.DSCOp) bool {
	if op == protos.DSCOp_DSCPreCheckForDisruptive ||
		op == protos.DSCOp_DSCPreCheckForUpgOnNextHostReboot {
		return true
	}
	return false
}

func checkComplete(dscName string, ros *RolloutState) bool {
	for _, dscStatus := range ros.Status.DSCsStatus {
		if dscStatus.Name == dscName && dscStatus.Phase == roproto.RolloutPhase_COMPLETE.String() {
			return true
		}
	}
	return false
}

func verifyAdmittedDSCState(dscState *SmartNICState, ros *RolloutState, op protos.DSCOp) bool {

	var phase roproto.RolloutPhase_Phases
	if op == protos.DSCOp_DSCPreCheckForUpgOnNextHostReboot || op == protos.DSCOp_DSCPreCheckForDisruptive {
		phase = roproto.RolloutPhase_PRE_CHECK
	} else {
		phase = roproto.RolloutPhase_PROGRESSING
	}
	numRetries := atomic.LoadUint32(&ros.numRetries)

	//if Admitted and Version is same
	if dscState.Status.DSCVersion == ros.Spec.Version {
		// not retry: skip DSC and set phase
		if numRetries == 0 {
			//this is counted as successful rollout so set the status
			ros.setSmartNICPhase(dscState.Name, opStatusSkipped, "Skipped DSC from upgrade: DSC running same version", roproto.RolloutPhase_COMPLETE)
		} else {
			//retry mode and same version, no need to to setPhase
			log.Infof("SKIPDSC: Retry mode and running same version nothing to do.")
		}
		return false
	}
	//if Admitted and Healthy but unknown Status : skip DSC
	for _, condition := range dscState.Status.Conditions {
		log.Debugf("Condition Status %+v Type %v", condition.Status, condition.Type)
		if condition.Type == cluster.NodeCondition_HEALTHY.String() && condition.Status == cluster.ConditionStatus_UNKNOWN.String() {
			//setting the phase to FAIL so that rollout retry counter is incremented
			//roFSM is not triggered for skipped DSC so set the failed counter
			atomic.AddUint32(&ros.numSkipped, 1)
			//take care of the case where status become unreachable after the precheck
			updateState := false
			for _, dscStatus := range ros.Status.DSCsStatus {
				if dscStatus.Name == dscState.Name && dscStatus.Phase == roproto.RolloutPhase_WAITING_FOR_TURN.String() {
					updateState = true
				}
			}
			if phase == roproto.RolloutPhase_PRE_CHECK || updateState {
				ros.setSmartNICPhase(dscState.Name, opStatusSkipped, "Skipped DSC from upgrade: DSC Unreachable", roproto.RolloutPhase_FAIL)
			}
			return false
		}
	}
	//version is different and upgradeType is NextHostReboot and retry and status is complete - do nothing
	if ros.Spec.UpgradeType == roproto.RolloutSpec_OnNextHostReboot.String() && numRetries > 0 && checkComplete(dscState.Name, ros) {
		log.Infof("SKIPDSC: Spec is OnNextReboot && status is complete for dsc %+v", dscState.Name)
		return false
	}
	//Admitted, version is different, healthy, status is true
	return true
}

// This function bins the smartNICs based on the user-requested labels and returns the slice of bins
// Each bin of smartnics are expected to complete in that order
func orderSmartNICs(labelSels []*labels.Selector, smartNICMustMatchConstraint bool, sn []*SmartNICState, ros *RolloutState, op protos.DSCOp) [][]*SmartNICState {
	retval := make([][]*SmartNICState, 0)
	curbin := make([]*SmartNICState, 0)

	snics := make(map[string]*SmartNICState)
	for _, s := range sn {
		snics[s.Name] = s
	}

	for _, ls := range labelSels {
		for name, s := range snics {
			if ls.Matches(labels.Set(s.ObjectMeta.Labels)) {
				log.Debugf("SmartNIC Phase is %+v", s.Status.AdmissionPhase)
				if s.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String() && verifyAdmittedDSCState(s, ros, op) {
					curbin = append(curbin, s)
					delete(snics, name)
				}
			}
		}
		if len(curbin) > 0 {
			retval = append(retval, curbin)
			curbin = make([]*SmartNICState, 0)
		}
	}
	if !smartNICMustMatchConstraint {
		// add the remaining SNICs
		for _, s := range snics {
			log.Debugf("SmartNIC Phase is %+v", s.Status.AdmissionPhase)
			if s.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String() && verifyAdmittedDSCState(s, ros, op) {
				curbin = append(curbin, s)
			}
		}
		if len(curbin) > 0 {
			retval = append(retval, curbin)
		}
	}
	return retval
}

func (ros *RolloutState) getVenicePendingPreCheckIssue() []string {
	sm := ros.Statemgr
	version := ros.Spec.Version

	var pendingVenice []string
	nodeStates, err := sm.ListNodes()
	if err != nil {
		log.Errorf("Error %v listing nodes", err)
		return []string{}
	}
	veniceRollouts, err := sm.ListVeniceRollouts()
	if err != nil {
		log.Errorf("Error %v listing VeniceRollouts", err)
		return []string{}
	}
	issuedVenices := make(map[string]bool)
	for _, v := range veniceRollouts {
		found := false
		for _, ops := range v.Spec.Ops {
			if ops.Op == protos.VeniceOp_VenicePreCheck && ops.Version == version {
				found = true
			}
		}
		if found {
			issuedVenices[v.Name] = true
		}
	}
	for _, n := range nodeStates {
		log.Debugf("Status of node %s is %+v", n.Name, n.Status.Phase)
		if !issuedVenices[n.ObjectMeta.Name] && n.Status.Phase == cluster.NodeStatus_JOINED.String() {
			pendingVenice = append(pendingVenice, n.ObjectMeta.Name)
		}
	}
	log.Infof("Rollout on pendingVenices %+v and issuedVenices %+v", pendingVenice, issuedVenices)
	return pendingVenice
}

func (ros *RolloutState) allVenicePreCheckSuccess() bool {
	sm := ros.Statemgr
	veniceROs, err := sm.ListVeniceRollouts()
	if err != nil {
		log.Errorf("Error %v listing VeniceRollouts", err)
		return false
	}
	for _, v := range veniceROs {
		if v.status[protos.VeniceOp_VenicePreCheck].OpStatus != "success" {
			return false
		}
	}

	return true
}

func (ros *RolloutState) allVeniceRolloutSuccess() bool {
	sm := ros.Statemgr

	veniceROs, err := sm.ListVeniceRollouts()
	if err != nil {
		log.Errorf("Error %v listing VeniceRollouts", err)
		return false
	}
	for _, v := range veniceROs {
		if v.status[protos.VeniceOp_VeniceRunVersion].OpStatus != "success" {
			return false
		}
	}

	return true
}

// send actual upgrade to venice node - the precheck is already done by this time.
// returns the  number of venice for which rollout has been issued and status not obtained yet
func (ros *RolloutState) startNextVeniceRollout() (int, error) {
	sm := ros.Statemgr
	version := ros.Spec.Version

	pendingStatus := ros.getVenicePendingRolloutStatus()
	if len(pendingStatus) > 0 {
		// some node has been issued rollout spec and we are waiting for status update.
		// nothing to do now
		return len(pendingStatus), nil
	}

	// all nodes which have been issued the request so far have responded.
	// if there is any node which has not yet been issued, issue it now

	veniceROs, err := sm.ListVeniceRollouts()
	if err != nil {
		log.Errorf("Error %v listing VeniceRollouts", err)
		return len(pendingStatus), err
	}
	pendingVenice := getVenicePendingRolloutIssue(version, veniceROs)
	if len(pendingVenice) == 0 {
		return 0, nil
	}

	selectedVenice := pendingVenice[0]
	for _, v := range veniceROs {
		if v.Name != selectedVenice {
			continue
		}
		v.Spec.Ops = append(v.Spec.Ops, protos.VeniceOpSpec{Op: protos.VeniceOp_VeniceRunVersion, Version: version})

		log.Debugf("setting VeniceRollout for %v with version %v", v.Name, version)
		err = sm.memDB.UpdateObject(v)
		if err == nil {
			log.Infof("Setting Rollout Status %#v", v.status[protos.VeniceOp_VenicePreCheck])
			ros.setVenicePhase(v.Name, v.status[protos.VeniceOp_VenicePreCheck].OpStatus, v.status[protos.VeniceOp_VenicePreCheck].Message, roproto.RolloutPhase_PROGRESSING)
		}
		return 1, err

	}
	return 0, fmt.Errorf("unexpected error - unknown venice %s selected for next rollout", selectedVenice)
}

// Precheck has been issued but status not received so far
func (ros *RolloutState) getVenicePendingPrecheckStatus() []string {
	sm := ros.Statemgr
	version := ros.Spec.Version

	var pendingVenice []string
	veniceRollouts, err := sm.ListVeniceRollouts()
	if err != nil {
		log.Errorf("Error %v listing VeniceRollouts", err)
		return []string{}
	}
	for _, v := range veniceRollouts {
		found := false
		for _, ops := range v.Spec.Ops {
			if ops.Op == protos.VeniceOp_VenicePreCheck && ops.Version == version {
				found = true
			}
		}
		if !found { // not issued for this venice
			continue
		}
		if v.status[protos.VeniceOp_VenicePreCheck].OpStatus == "" {
			pendingVenice = append(pendingVenice, v.Name)
		}
	}
	return pendingVenice
}

// Rollout has been issued but status not received so far
func (ros *RolloutState) getVenicePendingRolloutStatus() []string {
	sm := ros.Statemgr
	version := ros.Spec.Version

	var pendingVenice []string
	veniceRollouts, err := sm.ListVeniceRollouts()
	if err != nil {
		log.Errorf("Error %v listing VeniceRollouts", err)
		return []string{}
	}
	for _, v := range veniceRollouts {
		found := false
		for _, ops := range v.Spec.Ops {
			if ops.Op == protos.VeniceOp_VeniceRunVersion && ops.Version == version {
				found = true
			}
		}
		if !found { // not issued for this venice
			continue
		}
		if v.status[protos.VeniceOp_VeniceRunVersion].OpStatus == "" {
			pendingVenice = append(pendingVenice, v.Name)
		}
	}
	return pendingVenice

}

// send actual upgrade to venice node - the precheck is already done by this time.
// returns if the service is pending rollout
func (ros *RolloutState) issueServiceRollout() (bool, error) {
	sm := ros.Statemgr
	version := ros.Spec.Version

	serviceRollouts, err := sm.ListServiceRollouts()
	if err != nil {
		log.Errorf("Error %v listing ServiceRollouts", err)
		return false, err
	}
	if len(serviceRollouts) != 0 {
		v := serviceRollouts[0]
		found := false
		for _, ops := range v.Spec.Ops {
			if ops.Op == protos.ServiceOp_ServiceRunVersion && ops.Version == version {
				found = true
			}
		}

		statusFound := false
		if v.status[protos.ServiceOp_ServiceRunVersion].OpStatus != "" {
			statusFound = true
		}

		if !found {
			v.Spec.Ops = append(v.Spec.Ops, protos.ServiceOpSpec{Op: protos.ServiceOp_ServiceRunVersion, Version: version})
			log.Infof("setting serviceRollout with version %v", version)
			err = sm.memDB.UpdateObject(v)
			return true, err // return pending servicerollout with err
		}
		if statusFound {
			log.Infof("Spec and Status found. no pending serviceRollout")
			return false, nil // spec and status found. no pending servicerollout
		}
		log.Infof("Spec found but no Status. pending serviceRollout")
		return true, nil // spec found but status not found. return pending servicerollout
	}

	// the servicerollout object not found  - create one
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
					Version: version,
				},
			},
		},
	}
	log.Infof("Creating serviceRollout")
	err = sm.CreateServiceRolloutState(&serviceRollout, ros, nil)
	if err != nil {
		log.Errorf("Error %v creating service rollout state", err)
	}
	return true, err
}

func (sm *Statemgr) smartNICWorkers(workCh chan *SmartNICState, wg *sync.WaitGroup, ros *RolloutState, op protos.DSCOp) {
	version := ros.Spec.Version

	defer wg.Done()

Loop:
	for {
		select {
		case <-ros.stopChan:
			// Rollout is being stopped. Dont issue any more requests
			return
		case snicState, ok := <-workCh:
			{
				if !ok {
					log.Infof("End of Workchannel. Closing")
					return
				}
				numFailures := atomic.LoadUint32(&ros.numFailuresSeen)
				if numFailures > ros.Spec.MaxNICFailuresBeforeAbort {
					log.Infof("Skipping upgrade for %s as MaxFailures reached", snicState)
					continue
				}
				if ros.Spec.ScheduledStartTime != nil && ros.Spec.ScheduledEndTime != nil {
					endTime, _ := ros.Spec.ScheduledEndTime.Time()
					newduration := endTime.Sub(time.Now())
					log.Debugf("New duration %+v", newduration.Seconds())
					if newduration < 0 {
						log.Infof("Specified endtime is in the past. Skip upgrade for %s", snicState)
						continue
					}
				}

				log.Infof("Got work %#v Adding version:%+v op:%+v", snicState.Name, version, op)

				watcher := memdb.Watcher{Name: "rollout"}
				watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
				defer close(watcher.Channel)
				sm.WatchObjects("DSCRollout", &watcher)
				defer sm.StopWatchObjects("DSCRollout", &watcher)

				snicROState, err := sm.GetDSCRolloutState(snicState.Tenant, snicState.Name)
				if err == nil {
					if st := snicROState.status[op]; st.OpStatus != "" {
						log.Infof("smartnic %v already has status for version %s op %s status %v", snicState.Name, version, op, st.OpStatus)
						continue
					} else {
						snicROState.addSpecOp(version, op)
					}
				} else {
					// smartNICRollout Object does not exist. Create it
					snicRollout := protos.DSCRollout{
						TypeMeta: api.TypeMeta{
							Kind: kindDSCRollout,
						},
						ObjectMeta: api.ObjectMeta{
							Name:   snicState.Name,
							Tenant: snicState.Tenant,
						},
						Spec: protos.DSCRolloutSpec{
							Ops: []protos.DSCOpSpec{
								{
									Op:      op,
									Version: version,
								},
							},
						},
					}
					log.Infof("Creating new smartNICRolloutState %#v", snicRollout.Name)
					err = sm.CreateDSCRolloutState(&snicRollout, ros, nil)
					if err != nil {
						log.Errorf("Error %v creating smartnic rollout state", err)
						continue
					}
				}
				var phase roproto.RolloutPhase_Phases
				if op == protos.DSCOp_DSCPreCheckForUpgOnNextHostReboot || op == protos.DSCOp_DSCPreCheckForDisruptive {
					phase = roproto.RolloutPhase_PRE_CHECK
				} else {
					phase = roproto.RolloutPhase_PROGRESSING
				}
				ros.setSmartNICPhase(snicState.Name, "", "", phase)

				// Wait for response from  the NIC
				// Change the preUpgradeTimeout based on scale
				if ros.Spec.MaxParallel > 10 {
					naplesTimeoutSeconds := int(ros.Spec.MaxParallel/10) * dSCTimeoutSeconds
					preUpgradeTimeout = time.Duration(naplesTimeoutSeconds) * time.Second
				}
				timer := time.NewTimer(preUpgradeTimeout)
				defer timer.Stop()
			WaitLoop:
				for {
					select {
					case <-ros.stopChan:
						// Rollout is being stopped/deleted.
						// Can return immediately now not waiting for a response from NAPLES.
						// If needed, in the future we wait for a response from NAPLES before coming out of this loop
						return
					case evt, ok := <-watcher.Channel:
						if !ok {
							log.Errorf("Error reading from local watch channel. Closing watch")
							break Loop
						}

						snRolloutState, err := DSCRolloutStateFromObj(evt.Obj)
						if err != nil {
							log.Errorf("Error getting smartNICRollout from statemgr. Err: %v", err)
							break WaitLoop
						}
						if snRolloutState.Name == snicState.Name {
							log.Infof("Got status %+v to smartNIC request for %v", snRolloutState.status, snRolloutState.Name)
							if !snRolloutState.anyPendingOp() {
								break WaitLoop
							} else {
								log.Debugf("Still waiting for response from %s", snRolloutState.Name)
							}
						} else {
							log.Debugf("Got status for smartnic %s in context of %s", snRolloutState.Name, snicState.Name)
						}
					case <-timer.C:
						log.Debugf("Timeout waiting for status update of DSC %s", snicState.Name)
						snicROState, err := sm.GetDSCRolloutState(snicState.Tenant, snicState.Name)
						if err == nil {
							snicROState.UpdateDSCRolloutStatus(&protos.DSCRolloutStatus{
								OpStatus: []protos.DSCOpStatus{
									{
										Op:       op,
										Version:  version,
										OpStatus: "timeout",
										Message:  "Timeout waiting for status from DSC",
									},
								},
							})
						}
						break WaitLoop
					}
				}
			}
		}
	}
}

func (ros *RolloutState) preUpgradeSmartNICs() {
	sm := ros.Statemgr

	var op protos.DSCOp
	switch ros.Spec.UpgradeType {
	case roproto.RolloutSpec_Disruptive.String():
		op = protos.DSCOp_DSCPreCheckForDisruptive
	case roproto.RolloutSpec_OnNextHostReboot.String():
		op = protos.DSCOp_DSCPreCheckForUpgOnNextHostReboot
	default:
		op = protos.DSCOp_DSCPreCheckForDisruptive
	}

	log.Infof("starting smartNIC Rollout Preupgrade")
	snStatusList := make(map[string]string)

	for _, snicStatus := range ros.Status.DSCsStatus {
		log.Debugf("Adding smartNIC Status to the List %s", snicStatus.Name)
		snStatusList[snicStatus.Name] = snicStatus.Reason
	}

	snStates, err := sm.ListSmartNICs()
	if err != nil {
		log.Errorf("Error %v listing smartNICs", err)
		return
	}
	sn := orderSmartNICs(ros.Rollout.Spec.OrderConstraints, ros.Rollout.Spec.DSCMustMatchConstraint, snStates, ros, op)

	for _, s := range sn {
		log.Debugf("op:%s for %s", op.String(), spew.Sdump(s))
		if ros.Spec.Strategy == roproto.RolloutSpec_EXPONENTIAL.String() {
			ros.issueDSCOpExponential(s, op, snStatusList)
		} else {
			ros.issueDSCOpLinear(s, op, snStatusList)
		}
	}

	log.Infof("completed smartNIC Rollout Preupgrade")
}

func (ros *RolloutState) issueDSCOpLinear(snStates []*SmartNICState, op protos.DSCOp, snStatusList map[string]string) {
	sm := ros.Statemgr

	numParallel := ros.Spec.MaxParallel
	if numParallel == 0 {
		numParallel = defaultNumParallel
	}

	workCh := make(chan *SmartNICState, 10000)

	for i := uint32(0); i < numParallel; i++ {
		sm.smartNICWG.Add(1)
		go sm.smartNICWorkers(workCh, &sm.smartNICWG, ros, op)
	}
	// give work to worker threads and wait for all of them to complete
	for _, sn := range snStates {
		log.Debugf("Status is %+s", snStatusList[sn.Name])
		if len(snStatusList) != 0 && isPrecheckInProgress(op) == false && (snStatusList[sn.Name] == "" ||
			snStatusList[sn.Name] != opStatusSuccess) {
			//A new node may have joined or become active or failed precheck. skip it
			log.Infof("Status not found for %v. Or pre-check not done on this node earlier or precheck failed.", sn.Name)
			continue
		}
		log.Infof("Adding %s to work Status:%s", sn.Name, snStatusList[sn.Name])
		workCh <- sn
	}
	close(workCh)

	sm.smartNICWG.Wait()
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

func (ros *RolloutState) issueDSCOpExponential(snStates []*SmartNICState, op protos.DSCOp, snStatusList map[string]string) {
	sm := ros.Statemgr

	numParallel := int(ros.Spec.MaxParallel) // if numParallel is 0 then unlimited parallelism

	curParallel := 1
	curIndex := 0

	for curIndex < len(snStates) {
		workPending := len(snStates) - curIndex

		// In each iteration of the outer loop, we execute numJobs in parallel
		numJobs := min(curParallel, workPending)

		workCh := make(chan *SmartNICState, numJobs)
		for i := 0; i < numJobs; i++ {
			sm.smartNICWG.Add(1)
			go sm.smartNICWorkers(workCh, &sm.smartNICWG, ros, op)
			if len(snStatusList) != 0 && isPrecheckInProgress(op) == false && (snStatusList[snStates[curIndex].Name] == "" ||
				snStatusList[snStates[curIndex].Name] != opStatusSuccess) {
				log.Infof("Status not found for %v. Or pre-check not done on this node earlier.", snStates[curIndex].Name)
				curIndex++
				continue
			}
			log.Infof("Adding %s to work Status %v", snStates[curIndex].Name, snStatusList[snStates[curIndex].Name])
			workCh <- snStates[curIndex]
			curIndex++
		}
		close(workCh)

		curParallel = 2 * curParallel
		if numParallel != 0 { // user limited max parallelism to this
			curParallel = min(curParallel, numParallel)
		}
		sm.smartNICWG.Wait()
	}

}

func (ros *RolloutState) checkVeniceHealth(nodesToCheck []string) (name, msg string) {
	toCheck := make(map[string]bool)
	for _, n := range nodesToCheck {
		toCheck[n] = true
	}
	nodeStates, err := ros.Statemgr.ListNodes()
	if err != nil {
		log.Infof("Failed to get venice nodes")
		return "", "Failed to get any venice node"
	}
	for _, nodestate := range nodeStates {
		if !toCheck[nodestate.Name] {
			continue
		}

		var found = false
		if len(nodestate.Status.Conditions) == 0 {
			return nodestate.Name, "Couldnt determine the condition of venice node"
		}
		for _, condition := range nodestate.Status.Conditions {
			log.Infof("Condition Status %+v Type %v", condition.Status, condition.Type)

			if condition.Type == cluster.NodeCondition_HEALTHY.String() && condition.Status == cluster.ConditionStatus_TRUE.String() {
				found = true
				break
			}
		}
		if !found {
			return nodestate.Name, "Venice node is not healthy"
		}
	}

	return "", ""
}

func (ros *RolloutState) computeProgressDelta() {

	numVenice := 0
	numNaples := 0
	if !ros.Spec.DSCsOnly {
		nodeStates, err := ros.Statemgr.ListNodes()
		if err != nil {
			log.Infof("Failed to get venice nodes")
		}
		numVenice = len(nodeStates)
	}

	snStates, err := ros.Statemgr.ListSmartNICs()
	if err != nil {
		log.Errorf("Error %v listing smartNICs", err)
		return
	}

	sn := orderSmartNICs(ros.Rollout.Spec.OrderConstraints, ros.Rollout.Spec.DSCMustMatchConstraint, snStates, ros, protos.DSCOp_DSCPreCheckForDisruptive)

	for _, s := range sn {
		log.Debugf("Status %s", spew.Sdump(s))
		numNaples += len(s)
		numNaples += int(ros.numSkipped) //include the skipped ones
	}

	ros.completionDelta = float32(100 / float32(2*numVenice+2*numNaples+2))
	log.Infof("Completion Delta %+v NumNaples %v NumVenice %+v", ros.completionDelta, numNaples, numVenice)
}

func (ros *RolloutState) doUpdateSmartNICs() {
	sm := ros.Statemgr
	log.Infof("Starting DSC Rollout:length:%+v", len(ros.Status.DSCsStatus))
	snStatusList := make(map[string]string)

	for _, snicStatus := range ros.Status.DSCsStatus {
		log.Debugf("Adding smartNIC Status to the List %s", snicStatus.Name)
		snStatusList[snicStatus.Name] = snicStatus.Reason
	}

	var op protos.DSCOp
	switch ros.Spec.UpgradeType {
	case roproto.RolloutSpec_Disruptive.String():
		op = protos.DSCOp_DSCDisruptiveUpgrade
	case roproto.RolloutSpec_OnNextHostReboot.String():
		op = protos.DSCOp_DSCUpgOnNextHostReboot
	default:
		op = protos.DSCOp_DSCDisruptiveUpgrade
	}

	snStates, err := sm.ListSmartNICs()
	if err != nil {
		log.Errorf("Error %v listing smartNICs", err)
		return
	}

	sn := orderSmartNICs(ros.Rollout.Spec.OrderConstraints, ros.Rollout.Spec.DSCMustMatchConstraint, snStates, ros, op)
	for _, s := range sn {
		log.Debugf("op:%s for %s", op.String(), spew.Sdump(s))
		if ros.Spec.Strategy == roproto.RolloutSpec_EXPONENTIAL.String() {
			ros.issueDSCOpExponential(s, op, snStatusList)
		} else {
			ros.issueDSCOpLinear(s, op, snStatusList)
		}
	}
	log.Infof("completed smartNIC Rollout")
}

// Send precheck to one more venice node (which has not been requested yet)
// Returns the number of venice nodes with precheck issued (and status not obtained yet)
// When there is an error, it returns error and the node on which there is an error
func (ros *RolloutState) preCheckNextVeniceNode() (lenPendingStatus int, erroredNode string, err error) {
	sm := ros.Statemgr
	version := ros.Spec.Version
	pendingStatus := ros.getVenicePendingPrecheckStatus()

	// TODO: Current following check enforces that precheck are run sequentially on venice nodes
	// this is needed so that the peak disk pressure is not reached on the development machines
	// once the dev machines have more disk, this can be removed so that precheck can run in parallel
	const maxPrecheckInParallel = 1
	if len(pendingStatus) >= maxPrecheckInParallel {
		return len(pendingStatus), "", nil
	}

	veniceRollouts := ros.getVenicePendingPreCheckIssue()
	if len(veniceRollouts) == 0 {
		return 0, "", nil // all venice have been issued precheck already

	}
	n := veniceRollouts[0]
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
					Version: version,
				},
			},
		},
	}
	err = sm.CreateVeniceRolloutState(&veniceRollout, ros, nil)
	if err != nil {
		log.Errorf("Error %v creating venice rollout state", err)
		return len(pendingStatus), n, err
	}
	return len(pendingStatus) + 1, "", nil

}
