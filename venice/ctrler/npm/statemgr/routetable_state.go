// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"

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

var smgrRouteTable *SmRouteTable

// SmRouteTable is object statemgr for Route object
type SmRouteTable struct {
	featureMgrBase
	sm *Statemgr
}

// CompleteRegistration is the callback function statemgr calls after init is done
func (sm *SmRouteTable) CompleteRegistration() {
	// if featureflags.IsOVerlayRoutingEnabled() == false {
	// 	return
	// }

	sm.sm.SetRouteTableReactor(smgrRouteTable)
}

func init() {
	mgr := MustGetStatemgr()
	smgrRouteTable = &SmRouteTable{
		sm: mgr,
	}

	mgr.Register("statemgrroutetable", smgrRouteTable)
}

// RouteTableState is a wrapper for virtual router object
type RouteTableState struct {
	sync.Mutex
	RouteTable *ctkit.RouteTable `json:"-"` // RouteTable object
	stateMgr   *Statemgr         // pointer to the network manager
}

// RouteTableFromObj converts from memdb object to RouteTable state
func RouteTableFromObj(obj runtime.Object) (*RouteTableState, error) {
	switch obj.(type) {
	case *ctkit.RouteTable:
		vr := obj.(*ctkit.RouteTable)
		switch vr.HandlerCtx.(type) {
		case *RouteTableState:
			state := vr.HandlerCtx.(*RouteTableState)
			return state, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

func convertRouteTable(vr *RouteTableState) *netproto.RouteTable {
	creationTime, _ := types.TimestampProto(time.Now())
	ntn := netproto.RouteTable{
		TypeMeta:   vr.RouteTable.TypeMeta,
		ObjectMeta: vr.RouteTable.ObjectMeta,
	}
	ntn.CreationTime = api.Timestamp{Timestamp: *creationTime}
	return &ntn
}

//GetRouteTableWatchOptions gets options
func (sm *SmRouteTable) GetRouteTableWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
}

// FindRouteTable finds virtual router by name
func (sm *SmRouteTable) FindRouteTable(tenant, ns, name string) (*RouteTableState, error) {
	// find the object
	obj, err := sm.sm.FindObject(string(network.KindRouteTable), tenant, ns, name)
	if err != nil {
		return nil, err
	}

	return RouteTableFromObj(obj)
}

// NewRouteTableState creates a new RouteTableState
func NewRouteTableState(vir *ctkit.RouteTable, sm *Statemgr) (*RouteTableState, error) {
	vr := &RouteTableState{
		RouteTable: vir,
		stateMgr:   sm,
	}
	vir.HandlerCtx = vr
	return vr, nil
}

// OnRouteTableCreate creates local network state based on watch event
func (sm *SmRouteTable) OnRouteTableCreate(obj *ctkit.RouteTable) error {
	log.Info("OnRouteTableCreate: received: ", obj.Spec)

	// create new network state
	vr, err := NewRouteTableState(obj, sm.sm)
	if err != nil {
		log.Errorf("Error creating RouteTable state. Err: %v", err)
		return err
	}

	err = sm.sm.mbus.AddObjectWithReferences(obj.MakeKey(string(apiclient.GroupNetwork)), convertRouteTable(vr), references(obj))
	if err != nil {
		log.Errorf("could not add RouteTable to DB (%s)", err)
	}

	log.Info("OnRouteTableCreate: ", vr.RouteTable.Spec)
	return nil
}

// OnRouteTableUpdate handles RouteTable update
func (sm *SmRouteTable) OnRouteTableUpdate(oldvr *ctkit.RouteTable, newvr *network.RouteTable) error {
	log.Info("OnRouteTableUpdate: received: ", oldvr.Spec, newvr.Spec)

	// see if anything changed
	_, ok := ref.ObjDiff(oldvr.Spec, newvr.Spec)
	if (oldvr.GenerationID == newvr.GenerationID) && !ok {
		oldvr.ObjectMeta = newvr.ObjectMeta
		return nil
	}

	// update old state
	oldvr.ObjectMeta = newvr.ObjectMeta
	oldvr.Spec = newvr.Spec

	log.Info("OnRouteTableUpdate: found route table: ", oldvr.RouteTable.Spec)
	return nil
}

// OnRouteTableDelete deletes the RouteTable
func (sm *SmRouteTable) OnRouteTableDelete(obj *ctkit.RouteTable) error {
	log.Info("OnRouteTableDelete: received: ", obj.Spec)

	vr, err := sm.FindRouteTable(obj.Tenant, obj.Namespace, obj.Name)

	if err != nil {
		log.Error("FindRouteTable returned an error: ", err, "for: ", obj.Tenant, obj.Namespace, obj.Name)
		return errors.New("Object doesn't exist")
	}
	log.Info("OnRouteTableDelete: found route table: ", vr.RouteTable.Spec)
	// delete it from the DB
	return sm.sm.mbus.DeleteObjectWithReferences(obj.MakeKey(string(apiclient.GroupNetwork)),
		convertRouteTable(vr), references(obj))
}
