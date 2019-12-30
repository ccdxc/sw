package vchub

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils/pcache"
	"github.com/pensando/sw/venice/globals"
)

const vnicKind = "VNIC"

type vnicStruct struct {
	// All objects in pCache must be a apiserver runtime object
	api.ObjectMeta
	api.TypeMeta
	PG       string
	Port     string
	Workload string
}

func (v *VCHub) setupPCache() {
	pCache := pcache.NewPCache(v.StateMgr, v.Log)
	pCache.SetValidator(workloadKind, v.validateWorkload)
	pCache.SetValidator(vnicKind, validateVNIC)
	v.pCache = pCache
	v.Wg.Add(1)
	go pCache.Run(v.Ctx, v.Wg)
}

func validateVNIC(in interface{}) (bool, bool) {
	// This object should never go to statemgr
	return false, false
}

func createVNICMeta(macAddress string) *api.ObjectMeta {
	meta := &api.ObjectMeta{
		Name: macAddress,
		// TODO: Don't use default tenant
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	return meta
}

// GetVNIC retrieves a vnic
func (v *VCHub) getVNIC(macAddress string) *vnicStruct {
	p := v.pCache
	meta := createVNICMeta(macAddress)
	obj := p.Get(vnicKind, meta)
	if obj == nil {
		return nil
	}
	switch ret := obj.(type) {
	case *vnicStruct:
		return ret
	default:
		p.Log.Errorf("VNIC returned wasn't the expected type %v", ret)
	}
	return nil
}

func (v *VCHub) setVNIC(in *vnicStruct) {
	// Since we are only setting locally
	// we don't have to worry about errors
	v.pCache.Set(vnicKind, in)
}

func (v *VCHub) deleteVNIC(macAddress string) {
	meta := createVNICMeta(macAddress)
	v.pCache.Delete(vnicKind, meta)
}
