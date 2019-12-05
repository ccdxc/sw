package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// DistributedServiceCardState is a wrapper for cluster object
type DistributedServiceCardState struct {
	sync.Mutex
	DistributedServiceCard *ctkit.DistributedServiceCard
	stateMgr               *Statemgr
}

//GetDistributedServiceCardWatchOptions gets options
func (sm *Statemgr) GetDistributedServiceCardWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
}

// OnDistributedServiceCardCreate creates a DistributedServiceCard based on watch event
func (sm *Statemgr) OnDistributedServiceCardCreate(dsc *ctkit.DistributedServiceCard) error {
	_, err := NewDSCState(dsc, sm)
	return err
}

// OnDistributedServiceCardUpdate handles update event
func (sm *Statemgr) OnDistributedServiceCardUpdate(dsc *ctkit.DistributedServiceCard, ndsc *cluster.DistributedServiceCard) error {
	// TODO : act on the object
	_, err := DistributedServiceCardStateFromObj(dsc)
	return err
}

// OnDistributedServiceCardDelete deletes a DistributedServiceCard
func (sm *Statemgr) OnDistributedServiceCardDelete(dsc *ctkit.DistributedServiceCard) error {
	// TODO : act on the object
	_, err := DistributedServiceCardStateFromObj(dsc)
	return err
}

// NewDSCState create new DSC state
func NewDSCState(dsc *ctkit.DistributedServiceCard, stateMgr *Statemgr) (*DistributedServiceCardState, error) {
	w := &DistributedServiceCardState{
		DistributedServiceCard: dsc,
		stateMgr:               stateMgr,
	}
	dsc.HandlerCtx = w

	return w, nil
}

// DistributedServiceCardStateFromObj conerts from memdb object to cluster state
func DistributedServiceCardStateFromObj(obj runtime.Object) (*DistributedServiceCardState, error) {
	switch obj.(type) {
	case *ctkit.DistributedServiceCard:
		nobj := obj.(*ctkit.DistributedServiceCard)
		switch nobj.HandlerCtx.(type) {
		case *DistributedServiceCardState:
			nts := nobj.HandlerCtx.(*DistributedServiceCardState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}
