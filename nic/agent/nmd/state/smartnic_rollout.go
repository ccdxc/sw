package state

import (
	"context"
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/pensando/sw/venice/utils/imagestore"

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

	if n.ro.InProgressOp.Op != protos.SmartNICOp_SmartNICNoOp || len(n.ro.OpStatus) != 0 {
		log.Errorf("A previously issued Rollout %v state exists. Please DELETE it before proceeding.", n.ro)
		return fmt.Errorf("delete previous rollout before proceeding")
	}

	n.objectMeta = sro.ObjectMeta
	n.updateOps(sro.Spec.Ops)
	n.issueNextPendingOp()
	return nil
}

// DeleteSmartNICRollout as requested by Rollout Controller
func (n *NMD) DeleteSmartNICRollout(sro *protos.SmartNICRollout) error {
	n.Lock()
	defer n.Unlock()
	n.ro.ObjectMeta.Tenant = ""
	n.ro.OpStatus = []protos.SmartNICOpStatus{}
	n.updateOps([]protos.SmartNICOpSpec{})
	n.updateRolloutStatus(protos.SmartNICOpSpec{Op: protos.SmartNICOp_SmartNICNoOp})
	return nil
}

// GetSmartNICRolloutStatus returns RolloutStatus
func (n *NMD) GetSmartNICRolloutStatus() protos.SmartNICRolloutStatusUpdate {
	n.Lock()
	defer n.Unlock()
	// for deep copy
	OpStatus := make([]protos.SmartNICOpStatus, len(n.ro.OpStatus))
	copy(OpStatus, n.ro.OpStatus)

	return protos.SmartNICRolloutStatusUpdate{
		ObjectMeta: n.objectMeta,
		Status: protos.SmartNICRolloutStatus{
			OpStatus: OpStatus,
		},
	}
}

// === helper routines
func (n *NMD) updateRolloutStatus(inProgressOp protos.SmartNICOpSpec) {
	n.ro.InProgressOp = inProgressOp

	log.Infof("Persisting rollout object to nmd.db. PendingOps : %v OpStatus : %v InProgressOps : %v", n.ro.PendingOps, n.ro.OpStatus, n.ro.InProgressOp)
	err := n.store.Write(&n.ro)
	if err != nil {
		log.Errorf("Error persisting the default naples config in EmDB, err: %+v", err)
	}
}

// new/update request came from venice
/*
	Clear the pending list
	Iterate through each op from venice
	If an op is already complete (or) is currently in progress nothing more to do for that op.
	Else add the op to the pending list.
	if an already complete op is not found in the new request, remove it from the ro.CompletedOps and status
*/
func (n *NMD) updateOps(ops []protos.SmartNICOpSpec) error {
	newOps := make(map[protos.SmartNICOpSpec]bool)
	newPendingOps := []protos.SmartNICOpSpec{}
	for _, o := range ops {
		newOps[o] = true

		if n.completedOps[o] || (n.ro.InProgressOp == o) {
			continue
		} else {
			newPendingOps = append(newPendingOps, o)
		}
	}
	n.ro.PendingOps = newPendingOps
	for o := range n.completedOps {
		if !newOps[o] {
			delete(n.completedOps, o)
			// remove o from Status also
			newOpStatus := []protos.SmartNICOpStatus{}
			for _, st := range n.ro.OpStatus {
				if st.Op != o.Op || st.Version != o.Version {
					newOpStatus = append(newOpStatus, st)
				}
			}
			n.ro.OpStatus = newOpStatus
		}
	}
	log.Infof("state after updateOps is completed:%s inProgress:%s pending:%s ro.OpStatus:%s", spew.Sdump(n.completedOps), n.ro.InProgressOp, spew.Sdump(n.ro.PendingOps), spew.Sdump(n.ro.OpStatus))
	return nil
}

// MUST be called with lock held
func (n *NMD) issueNextPendingOp() {
	log.Infof("On entry of issueNextPendingOp state is completed:%s inProgress:%s pending:%s", spew.Sdump(n.completedOps), n.ro.InProgressOp, spew.Sdump(n.ro.PendingOps))
	if n.ro.InProgressOp.Op != protos.SmartNICOp_SmartNICNoOp {
		log.Infof("An op is currently in progress.  %v", n.ro.InProgressOp.Op)
		return // an op is already in progress...
	}

	if len(n.ro.PendingOps) == 0 {
		log.Info("No pending ops. Sending updates to Venice.")
		st := protos.SmartNICRolloutStatusUpdate{
			ObjectMeta: n.objectMeta,
			Status: protos.SmartNICRolloutStatus{
				OpStatus: n.ro.OpStatus,
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
		return
	}
	op := n.ro.PendingOps[0]
	n.ro.PendingOps = append(n.ro.PendingOps[:0], n.ro.PendingOps[1:]...)

	n.updateRolloutStatus(op)

	var err error
	log.Infof("Issuing %#v", n.ro.InProgressOp)
	switch n.ro.InProgressOp.Op {
	case protos.SmartNICOp_SmartNICDisruptiveUpgrade:
		err = n.Upgmgr.StartDisruptiveUpgrade("naples_fw.tar")
		if err != nil {
			log.Errorf("StartDisruptiveUpgrade returned %s", err)
			go n.UpgFailed(&[]string{fmt.Sprintf("StartDisruptiveUpgrade returned %s", err)})
			return
		}
	case protos.SmartNICOp_SmartNICUpgOnNextHostReboot:
		if _, err = os.Stat("/update/naples_fw.tar"); os.IsNotExist(err) {
			log.Errorf("/update/naples_fw.tar not found %s", err)
			go n.UpgFailed(&[]string{fmt.Sprintf("/update/naples_fw.tar not found %s", err)})
			return
		}
		_, err = naplesHostDisruptiveUpgrade("naples_fw.tar")
		if err != nil {
			log.Errorf("naplesHostDisruptiveUpgrade failed %s", err)
			go n.UpgFailed(&[]string{fmt.Sprintf("naplesHostDisruptiveUpgrade failed %s", err)})
			return
		}
		go n.UpgSuccessful()
	case protos.SmartNICOp_SmartNICPreCheckForUpgOnNextHostReboot:
		ctx, cancel := context.WithTimeout(context.Background(), 1*time.Minute)
		naplesVersion, err := imagestore.GetNaplesRolloutVersion(ctx, n.resolverClient, n.ro.InProgressOp.Version)
		if err != nil {
			log.Errorf("Failed to get naples version from objectstore %+v", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("Failed to get naples version from objectstore %+v", err)})
			cancel()
			return
		}
		cancel()
		ctx, cancel = context.WithTimeout(context.Background(), 4*time.Minute)
		err = imagestore.DownloadNaplesImage(ctx, n.resolverClient, naplesVersion, "/update/naples_fw.tar")
		if err != nil {
			log.Errorf("Failed to download naples image from objectstore %+v", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("Failed to download naples image from objectstore %+v", err)})
			cancel()
			return
		}
		cancel()
		_, err = naplesPkgVerify("naples_fw.tar")
		if err != nil {
			log.Errorf("Firmware image verification failed %s", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("Firmware image verification failed %s", err)})
			return
		}
		go n.UpgPossible()
	case protos.SmartNICOp_SmartNICPreCheckForDisruptive:
		ctx, cancel := context.WithTimeout(context.Background(), 1*time.Minute)
		naplesVersion, err := imagestore.GetNaplesRolloutVersion(ctx, n.resolverClient, n.ro.InProgressOp.Version)
		if err != nil {
			log.Errorf("Failed to get naples version from objectstore %+v", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("Failed to get naples version from objectstore %+v", err)})
			cancel()
			return
		}
		cancel()
		ctx, cancel = context.WithTimeout(context.Background(), 4*time.Minute)
		err = imagestore.DownloadNaplesImage(ctx, n.resolverClient, naplesVersion, "/update/naples_fw.tar")
		if err != nil {
			log.Errorf("Failed to download naples image from objectstore %+v", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("Failed to download naples image from objectstore %+v", err)})
			cancel()
			return
		}
		cancel()
		err = n.Upgmgr.StartPreCheckDisruptive(n.ro.InProgressOp.Version)
		if err != nil {
			log.Errorf("StartPreCheckDisruptive returned %s", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("StartPreCheckDisruptive returned %s", err)})
			return
		}
	case protos.SmartNICOp_SmartNICImageDownload:
		ctx, cancel := context.WithTimeout(context.Background(), 1*time.Minute)
		naplesVersion, err := imagestore.GetNaplesRolloutVersion(ctx, n.resolverClient, n.ro.InProgressOp.Version)
		if err != nil {
			log.Errorf("Failed to get naples version from objectstore %+v", err)
			cancel()
			return
		}
		cancel()
		ctx, cancel = context.WithTimeout(context.Background(), 4*time.Minute)
		err = imagestore.DownloadNaplesImage(ctx, n.resolverClient, naplesVersion, "/update/naples_fw.tar")
		if err != nil {
			log.Errorf("Failed to download naples image from objectstore %+v", err)
			cancel()
			return
		}
		cancel()
		_, err = naplesPkgVerify("naples_fw.tar")
		if err != nil {
			log.Errorf("Firmware image verification failed %s", err)
			return
		}
	}
}

// UpgSuccessful is called after upgrade is successful. Reply to venice
func (n *NMD) UpgSuccessful() {
	log.Infof("UpgSuccessful  got called")
	n.Lock()
	defer n.Unlock()

	message := ""

	if n.ro.InProgressOp.Op != protos.SmartNICOp_SmartNICNoOp {
		n.updateOpStatus(n.ro.InProgressOp.Op, n.ro.InProgressOp.Version, "success", message)
		n.completedOps[n.ro.InProgressOp] = true
		n.updateRolloutStatus(protos.SmartNICOpSpec{Op: protos.SmartNICOp_SmartNICNoOp})
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

	if n.ro.InProgressOp.Op != protos.SmartNICOp_SmartNICNoOp {
		n.updateOpStatus(n.ro.InProgressOp.Op, n.ro.InProgressOp.Version, "failure", message)
		n.completedOps[n.ro.InProgressOp] = true
		n.updateRolloutStatus(protos.SmartNICOpSpec{Op: protos.SmartNICOp_SmartNICNoOp})
	} else {
		log.Infof("UpgFailed got called when there is no pending Op")
	}

	n.issueNextPendingOp() // issue next request, if any
}

// UpgPossible is called on precheck success
func (n *NMD) UpgPossible() {
	n.Lock()
	defer n.Unlock()
	log.Infof("UpgPossible got called")
	message := ""
	if n.ro.InProgressOp.Op != protos.SmartNICOp_SmartNICNoOp {
		n.updateOpStatus(n.ro.InProgressOp.Op, n.ro.InProgressOp.Version, "success", message)
		n.completedOps[n.ro.InProgressOp] = true
		n.updateRolloutStatus(protos.SmartNICOpSpec{Op: protos.SmartNICOp_SmartNICNoOp})
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

	if n.ro.InProgressOp.Op != protos.SmartNICOp_SmartNICNoOp {
		n.updateOpStatus(n.ro.InProgressOp.Op, n.ro.InProgressOp.Version, "failure", message)
		n.completedOps[n.ro.InProgressOp] = true
		n.updateRolloutStatus(protos.SmartNICOpSpec{Op: protos.SmartNICOp_SmartNICNoOp})
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
	log.Infof("Updating op status. Op: %v | Status: %s", op, status)
	var OpStatus []protos.SmartNICOpStatus
	for _, o := range n.ro.OpStatus {
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

	n.ro.OpStatus = OpStatus
	log.Infof("Updated NMD internal op status: %v", n.ro.OpStatus)

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
