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
	probeChannelMax = 500
)

// OrchestratorState is a wrapper for orchestration object
type OrchestratorState struct {
	sync.Mutex
	Orchestrator     *ctkit.Orchestrator
	stateMgr         *Statemgr
	incompatibleDscs map[string]bool
}

//GetOrchestratorWatchOptions gets options
func (sm *Statemgr) GetOrchestratorWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
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
	sm.instanceManagerCh <- &kvstore.WatchEvent{Object: nw, Type: kvstore.Updated}
	_, err := OrchestratorStateFromObj(w)

	return err
}

// OnOrchestratorDelete deletes a orchestrator
func (sm *Statemgr) OnOrchestratorDelete(w *ctkit.Orchestrator) error {
	sm.instanceManagerCh <- &kvstore.WatchEvent{Object: &w.Orchestrator, Type: kvstore.Deleted}
	err := sm.RemoveProbeChannel(w.Orchestrator.Name)
	return err
}

// OnOrchestratorReconnect is called when ctkit reconnects to apiserver
func (sm *Statemgr) OnOrchestratorReconnect() {
	return
}

// OrchestratorStateFromObj converts from memdb object to orchestration state
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
		Orchestrator:     wrk,
		stateMgr:         stateMgr,
		incompatibleDscs: make(map[string]bool),
	}

	for _, dsc := range wrk.Orchestrator.Status.IncompatibleDSCs {
		w.incompatibleDscs[dsc] = true
	}

	wrk.HandlerCtx = w

	return w, nil
}

// AddIncompatibleDSC adds DSC MAC address to orchestrator incompatible list
func (o *OrchestratorState) AddIncompatibleDSC(dsc string) error {
	o.Lock()
	defer o.Unlock()

	_, ok := o.incompatibleDscs[dsc]
	if !ok {
		o.incompatibleDscs[dsc] = true
		if o.Orchestrator.Status.IncompatibleDSCs == nil {
			o.Orchestrator.Status.IncompatibleDSCs = []string{}
		}

		o.Orchestrator.Status.IncompatibleDSCs = append(o.Orchestrator.Status.IncompatibleDSCs, dsc)
		return o.Orchestrator.Write()
	}

	return nil
}

// RemoveIncompatibleDSC removes DSC from orchestrator incompatible list
func (o *OrchestratorState) RemoveIncompatibleDSC(dsc string) error {
	o.Lock()
	defer o.Unlock()

	_, ok := o.incompatibleDscs[dsc]
	if ok {
		delete(o.incompatibleDscs, dsc)
		if o.Orchestrator.Status.IncompatibleDSCs == nil {
			return nil
		}

		newList := []string{}

		for d := range o.incompatibleDscs {
			newList = append(newList, d)
		}

		o.Orchestrator.Status.IncompatibleDSCs = newList
		return o.Orchestrator.Write()
	}

	return nil
}

// AddIncompatibleDSCToOrch add incompat dsc to orch
func (sm *Statemgr) AddIncompatibleDSCToOrch(dsc, orch string) error {
	obj, err := sm.FindObject("Orchestrator", "", "", orch)
	if err != nil {
		sm.logger.Errorf("Failed to find object. Err :%v", err)
		return err
	}

	oState, err := OrchestratorStateFromObj(obj)
	if err != nil {
		return err
	}

	oState.AddIncompatibleDSC(dsc)
	return nil
}

// RemoveIncompatibleDSCFromOrch remove incompat dsc from orch
func (sm *Statemgr) RemoveIncompatibleDSCFromOrch(dsc, orch string) error {
	obj, err := sm.FindObject("Orchestrator", "", "", orch)
	if err != nil {
		sm.logger.Errorf("Failed to find object. Err :%v", err)
		return err
	}

	oState, err := OrchestratorStateFromObj(obj)
	if err != nil {
		return err
	}

	oState.RemoveIncompatibleDSC(dsc)
	return nil
}
