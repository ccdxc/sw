package impl

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/orchestration"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func makeOrchObj(name, uri string) *orchestration.Orchestrator {
	return &orchestration.Orchestrator{
		ObjectMeta: api.ObjectMeta{
			Name:            name,
			ResourceVersion: "1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "Orchestrator",
			APIVersion: "v1",
		},
		Spec: orchestration.OrchestratorSpec{
			URI: uri,
		},
	}
}

func TestOrchestratorPreCommit(t *testing.T) {
	hooks := &orchHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Orch-Hooks-Preommit-Test")),
	}

	_, result, err := hooks.validateOrchestrator(context.TODO(), nil, nil, "", apiintf.UpdateOper, false, nil)
	Assert(t, result == true && err != nil, "orch validator did not return error with invalid parameters")
	_, result, err = hooks.validateOrchestrator(context.TODO(), nil, nil, "key", apiintf.UpdateOper, false, "")
	Assert(t, result == true && err != nil, "orch validator did not return error with invalid parameters")

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")

	type testCase struct {
		op  apiintf.APIOperType
		obj *orchestration.Orchestrator
		err error
	}

	testCases := []testCase{
		{apiintf.CreateOper, makeOrchObj("orch1", "10.1.1.1"), nil},                                                     // First object no conflicts
		{apiintf.CreateOper, makeOrchObj("orch2", "11.1.1.1"), nil},                                                     // Second object no conflicts
		{apiintf.CreateOper, makeOrchObj("orch3", "11.1.1.1"), fmt.Errorf("URI is already used by Orchestrator orch2")}, // Third object with conflict
		{apiintf.UpdateOper, makeOrchObj("orch1", "11.1.1.1"), fmt.Errorf("URI is already used by Orchestrator orch2")}, // first object update with conflict
	}

	ctx := context.TODO()

	for i, tc := range testCases {
		key := tc.obj.MakeKey(string(apiclient.GroupOrchestration))
		// invoke the hook manually
		_, ok, err := hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key, tc.op, false, *tc.obj)
		if !ok && err == nil {
			t.Fatalf("hook failed but returned no error, test case: %d", i)
		}
		if tc.err == nil { // op expected to succeed
			AssertOk(t, err, fmt.Sprintf("hook returned unexpected error, testcase: %d", i))
			// execute the op if it was supposed to succeed
			switch tc.op {
			case apiintf.CreateOper:
				err := kv.Create(ctx, key, tc.obj)
				AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore, testcase: %d", i))
			case apiintf.UpdateOper:
				err := kv.Update(ctx, key, tc.obj)
				AssertOk(t, err, fmt.Sprintf("Error updating object in KVStore, testcase: %d", i))
			default:
				t.Fatalf("Unexpected op!")
			}
		} else {
			Assert(t, err != nil && (err.Error() == tc.err.Error()), fmt.Sprintf("hook did not return expected error. Testcase: %d, have: \"%v\", want: \"%v\"", i, err, tc.err))
		}
	}

}
