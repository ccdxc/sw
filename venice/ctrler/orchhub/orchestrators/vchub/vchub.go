package vchub

import (
	"net/url"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/store"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	storeQSize = 64
)

// VCHub instance
type VCHub struct {
	// Needed so that VCHub can update its Orchestrator
	// object status (connection status, etc..)
	configMeta   *api.ObjectMeta
	store        *store.VCHStore
	probe        *vcprobe.VCProbe
	vcOpsChannel chan *kvstore.WatchEvent
	vcReadCh     chan defs.Probe2StoreMsg
	vcWriteCh    chan defs.Store2ProbeMsg
	stateMgr     *statemgr.Statemgr
	logger       log.Logger
}

// LaunchVCHub starts VCHub
func LaunchVCHub(stateMgr *statemgr.Statemgr, config *orchestration.Orchestrator, logger log.Logger) *VCHub {
	logger.Infof("VCHub instance for %s is starting...", config.GetName())

	vchub := &VCHub{
		configMeta: &config.ObjectMeta,
		stateMgr:   stateMgr,
		logger:     logger,
		vcReadCh:   make(chan defs.Probe2StoreMsg, storeQSize),
		vcWriteCh:  make(chan defs.Store2ProbeMsg, storeQSize),
	}
	vchub.createStore()
	vchub.createProbe(config)

	return vchub

}

func (v *VCHub) createStore() {
	v.store = store.NewVCHStore(v.stateMgr, v.vcReadCh, v.vcWriteCh, v.logger)
	v.store.Start()
}

func (v *VCHub) createProbe(config *orchestration.Orchestrator) {
	vcURL := &url.URL{
		Scheme: "https",
		Host:   config.Spec.URI,
		Path:   "/sdk",
	}

	vcURL.User = url.UserPassword(config.Spec.Credentials.UserName, config.Spec.Credentials.Password)

	v.probe = vcprobe.NewVCProbe(config.GetName(), vcURL, v.vcReadCh, v.vcWriteCh, v.stateMgr, v.logger)
	v.probe.Start()
}

// Destroy tears down VCHub instance
func (v *VCHub) Destroy() {
	// Teardown probe and store
	if v.store != nil {
		v.store.Stop()
		v.store = nil
	}
	if v.probe != nil {
		v.probe.Stop()
		v.probe = nil
	}
}

// UpdateConfig handles if the Orchestrator config has changed
func (v *VCHub) UpdateConfig(config *orchestration.Orchestrator) {
	// Restart the probe
	v.probe.Stop()

	v.createProbe(config)
}

// Sync handles an instance manager request to reqsync the inventory
func (v *VCHub) Sync() {
	// Resync the inventory
	v.probe.Sync()
}
