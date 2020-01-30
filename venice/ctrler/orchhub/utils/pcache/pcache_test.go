package pcache

import (
	"context"
	"fmt"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
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
	logConfig.LogToStdout = true
	logConfig.Filter = log.AllowAllFilter
	logger := log.SetConfig(logConfig)
	// Set should check validator before writing to statemgr
	// Call to statemgr should be update/create
	stateMgr, err := newStateManager()
	AssertOk(t, err, "failed to create statemgr")
	pCache := NewPCache(stateMgr, logger)
	pCache.retryInterval = 1 * time.Second

	ctx, cancel := context.WithCancel(context.Background())
	wg := &sync.WaitGroup{}
	wg.Add(1)
	go pCache.Run(ctx, wg)
	defer func() {
		cancel()
		wg.Wait()
	}()

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

	// no validators so it should have written to statemgr
	_, err = stateMgr.Controller().Workload().Find(expMeta)
	AssertOk(t, err, "Failed to find workload in statemgr")

	testKey := fmt.Sprintf("%s%s", globals.SystemLabelPrefix, "test")
	validNoPushKey := fmt.Sprintf("%s%s", globals.SystemLabelPrefix, "valid_no_push")

	// Set validator
	pCache.SetValidator("Workload", func(in interface{}) (bool, bool) {
		w := in.(*workload.Workload)
		if w.Labels != nil && len(w.Labels[testKey]) != 0 {
			return true, true
		}
		if w.Labels != nil && len(w.Labels[validNoPushKey]) != 0 {
			return true, false
		}
		return false, false
	})

	// Update should sit in cache
	key1 := fmt.Sprintf("%s%s", globals.SystemLabelPrefix, "key1")
	key2 := fmt.Sprintf("%s%s", globals.SystemLabelPrefix, "key2")
	expWorkload = ref.DeepCopy(expWorkload).(*workload.Workload)
	userKey := "userKey"
	expWorkload.Labels = map[string]string{
		key1: "val1",
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
	AssertEquals(t, "val1", entryWorkload.Labels[key1], "Workload not in pcache")

	// Test isValid
	expWorkload = ref.DeepCopy(expWorkload).(*workload.Workload)
	expWorkload.Labels[validNoPushKey] = "test"
	err = pCache.Set("Workload", expWorkload)
	AssertOk(t, err, "Failed to write workload")
	// Should be in cache
	entry = pCache.kinds["Workload"].entries[expMeta.GetKey()]
	Assert(t, entry != nil, "Workload not in pcache")
	Assert(t, pCache.IsValid("Workload", expMeta), "expected object to be valid")

	// Update with correct value
	// Modifying object that lives in the cache
	expWorkload.Labels[testKey] = "test"
	pCache.RevalidateKind("Workload")

	// Should no longer be in cache
	entry = pCache.kinds["Workload"].entries[expMeta.GetKey()]
	Assert(t, entry == nil, "Workload still in pcache")

	// Should be in stateMgr
	stateMgrEntry, err := stateMgr.Controller().Workload().Find(expMeta)
	AssertOk(t, err, "Failed to find workload in statemgr")
	AssertEquals(t, "val1", stateMgrEntry.Workload.Labels[key1], "stateMgr did not have correct version of workload")

	// Update statemgr directly
	expWorkload = ref.DeepCopy(expWorkload).(*workload.Workload)
	expWorkload.Labels[key1] = "val2"
	err = stateMgr.Controller().Workload().Update(expWorkload)
	AssertOk(t, err, "Failed to update statemgr")

	// Get should return value in stateMgr
	expWorkload = pCache.GetWorkload(expMeta)
	Assert(t, expWorkload != nil, "Workload not in statemgr")
	AssertEquals(t, "val2", expWorkload.Labels[key1], "Workload not in statemgr")

	// Update from statemgr should merge labels
	expWorkload = ref.DeepCopy(expWorkload).(*workload.Workload)
	expWorkload.Labels[userKey] = "val1"
	err = stateMgr.Controller().Workload().Update(expWorkload)
	AssertOk(t, err, "Failed to update statemgr")

	expWorkload = pCache.GetWorkload(expMeta)
	Assert(t, expWorkload != nil, "Workload not in statemgr")

	expWorkload = ref.DeepCopy(expWorkload).(*workload.Workload)
	expWorkload.Labels[userKey] = "oldVal"
	expWorkload.Labels[key2] = "val1"
	err = pCache.Set("Workload", expWorkload)

	// Statemgr should have userKey: val1, and the system labels key1 and key2
	stateMgrEntry, err = stateMgr.Controller().Workload().Find(expMeta)
	AssertOk(t, err, "Failed to find workload in statemgr")
	AssertEquals(t, "val2", stateMgrEntry.Workload.Labels[key1], "stateMgr did not have correct version of workload")
	AssertEquals(t, "val1", stateMgrEntry.Workload.Labels[key2], "stateMgr did not have correct version of workload")
	AssertEquals(t, "val1", stateMgrEntry.Workload.Labels[userKey], "stateMgr did not have correct version of workload")

	// Delete
	err = pCache.Delete("Workload", expWorkload)
	AssertOk(t, err, "Failed to delete from statemgr")
	_, err = stateMgr.Controller().Workload().Find(expMeta)
	Assert(t, err != nil, "Item still in stateMgr")
}
