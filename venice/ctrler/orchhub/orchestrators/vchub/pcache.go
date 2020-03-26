package vchub

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils/pcache"
	"github.com/pensando/sw/venice/globals"
)

const workloadVnicKind = "workloadVnic"

type vnicEntry struct {
	PG              string
	Port            string
	IP              []string
	MacAddress      string
	portOverrideSet bool
}

type workloadVnics struct {
	// All objects in pCache must be a apiserver runtime object
	api.ObjectMeta
	api.TypeMeta
	Interfaces map[string]*vnicEntry
}

func (v *VCHub) setupPCache() {
	pCache := pcache.NewPCache(v.StateMgr, v.Log)
	pCache.SetValidator(workloadKind, v.validateWorkload)
	pCache.SetValidator(workloadVnicKind, validateWorkloadVnics)
	v.pCache = pCache
}

func validateWorkloadVnics(in interface{}) (bool, bool) {
	// This object should never go to statemgr
	return false, false
}

func createWorkloadVnicsMeta(workloadName string) *api.ObjectMeta {
	meta := &api.ObjectMeta{
		Name:      workloadName,
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	return meta
}

func (v *VCHub) getWorkloadVnics(workloadName string) *workloadVnics {
	p := v.pCache
	meta := createWorkloadVnicsMeta(workloadName)
	obj := p.Get(workloadVnicKind, meta)
	if obj == nil {
		return nil
	}
	switch ret := obj.(type) {
	case *workloadVnics:
		return ret
	default:
		p.Log.Errorf("VNIC returned wasn't the expected type %v", ret)
	}
	return nil
}

func (v *VCHub) setWorkloadVnicsObject(in *workloadVnics) {
	// Since we are only setting locally
	// we don't have to worry about errors
	v.pCache.Set(workloadVnicKind, in)
}

func (v *VCHub) deleteWorkloadVnicsObject(workloadName string) {
	meta := createWorkloadVnicsMeta(workloadName)
	v.pCache.Delete(workloadVnicKind, meta)
}

// adds vnic info for the given workload,mac
// Will create the workloadVnics object if it doesn't exist
func (v *VCHub) addVnicInfoForWorkload(workloadName string, inf *vnicEntry) {
	wl := v.getWorkloadVnics(workloadName)
	if wl == nil {
		wl = &workloadVnics{
			ObjectMeta: *createWorkloadVnicsMeta(workloadName),
			Interfaces: map[string]*vnicEntry{},
		}
	}
	wl.Interfaces[inf.MacAddress] = inf
	v.setWorkloadVnicsObject(wl)
}

// Gets vnic info for the given workload,mac
func (v *VCHub) getVnicInfoForWorkload(workloadName, macAddress string) *vnicEntry {
	wl := v.getWorkloadVnics(workloadName)
	if wl == nil {
		return nil
	}
	return wl.Interfaces[macAddress]
}

// Removes vnic info for the given mac. If there are no vnics left,
// it will remove the workloadVnic object wrapper as well
func (v *VCHub) removeVnicInfoForWorkload(workloadName, macAddress string) {
	wl := v.getWorkloadVnics(workloadName)
	if wl == nil {
		return
	}
	if _, ok := wl.Interfaces[macAddress]; ok {
		delete(wl.Interfaces, macAddress)
	}
	if len(wl.Interfaces) == 0 {
		v.deleteWorkloadVnicsObject(workloadName)
	}
}
