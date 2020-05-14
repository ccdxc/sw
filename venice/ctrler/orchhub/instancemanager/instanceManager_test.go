package instanceManager

import (
	"context"
	"fmt"
	"sync"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/orchestration"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type dummyOrch struct {
	sync.Mutex
	DestroyCalls      []bool
	UpdateConfigCalls []*orchestration.Orchestrator
	ReconnectCalls    []string
	DebugCalls        []string
}

func (d *dummyOrch) Destroy(delete bool) {
	d.Lock()
	defer d.Unlock()
	d.DestroyCalls = append(d.DestroyCalls, delete)
}

func (d *dummyOrch) UpdateConfig(config *orchestration.Orchestrator) {
	d.Lock()
	defer d.Unlock()
	d.UpdateConfigCalls = append(d.UpdateConfigCalls, config)
}

func (d *dummyOrch) Reset() {
	d.Lock()
	defer d.Unlock()
	d.DestroyCalls = []bool{}
	d.UpdateConfigCalls = []*orchestration.Orchestrator{}
	d.ReconnectCalls = []string{}
	d.DebugCalls = []string{}
}

func (d *dummyOrch) Debug(action string, params map[string]string) (interface{}, error) {
	d.DebugCalls = append(d.DebugCalls, action)
	return nil, nil
}

func (d *dummyOrch) Sync() {}

func (d *dummyOrch) Reconnect(kind string) {
	d.ReconnectCalls = append(d.ReconnectCalls, kind)
}

func newDummyOrch() *dummyOrch {
	return &dummyOrch{
		DestroyCalls:      []bool{},
		UpdateConfigCalls: []*orchestration.Orchestrator{},
		ReconnectCalls:    []string{},
	}
}

func TestHandleRestore(t *testing.T) {
	config := log.GetDefaultConfig("instMgr-onRestore")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	tcs := []struct {
		currConfig    []*orchestration.Orchestrator
		restoreConfig []*orchestration.Orchestrator
		verify        func(map[string]*dummyOrch)
	}{
		{
			// TC1: Config not changing
			currConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1"},
					},
				},
			},
			restoreConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1"},
					},
				},
			},
			verify: func(orchMap map[string]*dummyOrch) {
				// only should have delete(false)
				o, ok := orchMap["o1"]
				Assert(t, ok, "No entry for o1")
				AssertEquals(t, 0, len(o.UpdateConfigCalls), "Expected no update calls")
				AssertEquals(t, []bool{false}, o.DestroyCalls, "Calls were not equal")
			},
		},
		{
			// TC2: Config no changing but different orch name
			currConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1", "dc2"},
					},
				},
			},
			restoreConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o2",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1", "dc2"},
					},
				},
			},
			verify: func(orchMap map[string]*dummyOrch) {
				// only should have delete(false)
				o, ok := orchMap["o1"]
				Assert(t, ok, "No entry for o1")
				AssertEquals(t, 0, len(o.UpdateConfigCalls), "Expected no update calls")
				AssertEquals(t, []bool{false}, o.DestroyCalls, "Calls were not equal")
			},
		},
		{
			// TC3: 1 -> manageAll
			currConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1"},
					},
				},
			},
			restoreConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{utils.ManageAllDcs},
					},
				},
			},
			verify: func(orchMap map[string]*dummyOrch) {
				// only should have delete(false)
				o, ok := orchMap["o1"]
				Assert(t, ok, "No entry for o1")
				AssertEquals(t, 0, len(o.UpdateConfigCalls), "Expected no update calls")
				AssertEquals(t, []bool{false}, o.DestroyCalls, "Calls were not equal")
			},
		},
		{
			// TC4: Deletion
			currConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1"},
					},
				},
			},
			restoreConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc2"},
					},
				},
			},
			verify: func(orchMap map[string]*dummyOrch) {
				// only should have delete(false)
				o, ok := orchMap["o1"]
				Assert(t, ok, "No entry for o1")
				AssertEquals(t, 0, len(o.UpdateConfigCalls), "Expected no update call")
				AssertEquals(t, []bool{true}, o.DestroyCalls, "Calls were not equal")
			},
		},
		{
			// TC5: Addition shouldn't affect
			currConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1"},
					},
				},
			},
			restoreConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1", "dc2"},
					},
				},
			},
			verify: func(orchMap map[string]*dummyOrch) {
				// only should have delete(false)
				o, ok := orchMap["o1"]
				Assert(t, ok, "No entry for o1")
				AssertEquals(t, 0, len(o.UpdateConfigCalls), "Expected an update call")
				AssertEquals(t, []bool{false}, o.DestroyCalls, "Calls were not equal")
			},
		},
		{
			// TC6: Manage all -> 1
			currConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{utils.ManageAllDcs},
					},
				},
			},
			restoreConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1"},
					},
				},
			},
			verify: func(orchMap map[string]*dummyOrch) {
				// only should have delete(false)
				o, ok := orchMap["o1"]
				Assert(t, ok, "No entry for o1")
				AssertEquals(t, 1, len(o.UpdateConfigCalls), "Expected an update call")
				AssertEquals(t, []string{"dc1"}, o.UpdateConfigCalls[0].Spec.ManageNamespaces, "Expected an update call")
				AssertEquals(t, []bool{false}, o.DestroyCalls, "Calls were not equal")
			},
		},
		{
			// TC6: no matching config
			currConfig: []*orchestration.Orchestrator{
				&orchestration.Orchestrator{
					TypeMeta: api.TypeMeta{Kind: string(orchestration.KindOrchestrator)},
					ObjectMeta: api.ObjectMeta{
						Name: "o1",
					},
					Spec: orchestration.OrchestratorSpec{
						URI:              "1.1.1.1",
						ManageNamespaces: []string{"dc1"},
					},
				},
			},
			verify: func(orchMap map[string]*dummyOrch) {
				// only should have delete(false)
				o, ok := orchMap["o1"]
				Assert(t, ok, "No entry for o1")
				AssertEquals(t, []bool{true}, o.DestroyCalls, "Calls were not equal")
			},
		},
	}

	for i, tc := range tcs {
		logger.Infof("Running test case %d", i+1)
		sm, _, err := smmock.NewMockStateManager()
		AssertOk(t, err, "Failed to create state manager.")

		// Create initial state. Expect deletes to be called with the right values
		watchCtx, watchCancel := context.WithCancel(context.Background())

		instMgr := &InstanceManager{
			watchCtx:    watchCtx,
			watchCancel: watchCancel,
			stopFlag: syncFlag{
				flag: false,
			},
			logger:          logger,
			orchestratorMap: make(map[string]*orchestratorMapEntry),
			stateMgr:        sm,
		}
		orchMap := map[string]*dummyOrch{}
		// Create orch for tc.currConfig
		for _, orch := range tc.currConfig {
			o := newDummyOrch()
			orchMap[orch.GetName()] = o
			instMgr.orchestratorMap[orch.GetKey()] = &orchestratorMapEntry{
				orch:   o,
				config: orch,
			}
		}
		// Create new config in sm
		for _, orch := range tc.restoreConfig {
			err := sm.Controller().Orchestrator().Create(orch)
			AssertOk(t, err, "failed to create config")
		}
		instMgr.handleRestore()
		tc.verify(orchMap)
	}

}

func TestSnapshot(t *testing.T) {
	config := log.GetDefaultConfig("instMgr-snapshot")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	// Verify adding and removing allocators works
	sm, im, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager.")

	instMgr, err := NewInstanceManager(sm, logger, im.InstanceManagerCh, nil, nil)
	AssertOk(t, err, "Failed to create instance manager")

	snapshot := cluster.SnapshotRestore{
		TypeMeta: api.TypeMeta{Kind: "SnapshotRestore"},
		ObjectMeta: api.ObjectMeta{
			Name: "OrchSnapshot",
		},
		Spec: cluster.SnapshotRestoreSpec{
			SnapshotPath: "test",
		},
		Status: cluster.SnapshotRestoreStatus{
			Status: cluster.SnapshotRestoreStatus_Unknown.String(),
		},
	}
	err = sm.Controller().SnapshotRestore().Create(&snapshot)
	AssertOk(t, err, "failed to create snapshot")

	instMgr.Start()

	AssertEquals(t, false, instMgr.restoreActive, "restore isn't active")

	snapshot.Status.Status = cluster.SnapshotRestoreStatus_Active.String()
	err = sm.Controller().SnapshotRestore().Update(&snapshot)
	AssertOk(t, err, "failed to create snapshot")
	AssertEventually(t, func() (bool, interface{}) {
		if !instMgr.restoreActive {
			return false, nil
		}
		return true, nil
	}, "restore active had wrong value")

	// Failure should move to not active
	snapshot.Status.Status = cluster.SnapshotRestoreStatus_Failed.String()
	err = sm.Controller().SnapshotRestore().Update(&snapshot)
	AssertOk(t, err, "failed to create snapshot")
	AssertEventually(t, func() (bool, interface{}) {
		if instMgr.restoreActive {
			return false, nil
		}
		return true, nil
	}, "restore active had wrong value")

	// Duplicate event should have no affect
	snapshot.Status.Status = cluster.SnapshotRestoreStatus_Failed.String()
	err = sm.Controller().SnapshotRestore().Update(&snapshot)
	AssertOk(t, err, "failed to create snapshot")
	AssertEventually(t, func() (bool, interface{}) {
		if instMgr.restoreActive {
			return false, nil
		}
		return true, nil
	}, "restore active had wrong value")

	// Back to active
	snapshot.Status.Status = cluster.SnapshotRestoreStatus_Active.String()
	err = sm.Controller().SnapshotRestore().Update(&snapshot)
	AssertOk(t, err, "failed to create snapshot")
	AssertEventually(t, func() (bool, interface{}) {
		if !instMgr.restoreActive {
			return false, nil
		}
		return true, nil
	}, "restore active had wrong value")

	// Move to successful
	snapshot.Status.Status = cluster.SnapshotRestoreStatus_Completed.String()
	err = sm.Controller().SnapshotRestore().Update(&snapshot)
	AssertOk(t, err, "failed to create snapshot")
	AssertEventually(t, func() (bool, interface{}) {
		if instMgr.restoreActive {
			return false, nil
		}
		return true, nil
	}, "restore active had wrong value")

}

func TestIDAllocator(t *testing.T) {
	eventRecorder := mockevtsrecorder.NewRecorder("instanceMgr",
		log.GetNewLogger(log.GetDefaultConfig("instanceMgr")))
	_ = recorder.Override(eventRecorder)

	config := log.GetDefaultConfig("instMgr-testIDAllocator")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	// Verify adding and removing allocators works
	sm, im, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager.")

	orchConfig := smmock.GetOrchestratorConfig("o1", "user", "pass")
	orchConfig.Status.OrchID = 1
	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "Failed to create config")

	orchConfig = smmock.GetOrchestratorConfig("o2", "user", "pass")
	orchConfig.Status.OrchID = 2
	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "Failed to create config")

	orchConfig = smmock.GetOrchestratorConfig("o3", "user", "pass")
	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "Failed to create config")

	instMgr, err := NewInstanceManager(sm, logger, im.InstanceManagerCh, nil, nil)
	AssertOk(t, err, "Failed to create instance manager")

	instMgr.Start()

	// Verify allocators assigns existing IDs
	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		for i := 1; i < 4; i++ {
			meta := api.ObjectMeta{
				Name:      fmt.Sprintf("o%d", i),
				Namespace: "default",
			}
			if meta.GetKey() != instMgr.orchIDMgr.allocOrchID[i] {
				return false, fmt.Sprintf("Expected %s but found %s", meta.GetKey(), instMgr.orchIDMgr.allocOrchID[i])
			}
		}
		return true, nil
	}, "allocation incorrect")

	// Create orch without ID
	orchConfig = smmock.GetOrchestratorConfig("o4", "user", "pass")
	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "Failed to create config")

	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		for i := 1; i < 5; i++ {
			meta := api.ObjectMeta{
				Name:      fmt.Sprintf("o%d", i),
				Namespace: "default",
			}
			if meta.GetKey() != instMgr.orchIDMgr.allocOrchID[i] {
				return false, fmt.Sprintf("Expected %s but found %s", meta.GetKey(), instMgr.orchIDMgr.allocOrchID[i])
			}
		}
		return true, nil
	}, "allocation incorrect")

	// Redundant create without orch ID should be given same ID
	orchConfig.Status.OrchID = 0
	instMgr.assignOrchestratorID(orchConfig)

	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		for i := 1; i < 5; i++ {
			meta := api.ObjectMeta{
				Name:      fmt.Sprintf("o%d", i),
				Namespace: "default",
			}
			if meta.GetKey() != instMgr.orchIDMgr.allocOrchID[i] {
				return false, fmt.Sprintf("Expected %s but found %s", meta.GetKey(), instMgr.orchIDMgr.allocOrchID[i])
			}
		}
		return true, nil
	}, "allocation incorrect")

	// Create with existing ID should allocate the given ID
	orchConfig = smmock.GetOrchestratorConfig("o10", "user", "pass")
	orchConfig.Status.OrchID = 10
	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "Failed to create config")

	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		return orchConfig.GetKey() == instMgr.orchIDMgr.allocOrchID[10], fmt.Sprintf("Expected %s but found %s", orchConfig.GetKey(), instMgr.orchIDMgr.allocOrchID[10])
	}, "allocation incorrect")

	// Redundant create with status should no-op.
	instMgr.assignOrchestratorID(orchConfig)
	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		for i := 1; i < 5; i++ {
			meta := api.ObjectMeta{
				Name:      fmt.Sprintf("o%d", i),
				Namespace: "default",
			}
			if meta.GetKey() != instMgr.orchIDMgr.allocOrchID[i] {
				return false, fmt.Sprintf("Expected %s but found %s", meta.GetKey(), instMgr.orchIDMgr.allocOrchID[i])
			}
		}
		return orchConfig.GetKey() == instMgr.orchIDMgr.allocOrchID[10], fmt.Sprintf("Expected %s but found %s", orchConfig.GetKey(), instMgr.orchIDMgr.allocOrchID[10])
	}, "allocation incorrect")

	// Create with status ID that is in use should be assigned a new ID
	orchConfig = smmock.GetOrchestratorConfig("o5", "user", "pass")
	orchConfig.Status.OrchID = 10
	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "Failed to create config")

	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		for i := 1; i < 6; i++ {
			meta := api.ObjectMeta{
				Name:      fmt.Sprintf("o%d", i),
				Namespace: "default",
			}
			if meta.GetKey() != instMgr.orchIDMgr.allocOrchID[i] {
				return false, fmt.Sprintf("Expected %s but found %s", meta.GetKey(), instMgr.orchIDMgr.allocOrchID[i])
			}
		}
		return true, nil
	}, "allocation incorrect")

	// Free an ID
	err = sm.Controller().Orchestrator().Delete(orchConfig)
	AssertOk(t, err, "Failed to delete config")

	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		return len(instMgr.orchIDMgr.allocOrchID[5]) == 0, fmt.Sprintf("Expected %s but found %s", "", instMgr.orchIDMgr.allocOrchID[5])
	}, "allocation incorrect")

	// Duplicate free should be ok
	instMgr.freeOrchestratorID(orchConfig)
	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		return len(instMgr.orchIDMgr.allocOrchID[5]) == 0, fmt.Sprintf("Expected %s but found %s", "", instMgr.orchIDMgr.allocOrchID[5])
	}, "allocation incorrect")

	// Free through name isntead of orchID
	orchConfig = smmock.GetOrchestratorConfig("o1", "user", "pass")
	instMgr.freeOrchestratorID(orchConfig)
	AssertEventually(t, func() (bool, interface{}) {
		instMgr.orchIDMgr.Lock()
		defer instMgr.orchIDMgr.Unlock()
		return len(instMgr.orchIDMgr.allocOrchID[1]) == 0, fmt.Sprintf("Expected %s but found %s", "", instMgr.orchIDMgr.allocOrchID[1])
	}, "allocation incorrect")

	// Update call to increase test coverage
	orchConfig = smmock.GetOrchestratorConfig("o4", "user", "pass1")
	err = sm.Controller().Orchestrator().Update(orchConfig)
	AssertOk(t, err, "failed to update")

	instMgr.Stop()
	Assert(t, instMgr.stopped(), "failed to stop instMgr")
}

func TestDebug(t *testing.T) {
	config := log.GetDefaultConfig("instMgr-debug")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager.")

	// Create initial state. Expect deletes to be called with the right values
	watchCtx, watchCancel := context.WithCancel(context.Background())

	instMgr := &InstanceManager{
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
		logger:          logger,
		orchestratorMap: make(map[string]*orchestratorMapEntry),
		stateMgr:        sm,
	}
	// Create orch for tc.currConfig
	o := newDummyOrch()
	instMgr.orchestratorMap["o1"] = &orchestratorMapEntry{
		orch: o,
	}
	_, err = instMgr.Debug("logLevel", map[string]string{"level": "Info"})
	AssertOk(t, err, "Failed to set log level through debug")

	_, err = instMgr.Debug("action", map[string]string{"key": "o1"})
	AssertOk(t, err, "Failed to call debug on o1")

	AssertEquals(t, 1, len(o.DebugCalls), "Expected debug call")
	AssertEquals(t, "action", o.DebugCalls[0], "Expected debug call")
}
