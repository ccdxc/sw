// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

// VirtualRouterState is a wrapper for virtual router object
type VirtualRouterState struct {
	sync.Mutex
	VirtualRouter *ctkit.VirtualRouter `json:"-"` // VirtualRouter object
	stateMgr      *Statemgr            // pointer to the network manager
}

// VirtualRouterStateFromObj converts from memdb object to VirtualRouter state
func VirtualRouterStateFromObj(obj runtime.Object) (*VirtualRouterState, error) {
	switch obj.(type) {
	case *ctkit.VirtualRouter:
		vr := obj.(*ctkit.VirtualRouter)
		switch vr.HandlerCtx.(type) {
		case *VirtualRouterState:
			state := vr.HandlerCtx.(*VirtualRouterState)
			return state, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// FindVirtualRouter finds virtual router by name
func (sm *Statemgr) FindVirtualRouter(tenant, ns, name string) (*VirtualRouterState, error) {
	// find the object
	obj, err := sm.FindObject("VirtualRouter", tenant, ns, name)
	if err != nil {
		return nil, err
	}

	return VirtualRouterStateFromObj(obj)
}

// NewVirtualRouterState creates a new VirtualRouterState
func NewVirtualRouterState(vir *ctkit.VirtualRouter, sm *Statemgr) (*VirtualRouterState, error) {
	vr := &VirtualRouterState{
		VirtualRouter: vir,
		stateMgr:      sm,
	}
	vir.HandlerCtx = vr
	return vr, nil
}

// OnVirtualRouterCreate creates local network state based on watch event
func (sm *Statemgr) OnVirtualRouterCreate(obj *ctkit.VirtualRouter) error {
	log.Info("OnVirtualRouterCreate: received: ", obj.Spec)

	// create new network state
	vr, err := NewVirtualRouterState(obj, sm)
	if err != nil {
		log.Errorf("Error creating Virtual router state. Err: %v", err)
		return err
	}

	log.Info("OnVirtualRouterCreate: ", vr.VirtualRouter.Spec)
	return nil
}

// OnVirtualRouterUpdate handles VirtualRouter update
func (sm *Statemgr) OnVirtualRouterUpdate(oldvr *ctkit.VirtualRouter, newvr *network.VirtualRouter) error {
	log.Info("OnVirtualRouterUpdate: received: ", oldvr.Spec, newvr.Spec)

	// see if anything changed
	_, ok := ref.ObjDiff(oldvr.Spec, newvr.Spec)
	if (oldvr.GenerationID == newvr.GenerationID) && !ok {
		oldvr.ObjectMeta = newvr.ObjectMeta
		return nil
	}

	// update old state
	oldvr.ObjectMeta = newvr.ObjectMeta
	oldvr.Spec = newvr.Spec

	// find the vr state
	vr, err := VirtualRouterStateFromObj(oldvr)
	if err != nil {
		log.Errorf("Can't find virtual router for updating {%+v}. Err: {%v}", oldvr.ObjectMeta, err)
		return fmt.Errorf("Can not find virtual router")
	}

	log.Info("OnVirtualRouterUpdate: found virtual router: ", vr.VirtualRouter.Spec)
	return nil
}

// OnVirtualRouterDelete deletes the VirtualRouter
func (sm *Statemgr) OnVirtualRouterDelete(obj *ctkit.VirtualRouter) error {
	log.Info("OnVirtualRouterDelete: received: ", obj.Spec)

	vr, err := sm.FindVirtualRouter(obj.Tenant, obj.Namespace, obj.Name)

	if err != nil {
		log.Error("FindVirtualRouter returned an error: ", err, "for: ", obj.Tenant, obj.Namespace, obj.Name)
		return errors.New("Object doesn't exist")
	}
	log.Info("OnVirtualRouterDelete: found virtual router: ", vr.VirtualRouter.Spec)
	return nil
}
