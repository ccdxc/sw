package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/runtime"
)

// DistributedServiceCardState is a wrapper for cluster object
type DistributedServiceCardState struct {
	sync.Mutex
	DistributedServiceCard *ctkit.DistributedServiceCard
	stateMgr               *Statemgr
	profile                string
	admission              string
	orchVal                string
}

//GetDistributedServiceCardWatchOptions gets options
func (sm *Statemgr) GetDistributedServiceCardWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec.DSCProfile", "Status.AdmissionPhase"}
	return &opts
}

// OnDistributedServiceCardCreate creates a DistributedServiceCard based on watch event
func (sm *Statemgr) OnDistributedServiceCardCreate(dsc *ctkit.DistributedServiceCard) error {
	dscState, err := NewDSCState(dsc, sm)
	if err != nil {
		return err
	}

	dscState.profile = dsc.DistributedServiceCard.Spec.DSCProfile
	dscState.admission = dsc.DistributedServiceCard.Status.AdmissionPhase

	dps, err := sm.FindDSCProfile("", dsc.DistributedServiceCard.Spec.DSCProfile)
	if err != nil {
		return err
	}

	dps.Lock()
	dps.dscs[dsc.DistributedServiceCard.Name] = true
	dps.Unlock()

	return nil
}

// OnDistributedServiceCardUpdate handles update event
func (sm *Statemgr) OnDistributedServiceCardUpdate(dsc *ctkit.DistributedServiceCard, ndsc *cluster.DistributedServiceCard) error {
	dscState, err := DistributedServiceCardStateFromObj(dsc)
	if err != nil {
		sm.logger.Errorf("Error finding smartnic. Err: %v", err)
		return err
	}
	oldProfile := dscState.profile
	newProfile := ndsc.Spec.DSCProfile

	justAdmitted := false
	oldAdmit := dscState.admission
	newAdmit := ndsc.Status.AdmissionPhase
	if oldAdmit != cluster.DistributedServiceCardStatus_ADMITTED.String() && newAdmit == cluster.DistributedServiceCardStatus_ADMITTED.String() {
		// If the DSC is just admitted, we must check the mode incompatibility
		// This is for a case when the DSC was not admitted when the HOST was created
		justAdmitted = true
	}

	sm.logger.Infof("Old Profile : %v, New Profile : %v, Newly admitted : %v", oldProfile, newProfile, justAdmitted)

	oldOrchVal := dscState.orchVal

	// Update the DSC
	dscState.DistributedServiceCard.DistributedServiceCard = *ndsc
	dscState.profile = newProfile
	dscState.admission = newAdmit
	orchVal, ok := dscState.DistributedServiceCard.Labels[utils.OrchNameKey]
	if (oldProfile != newProfile) || justAdmitted {
		dscState.orchVal = orchVal

		// If label is found, and the DSC is not compatible, raise an event
		if ok {
			if !dscState.isOrchestratorCompatible() {
				sm.AddIncompatibleDSCToOrch(dscState.DistributedServiceCard.Name, orchVal)
				recorder.Event(eventtypes.ORCH_DSC_MODE_INCOMPATIBLE,
					fmt.Sprintf("Profile [%v] added to DSC[%v] is incompatible with orchestration feature", newProfile, dscState.DistributedServiceCard.DistributedServiceCard.Name),
					nil)
			} else {
				sm.RemoveIncompatibleDSCFromOrch(dscState.DistributedServiceCard.Name, orchVal)
			}
		}

		if len(oldOrchVal) > 0 && oldOrchVal != orchVal {
			sm.RemoveIncompatibleDSCFromOrch(dscState.DistributedServiceCard.Name, oldOrchVal)
		}

		oldDps, err := sm.FindDSCProfile("", oldProfile)
		if err != nil {
			return err
		}
		oldDps.Lock()
		delete(oldDps.dscs, dscState.DistributedServiceCard.DistributedServiceCard.Name)
		oldDps.Unlock()

		newDps, err := sm.FindDSCProfile("", newProfile)
		if err != nil {
			return err
		}

		newDps.Lock()
		newDps.dscs[dscState.DistributedServiceCard.DistributedServiceCard.Name] = true
		newDps.Unlock()
	}

	return nil
}

// OnDistributedServiceCardDelete deletes a DistributedServiceCard
func (sm *Statemgr) OnDistributedServiceCardDelete(dsc *ctkit.DistributedServiceCard) error {
	dps, err := sm.FindDSCProfile("", dsc.DistributedServiceCard.Spec.DSCProfile)
	if err != nil {
		return nil
	}

	orchVal, ok := dsc.DistributedServiceCard.Labels[utils.OrchNameKey]
	if ok {
		sm.RemoveIncompatibleDSCFromOrch(dsc.DistributedServiceCard.Name, orchVal)
	}

	dps.Lock()
	delete(dps.dscs, dsc.DistributedServiceCard.Name)
	dps.Unlock()

	return err
}

// OnDistributedServiceCardReconnect is called when ctkit reconnects to apiserver
func (sm *Statemgr) OnDistributedServiceCardReconnect() {
	return
}

// NewDSCState create new DSC state
func NewDSCState(dsc *ctkit.DistributedServiceCard, stateMgr *Statemgr) (*DistributedServiceCardState, error) {
	w := &DistributedServiceCardState{
		DistributedServiceCard: dsc,
		stateMgr:               stateMgr,
	}
	dsc.HandlerCtx = w

	return w, nil
}

// DistributedServiceCardStateFromObj conerts from memdb object to cluster state
func DistributedServiceCardStateFromObj(obj runtime.Object) (*DistributedServiceCardState, error) {
	switch obj.(type) {
	case *ctkit.DistributedServiceCard:
		nobj := obj.(*ctkit.DistributedServiceCard)
		switch nobj.HandlerCtx.(type) {
		case *DistributedServiceCardState:
			nts := nobj.HandlerCtx.(*DistributedServiceCardState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}

func (dscState *DistributedServiceCardState) isOrchestratorCompatible() bool {
	dscProfile := dscState.DistributedServiceCard.Spec.DSCProfile
	dscProfileState, _ := dscState.stateMgr.FindDSCProfile("default", dscProfile)
	if dscProfileState == nil {
		return false
	}

	if dscProfileState.DSCProfile.Spec.FwdMode != cluster.DSCProfileSpec_INSERTION.String() || dscProfileState.DSCProfile.Spec.FlowPolicyMode != cluster.DSCProfileSpec_ENFORCED.String() {
		return false
	}

	return true
}

// FindDSC Get DSC State by ID
func (sm *Statemgr) FindDSC(mac, id string) *DistributedServiceCardState {
	objs := sm.ctrler.ListObjects("DistributedServiceCard")

	for _, obj := range objs {
		snic, err := DistributedServiceCardStateFromObj(obj)
		if err != nil {
			continue
		}

		if snic.DistributedServiceCard.Status.PrimaryMAC == mac {
			return snic
		}
	}

	for _, obj := range objs {
		snic, err := DistributedServiceCardStateFromObj(obj)
		if err != nil {
			continue
		}

		if snic.DistributedServiceCard.Spec.ID == id {
			return snic
		}
	}
	return nil
}
