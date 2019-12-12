package store

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils/pcache"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
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

func validateVNIC(in interface{}) bool {
	// This object should never go to statemgr
	return false
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

// PCache has the exact same internals as the OrchHub pcache, but defines custom kinds used in vchub
type PCache struct {
	*pcache.PCache
}

// NewPCache creates a new pcache for vchub
func NewPCache(stateMgr *statemgr.Statemgr, logger log.Logger) *PCache {
	pCache := pcache.NewPCache(stateMgr, logger)

	pCache.SetValidator(vnicKind, validateVNIC)
	return &PCache{
		PCache: pCache,
	}
}

// GetVNIC retrieves a vnic
func (p *PCache) getVNIC(macAddress string) *vnicStruct {
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

func (p *PCache) setVNIC(in *vnicStruct) {
	// Since we are only setting locally
	// we don't have to worry about errors
	p.Set(vnicKind, in)
}

func (p *PCache) deleteVNIC(macAddress string) {
	meta := createVNICMeta(macAddress)
	p.Delete(vnicKind, meta)
}
