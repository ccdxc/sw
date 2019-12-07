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
const (
	//value taken from nic/sim/naples/start-naples.sh
	simSerialNumber = "SIM18440000"
)

// CreateUpdateDSCRollout as requested by Rollout Controller
func (n *NMD) CreateUpdateDSCRollout(sro *protos.DSCRollout) error {
	n.Lock()
	defer n.Unlock()
	n.ro.ObjectMeta.Name = n.config.Spec.PrimaryMAC
	n.ro.TypeMeta.Kind = "NaplesRollout"
	n.updateOps(sro.Spec.Ops)
	n.issueNextPendingOp()
	return nil
}

func (n *NMD) deleteDSCRollout() error {
	n.ro.ObjectMeta.Tenant = ""
	n.ro.OpStatus = []protos.DSCOpStatus{}
	n.updateOps([]protos.DSCOpSpec{})
	n.updateRolloutStatus(protos.DSCOpSpec{Op: protos.DSCOp_DSCNoOp})
	return nil
}

// DeleteDSCRollout as requested by Rollout Controller
func (n *NMD) DeleteDSCRollout() error {
	n.Lock()
	defer n.Unlock()

	return n.deleteDSCRollout()
}

// GetDSCRolloutStatus returns RolloutStatus
func (n *NMD) GetDSCRolloutStatus() protos.DSCRolloutStatusUpdate {
	n.Lock()
	defer n.Unlock()
	// for deep copy
	OpStatus := make([]protos.DSCOpStatus, len(n.ro.OpStatus))
	copy(OpStatus, n.ro.OpStatus)

	return protos.DSCRolloutStatusUpdate{
		ObjectMeta: n.ro.ObjectMeta,
		Status: protos.DSCRolloutStatus{
			OpStatus: OpStatus,
		},
	}
}

// === helper routines
func (n *NMD) updateRolloutStatus(inProgressOp protos.DSCOpSpec) {
	n.ro.InProgressOp = inProgressOp

	log.Infof("Persisting rollout object to nmd.db. n.ro : %v PendingOps : %v OpStatus : %v InProgressOps : %v", n.ro, n.ro.PendingOps, n.ro.OpStatus, n.ro.InProgressOp)
	err := n.store.Write(&n.ro)
	if err != nil {
		log.Errorf("Error persisting the default naples config in EmDB, err: %+v", err)
	}
}

func (n *NMD) isOpCompleted(op protos.DSCOpSpec) bool {
	for _, o := range n.ro.CompletedOps {
		if o == op {
			return true
		}
	}
	return false
}

func (n *NMD) appendCompletedOps(op protos.DSCOpSpec) {
	n.ro.CompletedOps = append(n.ro.CompletedOps, op)
}

func (n *NMD) removeFromCompletedOps(op protos.DSCOpSpec) {
	completedOps := []protos.DSCOpSpec{}
	for _, o := range n.ro.CompletedOps {
		if o == op {
			continue
		}
		completedOps = append(completedOps, o)
	}
	n.ro.CompletedOps = completedOps
}

// new/update request came from venice
/*
	Clear the pending list
	Iterate through each op from venice
	If an op is already complete (or) is currently in progress nothing more to do for that op.
	Else add the op to the pending list.
	if an already complete op is not found in the new request, remove it from the ro.CompletedOps and status
*/
func (n *NMD) updateOps(ops []protos.DSCOpSpec) error {
	newOps := make(map[protos.DSCOpSpec]bool)
	newPendingOps := []protos.DSCOpSpec{}
	for _, o := range ops {
		newOps[o] = true

		if n.isOpCompleted(o) || (n.ro.InProgressOp == o) {
			continue
		} else {
			newPendingOps = append(newPendingOps, o)
		}
	}
	n.ro.PendingOps = newPendingOps
	for _, o := range n.ro.CompletedOps {
		if !newOps[o] {
			n.removeFromCompletedOps(o)
			// remove o from Status also
			newOpStatus := []protos.DSCOpStatus{}
			for _, st := range n.ro.OpStatus {
				if st.Op != o.Op || st.Version != o.Version {
					newOpStatus = append(newOpStatus, st)
				}
			}
			n.ro.OpStatus = newOpStatus
		}
	}
	log.Infof("state after updateOps is completed:%s inProgress:%s pending:%s ro.OpStatus:%s", spew.Sdump(n.ro.CompletedOps), n.ro.InProgressOp, spew.Sdump(n.ro.PendingOps), spew.Sdump(n.ro.OpStatus))
	return nil
}

// IssueInProgressOp MUST be called with lock held
func (n *NMD) IssueInProgressOp() {
	n.Lock()
	defer n.Unlock()
	log.Infof("IssueInProgressOp called with %v", n.ro)
	if n.ro.InProgressOp.Op != protos.DSCOp_DSCNoOp {
		/*
			Currently pciemgr doesn't support upgrade during host reboot.
			So commenting out this code for now and just deleting any previous rollout requests
			n.ro.PendingOps = append([]protos.DSCOpSpec{n.ro.InProgressOp}, n.ro.PendingOps...)
			n.ro.InProgressOp = protos.DSCOpSpec{Op: protos.DSCOp_DSCNoOp}
			log.Infof("An op was in progress. Moving it to the head of PendingOps to process it. %v", n.ro)
			n.issueNextPendingOp()
		*/
		log.Infof("Deleting in progress op. Looks like we rebooted in the middle of upgrade.")
		n.deleteDSCRollout()
	}
}

func (n *NMD) issueNextPendingOp() {
	log.Infof("On entry of issueNextPendingOp state is completed:%s inProgress:%s pending:%s", spew.Sdump(n.ro.CompletedOps), n.ro.InProgressOp, spew.Sdump(n.ro.PendingOps))
	if n.ro.InProgressOp.Op != protos.DSCOp_DSCNoOp {
		log.Infof("An op is currently in progress.  %v", n.ro.InProgressOp.Op)
		return // an op is already in progress...
	}

	if len(n.ro.PendingOps) == 0 {
		log.Info("No pending ops. Sending updates to Venice.")
		st := protos.DSCRolloutStatusUpdate{
			ObjectMeta: n.ro.ObjectMeta,
			Status: protos.DSCRolloutStatus{
				OpStatus: n.ro.OpStatus,
			},
		}
		if n.rollout == nil {
			log.Errorf("RolloutIf empty while updating status")
			return
		}

		err := n.rollout.UpdateDSCRolloutStatus(&st)
		if err != nil {
			log.Errorf("Failed to update Rollout err: %v", err)
		}
		return
	}
	op := n.ro.PendingOps[0]
	n.ro.PendingOps = append(n.ro.PendingOps[:0], n.ro.PendingOps[1:]...)

	n.updateRolloutStatus(op)

	var err error
	log.Infof("Issuing %#v serialNumber %#v", n.ro.InProgressOp, n.config.Status.Fru.SerialNum)
	switch n.ro.InProgressOp.Op {
	case protos.DSCOp_DSCDisruptiveUpgrade:
		if n.config.Status.Fru.SerialNum == simSerialNumber {
			log.Infof("SIM: upgrade completed")
			go n.UpgSuccessful()
			return
		}
		if _, err = os.Stat("/update/naples_fw.tar"); os.IsNotExist(err) {
			log.Errorf("/update/naples_fw.tar not found %s", err)
			go n.UpgFailed(&[]string{fmt.Sprintf("/update/naples_fw.tar not found %s", err)})
			return
		}
		err = n.Upgmgr.StartDisruptiveUpgrade("naples_fw.tar")
		if err != nil {
			log.Errorf("StartDisruptiveUpgrade returned %s", err)
			go n.UpgFailed(&[]string{fmt.Sprintf("StartDisruptiveUpgrade returned %s", err)})
			return
		}
	case protos.DSCOp_DSCUpgOnNextHostReboot:
		if n.config.Status.Fru.SerialNum == simSerialNumber {
			log.Infof("SIM: upgrade completed")
			go n.UpgSuccessful()
			return
		}
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
	case protos.DSCOp_DSCPreCheckForUpgOnNextHostReboot:
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
		if _, err = os.Stat("/update/naples_fw.tar"); os.IsNotExist(err) {
			log.Errorf("/update/naples_fw.tar not found %s", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("/update/naples_fw.tar not found %s", err)})
			cancel()
			return
		}
		cancel()
		if n.config.Status.Fru.SerialNum == simSerialNumber {
			log.Infof("SIM: image download completed")
			rerr := os.Remove("/update/naples_fw.tar")
			if rerr != nil {
				log.Errorf("SIM: removal of naples_fw.tar returned %v", rerr)
			}
			go n.UpgPossible()
			return
		}
		_, err = naplesPkgVerify("naples_fw.tar")
		if err != nil {
			log.Errorf("Firmware image verification failed %s", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("Firmware image verification failed %s", err)})
			return
		}
		go n.UpgPossible()
	case protos.DSCOp_DSCPreCheckForDisruptive:
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
		if _, err = os.Stat("/update/naples_fw.tar"); os.IsNotExist(err) {
			log.Errorf("/update/naples_fw.tar not found %s", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("/update/naples_fw.tar not found %s", err)})
			cancel()
			return
		}
		cancel()
		if n.config.Status.Fru.SerialNum == simSerialNumber {
			log.Infof("SIM: image download completed")
			rerr := os.Remove("/update/naples_fw.tar")
			if rerr != nil {
				log.Errorf("SIM: removal of naples_fw.tar returned %v", rerr)
			}
			go n.UpgPossible()
			return
		}
		err = n.Upgmgr.StartPreCheckDisruptive(n.ro.InProgressOp.Version)
		if err != nil {
			log.Errorf("StartPreCheckDisruptive returned %s", err)
			go n.UpgNotPossible(&[]string{fmt.Sprintf("StartPreCheckDisruptive returned %s", err)})
			return
		}
	case protos.DSCOp_DSCImageDownload:
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

	if n.ro.InProgressOp.Op != protos.DSCOp_DSCNoOp {
		n.updateOpStatus(n.ro.InProgressOp.Op, n.ro.InProgressOp.Version, "success", message)
		n.appendCompletedOps(n.ro.InProgressOp)
		n.updateRolloutStatus(protos.DSCOpSpec{Op: protos.DSCOp_DSCNoOp})
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

	if n.ro.InProgressOp.Op != protos.DSCOp_DSCNoOp {
		n.updateOpStatus(n.ro.InProgressOp.Op, n.ro.InProgressOp.Version, "failure", message)
		n.appendCompletedOps(n.ro.InProgressOp)
		n.updateRolloutStatus(protos.DSCOpSpec{Op: protos.DSCOp_DSCNoOp})
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
	if n.ro.InProgressOp.Op != protos.DSCOp_DSCNoOp {
		n.updateOpStatus(n.ro.InProgressOp.Op, n.ro.InProgressOp.Version, "success", message)
		n.appendCompletedOps(n.ro.InProgressOp)
		n.updateRolloutStatus(protos.DSCOpSpec{Op: protos.DSCOp_DSCNoOp})
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

	if n.ro.InProgressOp.Op != protos.DSCOp_DSCNoOp {
		n.updateOpStatus(n.ro.InProgressOp.Op, n.ro.InProgressOp.Version, "failure", message)
		n.appendCompletedOps(n.ro.InProgressOp)
		n.updateRolloutStatus(protos.DSCOpSpec{Op: protos.DSCOp_DSCNoOp})
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
func (n *NMD) updateOpStatus(op protos.DSCOp, version string, status string, message string) {
	log.Infof("Updating op status. Op: %v | Status: %s", op, status)
	var OpStatus []protos.DSCOpStatus
	for _, o := range n.ro.OpStatus {
		if o.Op != op || o.Version != version {
			OpStatus = append(OpStatus, o)
		}
	}

	OpStatus = append(OpStatus,
		protos.DSCOpStatus{
			Op:       op,
			Version:  version,
			OpStatus: status,
			Message:  message,
		})

	n.ro.OpStatus = OpStatus
	log.Infof("Updated NMD internal op status: %v", n.ro.OpStatus)

	st := protos.DSCRolloutStatusUpdate{
		ObjectMeta: n.ro.ObjectMeta,
		Status: protos.DSCRolloutStatus{
			OpStatus: OpStatus,
		},
	}
	if n.rollout == nil {
		log.Errorf("RolloutIf empty while updating status")
		return
	}

	err := n.rollout.UpdateDSCRolloutStatus(&st)
	if err != nil {
		log.Errorf("Failed to update Rollout err: %v", err)
	}
}
