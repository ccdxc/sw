package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// ClusterState is a wrapper for Cluster object
type ClusterState struct {
	sync.Mutex
	Cluster  *ctkit.Cluster
	stateMgr *Statemgr
}

//GetClusterWatchOptions gets options
func (sm *Statemgr) GetClusterWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
}

// OnClusterCreate creates a Cluster based on watch event
func (sm *Statemgr) OnClusterCreate(nh *ctkit.Cluster) error {
	_, err := NewClusterState(nh, sm)
	return err
}

// OnClusterUpdate handles update event
func (sm *Statemgr) OnClusterUpdate(nh *ctkit.Cluster, nnw *cluster.Cluster) error {
	// TODO : act on the state object
	_, err := ClusterStateFromObj(nh)
	return err
}

// OnClusterDelete deletes a Cluster
func (sm *Statemgr) OnClusterDelete(nh *ctkit.Cluster) error {
	return nil
}

// OnClusterReconnect is called when ctkit reconnects to apiserver
func (sm *Statemgr) OnClusterReconnect() {
	return
}

// NewClusterState create new network state
func NewClusterState(Cluster *ctkit.Cluster, stateMgr *Statemgr) (*ClusterState, error) {
	w := &ClusterState{
		Cluster:  Cluster,
		stateMgr: stateMgr,
	}
	Cluster.HandlerCtx = w

	return w, nil
}

// ClusterStateFromObj converts from memdb object to Cluster state
func ClusterStateFromObj(obj runtime.Object) (*ClusterState, error) {
	switch obj.(type) {
	case *ctkit.Cluster:
		nobj := obj.(*ctkit.Cluster)
		switch nobj.HandlerCtx.(type) {
		case *ClusterState:
			nts := nobj.HandlerCtx.(*ClusterState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}
