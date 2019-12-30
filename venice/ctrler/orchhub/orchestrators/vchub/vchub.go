package vchub

import (
	"context"
	"fmt"
	"net/url"
	"sync"

	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils/pcache"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	storeQSize = 64
)

// VCHub instance
type VCHub struct {
	*defs.State
	cancel       context.CancelFunc
	vcOpsChannel chan *kvstore.WatchEvent
	vcReadCh     chan defs.Probe2StoreMsg
	pCache       *pcache.PCache
	probe        *vcprobe.VCProbe
	DcMapLock    sync.Mutex
	// TODO: don't use DC display name as key, use ID instead
	DcMap map[string]*PenDC
	// If supplied, we only process events if the DC name matches this
	// TODO: for testing locally only, remove eventually
	forceDCname string
	// Opts is options used during creation of this instance
	opts []Option
}

// Option specifies optional values for vchub
type Option func(*VCHub)

// WithScheme sets the scheme for the client to use when connecting to vcenter
func WithScheme(scheme string) Option {
	return func(v *VCHub) {
		vcURL := &url.URL{
			Scheme: scheme,
			Host:   v.OrchConfig.Spec.URI,
			Path:   "/sdk",
		}
		vcURL.User = url.UserPassword(v.OrchConfig.Spec.Credentials.UserName, v.OrchConfig.Spec.Credentials.Password)
		v.State.VcURL = vcURL
	}
}

// LaunchVCHub starts VCHub
func LaunchVCHub(stateMgr *statemgr.Statemgr, config *orchestration.Orchestrator, logger log.Logger, opts ...Option) *VCHub {
	logger.Infof("VCHub instance for %s is starting...", config.GetName())
	vchub := &VCHub{}
	vchub.setupVCHub(stateMgr, config, logger, opts...)
	return vchub
}

func (v *VCHub) setupVCHub(stateMgr *statemgr.Statemgr, config *orchestration.Orchestrator, logger log.Logger, opts ...Option) {
	ctx, cancel := context.WithCancel(context.Background())

	vcURL := &url.URL{
		Scheme: "https",
		Host:   config.Spec.URI,
		Path:   "/sdk",
	}
	vcURL.User = url.UserPassword(config.Spec.Credentials.UserName, config.Spec.Credentials.Password)

	state := defs.State{
		VcURL:      vcURL,
		VcID:       config.GetName(),
		Ctx:        ctx,
		Log:        logger.WithContext("submodule", fmt.Sprintf("VCHub-%s", config.GetName())),
		StateMgr:   stateMgr,
		OrchConfig: config,
		Wg:         &sync.WaitGroup{},
	}

	// TODO: remove forceDC
	forceDC, ok := config.Labels["force-dc-name"]
	if ok {
		logger.Infof("Foced DC %s: Only events for this DC will be processed", forceDC)
	}

	v.State = &state
	v.cancel = cancel
	v.DcMap = map[string]*PenDC{}
	v.vcReadCh = make(chan defs.Probe2StoreMsg, storeQSize)
	v.forceDCname = forceDC
	v.opts = opts
	v.setupPCache()

	for _, opt := range opts {
		if opt != nil {
			opt(v)
		}
	}

	// Store related go routines
	v.Wg.Add(1)
	go v.startEventsListener()

	// Store must be created before probe for sync to work properly
	v.createProbe(config)

	v.Sync()

	v.Wg.Add(1)
	go v.probe.StartWatchers()
}

func (v *VCHub) createProbe(config *orchestration.Orchestrator) {
	v.probe = vcprobe.NewVCProbe(v.vcReadCh, v.State)
	v.probe.Start()
}

// Destroy tears down VCHub instance
func (v *VCHub) Destroy() {
	// Teardown probe and store
	v.Log.Infof("Destroying VCHub....")
	v.cancel()

	v.Wg.Wait()
	// Clearing probe/session state after all routines finish
	// so that a thread in the middle of writing doesn't get a nil client
	v.probe.ClearState()
	v.Log.Infof("VCHub Destroyed")
}

// UpdateConfig handles if the Orchestrator config has changed
func (v *VCHub) UpdateConfig(config *orchestration.Orchestrator) {
	// Restart vchub
	v.Destroy()
	v.setupVCHub(v.StateMgr, v.OrchConfig, v.Log, v.opts...)
}

// Sync handles an instance manager request to reqsync the inventory
func (v *VCHub) Sync() {
	v.Log.Debugf("VCHub Sync starting")
	// Bring useg to VCHub struct
	v.Wg.Add(1)
	go v.sync()

	// Resync the inventory
	// Sync api server state needed for store.
	// v.store.Sync()

	// Gets diffs
	// Pushes deletes/creates as watch events
	// v.probe.Sync()
}
