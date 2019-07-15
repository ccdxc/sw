package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// KindModule is the kind of Module objects in string form
	KindModule = string(diagnostics.KindModule)
)

// ModuleState is the internal state for a module object
type ModuleState struct {
	*sync.Mutex
	*diagnostics.Module
}

// handleDiagnosticsModuleEvent is the handler invoked by stateMgr when it receives diagnostics Module object
// notifications from ApiServer
func (sm *Statemgr) handleDiagnosticsModuleEvent(evt *kvstore.WatchEvent) {
	var err error
	obj := evt.Object.(*diagnostics.Module)
	state, _ := sm.getModuleState(obj)

	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		if state != nil {
			state.Lock()
			defer state.Unlock()
			state.Module = obj
			err = sm.memDB.UpdateObject(state)
		} else {
			state = &ModuleState{
				Mutex:  &sync.Mutex{},
				Module: obj}
			state.Lock()
			defer state.Unlock()
			err = sm.memDB.AddObject(state)
		}
	case kvstore.Deleted:
		if state != nil {
			state.Lock()
			defer state.Unlock()
			err = sm.deleteModuleState(obj)
		} else {
			log.Infof("No state found for object %s, ignoring delete update", obj.GetObjectMeta().Name)
		}

	default:
		err = fmt.Errorf("unknown event type %+v, Object: %+v", evt.Type, obj)
	}

	if err != nil {
		log.Errorf("Error handling TechSupport Event: %+v, err: %v", evt, err)
	}

}

func (sm *Statemgr) getModuleState(obj *diagnostics.Module) (*ModuleState, error) {
	dbMs, err := sm.FindObject(obj.GetObjectKind(), obj.GetObjectMeta())
	if err != nil {
		return nil, err
	}
	return dbMs.(*ModuleState), nil
}

// deleteModuleState deletes memdb state for a diagnostics Module object
func (sm *Statemgr) deleteModuleState(obj *diagnostics.Module) error {
	state, err := sm.getModuleState(obj)
	if err != nil {
		return fmt.Errorf("error deleting Module object %s: %v", obj.GetObjectMeta().Name, err)
	}
	return sm.memDB.DeleteObject(state)
}

// PurgeDeletedModuleObjects deletes state for module objects that are not in the supplied list
// Used to clean up modules that are no longer in apiserver's DB
func (sm *Statemgr) PurgeDeletedModuleObjects(objs []*diagnostics.Module) {
	// incoming tsrList contains objects that exists in apiserver. It is possible that
	// some objects may have been deleted since the grpc connection was lost
	// loss and restore of grpc connection can be due to -
	// 1. apiserver instance (could be more than one) fail-over/restart
	// 2. network problem (connectivity to apiserver)
	// 3. tsm restart
	// This function is needed for cases 1 and 2

	objNameMap := make(map[string]bool)

	for _, obj := range objs {
		objNameMap[obj.GetObjectMeta().Name] = true
	}

	objStateList := sm.ListModuleState() // list of objects known to Statemgr
	for _, objState := range objStateList {
		// objects that are only in local DB and not in supplied list, must be deleted
		if !objNameMap[objState.GetObjectMeta().Name] {
			sm.deleteModuleState(objState.Module)
		}
	}
}

// ListModuleState returns state for diagnostics module objects
func (sm *Statemgr) ListModuleState() []*ModuleState {
	var msList []*ModuleState
	memDbObjs := sm.ListObjects(KindModule)
	for _, obj := range memDbObjs {
		os, ok := obj.(*ModuleState)
		if ok {
			msList = append(msList, os)
		}
	}
	return msList
}
