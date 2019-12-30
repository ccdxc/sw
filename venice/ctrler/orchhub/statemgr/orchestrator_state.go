package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	probeChannelMax = 128
)

// OrchestratorState is a wrapper for orchestration object
type OrchestratorState struct {
	sync.Mutex
	Orchestrator *ctkit.Orchestrator
	stateMgr     *Statemgr
}

//GetOrchestratorWatchOptions gets options
func (sm *Statemgr) GetOrchestratorWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
}

// OnOrchestratorCreate creates a orchestrator based on watch event
func (sm *Statemgr) OnOrchestratorCreate(w *ctkit.Orchestrator) error {
	_, ok := sm.probeCh[w.Orchestrator.Name]
	if ok {
		return fmt.Errorf("vc probe channel already created")
	}

	probeChannel := make(chan *kvstore.WatchEvent, probeChannelMax)

	err := sm.AddProbeChannel(w.Orchestrator.GetName(), probeChannel)
	if err != nil {
		return err
	}
	sm.instanceManagerCh <- &kvstore.WatchEvent{Object: &w.Orchestrator, Type: kvstore.Created}
	_, err = NewOrchestratorState(w, sm)

	return err
}

// OnOrchestratorUpdate handles update event
func (sm *Statemgr) OnOrchestratorUpdate(w *ctkit.Orchestrator, nw *orchestration.Orchestrator) error {
	// TODO : act on the state object recovered
	sm.instanceManagerCh <- &kvstore.WatchEvent{Object: nw, Type: kvstore.Updated}
	_, err := OrchestratorStateFromObj(w)

	return err
}

// OnOrchestratorDelete deletes a orchestrator
func (sm *Statemgr) OnOrchestratorDelete(w *ctkit.Orchestrator) error {
	sm.instanceManagerCh <- &kvstore.WatchEvent{Object: &w.Orchestrator, Type: kvstore.Deleted}
	return sm.RemoveProbeChannel(w.Orchestrator.Name)
}

// OrchestratorStateFromObj conerts from memdb object to orchestration state
func OrchestratorStateFromObj(obj runtime.Object) (*OrchestratorState, error) {
	switch obj.(type) {
	case *ctkit.Orchestrator:
		nobj := obj.(*ctkit.Orchestrator)
		switch nobj.HandlerCtx.(type) {
		case *OrchestratorState:
			nts := nobj.HandlerCtx.(*OrchestratorState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}

// NewOrchestratorState create new orchestration state
func NewOrchestratorState(wrk *ctkit.Orchestrator, stateMgr *Statemgr) (*OrchestratorState, error) {
	w := &OrchestratorState{
		Orchestrator: wrk,
		stateMgr:     stateMgr,
	}
	wrk.HandlerCtx = w

	return w, nil
}
