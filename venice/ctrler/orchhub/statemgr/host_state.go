package statemgr

import (
	"context"
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// HostState is a wrapper for host object
type HostState struct {
	sync.Mutex
	Host     *ctkit.Host
	stateMgr *Statemgr
}

//GetHostWatchOptions gets options
func (sm *Statemgr) GetHostWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
	return &opts
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

// HostStateFromObj converts from memdb object to host state
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

// ListHostByNamespace list all hosts which have the namespace label
func (sm *Statemgr) ListHostByNamespace(namespace string) ([]*ctkit.Host, error) {
	opts := api.ListWatchOptions{}
	opts.LabelSelector = fmt.Sprintf("%v=%v", utils.NamespaceKey, namespace)
	return sm.ctrler.Host().List(context.Background(), &opts)
}

// DeleteHostByNamespace deletes hosts by namespace
func (sm *Statemgr) DeleteHostByNamespace(namespace string) error {
	hosts, err := sm.ListHostByNamespace(namespace)
	if err != nil {
		return err
	}

	for _, host := range hosts {
		hostState, err := HostStateFromObj(host)
		if err != nil {
			log.Errorf("Failed to get host. Err : %v", err)
		}

		hostState.stateMgr.ctrler.Host().Delete(&host.Host)
	}

	return nil
}
