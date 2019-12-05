package pcache

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

var (
	logConfig = log.GetDefaultConfig("pcache_test")
	logger    = log.SetConfig(logConfig)
)

func newStateManager() (*statemgr.Statemgr, error) {
	tsdb.Init(context.Background(), &tsdb.Opts{})
	stateMgr, err := statemgr.NewStatemgr(globals.APIServer, nil, logger, nil)
	if err != nil {
		return nil, err
	}

	return stateMgr, nil
}

func TestPcache(t *testing.T) {
	// Set should check validator before writing to statemgr
	// Call to statemgr should be update/create
	stateMgr, err := newStateManager()
	AssertOk(t, err, "failed to create statemgr")
	pCache := NewPCache(stateMgr, logger)
	go pCache.Run()
	defer pCache.Stop()

	expMeta := &api.ObjectMeta{
		Name:      "127.0.0.1:8990-virtualmachine-41",
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	expWorkload := &workload.Workload{
		TypeMeta: api.TypeMeta{
			Kind:       "Workload",
			APIVersion: "v1",
		},
		ObjectMeta: *expMeta,
	}
	err = pCache.Set("Workload", expWorkload)
	AssertOk(t, err, "Failed to write workload")

	// no validators so it should have written to cache
	_, err = stateMgr.Controller().Workload().Find(expMeta)
	AssertOk(t, err, "Failed to find workload in statemgr")

	// Set validator
	pCache.SetValidator("Workload", func(in interface{}) bool {
		w := in.(*workload.Workload)
		if w.Labels != nil && len(w.Labels["test"]) != 0 {
			return true
		}
		return false
	})

	// Update should sit in cache
	expWorkload.Labels = map[string]string{
		"key1": "val1",
	}
	err = pCache.Set("Workload", expWorkload)
	AssertOk(t, err, "Failed to write workload")
	entry := pCache.kinds["Workload"].entries[expMeta.GetKey()]
	Assert(t, entry != nil, "Workload not in pcache")

	// Get should return value in cache
	entry = pCache.Get("Workload", expMeta)
	Assert(t, entry != nil, "Workload not in pcache")
	entryWorkload := pCache.GetWorkload(expMeta)
	AssertEquals(t, entryWorkload, entry, "Get from pcache returned different values")
	AssertEquals(t, "val1", entryWorkload.Labels["key1"], "Workload not in pcache")

	// Update with correct value, pcache should call statemgr update
	expWorkload.Labels["test"] = "test"

	// Allow pcache retry goroutine enough time to push the completed object to StateManager
	time.Sleep(3 * time.Second)

	// Should no longer be in cache
	entry = pCache.kinds["Workload"].entries[expMeta.GetKey()]
	Assert(t, entry == nil, "Workload still in pcache")

	// Should be in stateMgr
	stateMgrEntry, err := stateMgr.Controller().Workload().Find(expMeta)
	AssertOk(t, err, "Failed to find workload in statemgr")
	fmt.Printf("----\n %v\n", stateMgrEntry.Workload)
	AssertEquals(t, "test", stateMgrEntry.Workload.Labels["test"], "stateMgr did not have correct version of workload")

	// Update statemgr directly
	expWorkload.Labels["key1"] = "val2"
	err = stateMgr.Controller().Workload().Update(expWorkload)
	AssertOk(t, err, "Failed to update statemgr")

	// Get should return value in stateMgr
	entry = pCache.GetWorkload(expMeta)
	Assert(t, entry != nil, "Workload not in statemgr")
	AssertEquals(t, "val2", entryWorkload.Labels["key1"], "Workload not in statemgr")

	// Delete
	err = pCache.Delete("Workload", expWorkload)
	AssertOk(t, err, "Failed to delete from statemgr")
	_, err = stateMgr.Controller().Workload().Find(expMeta)
	Assert(t, err != nil, "Item still in stateMgr")
}
