// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb/objReceiver"
	"github.com/pensando/sw/venice/utils/runtime"
)

// DistributedServiceCardState is a wrapper for smartNic object
type DistributedServiceCardState struct {
	DistributedServiceCard *ctkit.DistributedServiceCard `json:"-"` // smartNic object
	stateMgr               *Statemgr                     // pointer to state manager
	recvHandle             objReceiver.Receiver
	profileVersion         dscProfileVersion
}

//GetDistributedServiceCardWatchOptions gets options
func (sm *Statemgr) GetDistributedServiceCardWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{}
	return &opts
}

// DistributedServiceCardStateFromObj converts from memdb object to smartNic state
func DistributedServiceCardStateFromObj(obj runtime.Object) (*DistributedServiceCardState, error) {
	switch obj.(type) {
	case *ctkit.DistributedServiceCard:
		sobj := obj.(*ctkit.DistributedServiceCard)
		switch sobj.HandlerCtx.(type) {
		case *DistributedServiceCardState:
			nsobj := sobj.HandlerCtx.(*DistributedServiceCardState)
			return nsobj, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewDistributedServiceCardState creates new smartNic state object
func NewDistributedServiceCardState(smartNic *ctkit.DistributedServiceCard, stateMgr *Statemgr) (*DistributedServiceCardState, error) {
	recvHandle, err := stateMgr.mbus.AddReceiver(smartNic.Status.PrimaryMAC)
	if err != nil {
		return nil, fmt.Errorf("Error add dsc %v", err)
	}
	log.Infof("Added DSC %v as a receiver", smartNic.Status.PrimaryMAC)
	hs := &DistributedServiceCardState{
		DistributedServiceCard: smartNic,
		stateMgr:               stateMgr,
		recvHandle:             recvHandle,
	}
	smartNic.HandlerCtx = hs

	return hs, nil
}

// OnDistributedServiceCardCreate handles smartNic creation
func (sm *Statemgr) OnDistributedServiceCardCreate(smartNic *ctkit.DistributedServiceCard) error {
	log.Infof("Creating smart nic: %+v", smartNic)

	// create new smartNic object
	sns, err := NewDistributedServiceCardState(smartNic, sm)
	if err != nil {
		log.Errorf("Error creating smartNic %+v. Err: %v", smartNic, err)
		return err
	}

	log.Infof("Profile %s", smartNic.DistributedServiceCard.Spec.DSCProfile)
	profName := smartNic.DistributedServiceCard.Spec.DSCProfile
	if profName != "" {
		profileState, err := sm.FindDSCProfile("", profName)
		if err == nil {
			log.Infof("Found a profile send it to the DSC")
			profileState.DscList[smartNic.ObjectMeta.Name] = dscProfileVersion{
				profileState.DSCProfile.Name,
				profileState.DSCProfile.GenerationID}
			profileState.DSCProfile.Lock()
			if sm.isDscAdmitted(&smartNic.DistributedServiceCard) {
				ret := profileState.PushObj.AddObjReceivers([]objReceiver.Receiver{sns.recvHandle})
				if ret != nil {
					log.Infof("Add receiver failed %v", ret)
				}
			}
			profileState.DSCProfile.Unlock()
		}
	}

	// see if smartnic is admitted
	if sm.isDscAdmitted(&smartNic.DistributedServiceCard) {
		// Update SGPolicies
		policies, _ := sm.ListSgpolicies()
		for _, policy := range policies {
			policy.NetworkSecurityPolicy.Lock()
			if _, ok := policy.NodeVersions[smartNic.DistributedServiceCard.Name]; ok == false {
				policy.NodeVersions[smartNic.DistributedServiceCard.Name] = ""
				sm.PeriodicUpdaterPush(policy)
			}
			policy.NetworkSecurityPolicy.Unlock()
		}

		// Update FirewallProfiles
		fwprofiles, _ := sm.ListFirewallProfiles()
		for _, fwprofile := range fwprofiles {
			fwprofile.FirewallProfile.Lock()
			if _, ok := fwprofile.NodeVersions[smartNic.DistributedServiceCard.Name]; ok == false {
				fwprofile.NodeVersions[smartNic.DistributedServiceCard.Name] = ""
				sm.PeriodicUpdaterPush(fwprofile)
			}
			fwprofile.FirewallProfile.Unlock()
		}
	}

	hosts, err := sm.ctrler.Host().List(context.Background(), &api.ListWatchOptions{})
	if err != nil {
		log.Errorf("failed to get list of all hosts. Err : %v", err)
	}

	// walk all hosts and ee if they need to be associated to this snic
	for _, host := range hosts {
		associated := false
		for _, snid := range host.Spec.DSCs {
			if (snid.ID == smartNic.DistributedServiceCard.Spec.ID) || (snid.MACAddress == smartNic.DistributedServiceCard.Status.PrimaryMAC) {
				associated = true
			}
		}

		// if this host and smartnic are associated, trigger workload reconciliation
		if associated {
			host.Lock()
			hs, err := sm.FindHost(host.Tenant, host.Name)
			if err == nil {
				hs.workloads.Range(func(key, value interface{}) bool {
					wmeta := value.(api.ObjectMeta)
					wrk, err := sm.FindWorkload(wmeta.Tenant, wmeta.Name)
					if err == nil {
						sm.reconcileWorkload(wrk.Workload, hs, sns)
					} else {
						log.Errorf("Error finding workload. Err: %v", err)
					}
					return true
				})
			} else {
				log.Errorf("Error finding host %v. Err: %v", host.Name, err)
			}
			host.Unlock()
		}
	}

	return nil
}

// OnDistributedServiceCardUpdate handles update event on smartnic
func (sm *Statemgr) OnDistributedServiceCardUpdate(smartNic *ctkit.DistributedServiceCard, nsnic *cluster.DistributedServiceCard) error {
	// see if we already have the smartNic
	log.Infof("Update of DistributedServiceCard")
	hs, err := DistributedServiceCardStateFromObj(smartNic)
	if err != nil {
		log.Errorf("Error finding smartnic. Err: %v", err)
		return err
	}

	newProfile := nsnic.Spec.DSCProfile
	oldProfile := smartNic.DistributedServiceCard.Spec.DSCProfile
	log.Infof("Old:%s new: %s", oldProfile, newProfile)

	if newProfile != oldProfile {
		//Find the newProfile
		if newProfile != "" {
			profileState, err := sm.FindDSCProfile("", newProfile)
			if err == nil {
				log.Infof("Found the profile: update the agent ")
				profileState.DscList[nsnic.ObjectMeta.Name] = dscProfileVersion{
					profileState.DSCProfile.Name, profileState.DSCProfile.GenerationID}

				profileState.DSCProfile.Lock()
				if sm.isDscAdmitted(nsnic) {
					profileState.PushObj.AddObjReceivers([]objReceiver.Receiver{hs.recvHandle})
				}
				profileState.DSCProfile.Unlock()
			}
		}

		if oldProfile != "" {
			oldProfileState, _ := sm.FindDSCProfile("", oldProfile)
			if _, ok := oldProfileState.DscList[nsnic.ObjectMeta.Name]; ok {
				delete(oldProfileState.DscList, nsnic.ObjectMeta.Name)

				oldProfileState.PushObj.RemoveObjReceivers([]objReceiver.Receiver{hs.recvHandle})
			}
		}
	} else {
		log.Infof("No change in profile")
	}

	hs.DistributedServiceCard.DistributedServiceCard = *nsnic

	// Update SGPolicies
	policies, _ := sm.ListSgpolicies()
	for _, policy := range policies {
		policy.NetworkSecurityPolicy.Lock()
		if sm.isDscAdmitted(nsnic) {
			if _, ok := policy.NodeVersions[nsnic.Name]; !ok {
				policy.NodeVersions[nsnic.Name] = ""
				sm.PeriodicUpdaterPush(policy)
			}
		} else {
			_, ok := policy.NodeVersions[nsnic.Name]
			if ok {
				delete(policy.NodeVersions, nsnic.Name)
				sm.PeriodicUpdaterPush(policy)
			}
		}
		policy.NetworkSecurityPolicy.Unlock()
	}

	// update firewall profiles
	fwprofiles, _ := sm.ListFirewallProfiles()
	for _, fwprofile := range fwprofiles {
		if sm.isDscAdmitted(nsnic) {
			fwprofile.FirewallProfile.Lock()
			if _, ok := fwprofile.NodeVersions[nsnic.Name]; ok == false {
				fwprofile.NodeVersions[nsnic.Name] = ""
				sm.PeriodicUpdaterPush(fwprofile)
			} else {
				_, ok := fwprofile.NodeVersions[nsnic.Name]
				if ok {
					delete(fwprofile.NodeVersions, nsnic.Name)
					sm.PeriodicUpdaterPush(fwprofile)
				}
			}
			fwprofile.FirewallProfile.Unlock()
		}
	}
	return nil
}

// OnDistributedServiceCardDelete handles smartNic deletion
func (sm *Statemgr) OnDistributedServiceCardDelete(smartNic *ctkit.DistributedServiceCard) error {
	// see if we have the smartNic
	hs, err := DistributedServiceCardStateFromObj(smartNic)
	if err != nil {
		log.Errorf("Could not find the smartNic %v. Err: %v", smartNic, err)
		return err
	}

	log.Infof("Deleting smart nic: %+v", smartNic)
	defer func() {
		err := sm.mbus.DeleteReceiver(hs.recvHandle)
		if err != nil {
			log.Errorf("Error deleting receiver %v", err.Error())
		}
		log.Infof("Removed DSC %v as a receiver", hs.DistributedServiceCard.Status.PrimaryMAC)
	}()
	// Update SGPolicies
	policies, _ := sm.ListSgpolicies()
	for _, policy := range policies {
		policy.NetworkSecurityPolicy.Lock()
		_, ok := policy.NodeVersions[hs.DistributedServiceCard.Name]
		if ok {
			delete(policy.NodeVersions, hs.DistributedServiceCard.Name)
			sm.PeriodicUpdaterPush(policy)
		}
		policy.NetworkSecurityPolicy.Unlock()
	}

	fwprofiles, _ := sm.ListFirewallProfiles()
	for _, fwprofile := range fwprofiles {
		fwprofile.FirewallProfile.Lock()
		_, ok := fwprofile.NodeVersions[hs.DistributedServiceCard.Name]
		if ok {
			delete(fwprofile.NodeVersions, hs.DistributedServiceCard.Name)
			sm.PeriodicUpdaterPush(fwprofile)
		}
		fwprofile.FirewallProfile.Unlock()
	}

	hosts, err := sm.ctrler.Host().List(context.Background(), &api.ListWatchOptions{})
	if err != nil {
		log.Errorf("Failed to get list of all hosts. Err : %v", err)
		return err
	}

	// walk all hosts and see if they need to be dis-associated to this snic
	for _, host := range hosts {
		associated := false
		for _, snid := range host.Spec.DSCs {
			if (snid.ID == smartNic.DistributedServiceCard.Spec.ID) || (snid.MACAddress == smartNic.DistributedServiceCard.Status.PrimaryMAC) {
				associated = true
			}
		}

		// if this host and smartnic are associated, trigger workload reconciliation
		if associated {
			hs, err := sm.FindHost(host.Tenant, host.Name)
			if err == nil {
				hs.workloads.Range(func(key, value interface{}) bool {
					wmeta := value.(api.ObjectMeta)
					wrk, err := sm.FindWorkload(wmeta.Tenant, wmeta.Name)
					if err == nil {
						sm.reconcileWorkload(wrk.Workload, hs, nil)
					} else {
						log.Errorf("Error finding workload. Err: %v", err)
					}
					return true
				})
			} else {
				log.Errorf("Error finding host %v. Err: %v", host.Name, err)
				return err
			}
		}
	}

	return nil
}

// FindDistributedServiceCard finds a smartNic
func (sm *Statemgr) FindDistributedServiceCard(tenant, name string) (*DistributedServiceCardState, error) {
	// find the object
	obj, err := sm.FindObject("DistributedServiceCard", "", "", name)
	if err != nil {
		return nil, err
	}

	return DistributedServiceCardStateFromObj(obj)
}

// FindDistributedServiceCardByMacAddr finds the smart nic by mac addr
func (sm *Statemgr) FindDistributedServiceCardByMacAddr(macAddr string) (*DistributedServiceCardState, error) {
	objs := sm.ListObjects("DistributedServiceCard")

	for _, obj := range objs {
		snic, err := DistributedServiceCardStateFromObj(obj)
		if err != nil {
			return nil, err
		}

		if snic.DistributedServiceCard.Status.PrimaryMAC == macAddr {
			return snic, nil
		}
	}

	return nil, fmt.Errorf("Smartnic not found for mac addr %v", macAddr)
}

// FindDistributedServiceCardByHname finds smart nic by used given name
func (sm *Statemgr) FindDistributedServiceCardByHname(hname string) (*DistributedServiceCardState, error) {
	objs := sm.ListObjects("DistributedServiceCard")

	for _, obj := range objs {
		snic, err := DistributedServiceCardStateFromObj(obj)
		if err != nil {
			return nil, err
		}

		if snic.DistributedServiceCard.Spec.ID == hname {
			return snic, nil
		}
	}

	//Update the DSC object with the profile, genID

	return nil, fmt.Errorf("Smartnic not found for name %v", hname)
}

// ListDistributedServiceCards lists all smart nics
func (sm *Statemgr) ListDistributedServiceCards() ([]*DistributedServiceCardState, error) {
	objs := sm.ListObjects("DistributedServiceCard")

	var dscs []*DistributedServiceCardState
	for _, obj := range objs {
		dsc, err := DistributedServiceCardStateFromObj(obj)
		if err != nil {
			return dscs, err
		}

		dscs = append(dscs, dsc)
	}

	return dscs, nil
}

// isDscHealthy returns true if smartnic is in healthry condition
func (sm *Statemgr) isDscHealthy(nsnic *cluster.DistributedServiceCard) bool {
	isHealthy := false
	log.Infof("DSC STATSUS %v", nsnic.Status.Conditions)
	if len(nsnic.Status.Conditions) > 0 {
		for _, cond := range nsnic.Status.Conditions {
			if cond.Type == cluster.DSCCondition_HEALTHY.String() && cond.Status == cluster.ConditionStatus_TRUE.String() {
				isHealthy = true
			}
		}
	}

	return isHealthy
}

// isDscAdmitted returns true if the DSC is admited into the cluster
func (sm *Statemgr) isDscAdmitted(nsnic *cluster.DistributedServiceCard) bool {
	return nsnic.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String()
}
