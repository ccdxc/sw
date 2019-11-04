package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/runtime"
)

// EndpointState is a wrapper for workload object, this is to write local functions which act on the ctkit object
type EndpointState struct {
	sync.Mutex
	Endpoint *ctkit.Endpoint
	stateMgr *Statemgr
}

// OnEndpointCreate creates a Endpoint based on watch event
func (sm *Statemgr) OnEndpointCreate(w *ctkit.Endpoint) error {
	_, err := NewEndpointState(w, sm)
	if err != nil {
		return err
	}

	return nil
}

// OnEndpointUpdate handles update event
func (sm *Statemgr) OnEndpointUpdate(w *ctkit.Endpoint, nw *workload.Endpoint) error {
	// TODO : act on the state object
	_, err := EndpointStateFromObj(w)
	if err != nil {
		return err
	}

	return nil
}

// OnEndpointDelete deletes a Endpoint
func (sm *Statemgr) OnEndpointDelete(w *ctkit.Endpoint) error {
	// TODO : act on the state object
	_, err := EndpointStateFromObj(w)
	if err != nil {
		return err
	}

	return nil
}

// EndpointStateFromObj conerts from memdb object to workload state
func EndpointStateFromObj(obj runtime.Object) (*EndpointState, error) {
	switch obj.(type) {
	case *ctkit.Endpoint:
		nobj := obj.(*ctkit.Endpoint)
		switch nobj.HandlerCtx.(type) {
		case *EndpointState:
			nts := nobj.HandlerCtx.(*EndpointState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}

// NewEndpointState create new workload state
func NewEndpointState(wrk *ctkit.Endpoint, stateMgr *Statemgr) (*EndpointState, error) {
	w := &EndpointState{
		Endpoint: wrk,
		stateMgr: stateMgr,
	}
	wrk.HandlerCtx = w

	return w, nil
}
