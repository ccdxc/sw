package instanceManager

import (
	"context"
	"fmt"
	"reflect"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
)

// Orchestrator is the interface all orchestrator components
// must implement in order to be managed by instance manager
type Orchestrator interface {
	// delete flag controls if the objects on the remote server (vcenter)
	// are to be cleaned up when Destroy is called as well as any apiserver
	// objets that were created for the orchestrator
	Destroy(delete bool)
	Debug(action string, params map[string]string) (interface{}, error)
	UpdateConfig(*orchestration.Orchestrator)
	Sync()
	// Kind indicates which kind apiclient was reconnected in ctkit
	Reconnect(kind string)
}

type orchestratorMapEntry struct {
	orch   Orchestrator
	config *orchestration.Orchestrator
}

type syncFlag struct {
	sync.RWMutex
	flag bool
}

const (
	maxOrchSupported = 64
	debugLogLevel    = "logLevel"
)

type orchIDMgr struct {
	sync.Mutex
	allocOrchID   [maxOrchSupported]string
	key2Alloc     map[string]int32
	isInitialized bool
}

// InstanceManager is the struct which watches for update to vc config objects
type InstanceManager struct {
	waitGrp           sync.WaitGroup
	watchCtx          context.Context
	watchCancel       context.CancelFunc
	stopFlag          syncFlag
	instanceManagerCh chan *kvstore.WatchEvent
	ctkitReconnectCh  chan string
	logger            log.Logger
	orchMapLock       sync.RWMutex
	orchestratorMap   map[string]*orchestratorMapEntry
	stateMgr          *statemgr.Statemgr
	orchIDMgr         orchIDMgr
	vcHubOpts         []vchub.Option
	restoreActive     bool
}

// Stop stops the watcher
func (w *InstanceManager) Stop() {
	w.stop()
	w.watchCancel()
	// TODO: stop orchestrators.
	for _, orchEntry := range w.orchestratorMap {
		orchEntry.orch.Destroy(false)
	}
	w.waitGrp.Wait()
}

func (w *InstanceManager) stopped() bool {
	w.stopFlag.RLock()
	defer w.stopFlag.RUnlock()
	return w.stopFlag.flag
}

func (w *InstanceManager) stop() {
	w.stopFlag.Lock()
	w.stopFlag.flag = true
	w.stopFlag.Unlock()
}

// Debug is the debug handler for diag debug requests
func (w *InstanceManager) Debug(action string, params map[string]string) (interface{}, error) {
	// Occasionally setting log level through diag module doesn't work.
	// Adding a second way to change the log level.
	if action == debugLogLevel {
		level, ok := params["level"]
		if !ok {
			return nil, fmt.Errorf("level is a required param")
		}
		w.logger.ResetFilter(diagnostics.GetLogFilter(level))
		w.logger.Infof("Setting log level to %s", diagnostics.GetLogFilter(level))
		return nil, nil
	}
	key, ok := params["key"]
	if !ok {
		return nil, fmt.Errorf("key is a required param")
	}
	w.orchMapLock.RLock()
	orchEntry, ok := w.orchestratorMap[key]
	if !ok {
		// Try with default key format
		meta := api.ObjectMeta{
			Name: key,
		}
		key = meta.GetKey()
	}
	orchEntry, ok = w.orchestratorMap[key]
	w.orchMapLock.RUnlock()

	if !ok {
		options := []string{}
		w.orchMapLock.RLock()
		for k := range w.orchestratorMap {
			options = append(options, k)
		}
		w.orchMapLock.RUnlock()
		return nil, fmt.Errorf("No instance with the given key %s was found. Known keys: %v", key, options)
	}

	return orchEntry.orch.Debug(action, params)
}

// NewInstanceManager creates a new watcher
func NewInstanceManager(stateMgr *statemgr.Statemgr, logger log.Logger, instanceManagerCh chan *kvstore.WatchEvent, ctkitReconnectCh chan string, vcHubOpts []vchub.Option) (*InstanceManager, error) {
	watchCtx, watchCancel := context.WithCancel(context.Background())

	instance := &InstanceManager{
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
		logger:            logger.WithContext("submodule", "Inst Mgr"),
		orchestratorMap:   make(map[string]*orchestratorMapEntry),
		stateMgr:          stateMgr,
		instanceManagerCh: instanceManagerCh,
		ctkitReconnectCh:  ctkitReconnectCh,
		vcHubOpts:         vcHubOpts,
	}

	diagSvc := diagsvc.GetDiagnosticsService(globals.OrchHub, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, logger)
	diagSvc.RegisterCustomAction(vchub.DebugUseg, instance.Debug)
	diagSvc.RegisterCustomAction(vchub.DebugCache, instance.Debug)
	diagSvc.RegisterCustomAction(vchub.DebugState, instance.Debug)
	diagSvc.RegisterCustomAction(vchub.DebugSync, instance.Debug)
	diagSvc.RegisterCustomAction(vchub.DebugVlanSpace, instance.Debug)
	diagSvc.RegisterCustomAction(debugLogLevel, instance.Debug)

	return instance, nil
}

// Start starts instance manager
func (w *InstanceManager) Start() {
	w.waitGrp.Add(1)
	go w.watchOrchestratorConfig()
	// w.waitGrp.Add(1)
	// go w.periodicSync()
}

type configEntry struct {
	key string
	dcs map[string]bool
}

func (c *configEntry) String() string {
	return fmt.Sprintf("key: %s, dcs: %v", c.key, c.dcs)
}

// handleRestore handles state cleanup need on snapshot restore.
// tears down all orch instances, re-intializes internal state for orchestrators.
// After this function returns, all watch events will be processed.
func (w *InstanceManager) handleRestore() {
	w.logger.Infof("Restore reconciliation running...")
	// Perform List and compute differences
	w.orchMapLock.Lock()
	defer w.orchMapLock.Unlock()

	// Remove all probe channels as config will be replayed
	for _, orchEntry := range w.orchestratorMap {
		err := w.stateMgr.RemoveProbeChannel(orchEntry.config.Name)
		if err != nil {
			w.logger.Errorf("Err while removing probe ch, err %s", err)
		}
	}

	// This list goes to apiserver and will return the objects post restore
	// These objects are not in ctkits cache yet (cache should have pre-restore config)
	restoreOrchs, err := w.stateMgr.Controller().Orchestrator().ApisrvList(w.watchCtx, &api.ListWatchOptions{})
	if err != nil {
		w.logger.Errorf("Failed to get orchestrators after restore, %s", err)
		// Assume all orchs are gone
		for _, orchEntry := range w.orchestratorMap {
			orchEntry.orch.Destroy(true)
		}
		w.orchestratorMap = make(map[string]*orchestratorMapEntry)
	} else {
		// If any orch objects share the same URL - DC, then that DC doesn't need to be destroyed.
		// Send updates to orch config with DCs that shouldn't be destroyed, in order to destory the other DCs.

		configs := map[string]*configEntry{}

		// Fill in existing config
		for key, orchEntry := range w.orchestratorMap {
			uri := orchEntry.config.Spec.URI
			dcMap := map[string]bool{}
			for _, dc := range orchEntry.config.Spec.ManageNamespaces {
				dcMap[dc] = true
			}
			configs[uri] = &configEntry{
				key: key,
				dcs: dcMap,
			}
		}
		w.logger.Infof("Existing config map %+v", configs)

		// Build similar map with new state
		newConfigs := map[string]*configEntry{}

		for _, newOrch := range restoreOrchs {
			uri := newOrch.Spec.URI
			dcMap := map[string]bool{}
			for _, dc := range newOrch.Spec.ManageNamespaces {
				dcMap[dc] = true
			}
			newConfigs[uri] = &configEntry{
				key: newOrch.GetKey(),
				dcs: dcMap,
			}
		}
		w.logger.Infof("Restore config map %+v", newConfigs)

		// Remove any DCs that would be new from newConfigs
		for uri, entry := range newConfigs {
			oldEntry, ok := configs[uri]
			if !ok {
				delete(newConfigs, uri)
				continue
			}
			if _, ok := oldEntry.dcs[utils.ManageAllDcs]; ok {
				// There can't be any additions, call updateDCs with the given
				continue
			}
			if _, ok := entry.dcs[utils.ManageAllDcs]; ok {
				// Going to all DCS, leave existing DCs as is.
				entry.dcs = oldEntry.dcs
				continue
			}
			// Remove any additions
			for dc := range entry.dcs {
				if _, ok := oldEntry.dcs[dc]; !ok {
					// Addition, needs to be removed
					delete(entry.dcs, dc)
				}
			}
		}
		w.logger.Infof("Resulting config map %+v", newConfigs)

		// Call update config with the new list to cleanup dcs that should be destroyed
		for uri, entry := range configs {
			orchInst := w.orchestratorMap[entry.key]
			newEntry, ok := newConfigs[uri]
			if !ok || len(newEntry.dcs) == 0 {
				// Delete all namespaces
				orchInst.orch.Destroy(true)
				delete(w.orchestratorMap, entry.key)
				continue
			}
			currDCs := map[string]bool{}
			for _, dc := range orchInst.config.Spec.ManageNamespaces {
				currDCs[dc] = true
			}
			if !reflect.DeepEqual(newEntry.dcs, currDCs) {
				keepDCs := []string{}
				for dc := range newEntry.dcs {
					keepDCs = append(keepDCs, dc)
				}
				orchInst.config.Spec.ManageNamespaces = keepDCs
				orchInst.orch.UpdateConfig(orchInst.config)
			}
		}

	}

	// Tear down all vchubs and reset state
	for _, orchEntry := range w.orchestratorMap {
		orchEntry.orch.Destroy(false)
	}
	w.orchestratorMap = make(map[string]*orchestratorMapEntry)
	w.resetOrchIDMgr()
}

func (w *InstanceManager) watchOrchestratorConfig() {
	defer w.waitGrp.Done()
	w.logger.Info("instance manager watch started")
	for {
		select {
		case <-w.watchCtx.Done():
			w.logger.Info("Exiting watch for orchestration configuration")
			return
		case evt, ok := <-w.instanceManagerCh:
			if ok {
				switch obj := evt.Object.(type) {
				case *cluster.SnapshotRestore:
					w.logger.Infof("Snapshot restore event")
					// Snapshot restore event flow
					// Status moves to active, apiserver is locked.
					// apiserver creates a buffer of the diffs and commits.
					// Apiserver moves snapshot status to complete
					//
					// On active status, shutdown watchers so ctkit cache remains as is and
					// we process no more events. instanceManager.restoreActive is set to prevent
					// processing of any events that have been received before watchers are closed.
					//
					// On status failure, restart stopped watchers.
					// On status complete, we compute the diff of which Datacenters
					// are no longer managed. The datacenters that are no longer managed should
					// have their DVS destroyed, while those that are still managed should
					// only have a soft destroy.
					// We then shutodwn all vchubs, restart instance manager, and then
					// restart the watchers
					switch obj.Status.Status {
					case cluster.SnapshotRestoreStatus_Active.String():
						if w.restoreActive {
							// nothing to do, already active
							continue
						}
						w.logger.Infof("Snapshot restore is active")
						w.restoreActive = true
						// Stop the watchers
						w.stateMgr.StopWatchersOnRestore()
					case cluster.SnapshotRestoreStatus_Failed.String():
						if !w.restoreActive {
							// nothing to do, could be old status
							continue
						}
						w.logger.Infof("Snapshot restore failed")
						w.restoreActive = false
						w.stateMgr.RestartWatchersOnRestore()
					case cluster.SnapshotRestoreStatus_Completed.String():
						if !w.restoreActive {
							// nothing to do, could be old status
							continue
						}
						w.logger.Infof("Snapshot restore completed successfully")
						w.restoreActive = false
						// Tell instance manager to restart itself and the watchers
						w.handleRestore()
						w.stateMgr.RestartWatchersOnRestore()
					}
				case *orchestration.Orchestrator:
					if !w.restoreActive {
						w.handleConfigEvent(evt.Type, obj)
					} else {
						w.logger.Infof("Snapshot restore in progress, skipping event for %s", obj.GetName())
					}
				}
			}
		case kind, ok := <-w.ctkitReconnectCh:
			if ok {
				w.orchMapLock.RLock()
				for key, orchEntry := range w.orchestratorMap {
					w.logger.Infof("sending reconnect event to %s", key)
					orchEntry.orch.Reconnect(kind)
				}
				w.orchMapLock.RUnlock()
			}
		}
	}
}

func (w *InstanceManager) resetOrchIDMgr() {
	w.orchIDMgr.isInitialized = false
	w.orchIDMgr.allocOrchID = [maxOrchSupported]string{}
	w.orchIDMgr.key2Alloc = map[string]int32{}
}

func (w *InstanceManager) assignOrchestratorID(config *orchestration.Orchestrator) error {
	w.orchIDMgr.Lock()
	defer w.orchIDMgr.Unlock()

	// Handle orchhub restarts
	if w.orchIDMgr.isInitialized == false {
		w.orchIDMgr.key2Alloc = map[string]int32{}
		opts := api.ListWatchOptions{}
		orchs, err := w.stateMgr.Controller().Orchestrator().List(w.watchCtx, &opts)
		if err != nil {
			w.logger.Errorf("Error retrieving orchestrators: %v", err)
			return err
		}

		// When orchhub restarts, we initialize allocOrchID based on the information query from stateMgr
		for _, orch := range orchs {
			// On restart, we might get some old and some new orch configs in the List().
			// Skip the new ones(OrchID equals to 0) and wait for configHandler to be called for those
			if orch.Status.OrchID != 0 {
				if len(w.orchIDMgr.allocOrchID[orch.Status.OrchID]) == 0 {
					w.orchIDMgr.allocOrchID[orch.Status.OrchID] = orch.GetKey()
					w.orchIDMgr.key2Alloc[orch.GetKey()] = orch.Status.OrchID
				} else {
					existing := w.orchIDMgr.allocOrchID[orch.Status.OrchID]
					w.logger.Errorf("Got conflicting orch IDs for config %s and %s. ID %d already given to %s", orch.GetKey(), existing, orch.Status.OrchID, existing)
				}
			}
		}
		w.orchIDMgr.isInitialized = true
	}

	if config.Status.OrchID != 0 {
		if w.orchIDMgr.allocOrchID[config.Status.OrchID] == config.GetKey() {
			// Since config.Status.OrchID is already assigned, we don't need to do anything
			return nil
		}
		if len(w.orchIDMgr.allocOrchID[config.Status.OrchID]) == 0 {
			// Should have already been assigned, but id is free
			w.orchIDMgr.allocOrchID[config.Status.OrchID] = config.GetKey()
			return nil
		}
		// This should ideally never happen. Attempt to recover by assigning new id
		existing := w.orchIDMgr.allocOrchID[config.Status.OrchID]
		w.logger.Errorf("OrchID %d of orch %s is in use by %s, assigning another ID", config.Status.OrchID, config.GetKey(), existing)
	}
	// Check we haven't already given an ID
	if id, ok := w.orchIDMgr.key2Alloc[config.GetKey()]; ok {
		// Already assigned an ID
		w.orchIDMgr.allocOrchID[id] = config.GetKey()
		return nil
	}
	// Assign an ID
	var index int32
	for index = 1; index < maxOrchSupported; index++ {
		if len(w.orchIDMgr.allocOrchID[index]) == 0 {
			w.orchIDMgr.allocOrchID[index] = config.GetKey()
			w.orchIDMgr.key2Alloc[config.GetKey()] = index
			config.Status.OrchID = index
			break
		}
	}

	return nil
}

func (w *InstanceManager) freeOrchestratorID(config *orchestration.Orchestrator) {
	w.orchIDMgr.Lock()
	defer w.orchIDMgr.Unlock()

	if config.Status.OrchID != 0 &&
		w.orchIDMgr.allocOrchID[config.Status.OrchID] == config.GetKey() {
		w.orchIDMgr.allocOrchID[config.Status.OrchID] = ""
		delete(w.orchIDMgr.key2Alloc, config.GetKey())
	} else if id, ok := w.orchIDMgr.key2Alloc[config.GetKey()]; ok {
		w.orchIDMgr.allocOrchID[id] = ""
		delete(w.orchIDMgr.key2Alloc, config.GetKey())
	} else {
		w.logger.Warnf("Unexpected OrchID free request for orch %s ignored. OrchID: %v, w.orchIDMgr.allocOrchID[%v]: %v",
			config.GetKey(), config.Status.OrchID, config.Status.OrchID, w.orchIDMgr.allocOrchID[config.Status.OrchID])
	}
}

func (w *InstanceManager) createOrch(config *orchestration.Orchestrator) {
	switch config.Spec.Type {
	case orchestration.OrchestratorSpec_VCenter.String():
		// If vCenter, call launch VCHub
		vchubInst := vchub.LaunchVCHub(w.stateMgr, config, w.logger, w.vcHubOpts...)
		w.orchMapLock.Lock()
		defer w.orchMapLock.Unlock()
		w.orchestratorMap[config.GetKey()] = &orchestratorMapEntry{
			orch:   vchubInst,
			config: ref.DeepCopy(config).(*orchestration.Orchestrator),
		}
	}
}

func (w *InstanceManager) handleConfigEvent(evtType kvstore.WatchEventType, config *orchestration.Orchestrator) {
	config.ApplyStorageTransformer(context.Background(), true /*encrypt*/)
	w.logger.Infof("Handle Orchestrator config event. Event: %v, obj: %v", evtType, config)
	config.ApplyStorageTransformer(context.Background(), false /*decrypt*/)
	switch evtType {
	case kvstore.Created:
		err := w.assignOrchestratorID(config)
		if err != nil {
			w.logger.Errorf("Failed at assigning OrchID, %v", err)
			return
		}
		w.createOrch(config)
	case kvstore.Updated:
		w.orchMapLock.RLock()
		orchEntry, ok := w.orchestratorMap[config.GetKey()]
		w.orchMapLock.RUnlock()
		if !ok {
			w.createOrch(config)
			return
		}
		orchEntry.config = ref.DeepCopy(config).(*orchestration.Orchestrator)
		orchEntry.orch.UpdateConfig(config)
	case kvstore.Deleted:
		w.orchMapLock.Lock()
		defer w.orchMapLock.Unlock()
		w.logger.Infof("Config item deleted. %v", config.ObjectMeta)
		orchEntry, ok := w.orchestratorMap[config.GetKey()]
		if !ok {
			return
		}
		orchEntry.orch.Destroy(true)
		w.freeOrchestratorID(config)
		delete(w.orchestratorMap, config.GetKey())
	}
}

func (w *InstanceManager) periodicSync() {
	defer w.waitGrp.Done()

	ticker := time.NewTicker(5 * time.Minute)
	inProgress := false

	w.logger.Info("Starting periodic sync")
	for {
		select {
		case <-w.watchCtx.Done():
			w.logger.Info("Exiting periodic sync")
			return
		case <-ticker.C:
			if !inProgress {
				w.logger.Info("periodic sync running")
				inProgress = true
				w.orchMapLock.RLock()
				for _, v := range w.orchestratorMap {
					v.orch.Sync()
				}
				w.orchMapLock.RUnlock()
				inProgress = false
			}
		}
	}
}
