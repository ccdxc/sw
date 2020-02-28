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
func NewInstanceManager(stateMgr *statemgr.Statemgr, vcenterList string, logger log.Logger, instanceManagerCh chan *kvstore.WatchEvent) (*InstanceManager, error) {
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

	return instance, nil
}

// Start starts instance manager
func (w *InstanceManager) Start() {
	go w.watchOrchestratorConfig()
	go w.periodicSync()
}

func (w *InstanceManager) watchOrchestratorConfig() {
	w.waitGrp.Add(1)
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

func (w *InstanceManager) createOrch(config *orchestration.Orchestrator) {
	switch config.Spec.Type {
	case orchestration.OrchestratorSpec_VCenter.String():
		// If vCenter, call launch VCHub
		vchubInst := vchub.LaunchVCHub(w.stateMgr, config, w.logger)
		w.orchestratorMap[config.GetKey()] = vchubInst
	}
}

func (w *InstanceManager) handleConfigEvent(evtType kvstore.WatchEventType, config *orchestration.Orchestrator) {
	w.logger.Infof("Handle Orchestrator config event. %v", config)
	switch evtType {
	case kvstore.Created:
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
		delete(w.orchestratorMap, config.GetKey())
	}
}

func (w *InstanceManager) periodicSync() {
	w.waitGrp.Add(1)
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
