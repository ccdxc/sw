// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"
	"fmt"

	"github.com/pensando/sw/venice/utils/featureflags"

	"sync"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

var smgrVirtualRouter *SmVirtualRouter

// SmVirtualRouter is object statemgr for Route object
type SmVirtualRouter struct {
	featureMgrBase
	sm *Statemgr
}

// CompleteRegistration is the callback function statemgr calls after init is done
func (sma *SmVirtualRouter) CompleteRegistration() {
	if featureflags.IsOVerlayRoutingEnabled() == false {
		sma.sm.SetVirtualRouterReactor(sma.sm)
	}

	sma.sm.SetVirtualRouterReactor(sma)
}

func init() {
	mgr := MustGetStatemgr()
	smgrVirtualRouter = &SmVirtualRouter{
		sm: mgr,
	}

	mgr.Register("statemgrvirtualroute", smgrVirtualRouter)
}

// VirtualRouterState is a wrapper for virtual router object
type VirtualRouterState struct {
	sync.Mutex
	VirtualRouter *ctkit.VirtualRouter `json:"-"` // VirtualRouter object
	stateMgr      *Statemgr            // pointer to the network manager
}

// GetVirtualRouterWatchOptions gets options
func (sma *SmVirtualRouter) GetVirtualRouterWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
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

func defaultRTName(in *network.VirtualRouter) string {
	return in.Name + "." + "default"
}

func convertVirtualRouter(vr *VirtualRouterState) *netproto.Vrf {
	creationTime, _ := types.TimestampProto(time.Now())
	ntn := netproto.Vrf{
		TypeMeta: api.TypeMeta{
			Kind:       "Vrf",
			APIVersion: vr.VirtualRouter.TypeMeta.APIVersion,
		},
		ObjectMeta: vr.VirtualRouter.ObjectMeta,
	}

	ntn.Spec.VxLANVNI = vr.VirtualRouter.Spec.VxLanVNI
	if vr.VirtualRouter.Spec.RouteImportExport != nil {
		ntn.Spec.RouteImportExport = &netproto.RDSpec{
			AddressFamily: vr.VirtualRouter.Spec.RouteImportExport.AddressFamily,
			RDAuto:        vr.VirtualRouter.Spec.RouteImportExport.RDAuto,
		}
		ntn.Spec.RouteImportExport.RD = cloneRD(vr.VirtualRouter.Spec.RouteImportExport.RD)
		for _, r := range vr.VirtualRouter.Spec.RouteImportExport.ImportRTs {
			ntn.Spec.RouteImportExport.ImportRTs = append(ntn.Spec.RouteImportExport.ImportRTs, cloneRD(r))
		}
		for _, r := range vr.VirtualRouter.Spec.RouteImportExport.ExportRTs {
			ntn.Spec.RouteImportExport.ExportRTs = append(ntn.Spec.RouteImportExport.ExportRTs, cloneRD(r))
		}
	}
	ntn.Spec.IPAMPolicy = vr.VirtualRouter.Spec.DefaultIPAMPolicy
	ntn.Spec.RouterMAC = vr.VirtualRouter.Spec.RouterMACAddress
	switch vr.VirtualRouter.Spec.Type {
	case network.VirtualRouterSpec_Tenant.String():
		ntn.Spec.VrfType = netproto.VrfSpec_Type_name[int32(netproto.VrfSpec_CUSTOMER)]
	case network.VirtualRouterSpec_Infra.String():
		ntn.Spec.VrfType = netproto.VrfSpec_Type_name[int32(netproto.VrfSpec_INFRA)]
	}
	ntn.Spec.V4RouteTable = defaultRTName(&vr.VirtualRouter.VirtualRouter)

	ntn.CreationTime = api.Timestamp{Timestamp: *creationTime}
	return &ntn
}

// FindVirtualRouter finds virtual router by name
func (sma *SmVirtualRouter) FindVirtualRouter(tenant, ns, name string) (*VirtualRouterState, error) {
	// find the object
	obj, err := sma.sm.FindObject("VirtualRouter", tenant, ns, name)
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
func (sma *SmVirtualRouter) OnVirtualRouterCreate(obj *ctkit.VirtualRouter) error {
	log.Info("OnVirtualRouterCreate: received: ", obj.Spec)

	// create new network state
	vr, err := NewVirtualRouterState(obj, sma.sm)
	if err != nil {
		log.Errorf("Error creating Virtual router state. Err: %v", err)
		return err
	}

	// Update Object with default Route Table
	defRTName := defaultRTName(&vr.VirtualRouter.VirtualRouter)
	vr.VirtualRouter.Status.RouteTable = defRTName

	err = sma.sm.mbus.AddObjectWithReferences(obj.MakeKey(string(apiclient.GroupNetwork)), convertVirtualRouter(vr), references(obj))
	if err != nil {
		log.Errorf("could not add VirtualRouter to DB (%s)", err)
	}

	log.Info("OnVirtualRouterCreate: ", vr.VirtualRouter.Spec)
	return nil
}

// OnVirtualRouterUpdate handles VirtualRouter update
func (sma *SmVirtualRouter) OnVirtualRouterUpdate(oldvr *ctkit.VirtualRouter, newvr *network.VirtualRouter) error {
	log.Info("OnVirtualRouterUpdate: received: ", oldvr.Spec, newvr.Spec)

	// see if anything changed
	_, ok := ref.ObjDiff(oldvr.Spec, newvr.Spec)
	if (oldvr.GenerationID == newvr.GenerationID) && !ok {
		oldvr.ObjectMeta = newvr.ObjectMeta
		return nil
	}

	// find the vr state
	vr, err := VirtualRouterStateFromObj(oldvr)
	if err != nil {
		log.Errorf("Can't find virtual router for updating {%+v}. Err: {%v}", oldvr.ObjectMeta, err)
		return fmt.Errorf("Can not find virtual router")
	}

	// update old state
	oldvr.ObjectMeta = newvr.ObjectMeta
	oldvr.Spec = newvr.Spec

	err = sma.sm.mbus.UpdateObjectWithReferences(newvr.MakeKey(string(apiclient.GroupNetwork)), convertVirtualRouter(vr), references(newvr))
	if err != nil {
		log.Errorf("could not add VirtualRouter to DB (%s)", err)
	}

	log.Info("OnVirtualRouterUpdate: found virtual router: ", oldvr.VirtualRouter.Spec)
	return nil
}

// OnVirtualRouterDelete deletes the VirtualRouter
func (sma *SmVirtualRouter) OnVirtualRouterDelete(obj *ctkit.VirtualRouter) error {
	log.Info("OnVirtualRouterDelete: received: ", obj.Spec)

	vr, err := sma.FindVirtualRouter(obj.Tenant, obj.Namespace, obj.Name)

	if err != nil {
		log.Error("FindVirtualRouter returned an error: ", err, "for: ", obj.Tenant, obj.Namespace, obj.Name)
		return errors.New("Object doesn't exist")
	}
	log.Info("OnVirtualRouterDelete: found virtual router: ", vr.VirtualRouter.Spec)
	// delete it from the DB
	return sma.sm.mbus.DeleteObjectWithReferences(obj.MakeKey(string(apiclient.GroupNetwork)),
		convertVirtualRouter(vr), references(obj))
}

// Default Statemanager implememtation

// GetVirtualRouterWatchOptions gets options
func (sm *Statemgr) GetVirtualRouterWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
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
