package store

import (
	"context"
	"sync"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
)

// VCHStore maintains information about a store instance
type VCHStore struct {
	sync.Mutex
	ctx      context.Context
	cancel   context.CancelFunc
	wg       sync.WaitGroup
	Log      log.Logger
	stateMgr *statemgr.Statemgr
	inbox    <-chan defs.Probe2StoreMsg
	outbox   chan<- defs.Store2ProbeMsg
}

// NewVCHStore returns an instance of VCHStore
func NewVCHStore(stateMgr *statemgr.Statemgr, inbox <-chan defs.Probe2StoreMsg, outbox chan<- defs.Store2ProbeMsg, l log.Logger) *VCHStore {
	logger := l.WithContext("submodule", "VCStore")

	return &VCHStore{
		Log:      logger,
		stateMgr: stateMgr,
		inbox:    inbox,
		outbox:   outbox,
	}
}

// Start starts the worker pool
func (v *VCHStore) Start() {
	v.Lock()
	defer v.Unlock()
	if v.cancel != nil {
		v.Log.Errorf("Store already started")
		return
	}
	v.ctx, v.cancel = context.WithCancel(context.Background())
	go v.run()
}

// Stop stops the sessions
func (v *VCHStore) Stop() {
	v.Lock()
	defer v.Unlock()
	if v.cancel != nil {
		v.cancel()
		v.cancel = nil
		v.wg.Wait()
	}
}

// run processes updates sent on the input channel
func (v *VCHStore) run() {
	v.Log.Infof("Running store")

	v.wg.Add(1)
	defer v.wg.Done()

	for {
		select {
		case <-v.ctx.Done():
			return

		case m, active := <-v.inbox:
			if !active {
				return
			}

			v.Log.Infof("Msg from %v, key: %s prop: %s", m.Originator, m.Key, m.VcObject)
			switch m.VcObject {
			case defs.VirtualMachine:
				v.handleWorkload(m)
			case defs.HostSystem:
				v.handleHost(m)
			default:
				v.Log.Errorf("Unknown object %s", m.VcObject)
				continue
			}
		}
	}
}
