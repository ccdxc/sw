package state

import (
	"strings"

	"github.com/davecgh/go-spew/spew"

	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/log"
)

// =====   RolloutAPI interface =====
//  Provided by NMD to Rollout Controller

// CreateUpdateSmartNICRollout as requested by Rollout Controller
func (n *NMD) CreateUpdateSmartNICRollout(sro *protos.SmartNICRollout) error {
	n.Lock()
	defer n.Unlock()

	n.objectMeta = sro.ObjectMeta
	n.updateOps(sro.Spec.Ops)
	n.issueNextPendingOp()
	return nil
}

// DeleteSmartNICRollout as requested by Rollout Controller
func (n *NMD) DeleteSmartNICRollout(sro *protos.SmartNICRollout) error {
	n.Lock()
	defer n.Unlock()

	n.updateOps([]*protos.SmartNICOpSpec{})
	return nil
}

// === helper routines

// new/update request came from venice
/*
	Clear the pending list
	Iterate through each op from venice
	If an op is already complete (or) is currently in progress nothing more to do for that op.
	Else add the op to the pending list.
	if an already complete op is not found in the new request, remove it from the completedOps and status
*/
func (n *NMD) updateOps(ops []*protos.SmartNICOpSpec) error {
	newOps := make(map[protos.SmartNICOpSpec]bool)
	newPendingOps := []protos.SmartNICOpSpec{}
	for _, o := range ops {
		newOps[*o] = true

		if n.completedOps[*o] || (n.inProgressOps != nil && *n.inProgressOps == *o) {
			continue
		} else {
			newPendingOps = append(newPendingOps, *o)
		}
	}
	n.pendingOps = newPendingOps
	for o := range n.completedOps {
		if !newOps[o] {
			delete(n.completedOps, o)
			// remove o from Status also
			newOpStatus := []protos.SmartNICOpStatus{}
			for _, st := range n.opStatus {
				if st.Op != o.Op || st.Version != o.Version {
					newOpStatus = append(newOpStatus, st)
				}
			}
			n.opStatus = newOpStatus
		}
	}
	log.Debugf("state after updateOps is completed:%s inProgress:%s pending:%s opStatus:%s", spew.Sdump(n.completedOps), spew.Sdump(n.inProgressOps), spew.Sdump(n.pendingOps), spew.Sdump(n.opStatus))
	return nil
}

// MUST be called with lock held
func (n *NMD) issueNextPendingOp() {
	log.Debugf("On entry of issueNextPendingOp state is completed:%s inProgress:%s pending:%s", spew.Sdump(n.completedOps), spew.Sdump(n.inProgressOps), spew.Sdump(n.pendingOps))
	if n.inProgressOps != nil {
		return // an op is already in progress...
	}
	if len(n.pendingOps) == 0 {
		return
	}
	op := n.pendingOps[0]
	n.pendingOps = append(n.pendingOps[:0], n.pendingOps[1:]...)
	n.inProgressOps = &op

	var err error
	log.Debugf("Issuing %#v", *n.inProgressOps)
	switch n.inProgressOps.Op {
	case protos.SmartNICOp_SmartNICDisruptiveUpgrade:
		err = n.upgmgr.StartDisruptiveUpgrade()

		if err != nil {
			log.Errorf("StartDisruptiveUpgrade returned %s", err)
			return
		}
	case protos.SmartNICOp_SmartNICUpgOnNextHostReboot:
		err = n.upgmgr.StartUpgOnNextHostReboot()
		if err != nil {
			log.Errorf("StartDisruptiveUpgrade returned %s", err)
			return
		}
	case protos.SmartNICOp_SmartNICPreCheckForDisruptive:
		err = n.upgmgr.StartPreCheckDisruptive(n.inProgressOps.Version)
		if err != nil {
			log.Errorf("Precheck returned %s", err)
			return
		}
	}
}

// UpgSuccessful is called after upgrade is succesful. Reply to venice
func (n *NMD) UpgSuccessful() {
	log.Infof("UpgSuccessful  got called")
	n.Lock()
	defer n.Unlock()

	message := ""

	if n.inProgressOps != nil {
		n.updateOpStatus(n.inProgressOps.Op, n.inProgressOps.Version, "success", message)
		n.completedOps[*n.inProgressOps] = true
		n.inProgressOps = nil
	} else {
		log.Infof("UpgSuccessful got called when there is no pending Op")
	}

	n.issueNextPendingOp() // issue next request, if any
}

// UpgFailed is called after upgrade is declared fail. Reply to venice
func (n *NMD) UpgFailed(errStrList *[]string) {
	log.Infof("UpgFailed got called")

	n.Lock()
	defer n.Unlock()
	message := strings.Join(*errStrList, ", ")

	if n.inProgressOps != nil {
		n.updateOpStatus(n.inProgressOps.Op, n.inProgressOps.Version, "failure", message)
		n.completedOps[*n.inProgressOps] = true
		n.inProgressOps = nil
	} else {
		log.Infof("UpgFailed got called when there is no pending Op")
	}

	n.issueNextPendingOp() // issue next request, if any
}

// UpgPossible is called on precheck success
func (n *NMD) UpgPossible() {
	log.Infof("UpgPossible got called")
	n.Lock()
	defer n.Unlock()

	message := ""
	if n.inProgressOps != nil {
		n.updateOpStatus(n.inProgressOps.Op, n.inProgressOps.Version, "success", message)
		n.completedOps[*n.inProgressOps] = true
		n.inProgressOps = nil
	} else {
		log.Infof("UpgPossible got called when there is no pending Op")
	}

	n.issueNextPendingOp() // issue next request, if any

}

// UpgNotPossible - some precheck failed
func (n *NMD) UpgNotPossible(errStrList *[]string) {
	n.Lock()
	defer n.Unlock()

	log.Infof("UpgNotPossible got called")
	message := strings.Join(*errStrList, ", ")

	if n.inProgressOps != nil {
		n.updateOpStatus(n.inProgressOps.Op, n.inProgressOps.Version, "failure", message)
		n.completedOps[*n.inProgressOps] = true
		n.inProgressOps = nil
	} else {
		log.Infof("UpgNotPossible got called when there is no pending Op")
	}

	n.issueNextPendingOp() // issue next request, if any
}

// UpgAborted is called after upgrade is aborted
func (n *NMD) UpgAborted(errStrList *[]string) {
	log.Infof("UpgAborted got called")
	n.UpgFailed(errStrList)
}

// Must be called with LOCK held
func (n *NMD) updateOpStatus(op protos.SmartNICOp, version string, status string, message string) {
	var OpStatus []protos.SmartNICOpStatus
	for _, o := range n.opStatus {
		if o.Op != op || o.Version != version {
			OpStatus = append(OpStatus, o)
		}
	}

	OpStatus = append(OpStatus,
		protos.SmartNICOpStatus{
			Op:       op,
			Version:  version,
			OpStatus: status,
			Message:  message,
		})

	n.opStatus = OpStatus

	st := protos.SmartNICRolloutStatusUpdate{
		ObjectMeta: n.objectMeta,
		Status: protos.SmartNICRolloutStatus{
			OpStatus: OpStatus,
		},
	}
	if n.rollout == nil {
		log.Errorf("RolloutIf empty while updating status")
		return
	}

	err := n.rollout.UpdateSmartNICRolloutStatus(&st)
	if err != nil {
		log.Errorf("Failed to update Rollout err: %v", err)
	}
}
