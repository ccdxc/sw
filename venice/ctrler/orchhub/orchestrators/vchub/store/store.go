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
	ctx      context.Context
	wg       sync.WaitGroup
	Log      log.Logger
	stateMgr *statemgr.Statemgr
	inbox    <-chan defs.Probe2StoreMsg
	outbox   chan<- defs.Store2ProbeMsg
}

// NewVCHStore returns an instance of VCHStore
func NewVCHStore(ctx context.Context, stateMgr *statemgr.Statemgr, inbox <-chan defs.Probe2StoreMsg, outbox chan<- defs.Store2ProbeMsg, l log.Logger) *VCHStore {
	logger := l.WithContext("submodule", "VCStore")

	return &VCHStore{
		ctx:      ctx,
		Log:      logger,
		stateMgr: stateMgr,
		inbox:    inbox,
		outbox:   outbox,
	}
}

// Run starts a go func that processes updates sent on the input channel
func (v *VCHStore) Run() {
	v.Log.Infof("Running store")
	go v.run()
}

// WaitForExit waits for the store thread to exit
func (v *VCHStore) WaitForExit() {
	v.wg.Wait()
}

// run processes updates sent on the input channel
func (v *VCHStore) run() {

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
