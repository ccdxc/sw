package instanceManager

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/globals"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
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
}

type syncFlag struct {
	sync.RWMutex
	flag bool
}

const (
	maxOrchSupported = 64
)

type orchIDMgr struct {
	sync.Mutex
	allocOrchID   [maxOrchSupported]bool
	isInitialized bool
}

// InstanceManager is the struct which watches for update to vc config objects
type InstanceManager struct {
	waitGrp           sync.WaitGroup
	watchCtx          context.Context
	watchCancel       context.CancelFunc
	stopFlag          syncFlag
	instanceManagerCh chan *kvstore.WatchEvent
	logger            log.Logger
	orchestratorMap   map[string]Orchestrator
	stateMgr          *statemgr.Statemgr
	vcenterList       string
	orchIDMgr         orchIDMgr
	vcHubOpts         []vchub.Option
}

// Stop stops the watcher
func (w *InstanceManager) Stop() {
	w.stop()
	w.watchCancel()
	// TODO: stop orchestrators.
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

// NewInstanceManager creates a new watcher
func NewInstanceManager(stateMgr *statemgr.Statemgr, vcenterList string, logger log.Logger, instanceManagerCh chan *kvstore.WatchEvent, vcHubOpts []vchub.Option) (*InstanceManager, error) {
	watchCtx, watchCancel := context.WithCancel(context.Background())

	instance := &InstanceManager{
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
		vcenterList:       vcenterList,
		logger:            logger.WithContext("submodule", "Inst Mgr"),
		orchestratorMap:   make(map[string]Orchestrator),
		stateMgr:          stateMgr,
		instanceManagerCh: instanceManagerCh,
		vcHubOpts:         vcHubOpts,
	}

	diagSvc := diagsvc.GetDiagnosticsService(globals.OrchHub, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, logger)
	handlerFn := func(action string, params map[string]string) (interface{}, error) {
		key, ok := params["key"]
		if !ok {
			return nil, fmt.Errorf("key is a required param")
		}
		orch, ok := instance.orchestratorMap[key]
		if !ok {
			// Try with default key format
			meta := api.ObjectMeta{
				Name: key,
			}
			key = meta.GetKey()
		}
		orch, ok = instance.orchestratorMap[key]

		if !ok {
			options := []string{}
			for k := range instance.orchestratorMap {
				options = append(options, k)
			}
			return nil, fmt.Errorf("No instance with the given key %s was found. Known keys: %v", key, options)
		}

		return orch.Debug(action, params)
	}
	diagSvc.RegisterCustomAction(vchub.DebugUseg, handlerFn)
	diagSvc.RegisterCustomAction(vchub.DebugCache, handlerFn)
	diagSvc.RegisterCustomAction(vchub.DebugState, handlerFn)
	diagSvc.RegisterCustomAction(vchub.DebugSync, handlerFn)
	diagSvc.RegisterCustomAction(vchub.DebugVlanSpace, handlerFn)

	return instance, nil
}

// Start starts instance manager
func (w *InstanceManager) Start() {
	w.waitGrp.Add(1)
	go w.watchOrchestratorConfig()
	// w.waitGrp.Add(1)
	// go w.periodicSync()
}

func (w *InstanceManager) watchOrchestratorConfig() {
	defer w.waitGrp.Done()

	for {
		select {
		case <-w.watchCtx.Done():
			w.logger.Info("Exiting watch for orchestration configuration")
			return
		case evt, ok := <-w.instanceManagerCh:
			if ok {
				orchConfig := evt.Object.(*orchestration.Orchestrator)
				w.handleConfigEvent(evt.Type, orchConfig)
			}
		}
	}
}

func (w *InstanceManager) assignOrchestratorID(config *orchestration.Orchestrator) error {
	w.orchIDMgr.Lock()
	defer w.orchIDMgr.Unlock()

	// Handle orchhub restarts
	if w.orchIDMgr.isInitialized == false {
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
				if w.orchIDMgr.allocOrchID[orch.Status.OrchID] == false {
					w.orchIDMgr.allocOrchID[orch.Status.OrchID] = true
				} else {
					w.logger.Errorf("Stale orchestrator config object, ignored")
				}
			}
		}
		w.orchIDMgr.isInitialized = true
	}

	if config.Status.OrchID == 0 {
		var index int32
		for index = 1; index < maxOrchSupported; index++ {
			if w.orchIDMgr.allocOrchID[index] == false {
				w.orchIDMgr.allocOrchID[index] = true
				config.Status.OrchID = index
				break
			}
		}
	} else {
		// For reconnect situation, existing objects are fetched, and are then processed as create events
		if w.orchIDMgr.allocOrchID[config.Status.OrchID] == true {
			// Since config.Status.OrchID is already assigned, we don't need to do anything
			return nil
		}

		// w.orchIDMgr.allocOrchID doesn't reflect the current allocation, we need to update
		w.orchIDMgr.allocOrchID[config.Status.OrchID] = true
		w.logger.Errorf("orchIDMgr.allocOrchID[%v] is out of sync", config.Status.OrchID)
		return nil
	}

	return nil
}

func (w *InstanceManager) freeOrchestratorID(config *orchestration.Orchestrator) {
	w.orchIDMgr.Lock()
	defer w.orchIDMgr.Unlock()

	if config.Status.OrchID != 0 &&
		w.orchIDMgr.allocOrchID[config.Status.OrchID] == true {
		w.orchIDMgr.allocOrchID[config.Status.OrchID] = false
	} else {
		w.logger.Warnf("Unexpected OrchID free request ignored. OrchID: %v, w.orchIDMgr.allocOrchID[%v]: %v",
			config.Status.OrchID, config.Status.OrchID, w.orchIDMgr.allocOrchID[config.Status.OrchID])
	}
}

func (w *InstanceManager) createOrch(config *orchestration.Orchestrator) {
	switch config.Spec.Type {
	case orchestration.OrchestratorSpec_VCenter.String():
		// If vCenter, call launch VCHub
		vchubInst := vchub.LaunchVCHub(w.stateMgr, config, w.logger, w.vcHubOpts...)
		w.orchestratorMap[config.GetKey()] = vchubInst
	}
}

func (w *InstanceManager) handleConfigEvent(evtType kvstore.WatchEventType, config *orchestration.Orchestrator) {
	w.logger.Infof("Handle Orchestrator config event. %v", config)
	switch evtType {
	case kvstore.Created:
		err := w.assignOrchestratorID(config)
		if err != nil {
			w.logger.Errorf("Failed at assigning OrchID, %v", err)
			return
		}
		w.createOrch(config)
	case kvstore.Updated:
		orchInst, ok := w.orchestratorMap[config.GetKey()]
		if !ok {
			w.createOrch(config)
			return
		}
		orchInst.UpdateConfig(config)
	case kvstore.Deleted:
		w.logger.Infof("Config item deleted. %v", config)
		orchInst, ok := w.orchestratorMap[config.GetKey()]
		if !ok {
			return
		}
		orchInst.Destroy(true)
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
				for _, v := range w.orchestratorMap {
					v.Sync()
				}
				inProgress = false
			}
		}
	}
}
