package vchub

import (
	"context"
	"fmt"
	"net/url"
	"strings"
	"sync"
	"time"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils/pcache"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	storeQSize        = 64
	defaultRetryCount = 3
)

// VCHub instance
type VCHub struct {
	*defs.State
	cancel       context.CancelFunc
	vcOpsChannel chan *kvstore.WatchEvent
	vcReadCh     chan defs.Probe2StoreMsg
	vcEventCh    chan defs.Probe2StoreMsg
	pCache       *pcache.PCache
	probe        vcprobe.ProbeInf
	DcMapLock    sync.Mutex
	// TODO: don't use DC display name as key, use ID instead
	DcMap        map[string]*PenDC
	DcID2NameMap map[string]string
	// Will be taken with write lock by sync
	syncLock sync.RWMutex
	// whether to act upon venice network events
	// When we are disconnected, we do not want to act upon network events
	// until we are reconnected and sync has finished.
	// processVeniceEvents will be set to false once we are disconnected.
	// Any network events will be drained and thrown out.
	// Once connection is restored, sync will take syncLock and set processVeniceEvents to be
	// true. Any new network events will be blocked until sync has finished, and then
	// will be reacted to.
	// A lock is needed for this flag since periodic sync can be triggered by another thread
	// while the connection status is being reacted to in store
	processVeniceEvents     bool
	processVeniceEventsLock sync.Mutex
	// Opts is options used during creation of this instance
	opts         []Option
	useMockProbe bool
}

// Option specifies optional values for vchub
type Option func(*VCHub)

// WithMockProbe uses a probe interceptor to handle issues with vcsim
func WithMockProbe(v *VCHub) {
	v.useMockProbe = true
}

// WithVcEventsCh listens to the supplied channel for vC notifcation events
func WithVcEventsCh(ch chan defs.Probe2StoreMsg) Option {
	return func(v *VCHub) {
		v.vcEventCh = ch
	}
}

// LaunchVCHub starts VCHub
func LaunchVCHub(stateMgr *statemgr.Statemgr, config *orchestration.Orchestrator, logger log.Logger, opts ...Option) *VCHub {
	logger.Infof("VCHub instance for %s(orch-%d) is starting...", config.GetName(), config.Status.OrchID)
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

	if config.Labels == nil {
		logger.Infof("No DCs specified, handle all DCs in a vcenter")
		config.Labels = map[string]string{}
	}
	forceDCMap := map[string]bool{}
	forceDC, ok := config.Labels["force-dc-names"]
	if ok {
		logger.Infof("Forced DC %s: Only events for this DC will be processed", forceDC)
		forceDCs := strings.Split(forceDC, ",")
		for _, dc := range forceDCs {
			forceDCMap[dc] = true
		}
	}
	orchID := fmt.Sprintf("orch%d", config.Status.OrchID)
	state := defs.State{
		VcURL:        vcURL,
		VcID:         config.GetName(),
		OrchID:       orchID,
		Ctx:          ctx,
		Log:          logger.WithContext("submodule", fmt.Sprintf("VCHub-%s(%s)", config.GetName(), orchID)),
		StateMgr:     stateMgr,
		OrchConfig:   config,
		Wg:           &sync.WaitGroup{},
		ForceDCNames: forceDCMap,
	}

	v.State = &state
	v.cancel = cancel
	v.DcMap = map[string]*PenDC{}
	v.DcID2NameMap = map[string]string{}
	v.vcReadCh = make(chan defs.Probe2StoreMsg, storeQSize)
	v.vcEventCh = make(chan defs.Probe2StoreMsg, storeQSize)
	v.opts = opts
	v.setupPCache()

	clusterItems, err := v.StateMgr.Controller().Cluster().List(context.Background(), &api.ListWatchOptions{})
	if err != nil {
		logger.Errorf("Failed to get cluster object, %s", err)
	} else if len(clusterItems) == 0 {
		logger.Errorf("Cluster list returned 0 objects, %s", err)
	} else {
		cluster := clusterItems[0]
		state.ClusterID = defs.CreateClusterID(cluster.Cluster)
	}

	for _, opt := range opts {
		if opt != nil {
			opt(v)
		}
	}

	v.Wg.Add(1)
	go v.startEventsListener()
	v.createProbe(config)
}

func (v *VCHub) createProbe(config *orchestration.Orchestrator) {
	probe := vcprobe.NewVCProbe(v.vcReadCh, v.vcEventCh, v.State)
	if v.useMockProbe {
		v.probe = mock.NewProbeMock(probe)
	} else {
		v.probe = probe
	}
	v.probe.Start(false)
}

// Destroy tears down VCHub instance
func (v *VCHub) Destroy(delete bool) {
	// Teardown probe and store
	v.Log.Infof("Destroying VCHub....")

	v.cancel()
	v.Wg.Wait()

	v.probe.ClearState()

	if delete {
		ctx, cancel := context.WithCancel(context.Background())
		v.Ctx = ctx
		v.cancel = cancel

		// Create new probe in write only mode (we don't start watchers or update vcenter status)
		v.probe = vcprobe.NewVCProbe(nil, nil, v.State)
		v.probe.Start(true)

		v.Log.Infof("Cleaning up state on VCenter.")
		waitCh := make(chan bool, 1)

		// Cleanup might end up getting blocked forever if vCenter cannot be reached
		// Give up after 500 ms
		v.Wg.Add(1)
		go func() {
			defer v.Wg.Done()
			for !v.probe.IsSessionReady() {
				select {
				case <-v.Ctx.Done():
					return
				case <-time.After(50 * time.Millisecond):
				}
			}
			v.deleteAllDVS()
			waitCh <- true
		}()
		// Timeout limit for vcenter cleanup
		select {
		case <-waitCh:
			v.Log.Infof("VCenter cleanup finished")
		case <-time.After(500 * time.Millisecond):
			v.Log.Infof("Failed to cleanup vcenter within 500ms")
		}
		v.cancel()
		v.Wg.Wait()

		v.DeleteHosts()
	}

	v.probe.ClearState()

	v.Log.Infof("VCHub Destroyed")
}

// UpdateConfig handles if the Orchestrator config has changed
func (v *VCHub) UpdateConfig(config *orchestration.Orchestrator) {
	// Restart vchub
	v.Log.Infof("VCHub config updated, restarting...")
	v.Destroy(false)
	v.setupVCHub(v.StateMgr, v.OrchConfig, v.Log, v.opts...)
}

// deleteAllDVS cleans up all the PensandoDVS present in the DCs within the VC deployment
func (v *VCHub) deleteAllDVS() {
	v.DcMapLock.Lock()
	defer v.DcMapLock.Unlock()

	for _, dc := range v.DcMap {
		_, ok := v.ForceDCNames[dc.Name]
		if len(v.ForceDCNames) > 0 && !ok {
			v.Log.Infof("Skipping deletion of DVS from %v.", dc.Name)
			continue
		}

		dc.Lock()
		for _, penDVS := range dc.DvsMap {
			err := v.probe.RemovePenDVS(dc.Name, penDVS.DvsName, defaultRetryCount)
			if err != nil {
				v.Log.Errorf("Failed deleting DVS %v in DC %v. Err : %v", penDVS.DvsName, dc.Name, err)
			}
		}
		dc.Unlock()
	}
}

// Sync handles an instance manager request to reqsync the inventory
func (v *VCHub) Sync() {
	v.Log.Debugf("VCHub Sync starting")
	// Bring useg to VCHub struct
	v.Wg.Add(1)
	go func() {
		defer v.Wg.Done()
		v.sync()
	}()
}

// Debug returns debug info
func (v *VCHub) Debug(action string, params map[string]string) (interface{}, error) {
	return v.debugHandler(action, params)
}

// ListPensandoHosts List only Pensando Hosts from vCenter
func (v *VCHub) ListPensandoHosts(dcRef *types.ManagedObjectReference) []mo.HostSystem {
	hosts := v.probe.ListHosts(dcRef)
	var penHosts []mo.HostSystem
	for _, host := range hosts {
		if !isPensandoHost(host.Config) {
			v.Log.Debugf("Skipping non-Pensando Host %s", host.Name)
			continue
		}
		penHosts = append(penHosts, host)
	}
	return penHosts
}

// CreateVmkWorkloadName returns vmk workload name string
func (v *VCHub) createVmkWorkloadName(namespace, objName string) string {
	// don't include namespace (DC name) in the vmk workload name
	return fmt.Sprintf("%s%s%s", defs.VmkWorkloadPrefix, utils.Delim, v.createHostName("", objName))
}

// createHostName returns host name string
func (v *VCHub) createHostName(namespace, objName string) string {
	// Remove DC name from the host name. On events like workload update, the DC of the event is
	// different from the DC where the host is
	return fmt.Sprintf("%s", utils.CreateGlobalKey(v.OrchID, "" /* namespace */, objName))
}

func (v *VCHub) createVMWorkloadName(namespace, objName string) string {
	// don't include namespace (DC name) in the workload name
	return fmt.Sprintf("%s", utils.CreateGlobalKey(v.OrchID, "", objName))
}

func (v *VCHub) parseVMKeyFromWorkloadName(workloadName string) (vmKey string) {
	return fmt.Sprintf("%s", utils.ParseGlobalKey(v.OrchID, "", workloadName))
}
