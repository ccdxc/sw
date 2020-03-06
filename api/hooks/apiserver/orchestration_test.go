package impl

import (
	"context"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	"net"
	"sync"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/kvstore/memkv"
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

func TestOrchCheckHook(t *testing.T) {
	precommitFn := createOrchCheckHook("Workload")
	work := &workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
			Labels: map[string]string{
				"userLabel": "v1",
			},
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{},
		},
	}
	workWithLabel := &workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-2",
			Labels: map[string]string{
				"userLabel":       "v1",
				utils.OrchNameKey: "test",
			},
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{},
		},
	}

	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}

	// oper, fromApigw, isAdmin, object, commit
	tcs := []struct {
		oper         apiintf.APIOperType
		fromAPIGw    bool
		isAdmin      bool
		shouldCommit bool
		err          error
		obj          *workload.Workload
	}{
		{ // TC0 - Create workload
			oper:         apiintf.CreateOper,
			fromAPIGw:    true,
			isAdmin:      false,
			shouldCommit: true,
			obj:          work,
			err:          nil,
		},
		{ // TC1 - Update workload w/o O-hub label should pass
			oper:         apiintf.UpdateOper,
			fromAPIGw:    true,
			isAdmin:      false,
			shouldCommit: true,
			obj:          work,
			err:          nil,
		},
		{ // TC2 - Delete workload w/o O-hub label should pass
			oper:         apiintf.DeleteOper,
			fromAPIGw:    true,
			isAdmin:      false,
			shouldCommit: false,
			obj:          work,
			err:          nil,
		},
		{ // TC3 - Create workload with O-hub label
			oper:         apiintf.CreateOper,
			fromAPIGw:    true,
			isAdmin:      false,
			shouldCommit: true,
			obj:          workWithLabel,
			err:          nil,
		},
		{ // TC4 - Update workload w O-hub label should fail
			oper:         apiintf.UpdateOper,
			fromAPIGw:    true,
			isAdmin:      false,
			shouldCommit: false,
			obj:          workWithLabel,
			err:          ErrOrchManaged,
		},
		{ // TC5 - Update workload w O-hub label as admin should fail
			oper:         apiintf.UpdateOper,
			fromAPIGw:    true,
			isAdmin:      true,
			shouldCommit: false,
			obj:          workWithLabel,
			err:          ErrOrchManaged,
		},
		{ // TC6 - Delete workload w O-hub label should fail
			oper:         apiintf.DeleteOper,
			fromAPIGw:    true,
			isAdmin:      false,
			shouldCommit: false,
			obj:          workWithLabel,
			err:          ErrOrchManaged,
		},
		{ // TC7 - Delete workload w O-hub label as admin should pass
			oper:         apiintf.DeleteOper,
			fromAPIGw:    true,
			isAdmin:      true,
			shouldCommit: true,
			obj:          workWithLabel,
			err:          nil,
		},
	}

	kvs, err := store.New(storecfg)
	AssertOk(t, err, "Failed to create kv store")

	for i, tc := range tcs {
		// create incoming ctx
		ctx := context.Background()
		if tc.fromAPIGw {
			addr := &net.IPAddr{
				IP: net.ParseIP("1.2.3.4"),
			}
			privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
			if err != nil {
				t.Fatalf("error generating key: %v", err)
			}
			cert, err := certs.SelfSign(globals.APIGw, privateKey, certs.WithValidityDays(1))
			if err != nil {
				t.Fatalf("error generating certificate: %v", err)
			}
			ctx = ctxutils.MakeMockContext(addr, cert)
		}
		user := &auth.User{
			TypeMeta: api.TypeMeta{Kind: "User"},
			ObjectMeta: api.ObjectMeta{
				Tenant: globals.DefaultTenant,
				Name:   "testUser",
			},
			Spec: auth.UserSpec{
				Fullname: "Test User",
				Password: "password",
				Email:    "testuser@pensandio.io",
				Type:     auth.UserSpec_Local.String(),
			},
		}
		ctx, err := authzgrpcctx.NewIncomingContextWithUserPerms(ctx, user, tc.isAdmin, nil)
		AssertOk(t, err, "Failed to create context")

		key := tc.obj.MakeKey("Workload")

		switch tc.oper {
		case apiintf.CreateOper:
			if tc.shouldCommit {
				err = kvs.Create(context.Background(), key, tc.obj)
				AssertOk(t, err, "TC %d, Error creating object in KVStore", i)
			}
		case apiintf.UpdateOper:
			_, commitKV, err := precommitFn(ctx, kvs, kvs.NewTxn(), key, tc.oper, false, tc.obj)
			Assert(t, commitKV, "TC %d, commitKV flag should not be false", i)
			AssertEquals(t, tc.err, err, "TC %d, Errors did not match", i)
			if err == nil && tc.shouldCommit {
				err = kvs.Update(ctx, key, tc.obj)
				AssertOk(t, err, "TC %d, Failed to update object in KVStore", i)
			}
		case apiintf.DeleteOper:
			_, commitKV, err := precommitFn(ctx, kvs, kvs.NewTxn(), key, tc.oper, false, tc.obj)
			Assert(t, commitKV, "TC %d, commitKV flag should not be false", i)
			AssertEquals(t, tc.err, err, "TC %d, Errors did not match", i)
			if err == nil && tc.shouldCommit {
				err = kvs.Delete(ctx, key, tc.obj)
				AssertOk(t, err, "TC %d, Failed to update object in KVStore", i)
			}
		}
	}
}

func TestOrchestratorPreCommit(t *testing.T) {
	hooks := &orchHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Orch-Hooks-Precommit-Test")),
	}

	_, result, err := hooks.validateOrchestrator(context.TODO(), nil, nil, "", apiintf.UpdateOper, false, nil)
	Assert(t, result == true && err != nil, "orch validator did not return error with invalid parameters")
	_, result, err = hooks.validateOrchestrator(context.TODO(), nil, nil, "key", apiintf.UpdateOper, false, "")
	Assert(t, result == true && err != nil, "orch validator did not return error with invalid parameters")

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")
	defer memkv.DeleteClusters()

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
	key := make([]string, len(testCases))
	for i, tc := range testCases {
		key[i] = tc.obj.MakeKey(string(apiclient.GroupOrchestration))
		// invoke the hook manually
		_, ok, err := hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key[i], tc.op, false, *tc.obj)
		if !ok && err == nil {
			t.Fatalf("hook failed but returned no error, test case: %d", i)
		}

		if tc.err == nil { // op expected to succeed
			AssertOk(t, err, fmt.Sprintf("hook returned unexpected error, testcase: %d", i))
			// execute the op if it was supposed to succeed
			tcObjIntf, _, _ := hooks.getOrchestratorID(ctx, kv, kv.NewTxn(), key[i], tc.op, false, *tc.obj)
			*tc.obj = tcObjIntf.(orchestration.Orchestrator)
			switch tc.op {
			case apiintf.CreateOper:
				err := kv.Create(ctx, key[i], tc.obj)
				AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore, testcase: %d", i))
			case apiintf.UpdateOper:
				err := kv.Update(ctx, key[i], tc.obj)
				AssertOk(t, err, fmt.Sprintf("Error updating object in KVStore, testcase: %d", i))
			default:
				t.Fatalf("Unexpected op!")
			}
		} else {
			Assert(t, err != nil && (err.Error() == tc.err.Error()), fmt.Sprintf("hook did not return expected error. Testcase: %d, have: \"%v\", want: \"%v\"", i, err, tc.err))
		}
	}

	// Validate Orch ID
	var orchs orchestration.OrchestratorList
	kindKey := orchs.MakeKey(string(apiclient.GroupOrchestration))
	err = kv.List(ctx, kindKey, &orchs)
	if err != nil {
		t.Fatalf("Error retrieving orchestrators: %v", err)
	}

	// Since we only create orch1 and orch2, the orchs should only contain two items
	AssertEquals(t, hooks.numAllocOrchID, int32(len(orchs.Items)), "Incorrect number of items in OrchestratorList")

	for _, orch := range orchs.Items {
		switch orch.Name {
		case "orch1":
			AssertEquals(t, int32(0), orch.Status.OrchID, "Incorrect orch ID for orch1")
		case "orch2":
			AssertEquals(t, int32(1), orch.Status.OrchID, "Incorrect orch ID for orch2")
			kv.Delete(ctx, key[1], nil)
		default:
		}
	}
}

func TestOrchestratorConcurrentOperations(t *testing.T) {
	hooks := &orchHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Orch-Hooks-Concurrent-Operations-Test")),
	}

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")
	defer memkv.DeleteClusters()

	type testCase struct {
		op  apiintf.APIOperType
		obj *orchestration.Orchestrator
	}

	tcs := []testCase{
		{apiintf.CreateOper, makeOrchObj("orch1", "20.1.1.1")},
		{apiintf.CreateOper, makeOrchObj("orch2", "21.1.1.1")},
		{apiintf.CreateOper, makeOrchObj("orch3", "22.1.1.1")},
		{apiintf.DeleteOper, makeOrchObj("orch2", "21.1.1.1")},
		{apiintf.DeleteOper, makeOrchObj("orch3", "22.1.1.1")},
	}

	ctx := context.TODO()

	key := make([]string, len(tcs))
	for i := 0; i < len(tcs); i++ {
		key[i] = tcs[i].obj.MakeKey(string(apiclient.GroupOrchestration))
	}

	var wg sync.WaitGroup
	wg.Add(3)

	// tcs[0]
	go func() {
		defer wg.Done()
		key[0] = tcs[0].obj.MakeKey(string(apiclient.GroupOrchestration))
		_, ok, err := hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key[0], tcs[0].op, false, *tcs[0].obj)
		if !ok && err == nil {
			t.Fatalf("hook failed but returned no error, test case: %d", 0)
		}
		AssertOk(t, err, fmt.Sprintf("hook returned unexpected error, testcase: %d", 0))
		tcObjIntf0, _, _ := hooks.getOrchestratorID(ctx, kv, kv.NewTxn(), key[0], tcs[0].op, false, *tcs[0].obj)
		*tcs[0].obj = tcObjIntf0.(orchestration.Orchestrator)

		err = kv.Create(ctx, key[0], tcs[0].obj)
		AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore, testcase: %d", 0))
	}()

	// tcs[1]
	go func() {
		defer wg.Done()
		key[1] = tcs[1].obj.MakeKey(string(apiclient.GroupOrchestration))
		_, ok, err := hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key[1], tcs[1].op, false, *tcs[1].obj)
		if !ok && err == nil {
			t.Fatalf("hook failed but returned no error, test case: %d", 1)
		}
		AssertOk(t, err, fmt.Sprintf("hook returned unexpected error, testcase: %d", 1))
		tcObjIntf1, _, _ := hooks.getOrchestratorID(ctx, kv, kv.NewTxn(), key[1], tcs[1].op, false, *tcs[1].obj)
		*tcs[1].obj = tcObjIntf1.(orchestration.Orchestrator)

		err = kv.Create(ctx, key[1], tcs[1].obj)
		AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore, testcase: %d", 1))
	}()

	// tcs[2]
	go func() {
		defer wg.Done()
		key[2] = tcs[2].obj.MakeKey(string(apiclient.GroupOrchestration))
		_, ok, err := hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key[2], tcs[2].op, false, *tcs[2].obj)
		if !ok && err == nil {
			t.Fatalf("hook failed but returned no error, test case: %d", 2)
		}
		AssertOk(t, err, fmt.Sprintf("hook returned unexpected error, testcase: %d", 2))
		tcObjIntf2, _, _ := hooks.getOrchestratorID(ctx, kv, kv.NewTxn(), key[2], tcs[2].op, false, *tcs[2].obj)
		*tcs[2].obj = tcObjIntf2.(orchestration.Orchestrator)

		err = kv.Create(ctx, key[2], tcs[2].obj)
		AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore, testcase: %d", 2))
	}()

	wg.Wait()

	// Validate Orch ID
	var orchs orchestration.OrchestratorList
	kindKey := orchs.MakeKey(string(apiclient.GroupOrchestration))
	err = kv.List(ctx, kindKey, &orchs)
	if err != nil {
		t.Fatalf("Error retrieving orchestrators: %v", err)
	}

	// Since we create orch1, orch2 and orch3, the orchs should contain three items
	AssertEquals(t, hooks.numAllocOrchID, int32(len(orchs.Items)), "Incorrect number of items in OrchestratorList")

	// Since we create these orchs in parallel, we don't know the orch ID assignment
	// What we can check is making sure that the first 3 elements on allocOrchID are
	// set to be true
	for i := 0; i < len(orchs.Items); i++ {
		AssertEquals(t, true, hooks.allocOrchID[i], "Incorrect orch ID assignment")
	}

	var newWg sync.WaitGroup
	newWg.Add(2)

	// Test concurrent deletion
	go func() {
		defer newWg.Done()
		hooks.deleteOrchestrator(ctx, tcs[3].op, *tcs[3].obj, false)
		err = kv.Delete(ctx, key[1], nil)
		AssertOk(t, err, fmt.Sprintf("Error deleting object in KVStore, testcase: %d", 3))
	}()

	go func() {
		defer newWg.Done()
		hooks.deleteOrchestrator(ctx, tcs[4].op, *tcs[4].obj, false)
		err = kv.Delete(ctx, key[2], nil)
		AssertOk(t, err, fmt.Sprintf("Error deleting object in KVStore, testcase: %d", 4))
	}()

	newWg.Wait()

	// Validate Orch ID
	var newOrchs orchestration.OrchestratorList
	newKindKey := newOrchs.MakeKey(string(apiclient.GroupOrchestration))
	err = kv.List(ctx, newKindKey, &newOrchs)
	if err != nil {
		t.Fatalf("Error retrieving orchestrators: %v", err)
	}

	// Since we delete  orch2 and orch3, the orchs should only contain one item
	AssertEquals(t, hooks.numAllocOrchID, int32(len(newOrchs.Items)), "Incorrect number of items in OrchestratorList")
}

func TestOrchestratorApiServerRestart(t *testing.T) {
	hooks := &orchHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Orch-Hooks-Api-Server-Restart-Test")),
	}

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")
	defer memkv.DeleteClusters()

	type testCase struct {
		op  apiintf.APIOperType
		obj *orchestration.Orchestrator
	}

	tcs := []testCase{
		{apiintf.CreateOper, makeOrchObj("orch1", "20.1.1.1")},
		{apiintf.CreateOper, makeOrchObj("orch2", "21.1.1.1")},
		{apiintf.CreateOper, makeOrchObj("orch3", "22.1.1.1")},
	}

	ctx := context.TODO()
	key := make([]string, len(tcs))
	tcObjIntfs := make([]interface{}, len(tcs))

	for i, tc := range tcs {
		key[i] = tc.obj.MakeKey(string(apiclient.GroupOrchestration))
		_, ok, err := hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key[i], tc.op, false, *tc.obj)
		if !ok && err == nil {
			t.Fatalf("hook failed but returned no error, test case: %d", i)
		}
		AssertOk(t, err, fmt.Sprintf("hook returned unexpected error, testcase: %d", i))
		tcObjIntfs[i], _, _ = hooks.getOrchestratorID(ctx, kv, kv.NewTxn(), key[i], tc.op, false, *tc.obj)
		*tc.obj = tcObjIntfs[i].(orchestration.Orchestrator)

		err = kv.Create(ctx, key[i], tc.obj)
		AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore, testcase: %d", i))

	}

	// Validate Orch ID
	var orchs orchestration.OrchestratorList
	kindKey := orchs.MakeKey(string(apiclient.GroupOrchestration))
	err = kv.List(ctx, kindKey, &orchs)
	if err != nil {
		t.Fatalf("Error retrieving orchestrators: %v", err)
	}

	// Since we create orch1, orch2 and orch3, the orchs should contain three items
	AssertEquals(t, hooks.numAllocOrchID, int32(len(orchs.Items)), "Incorrect number of items in OrchestratorList")

	// New orchHooks that we use to simulate API server restart
	newTc := testCase{apiintf.CreateOper, makeOrchObj("orch4", "23.1.1.1")}
	newHooks := &orchHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Orch-Hooks-Api-Server-Restart-Test1")),
	}
	newKey := newTc.obj.MakeKey(string(apiclient.GroupOrchestration))
	newHooks.getOrchestratorID(ctx, kv, kv.NewTxn(), newKey, newTc.op, false, *newTc.obj)

	AssertEquals(t, int32(4), newHooks.numAllocOrchID, "Incorrect number of allocated orch ID")
	for i := 0; i < maxOrchSupported; i++ {
		if i < len(tcs)+1 {
			AssertEquals(t, true, newHooks.allocOrchID[i], "Incorrect value from allocOrchID[%d]", i)
		} else {
			AssertEquals(t, false, newHooks.allocOrchID[i], "Incorrect value from allocOrchID[%d]", i)
		}
	}
}
