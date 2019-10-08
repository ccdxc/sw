package store

import (
	"context"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch/vchub/defs"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// VCHStore maintains information about a store instance
type VCHStore struct {
	ctx   context.Context
	wg    sync.WaitGroup
	apicl apiclient.Services
}

// NewVCHStore returns an instance of VCHStore
func NewVCHStore(ctx context.Context, apisrvURL string, resolver resolver.Interface) *VCHStore {
	config := log.GetDefaultConfig("VCStore")
	l := log.GetNewLogger(config)
	b := balancer.New(resolver)

	apicl, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		client, err := apiclient.NewGrpcAPIClient(globals.VCHub, apisrvURL, l, rpckit.WithBalancer(b))
		if err != nil {
			return nil, err
		}
		return client, nil
	}, 100*time.Millisecond, 20)

	if err != nil {
		// TODO: Make this recoverable
		// TODO: If the connection to api server goes down,
		// VCStore must be able to reconnect the client and recover
		log.Errorf("Failed to create API Server client: %+v", err)
	}

	return &VCHStore{
		ctx:   ctx,
		apicl: apicl.(apiclient.Services),
	}
}

// Run starts a go func that processes updates sent on the input channel
func (v *VCHStore) Run(inbox <-chan defs.StoreMsg) {
	go v.run(inbox)
}

// WaitForExit waits for the store thread to exit
func (v *VCHStore) WaitForExit() {
	v.wg.Wait()
}

// run processes updates sent on the input channel
func (v *VCHStore) run(inbox <-chan defs.StoreMsg) {

	v.wg.Add(1)
	defer v.wg.Done()

	for {
		select {
		case <-v.ctx.Done():
			return

		case m, active := <-inbox:
			if !active {
				return
			}

			log.Infof("Msg from %v, key: %s prop: %s", m.Originator, m.Key, m.VcObject)
			switch m.VcObject {
			case defs.VirtualMachine:
				v.handleWorkload(m)
			case defs.HostSystem:
				v.handleHost(m)
			default:
				log.Errorf("Unknown object %s", m.VcObject)
				continue
			}
		}
	}
}
