package vchub

import (
	"fmt"

	"github.com/vmware/govmomi/vim25/soap"
	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/store"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	storeQSize = 64
)

// VCHub instance
type VCHub struct {
	// Needed so that VCHub can update its Orchestrator
	// object status (connection status, etc..)
	configMeta *api.ObjectMeta
	store      *store.VCHStore
	probe      *vcprobe.VCProbe
}

// LaunchVCHub starts VCHub
func LaunchVCHub(stateMgr *statemgr.Statemgr, config *orchestration.Orchestrator, logger log.Logger) *VCHub {
	logger.Infof("VCHub instance for %s is starting...", config.GetName())

	vc := fmt.Sprintf("https://%s:%s@%s/sdk", config.Spec.Credentials.UserName, config.Spec.Credentials.Password, config.Spec.URI)
	vcURL, err := soap.ParseURL(vc)
	if err != nil {
		logger.Errorf("Failed to parse VCenter URL [%v]. Err : %v", vc, err)
		vcURL = nil
	}

	vcReadCh := make(chan defs.Probe2StoreMsg, storeQSize)
	vcWriteCh := make(chan defs.Store2ProbeMsg, storeQSize)
	vchStore := store.NewVCHStore(context.Background(), stateMgr, vcReadCh, vcWriteCh, logger)
	vchStore.Run()

	// Pass in statemgr so it can update config connection status status
	vcp := vcprobe.NewVCProbe(config.GetName(), vcURL, vcReadCh, vcWriteCh, stateMgr, logger)
	return &VCHub{
		configMeta: &config.ObjectMeta,
		store:      vchStore,
		probe:      vcp,
	}
}

// Destroy tears down VCHub instance
func (v VCHub) Destroy() {
	// Teardown probe and store
}

// UpdateConfig handles if the Orchestrator config has changed
func (v VCHub) UpdateConfig(*orchestration.Orchestrator) {
	// Restart the probe
}

// Sync handles an instance manager request to reqsync the inventory
func (v VCHub) Sync() {
	// Resync the inventory
}
