package store

import (
	"context"
	"sync"

	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
)

// VCHStore maintains information about a store instance
type VCHStore struct {
	sync.Mutex
	ctx        context.Context
	cancel     context.CancelFunc
	wg         sync.WaitGroup
	Log        log.Logger
	stateMgr   *statemgr.Statemgr
	inbox      <-chan defs.Probe2StoreMsg
	outbox     chan<- defs.Store2ProbeMsg
	pCache     *PCache
	usegMgr    useg.Inf
	orchConfig *orchestration.Orchestrator
}

// NewVCHStore returns an instance of VCHStore
func NewVCHStore(stateMgr *statemgr.Statemgr, inbox <-chan defs.Probe2StoreMsg, outbox chan<- defs.Store2ProbeMsg, l log.Logger, orchConfig *orchestration.Orchestrator) *VCHStore {
	logger := l.WithContext("submodule", "VCStore")
	pCache := NewPCache(stateMgr, logger)
	useg, err := useg.NewUsegAllocator()
	if err != nil {
		logger.Errorf("Creating useg mgr failed, %v", err)
		return nil
	}

	vstore := &VCHStore{
		Log:        logger,
		stateMgr:   stateMgr,
		inbox:      inbox,
		outbox:     outbox,
		pCache:     pCache,
		usegMgr:    useg,
		orchConfig: orchConfig,
	}

	pCache.SetValidator("Workload", vstore.validateWorkload)

	return vstore
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
	v.wg.Add(1)
	go v.run()
	v.pCache.Run()
}

// Stop stops the sessions
func (v *VCHStore) Stop() {
	v.Lock()
	defer v.Unlock()
	v.pCache.Stop()

	if v.cancel != nil {
		v.cancel()
		v.cancel = nil
		v.wg.Wait()
	}
}

// run processes updates sent on the input channel
func (v *VCHStore) run() {
	v.Log.Infof("Running store")

	defer v.wg.Done()

	for {
		select {
		case <-v.ctx.Done():
			return

		case m, active := <-v.inbox:
			if !active {
				return
			}

			switch m.MsgType {
			case defs.VCEvent:
				v.handleVCEvent(m.Val.(defs.VCEventMsg))
			default:
				v.Log.Errorf("Unknown event %s", m.MsgType)
			}
		}
	}
}

func (v *VCHStore) handleVCEvent(m defs.VCEventMsg) {
	v.Log.Infof("Msg from %v, key: %s prop: %s", m.Originator, m.Key, m.VcObject)
	switch m.VcObject {
	case defs.VirtualMachine:
		v.handleWorkload(m)
	case defs.HostSystem:
		v.handleHost(m)
	default:
		v.Log.Errorf("Unknown object %s", m.VcObject)
	}
}
