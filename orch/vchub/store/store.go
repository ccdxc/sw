package store

import (
	"context"

	"github.com/pensando/sw/orch/vchub/defs"
	"github.com/pensando/sw/utils/log"
)

// VCHStore maintains information about a store instance
type VCHStore struct {
	ctx    context.Context
	snicDB *snicStore
	nwifDB *nwifStore
}

// NewVCHStore returns an instance of VCHStore
func NewVCHStore(ctx context.Context) *VCHStore {
	ss := newSnicStore(ctx)
	finder := func(hostKey, dvsUuid string) string {
		return getSNICId(ss, hostKey, dvsUuid)
	}

	ns := newNwifStore(ctx, finder)

	notifier := func(hostKey string) {
		ns.hostUpdate(hostKey)
	}
	ss.registerNotify(notifier)

	return &VCHStore{
		ctx:    ctx,
		snicDB: ss,
		nwifDB: ns,
	}
}

// Run processes updates sent on the input channel
func (v *VCHStore) Run(inbox <-chan defs.StoreMsg) {

	for {
		select {
		case <-v.ctx.Done():
			return

		case m, active := <-inbox:
			if !active {
				return
			}

			log.Infof("Msg from %v, key: %s prop: %s", m.Originator, m.Key, m.Property)
			switch m.Property {
			case defs.HostPropConfig:
				var hc *defs.ESXHost
				if m.Value != nil {
					hc = m.Value.(*defs.ESXHost)
				}
				v.snicDB.processHostConfig(m.Op, m.Key, hc)

			case defs.VMPropConfig:
				var vc *defs.VMConfig
				if m.Value != nil {
					vc = m.Value.(*defs.VMConfig)
				}
				v.nwifDB.processVMConfig(m.Op, m.Key, vc)

			case defs.VMPropName:
				if m.Value != nil {
					v.nwifDB.processVMName(m.Op, m.Key, m.Value.(string))
				}

			case defs.VMPropRT:
				var vr *defs.VMRuntime
				if m.Value != nil {
					vr = m.Value.(*defs.VMRuntime)
				}
				v.nwifDB.processVMRunTime(m.Op, m.Key, vr)

			}
		}
	}
}
