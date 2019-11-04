package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// HostState is a wrapper for host object
type HostState struct {
	sync.Mutex
	Host     *ctkit.Host
	stateMgr *Statemgr
}

// OnHostCreate creates a host based on watch event
func (sm *Statemgr) OnHostCreate(nh *ctkit.Host) error {
	_, err := NewHostState(nh, sm)
	return err
}

// OnHostUpdate handles update event
func (sm *Statemgr) OnHostUpdate(nh *ctkit.Host, nnw *cluster.Host) error {
	// TODO : act on the state object
	_, err := HostStateFromObj(nh)
	return err
}

// OnHostDelete deletes a host
func (sm *Statemgr) OnHostDelete(nh *ctkit.Host) error {
	return nil
}

// NewHostState create new network state
func NewHostState(host *ctkit.Host, stateMgr *Statemgr) (*HostState, error) {
	w := &HostState{
		Host:     host,
		stateMgr: stateMgr,
	}
	host.HandlerCtx = w

	return w, nil
}

// HostStateFromObj conerts from memdb object to host state
func HostStateFromObj(obj runtime.Object) (*HostState, error) {
	switch obj.(type) {
	case *ctkit.Host:
		nobj := obj.(*ctkit.Host)
		switch nobj.HandlerCtx.(type) {
		case *HostState:
			nts := nobj.HandlerCtx.(*HostState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}
