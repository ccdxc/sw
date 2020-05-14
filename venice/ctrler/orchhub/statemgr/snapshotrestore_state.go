package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

// SnapshotRestoreState is a wrapper for SnapshotRestore object
type SnapshotRestoreState struct {
	sync.Mutex
	SnapshotRestore *ctkit.SnapshotRestore
	stateMgr        *Statemgr
}

//GetSnapshotRestoreWatchOptions gets options
func (sm *Statemgr) GetSnapshotRestoreWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{}
	return &opts
}

// OnSnapshotRestoreCreate creates a SnapshotRestore based on watch event
func (sm *Statemgr) OnSnapshotRestoreCreate(obj *ctkit.SnapshotRestore) error {
	sm.logger.Infof("Snapshot restore event")
	sm.instanceManagerCh <- &kvstore.WatchEvent{Object: &obj.SnapshotRestore, Type: kvstore.Created}
	_, err := NewSnapshotRestoreState(obj, sm)
	return err
}

// OnSnapshotRestoreUpdate handles update event
func (sm *Statemgr) OnSnapshotRestoreUpdate(oldObj *ctkit.SnapshotRestore, newObj *cluster.SnapshotRestore) error {
	sm.instanceManagerCh <- &kvstore.WatchEvent{Object: newObj, Type: kvstore.Updated}
	_, err := SnapshotRestoreStateFromObj(oldObj)
	return err
}

// OnSnapshotRestoreDelete deletes a SnapshotRestore
func (sm *Statemgr) OnSnapshotRestoreDelete(nh *ctkit.SnapshotRestore) error {
	return nil
}

// OnSnapshotRestoreReconnect is called when ctkit reconnects to apiserver
func (sm *Statemgr) OnSnapshotRestoreReconnect() {
	return
}

// NewSnapshotRestoreState create new snapshotRestore state
func NewSnapshotRestoreState(SnapshotRestore *ctkit.SnapshotRestore, stateMgr *Statemgr) (*SnapshotRestoreState, error) {
	w := &SnapshotRestoreState{
		SnapshotRestore: SnapshotRestore,
		stateMgr:        stateMgr,
	}
	SnapshotRestore.HandlerCtx = w

	return w, nil
}

// SnapshotRestoreStateFromObj converts from memdb object to SnapshotRestore state
func SnapshotRestoreStateFromObj(obj runtime.Object) (*SnapshotRestoreState, error) {
	switch obj.(type) {
	case *ctkit.SnapshotRestore:
		nobj := obj.(*ctkit.SnapshotRestore)
		switch nobj.HandlerCtx.(type) {
		case *SnapshotRestoreState:
			nts := nobj.HandlerCtx.(*SnapshotRestoreState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}
