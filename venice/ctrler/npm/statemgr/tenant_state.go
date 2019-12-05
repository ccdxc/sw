package statemgr

import (
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// TenantState is a wrapper for tenant object
type TenantState struct {
	Tenant   *ctkit.Tenant `json:"-"` // embedding tenant object
	stateMgr *Statemgr     // state manager
}

// newTenantState returns a new tenant state object
func newTenantState(tn *ctkit.Tenant, stateMgr *Statemgr) (*TenantState, error) {
	// build the tenant state
	ts := TenantState{
		Tenant:   tn,
		stateMgr: stateMgr,
	}
	tn.HandlerCtx = &ts

	return &ts, nil
}

func convertTenant(tns *TenantState) *netproto.Tenant {
	creationTime, _ := types.TimestampProto(time.Now())
	ntn := netproto.Tenant{
		TypeMeta:   tns.Tenant.TypeMeta,
		ObjectMeta: tns.Tenant.ObjectMeta,
	}

	ntn.CreationTime = api.Timestamp{Timestamp: *creationTime}
	return &ntn
}

//GetTenantWatchOptions gets tenant options
func (sm *Statemgr) GetTenantWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
	return &opts
}

// OnTenantCreate creates a tenant based on watch event
func (sm *Statemgr) OnTenantCreate(tn *ctkit.Tenant) error {
	// create new tenant state
	ts, err := newTenantState(tn, sm)
	if err != nil {
		log.Errorf("Error creating new network state. Err: %v", err)
		return err
	}

	log.Infof("Created Tenant state {Meta: %+v, Spec: %+v}", ts.Tenant.ObjectMeta, ts.Tenant.Spec)

	// store it in local DB
	err = sm.mbus.AddObjectWithReferences(tn.MakeKey("cluster"), convertTenant(ts), references(tn))

	return nil
}

// OnTenantUpdate handles update event
func (sm *Statemgr) OnTenantUpdate(tn *ctkit.Tenant, ntn *cluster.Tenant) error {
	return nil
}

// OnTenantDelete deletes a tenant
func (sm *Statemgr) OnTenantDelete(tn *ctkit.Tenant) error {
	if tn.Name == "default" {
		return ErrDefaultTenantDeleteNotPermitted
	}
	// see if we already have it
	tso, err := sm.FindObject("Tenant", "", "", tn.Name)
	if err != nil {
		log.Errorf("Can not find the tenant %s", tn.Name)
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
	return sm.mbus.DeleteObjectWithReferences(tn.MakeKey("cluster"),
		convertTenant(ts), references(tn))
}

// FindTenant finds a tenant
func (sm *Statemgr) FindTenant(tenant string) (*TenantState, error) {
	// find the object
	obj, err := sm.FindObject("Tenant", "", "", tenant)
	if err != nil {
		return nil, err
	}

	return TenantStateFromObj(obj)
}

// TenantStateFromObj converts from memdb object to tenant state
func TenantStateFromObj(obj runtime.Object) (*TenantState, error) {
	switch obj.(type) {
	case *ctkit.Tenant:
		tobj := obj.(*ctkit.Tenant)
		switch tobj.HandlerCtx.(type) {
		case *TenantState:
			tsobj := tobj.HandlerCtx.(*TenantState)
			return tsobj, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}
