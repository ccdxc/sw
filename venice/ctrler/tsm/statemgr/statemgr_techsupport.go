// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sync"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	// KindTechSupportRequest is the kind of TechSupportRequest objects in string form
	KindTechSupportRequest = string(monitoring.KindTechSupportRequest)
	// KindControllerNode  is the kind of Node objects in string form
	KindControllerNode = string(cluster.KindNode)
	// KindSmartNICNode is the kind of SmartNIC objects in string form
	KindSmartNICNode = string(cluster.KindSmartNIC)
)

/* *** TECHSUPPORT CONTROLLER CONCURRENCY MODEL ***

	TechSupport state consists of the following objects:
	- TechSupportRequests
	- Controller Nodes
	- SmartNIC Nodes

	Objects are encapsulated in StateMgr *State structures that allow statemgr APIs to be generic.

  Updates for all objects are received from ApiServer and are applied (serially) to the corresponding state objects in memdb.
	Memdb has its own mechanism for concurrent access, so no locks are needed for creating or deleting State objects.
	However, the actual content of a State object is opaque to memdb, so it is responsibility of the memdb clients to ensure
	that it is protected against race conditions.

	These are the access pattern for the different TechSupport kinds:
	- TechSupportRequest: multiple readers (agent watch RPCs), multiple writers (ApiServer watch RPCs, agent status updates)
	- Controller and SmartNIC nodes: multiple readers (agent watch RPCs), single writer (ApiServer watch RPCs)

	The entry-points where locks are acquired and release are:
	- On the ApiServer watch side, the statemgr handleTechSupportEvent handler
	- On the RPC side, the rpcserver handlers WatchTechSupportRequests and UpdateResult

	The pattern is that whenever a client needs an object from statemgr, it must firs retrieve the corresponding State object
	and acquire the lock. After it is done, it must release the lock and make sure that no further operations (including
	logging, serialization, etc.) are performed.

*/

// TechSupportObject is an interface for a generic object
type TechSupportObject interface {
	runtime.Object
	runtime.ObjectMetaAccessor
}

// TechSupportObjectState is an interface for the internal state of a generic object
type TechSupportObjectState interface {
	TechSupportObject
	Lock()
	Unlock()
}

// TechSupportRequestState is the internal state for a tech-support request
type TechSupportRequestState struct {
	*sync.Mutex
	*monitoring.TechSupportRequest
}

// ControllerNodeState is the internal state for a tech-support request
type ControllerNodeState struct {
	*sync.Mutex
	*cluster.Node
}

// SmartNICNodeState is the internal state for a tech-support request
type SmartNICNodeState struct {
	*sync.Mutex
	*cluster.SmartNIC
}

// newTechSupportObjectState returns a new state wrapper for the supplied TechSupport object
func (sm *Statemgr) newTechSupportObjectState(obj TechSupportObject) TechSupportObjectState {
	switch kind := obj.GetObjectKind(); kind {
	case KindTechSupportRequest:
		return &TechSupportRequestState{
			Mutex:              &sync.Mutex{},
			TechSupportRequest: obj.(*monitoring.TechSupportRequest),
		}
	case KindControllerNode:
		return &ControllerNodeState{
			Mutex: &sync.Mutex{},
			Node:  obj.(*cluster.Node),
		}
	case KindSmartNICNode:
		return &SmartNICNodeState{
			Mutex:    &sync.Mutex{},
			SmartNIC: obj.(*cluster.SmartNIC),
		}
	default:
		log.Errorf("Unknown kind: %s", kind)
	}
	return nil
}

// updateTechSupportObjectState updates the TechSuport object in the supplied state wrapper
// Caller must have acquired the state lock before invoking the method
func (sm *Statemgr) updateTechSupportObjectState(state TechSupportObjectState, obj TechSupportObject) TechSupportObjectState {
	switch kind := obj.GetObjectKind(); kind {
	case KindTechSupportRequest:
		state.(*TechSupportRequestState).TechSupportRequest = obj.(*monitoring.TechSupportRequest)
	case KindControllerNode:
		state.(*ControllerNodeState).Node = obj.(*cluster.Node)
	case KindSmartNICNode:
		state.(*SmartNICNodeState).SmartNIC = obj.(*cluster.SmartNIC)
	default:
		log.Errorf("Unknown kind: %s", kind)
		state = nil
	}
	return state
}

// deleteTechSupportObjectState deletes memdb state for a TechSupport object
func (sm *Statemgr) deleteTechSupportObjectState(obj TechSupportObject) error {
	state, err := sm.GetTechSupportObjectState(obj)
	if err != nil {
		return fmt.Errorf("Error deleting TechSupport object %s: %v", obj.GetObjectMeta().Name, err)
	}
	return sm.memDB.DeleteObject(state)
}

func initTechSupportRequestStatus(tsr *monitoring.TechSupportRequest) {
}

// handleTechSupportEvent is the handler invoked by stateMgr when it receives TechSupport object
// notifications from ApiServer
func (sm *Statemgr) handleTechSupportEvent(evt *kvstore.WatchEvent) {
	var err error
	obj := evt.Object.(TechSupportObject)
	state, _ := sm.GetTechSupportObjectState(obj)

	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		update := false // do we need to send back an update to ApiServer ?

		// if this is a TechSupportRequest, we may have to initialize status and push the update back to ApiServer
		tsr, ok := obj.(*monitoring.TechSupportRequest)
		if ok {
			if tsr.Status.ControllerNodeResults == nil {
				tsr.Status.ControllerNodeResults = make(map[string]*monitoring.TechSupportNodeResult)
			}
			if tsr.Status.SmartNICNodeResults == nil {
				tsr.Status.SmartNICNodeResults = make(map[string]*monitoring.TechSupportNodeResult)
			}
			if tsr.Status.InstanceID == "" {
				tsr.Status.InstanceID = uuid.NewV4().String()
				tsr.Status.Status = monitoring.TechSupportJobStatus_Scheduled.String()
				update = true
			}
		}

		if state != nil {
			state.Lock()
			defer state.Unlock()
			err = sm.memDB.UpdateObject(sm.updateTechSupportObjectState(state, obj))
		} else {
			state = sm.newTechSupportObjectState(obj)
			state.Lock()
			defer state.Unlock()
			err = sm.memDB.AddObject(state)
		}

		if err == nil && update {
			err = sm.writer.WriteTechSupportRequest(tsr)
		}

	case kvstore.Deleted:
		if state != nil {
			state.Lock()
			defer state.Unlock()
			err = sm.deleteTechSupportObjectState(obj)
		} else {
			log.Infof("No state found for object %s, ignoring delete update", obj.GetObjectMeta().Name)
		}

	default:
		err = fmt.Errorf("Unknown event type %+v, Object: %+v", evt.Type, obj)
	}

	if err != nil {
		log.Errorf("Error handling TechSupport Event: %+v, err: %v", evt, err)
	}

}

// FindTechSupportObject gets state of a TechSupport object by name and kind
func (sm *Statemgr) FindTechSupportObject(name, ns, kind string) (TechSupportObjectState, error) {
	searchMeta := &api.ObjectMeta{
		Name:      name,
		Namespace: ns,
	}
	dbMs, err := sm.FindObject(kind, searchMeta)
	if err != nil {
		return nil, err
	}
	return dbMs.(TechSupportObjectState), nil
}

// GetTechSupportObjectState gets state for the specified tech-support object
func (sm *Statemgr) GetTechSupportObjectState(obj TechSupportObject) (TechSupportObjectState, error) {
	dbMs, err := sm.FindObject(obj.GetObjectKind(), obj.GetObjectMeta())
	if err != nil {
		return nil, err
	}
	return dbMs.(TechSupportObjectState), nil
}

// ListTechSupportObjectState returns state for tech-support objects (requests, nodes, ...)
func (sm *Statemgr) ListTechSupportObjectState(kind string) []TechSupportObjectState {
	var msList []TechSupportObjectState
	memDbObjs := sm.ListObjects(kind)
	for _, obj := range memDbObjs {
		os, ok := obj.(TechSupportObjectState)
		if ok {
			msList = append(msList, os)
		}
	}
	return msList
}

// UpdateTechSupportObject receives a status updated for a managed object and pushes it back to ApiServer
// Caller must have acquired the state lock before invoking the method
func (sm *Statemgr) UpdateTechSupportObject(obj TechSupportObject) error {
	state, err := sm.GetTechSupportObjectState(obj)
	if err != nil {
		log.Infof("No state found for object %s, ignoring update", obj.GetObjectMeta().Name)
		return nil
	}

	tsr, ok := obj.(*monitoring.TechSupportRequest)
	if !ok {
		return fmt.Errorf("Unsupported object type: %T", obj)
	}

	err = sm.memDB.UpdateObject(sm.updateTechSupportObjectState(state, tsr))
	if err != nil {
		return fmt.Errorf("Error updating state for object %+v: %v", tsr, err)
	}
	return sm.writer.WriteTechSupportRequest(tsr)
}

// PurgeDeletedTechSupportObjects deletes state for tech-support requests that are not in the supplied list
// Used to clean up requests that are no longer in apiserver's DB
// Objects can be TechSupportRequests, Controller Nodes or SmartNICNodes
func (sm *Statemgr) PurgeDeletedTechSupportObjects(objList interface{}) {
	// incoming tsrList contains objects that exists in apiserver. It is possible that
	// some objects may have been deleted since the grpc connection was lost
	// loss and restore of grpc connection can be due to -
	// 1. apiserver instance (could be more than one) fail-over/restart
	// 2. network problem (connectivity to apiserver)
	// 3. tsm restart
	// This function is needed for cases 1 and 2

	objNameMap := make(map[string]bool)
	var kind string

	switch objs := objList.(type) {
	case []*monitoring.TechSupportRequest:
		kind = KindTechSupportRequest
		for _, obj := range objs {
			objNameMap[obj.GetObjectMeta().Name] = true
		}
	case []*cluster.SmartNIC:
		kind = KindSmartNICNode
		for _, obj := range objs {
			objNameMap[obj.GetObjectMeta().Name] = true
		}
	case []*cluster.Node:
		kind = KindControllerNode
		for _, obj := range objs {
			objNameMap[obj.GetObjectMeta().Name] = true
		}
	default:
		log.Errorf("PurgeDeletedTechSupportObjects: unsupported kind %T", objs)
		return
	}

	objStateList := sm.ListTechSupportObjectState(kind) // list of objects known to Statemgr
	for _, objState := range objStateList {
		// objects that are only in local DB and not in supplied list, must be deleted
		if !objNameMap[objState.GetObjectMeta().Name] {
			sm.deleteTechSupportObjectState(objState)
		}
	}
}
