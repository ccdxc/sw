// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// SmartNICState is a wrapper for smartNic object
type SmartNICState struct {
	SmartNIC *ctkit.SmartNIC `json:"-"` // smartNic object
	stateMgr *Statemgr       // pointer to state manager
}

// SmartNICStateFromObj conerts from memdb object to smartNic state
func SmartNICStateFromObj(obj runtime.Object) (*SmartNICState, error) {
	switch obj.(type) {
	case *ctkit.SmartNIC:
		sobj := obj.(*ctkit.SmartNIC)
		switch sobj.HandlerCtx.(type) {
		case *SmartNICState:
			nsobj := sobj.HandlerCtx.(*SmartNICState)
			return nsobj, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewSmartNICState creates new smartNic state object
func NewSmartNICState(smartNic *ctkit.SmartNIC, stateMgr *Statemgr) (*SmartNICState, error) {
	hs := &SmartNICState{
		SmartNIC: smartNic,
		stateMgr: stateMgr,
	}
	smartNic.HandlerCtx = hs

	return hs, nil
}

// OnSmartNICCreate handles smartNic creation
func (sm *Statemgr) OnSmartNICCreate(smartNic *ctkit.SmartNIC) error {
	// see if we already have the smartNic
	sns, err := sm.FindSmartNIC(smartNic.Tenant, smartNic.Name)
	if err == nil {
		sns.SmartNIC = smartNic
		return nil
	}

	log.Infof("Creating smart nic: %+v", smartNic)

	// create new smartNic object
	sns, err = NewSmartNICState(smartNic, sm)
	if err != nil {
		log.Errorf("Error creating smartNic %+v. Err: %v", smartNic, err)
		return err
	}

	// see if smartnic is haelthy
	if sm.isSmartNICHealthy(&smartNic.SmartNIC) {
		// Update SGPolicies
		policies, _ := sm.ListSgpolicies()
		for _, policy := range policies {
			if _, ok := policy.NodeVersions[smartNic.SmartNIC.Name]; ok == false {
				policy.NodeVersions[smartNic.SmartNIC.Name] = ""
				sm.PeriodicUpdaterPush(policy)
			}
		}

		// Update FirewallProfiles
		fwprofiles, _ := sm.ListFirewallProfiles()
		for _, fwprofile := range fwprofiles {
			if _, ok := fwprofile.NodeVersions[smartNic.SmartNIC.Name]; ok == false {
				fwprofile.NodeVersions[smartNic.SmartNIC.Name] = ""
				sm.PeriodicUpdaterPush(fwprofile)
			}
		}
	}

	// walk all hosts and see if they need to be associated to this snic
	for _, host := range sm.ctrler.Host().List() {
		associated := false
		for _, snid := range host.Spec.SmartNICs {
			if (snid.ID == smartNic.SmartNIC.Spec.ID) || (snid.MACAddress == smartNic.SmartNIC.Status.PrimaryMAC) {
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

// OnSmartNICUpdate handles update event on smartnic
func (sm *Statemgr) OnSmartNICUpdate(smartNic *ctkit.SmartNIC, nsnic *cluster.SmartNIC) error {
	// see if we already have the smartNic
	hs, err := SmartNICStateFromObj(smartNic)
	if err != nil {
		log.Errorf("Error finding smartnic. Err: %v", err)
		return err
	}

	hs.SmartNIC.SmartNIC = *nsnic

	// Update SGPolicies
	policies, _ := sm.ListSgpolicies()
	for _, policy := range policies {
		if sm.isSmartNICHealthy(nsnic) {
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
	}

	// update firewall profiles
	fwprofiles, _ := sm.ListFirewallProfiles()
	for _, fwprofile := range fwprofiles {
		if sm.isSmartNICHealthy(nsnic) {
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

		}
	}

	return nil
}

// OnSmartNICDelete handles smartNic deletion
func (sm *Statemgr) OnSmartNICDelete(smartNic *ctkit.SmartNIC) error {
	// see if we have the smartNic
	hs, err := SmartNICStateFromObj(smartNic)
	if err != nil {
		log.Errorf("Could not find the smartNic %v. Err: %v", smartNic, err)
		return err
	}

	log.Infof("Deleting smart nic: %+v", smartNic)

	// Update SGPolicies
	policies, _ := sm.ListSgpolicies()
	for _, policy := range policies {
		_, ok := policy.NodeVersions[hs.SmartNIC.Name]
		if ok {
			delete(policy.NodeVersions, hs.SmartNIC.Name)
			sm.PeriodicUpdaterPush(policy)
		}
	}

	fwprofiles, _ := sm.ListFirewallProfiles()
	for _, fwprofile := range fwprofiles {
		_, ok := fwprofile.NodeVersions[hs.SmartNIC.Name]
		if ok {
			delete(fwprofile.NodeVersions, hs.SmartNIC.Name)
			sm.PeriodicUpdaterPush(fwprofile)
		}
	}

	// walk all hosts and see if they need to be dis-associated to this snic
	for _, host := range sm.ctrler.Host().List() {
		associated := false
		for _, snid := range host.Spec.SmartNICs {
			if (snid.ID == smartNic.SmartNIC.Spec.ID) || (snid.MACAddress == smartNic.SmartNIC.Status.PrimaryMAC) {
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

// FindSmartNIC finds a smartNic
func (sm *Statemgr) FindSmartNIC(tenant, name string) (*SmartNICState, error) {
	// find the object
	obj, err := sm.FindObject("SmartNIC", "", "", name)
	if err != nil {
		return nil, err
	}

	return SmartNICStateFromObj(obj)
}

// FindSmartNICByMacAddr finds the smart nic by mac addr
func (sm *Statemgr) FindSmartNICByMacAddr(macAddr string) (*SmartNICState, error) {
	objs := sm.ListObjects("SmartNIC")

	for _, obj := range objs {
		snic, err := SmartNICStateFromObj(obj)
		if err != nil {
			return nil, err
		}

		if snic.SmartNIC.Status.PrimaryMAC == macAddr {
			return snic, nil
		}
	}

	return nil, fmt.Errorf("Smartnic not found for mac addr %v", macAddr)
}

// FindSmartNICByHname finds smart nic by used given name
func (sm *Statemgr) FindSmartNICByHname(hname string) (*SmartNICState, error) {
	objs := sm.ListObjects("SmartNIC")

	for _, obj := range objs {
		snic, err := SmartNICStateFromObj(obj)
		if err != nil {
			return nil, err
		}

		if snic.SmartNIC.Spec.ID == hname {
			return snic, nil
		}
	}

	return nil, fmt.Errorf("Smartnic not found for name %v", hname)
}

// isSmartNICHealthy returns true if smartnic is in healthry condition
func (sm *Statemgr) isSmartNICHealthy(nsnic *cluster.SmartNIC) bool {
	isHealthy := false
	if len(nsnic.Status.Conditions) > 0 {
		for _, cond := range nsnic.Status.Conditions {
			if cond.Type == "HEALTHY" && cond.Status == "TRUE" {
				isHealthy = true
			}
		}
	}

	return isHealthy
}
