package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

// NetworkState is a wrapper for network object
type NetworkState struct {
	sync.Mutex
	Network  *ctkit.Network
	stateMgr *Statemgr
}

//GetNetworkWatchOptions gets options
func (sm *Statemgr) GetNetworkWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec", "Status.OperState"}
	return &opts
}

// OnNetworkCreate creates a network based on watch event
func (sm *Statemgr) OnNetworkCreate(w *ctkit.Network) error {
	_, err := NewNetworkState(w, sm)
	if err != nil {
		return err
	}
	// Act on network only after it is activated by NPM
	if w.Status.OperState == network.OperState_Active.String() {
		sm.SendNetworkProbeEvent(&w.Network, kvstore.Created)
	}
	return nil
}

// OnNetworkUpdate handles update event
func (sm *Statemgr) OnNetworkUpdate(w *ctkit.Network, nw *network.Network) error {
	_, err := NetworkStateFromObj(w)
	if err != nil {
		return err
	}

	if nw.Status.OperState == network.OperState_Active.String() {
		sm.SendNetworkProbeEvent(nw, kvstore.Updated)
	}

	return err
}

// OnNetworkDelete deletes a network
func (sm *Statemgr) OnNetworkDelete(w *ctkit.Network) error {
	sm.SendNetworkProbeEvent(&w.Network, kvstore.Deleted)
	return nil
}

// NetworkStateFromObj conerts from memdb object to network state
func NetworkStateFromObj(obj runtime.Object) (*NetworkState, error) {
	switch obj.(type) {
	case *ctkit.Network:
		nobj := obj.(*ctkit.Network)
		switch nobj.HandlerCtx.(type) {
		case *NetworkState:
			nts := nobj.HandlerCtx.(*NetworkState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}

// NewNetworkState create new network state
func NewNetworkState(wrk *ctkit.Network, stateMgr *Statemgr) (*NetworkState, error) {
	w := &NetworkState{
		Network:  wrk,
		stateMgr: stateMgr,
	}
	wrk.HandlerCtx = w

	return w, nil
}
