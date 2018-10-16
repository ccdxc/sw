package statemgr

import (
	"fmt"
	"sync"
	"time"

	"github.com/davecgh/go-spew/spew"

	"github.com/pensando/sw/api"
	roproto "github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
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

// This function bins the smartNICs based on the user-requested labels and returns the slice of bins
// Each bin of smartnics are expected to complete in that order
func orderSmartNICs(labelSels []*labels.Selector, smartNICMustMatchConstraint bool, sn []*SmartNICState) [][]*SmartNICState {
	retval := make([][]*SmartNICState, 0)
	curbin := make([]*SmartNICState, 0)

	snics := make(map[string]*SmartNICState)
	for _, s := range sn {
		snics[s.Name] = s
	}

	for _, ls := range labelSels {
		for name, s := range snics {
			if ls.Matches(labels.Set(s.ObjectMeta.Labels)) {
				curbin = append(curbin, s)
				delete(snics, name)
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
			curbin = append(curbin, s)
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
		if !issuedVenices[n.ObjectMeta.Name] {
			pendingVenice = append(pendingVenice, n.ObjectMeta.Name)
		}
	}
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
		if v.Name == selectedVenice {
			v.Spec.Ops = append(v.Spec.Ops, &protos.VeniceOpSpec{Op: protos.VeniceOp_VeniceRunVersion, Version: version})

			log.Debugf("setting VeniceRollout for %v with version %v", v.Name, version)
			err = sm.memDB.UpdateObject(v)
			return 1, err
		}
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
	for _, v := range serviceRollouts {
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
			v.Spec.Ops = append(v.Spec.Ops, &protos.ServiceOpSpec{Op: protos.ServiceOp_ServiceRunVersion, Version: version})
			log.Debugf("setting serviceRollout with version %v", version)
			err = sm.memDB.UpdateObject(v)
			return true, err // return pending servicerollout with err
		}
		if statusFound {
			return false, nil // spec and status found. no pending servicerollout
		}
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
			Ops: []*protos.ServiceOpSpec{
				{
					Op:      protos.ServiceOp_ServiceRunVersion,
					Version: version,
				},
			},
		},
	}
	err = sm.CreateServiceRolloutState(&serviceRollout, ros)
	if err != nil {
		log.Errorf("Error %v creating service rollout state", err)
	}
	return true, err
}

func (sm *Statemgr) smartNICWorkers(workCh chan *SmartNICState, wg *sync.WaitGroup, ros *RolloutState, op protos.SmartNICOp) {
	version := ros.Spec.Version

	defer wg.Done()

Loop:
	for {
		select {
		case <-ros.stopChan:
			// Rollout is being stopped. Dont issue any more requests
			return
		case snicState, ok := <-workCh:
			if !ok {
				log.Debugf("End of Workchannel. Closing")
				return
			}
			log.Debugf("Got work %#v", snicState)

			watchChan := make(chan memdb.Event, memdb.WatchLen)
			defer close(watchChan)
			sm.WatchObjects("SmartNICRollout", watchChan)
			defer sm.StopWatchObjects("SmartNICRollout", watchChan)

			snicROState, err := sm.GetSmartNICRolloutState(snicState.Tenant, snicState.Name)
			if err == nil {
				if st := snicROState.status[op]; st.OpStatus != "" {
					log.Debugf("smartnic %v already has status for version %s op %s", snicState.Name, version, op)
					continue
				} else {
					snicROState.addSpecOp(version, op)
				}
			} else {
				// smartNICRollout Object does not exist. Create it
				snicRollout := protos.SmartNICRollout{
					TypeMeta: api.TypeMeta{
						Kind: kindSmartNICRollout,
					},
					ObjectMeta: api.ObjectMeta{
						Name:   snicState.Name,
						Tenant: snicState.Tenant,
					},
					Spec: protos.SmartNICRolloutSpec{
						Ops: []*protos.SmartNICOpSpec{
							{
								Op:      op,
								Version: version,
							},
						},
					},
				}
				log.Debugf("Creating smartNICRolloutState %#v", snicRollout)
				err = sm.CreateSmartNICRolloutState(&snicRollout, ros)
				if err != nil {
					log.Errorf("Error %v creating smartnic rollout state", err)
					continue
				}
			}

			// Wait for response from  the NIC
		WaitLoop:
			for {
				select {
				case <-ros.stopChan:
					// Rollout is being stopped/deleted.
					// Can return immediately now not waiting for a response from NAPLES.
					// If needed, in the future we wait for a response from NAPLES before coming out of this loop
					return
				case evt, ok := <-watchChan:
					if !ok {
						log.Errorf("Error reading from local watch channel. Closing watch")
						break Loop
					}

					snRolloutState, err := SmartNICRolloutStateFromObj(evt.Obj)
					if err != nil {
						log.Errorf("Error getting smartNICRollout from statemgr. Err: %v", err)
						break WaitLoop
					}
					if snRolloutState.Name == snicState.Name {
						log.Debugf("Got status %#v to smartNIC request for %v", snRolloutState.status, snRolloutState.Name)
						if !snRolloutState.anyPendingOp() {
							break WaitLoop
						} else {
							log.Debugf("Still waiting for response from %s", snRolloutState.Name)
						}
					} else {
						log.Debugf("Got status for smartnic %s in context of %s", snRolloutState.Name, snicState.Name)
					}
				case <-time.After(preUpgradeTimeout):
					log.Debugf("Timeout waiting for status update of smartNIC %s", snicState.Name)
					snicROState, err := sm.GetSmartNICRolloutState(snicState.Tenant, snicState.Name)
					if err == nil {
						snicROState.UpdateSmartNICRolloutStatus(&protos.SmartNICRolloutStatus{
							OpStatus: []protos.SmartNICOpStatus{
								{
									Op:       op,
									Version:  version,
									OpStatus: "timeout",
									Message:  "Timeout waiting for status from smartNIC",
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

func (ros *RolloutState) preUpgradeSmartNICs() {
	sm := ros.Statemgr

	var op protos.SmartNICOp
	switch ros.Spec.UpgradeType {
	case roproto.RolloutSpec_Disruptive.String():
		op = protos.SmartNICOp_SmartNICPreCheckForDisruptive
	case roproto.RolloutSpec_OnNextHostReboot.String():
		op = protos.SmartNICOp_SmartNICPreCheckForUpgOnNextHostReboot
	default:
		op = protos.SmartNICOp_SmartNICPreCheckForDisruptive
	}

	log.Infof("starting smartNIC Rollout Preupgrade")

	snStates, err := sm.ListSmartNICs()
	if err != nil {
		log.Errorf("Error %v listing smartNICs", err)
		return
	}
	sn := orderSmartNICs(ros.Rollout.Spec.OrderConstraints, ros.Rollout.Spec.SmartNICMustMatchConstraint, snStates)

	for _, s := range sn {
		log.Infof("op:%s for %s", op.String(), spew.Sdump(s))
		ros.issueSmartNICOp(s, op)
	}

	log.Infof("completed smartNIC Rollout Preupgrade")
}

func (ros *RolloutState) issueSmartNICOp(snStates []*SmartNICState, Op protos.SmartNICOp) {
	sm := ros.Statemgr

	numParallel := ros.Spec.MaxParallel
	if numParallel == 0 {
		numParallel = defaultNumParallel
	}

	workCh := make(chan *SmartNICState, 10000)

	for i := uint32(0); i < numParallel; i++ {
		sm.smartNICWG.Add(1)
		go sm.smartNICWorkers(workCh, &sm.smartNICWG, ros, Op)
	}

	// TODO: Implement Exponential policy
	// TODO: Implement max-failures

	// give work to worker threads and wait for all of them to complete
	for _, sn := range snStates {
		log.Debugf("Adding %v to work", sn)
		workCh <- sn
	}
	close(workCh)

	sm.smartNICWG.Wait()
}

func (ros *RolloutState) doUpdateSmartNICs() {
	sm := ros.Statemgr
	log.Infof("starting smartNIC Rollout")

	var op protos.SmartNICOp
	switch ros.Spec.UpgradeType {
	case roproto.RolloutSpec_Disruptive.String():
		op = protos.SmartNICOp_SmartNICDisruptiveUpgrade
	case roproto.RolloutSpec_OnNextHostReboot.String():
		op = protos.SmartNICOp_SmartNICUpgOnNextHostReboot
	default:
		op = protos.SmartNICOp_SmartNICDisruptiveUpgrade
	}

	snStates, err := sm.ListSmartNICs()
	if err != nil {
		log.Errorf("Error %v listing smartNICs", err)
		return
	}

	sn := orderSmartNICs(ros.Rollout.Spec.OrderConstraints, ros.Rollout.Spec.SmartNICMustMatchConstraint, snStates)
	for _, s := range sn {
		log.Infof("op:%s for %s", op.String(), spew.Sdump(s))
		ros.issueSmartNICOp(s, op)
	}
	log.Infof("completed smartNIC Rollout")
}

// Send precheck to one more venice node (which has not been requested yet)
// Returns the number of venice nodes with precheck issued (and status not obtained yet)
func (ros *RolloutState) preCheckNextVeniceNode() (int, error) {
	sm := ros.Statemgr
	version := ros.Spec.Version
	pendingStatus := ros.getVenicePendingPrecheckStatus()

	// TODO: Current following check enforces that precheck are run sequentially on venice nodes
	// this is needed so that the peak disk pressure is not reached on the development machines
	// once the dev machines have more disk, this can be removed so that precheck can run in parallel
	const maxPrecheckInParallel = 1
	if len(pendingStatus) >= maxPrecheckInParallel {
		return len(pendingStatus), nil
	}

	veniceRollouts := ros.getVenicePendingPreCheckIssue()
	for _, n := range veniceRollouts {
		veniceRollout := protos.VeniceRollout{
			TypeMeta: api.TypeMeta{
				Kind: "VeniceRollout",
			},
			ObjectMeta: api.ObjectMeta{
				Name: n,
			},
			Spec: protos.VeniceRolloutSpec{
				Ops: []*protos.VeniceOpSpec{
					{
						Op:      protos.VeniceOp_VenicePreCheck,
						Version: version,
					},
				},
			},
		}
		err := sm.CreateVeniceRolloutState(&veniceRollout, ros)
		if err != nil {
			log.Errorf("Error %v creating venice rollout state", err)
			return len(pendingStatus), err
		}
		return len(pendingStatus) + 1, nil
	}
	return 0, nil // all venice have been issued precheck already
}
