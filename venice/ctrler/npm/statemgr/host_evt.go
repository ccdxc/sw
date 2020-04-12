// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	orchutils "github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// HostState is a wrapper for host object
type HostState struct {
	Host      *ctkit.Host `json:"-"` // host object
	stateMgr  *Statemgr   // pointer to state manager
	workloads sync.Map    // list of workloads
}

// HostStateFromObj conerts from memdb object to host state
func HostStateFromObj(obj runtime.Object) (*HostState, error) {
	switch obj.(type) {
	case *ctkit.Host:
		hobj := obj.(*ctkit.Host)
		switch hobj.HandlerCtx.(type) {
		case *HostState:
			nsobj := hobj.HandlerCtx.(*HostState)
			return nsobj, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewHostState creates new host state object
func NewHostState(host *ctkit.Host, stateMgr *Statemgr) (*HostState, error) {
	hs := &HostState{
		Host:     host,
		stateMgr: stateMgr,
	}
	host.HandlerCtx = hs

	return hs, nil
}

//GetHostWatchOptions gets options
func (sm *Statemgr) GetHostWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
	return &opts
}

// addWorkload adds a workload to host
func (hst *HostState) addWorkload(wrk *ctkit.Workload) {
	hst.workloads.Store(wrk.Name, wrk.ObjectMeta)
}

// removeWorkload removes a workload from host
func (hst *HostState) removeWorkload(wrk *ctkit.Workload) {
	hst.workloads.Delete(wrk.Name)
}

// OnHostCreate handles host creation
func (sm *Statemgr) OnHostCreate(host *ctkit.Host) error {
	// see if we already have the host
	hs, err := sm.FindHost(host.Tenant, host.Name)
	if err == nil {
		hs.Host = host
		return nil
	}

	log.Infof("Creating host: %+v", host)

	// create new host object
	hs, err = NewHostState(host, sm)
	if err != nil {
		log.Errorf("Error creating host %+v. Err: %v", host, err)
		return err
	}

	return nil
}

// OnHostUpdate handles host object update
func (sm *Statemgr) OnHostUpdate(host *ctkit.Host, nhst *cluster.Host) error {
	// see if we already have the host
	hs, err := sm.FindHost(host.Tenant, host.Name)
	if err != nil {
		return err
	}

	rescanEps := false
	// check if host mac address changed
	if len(host.Spec.DSCs) != len(nhst.Spec.DSCs) {
		rescanEps = true
	} else {
		for idx, sn := range nhst.Spec.DSCs {
			if host.Spec.DSCs[idx].ID != sn.ID || host.Spec.DSCs[idx].MACAddress != sn.MACAddress {
				rescanEps = true
			}
		}
	}

	hs.Host.Host = *nhst
	var snic *DistributedServiceCardState
	// find the smart nic by name or mac addr
	for jj := range hs.Host.Spec.DSCs {
		if hs.Host.Spec.DSCs[jj].ID != "" {
			snic, err = sm.FindDistributedServiceCardByHname(hs.Host.Spec.DSCs[jj].ID)
			if err != nil {
				log.Errorf("Error finding smart nic for name %v", hs.Host.Spec.DSCs[jj].ID)
			}
		} else if hs.Host.Spec.DSCs[jj].MACAddress != "" {
			snicMac := hs.Host.Spec.DSCs[jj].MACAddress
			snic, err = sm.FindDistributedServiceCardByMacAddr(snicMac)
			if err != nil {
				log.Errorf("Error finding smart nic for mac add %v", snicMac)
			}
		}
	}

	if snic != nil {
		ok := false
		isHostOrchhubManaged := false
		orchNameValue := ""

		if len(hs.Host.Labels) > 0 {
			orchNameValue, isHostOrchhubManaged = hs.Host.Labels[orchutils.OrchNameKey]
		}

		if len(snic.DistributedServiceCard.Labels) > 0 {
			_, ok = snic.DistributedServiceCard.Labels[orchutils.OrchNameKey]
		}

		if !ok && isHostOrchhubManaged {
			// Add Orchhub label to the DSC object
			if len(snic.DistributedServiceCard.Labels) == 0 {
				snic.DistributedServiceCard.Labels = make(map[string]string)
			}

			snic.DistributedServiceCard.Labels[orchutils.OrchNameKey] = orchNameValue
			err := snic.stateMgr.ctrler.DistributedServiceCard().Update(&snic.DistributedServiceCard.DistributedServiceCard)
			if err != nil {
				log.Errorf("Failed to update orchhub label for DSC [%v]. Err : %v", snic.DistributedServiceCard.Name, err)
			}
		} else if ok && !isHostOrchhubManaged && len(snic.DistributedServiceCard.Labels) > 0 {
			// Remove Label from DSC object
			delete(snic.DistributedServiceCard.Labels, orchutils.OrchNameKey)
			err := snic.stateMgr.ctrler.DistributedServiceCard().Update(&snic.DistributedServiceCard.DistributedServiceCard)
			if err != nil {
				log.Errorf("Failed to remove orhchub label from DSC object [%v]. Err : %v", snic.DistributedServiceCard.Name, err)
			}
		}
	}

	if rescanEps {
		hs.workloads.Range(func(key, value interface{}) bool {
			wmeta := value.(api.ObjectMeta)
			wrk, err := sm.FindWorkload(wmeta.Tenant, wmeta.Name)
			if err == nil {
				sm.reconcileWorkload(wrk.Workload, hs, snic)
			}
			return true
		})
	}

	return nil
}

// OnHostDelete handles host deletion
func (sm *Statemgr) OnHostDelete(host *ctkit.Host) error {
	// nothing to do
	hs, err := sm.FindHost(host.Tenant, host.Name)
	if err != nil {
		return err
	}

	if len(hs.Host.Labels) > 0 {
		_, isHostOrchhubManaged := hs.Host.Labels[orchutils.OrchNameKey]
		if isHostOrchhubManaged {
			var snic *DistributedServiceCardState
			// find the smart nic by name or mac addr
			for jj := range hs.Host.Spec.DSCs {
				if hs.Host.Spec.DSCs[jj].ID != "" {
					snic, err = sm.FindDistributedServiceCardByHname(hs.Host.Spec.DSCs[jj].ID)
					if err != nil {
						log.Errorf("Error finding smart nic for name %v", hs.Host.Spec.DSCs[jj].ID)
					}
				} else if hs.Host.Spec.DSCs[jj].MACAddress != "" {
					snicMac := hs.Host.Spec.DSCs[jj].MACAddress
					snic, err = sm.FindDistributedServiceCardByMacAddr(snicMac)
					if err != nil {
						log.Errorf("Error finding smart nic for mac add %v", snicMac)
					}
				}
			}

			if snic != nil && len(snic.DistributedServiceCard.ObjectMeta.Labels) > 0 {
				delete(snic.DistributedServiceCard.Labels, orchutils.OrchNameKey)
				err := snic.stateMgr.ctrler.DistributedServiceCard().Update(&snic.DistributedServiceCard.DistributedServiceCard)
				if err != nil {
					log.Errorf("Failed to update DSC. Err : %v", err)
				}
			}
		}
	}

	return nil
}

// FindHost finds a host
func (sm *Statemgr) FindHost(tenant, name string) (*HostState, error) {
	// find the object
	obj, err := sm.FindObject("Host", "", "", name)
	if err != nil {
		return nil, err
	}

	return HostStateFromObj(obj)
}
