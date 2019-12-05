package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/runtime"
)

// WorkloadState is a wrapper for workload object
type WorkloadState struct {
	sync.Mutex
	Workload *ctkit.Workload
	stateMgr *Statemgr
}

//GetWorkloadWatchOptions gets options
func (sm *Statemgr) GetWorkloadWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
}

// OnWorkloadCreate creates a workload based on watch event
func (sm *Statemgr) OnWorkloadCreate(w *ctkit.Workload) error {
	_, err := NewWorkloadState(w, sm)
	return err
}

// OnWorkloadUpdate handles update event
func (sm *Statemgr) OnWorkloadUpdate(w *ctkit.Workload, nw *workload.Workload) error {
	// TODO : act on the state object
	_, err := WorkloadStateFromObj(w)
	return err
}

// OnWorkloadDelete deletes a workload
func (sm *Statemgr) OnWorkloadDelete(w *ctkit.Workload) error {
	// TODO : act on the state object
	_, err := WorkloadStateFromObj(w)
	return err
}

// WorkloadStateFromObj conerts from memdb object to workload state
func WorkloadStateFromObj(obj runtime.Object) (*WorkloadState, error) {
	switch obj.(type) {
	case *ctkit.Workload:
		nobj := obj.(*ctkit.Workload)
		switch nobj.HandlerCtx.(type) {
		case *WorkloadState:
			nts := nobj.HandlerCtx.(*WorkloadState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}

// NewWorkloadState create new workload state
func NewWorkloadState(wrk *ctkit.Workload, stateMgr *Statemgr) (*WorkloadState, error) {
	w := &WorkloadState{
		Workload: wrk,
		stateMgr: stateMgr,
	}
	wrk.HandlerCtx = w

	return w, nil
}
