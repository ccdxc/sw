package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// TenantState is a wrapper for tenant object
type TenantState struct {
	sync.Mutex
	network.Tenant           // embedding tenant object
	stateMgr       *Statemgr // state manager
}

// newTenantState returns a new tenant state object
func newTenantState(tn *network.Tenant, stateMgr *Statemgr) (*TenantState, error) {
	// build the tenant state
	ts := TenantState{
		Tenant:   *tn,
		stateMgr: stateMgr,
	}

	return &ts, nil
}

// CreateTenant creates a tenant based on watch event
func (sm *Statemgr) CreateTenant(tn *network.Tenant) error {
	oldTn, err := sm.FindObject("Tenant", tn.ObjectMeta.Tenant, tn.ObjectMeta.Name)
	if err == nil {
		// FIXME: how do we handle an existing tenant object changing?
		log.Errorf("Can not change existing tenant {%+v}. New state: {%+v}", oldTn, tn)
		return fmt.Errorf("can not change tenant after its created")
	}

	// create new tenant state
	ts, err := newTenantState(tn, sm)
	if err != nil {
		log.Errorf("Error creating new network state. Err: %v", err)
		return err
	}

	log.Infof("Created Tenant state {Meta: %+v, Spec: %+v}", ts.ObjectMeta, ts.Spec)

	// store it in local DB
	sm.memDB.AddObject(ts)

	return nil
}

// DeleteTenant deletes a tenant
func (sm *Statemgr) DeleteTenant(tenant string) error {
	if tenant == "default" {
		return ErrDefaultTenantDeleteNotPermitted
	}
	// see if we already have it
	tso, err := sm.FindObject("Tenant", tenant, tenant)
	if err != nil {
		log.Errorf("Can not find the tenant %s", tenant)
		return ErrTenantNotFound
	}

	// convert it to network state
	ts, err := TenantStateFromObj(tso)
	if err != nil {
		return err
	}

	if err != nil {
		log.Errorf("Error deleting the tenant {%+v}. Err: %v", ts, err)
		return err
	}

	// delete it from the DB
	return sm.memDB.DeleteObject(tso)
}

// FindTenant finds a tenant
func (sm *Statemgr) FindTenant(tenant string) (*TenantState, error) {
	// find the object
	obj, err := sm.FindObject("Tenant", tenant, tenant)
	if err != nil {
		return nil, err
	}

	return TenantStateFromObj(obj)
}

// TenantStateFromObj converts from memdb object to tenant state
func TenantStateFromObj(obj memdb.Object) (*TenantState, error) {
	switch obj.(type) {
	case *TenantState:
		tsobj := obj.(*TenantState)
		return tsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}
