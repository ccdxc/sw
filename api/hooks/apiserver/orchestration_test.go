package impl

import (
	"context"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	"net"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/kvstore"
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
			Credentials: &monitoring.ExternalCred{
				AuthType: "username-password",
				UserName: "user",
				Password: "pass",
			},
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

func TestOrchHandleCredentialUpdate(t *testing.T) {
	kind := string(orchestration.KindOrchestrator)
	objName := "testObj"
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		existing *orchestration.Orchestrator
		out      interface{}
		result   bool
		err      error
	}{
		{
			name: "invalid input object",
			oper: apiintf.UpdateOper,
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			result: true,
			err:    fmt.Errorf("Invalid input type"),
		},
		{
			name: "invalid operation type",
			oper: apiintf.CreateOper,
			in: orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
			},
			out: orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
			},
			result: true,
			err:    fmt.Errorf("Invalid input type"),
		},
		{
			name: "missing existing obj",
			oper: apiintf.UpdateOper,
			in: orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
			},
			existing: nil,
			out: orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
			},
			result: true,
			err:    kvstore.NewKeyNotFoundError("/venice/config/orchestration/orchestrator/testObj", 0),
		},
		{
			name: "populate existing credentials",
			oper: apiintf.UpdateOper,
			in: orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "newUri",
				},
			},
			existing: &orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "oldUri",
					Credentials: &monitoring.ExternalCred{
						Password: "testPassword",
					},
				},
			},
			out: orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "newUri",
					Credentials: &monitoring.ExternalCred{
						Password: "testPassword",
					},
				},
			},
			result: true,
			err:    nil,
		},
		{
			name: "Update existing",
			oper: apiintf.UpdateOper,
			in: orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "newUri",
					Credentials: &monitoring.ExternalCred{
						Password: "newPassword",
					},
				},
			},
			existing: &orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "oldUri",
					Credentials: &monitoring.ExternalCred{
						Password: "testPassword",
					},
				},
			},
			out: orchestration.Orchestrator{
				TypeMeta: api.TypeMeta{Kind: kind},
				ObjectMeta: api.ObjectMeta{
					Name: objName,
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "newUri",
					Credentials: &monitoring.ExternalCred{
						Password: "newPassword",
					},
				},
			},
			result: true,
			err:    nil,
		},
	}

	logConfig := log.GetDefaultConfig("TestOrchHooks-handleCredentials")
	l := log.GetNewLogger(logConfig)
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}
	dummyObj := &orchestration.Orchestrator{
		TypeMeta: api.TypeMeta{Kind: kind},
		ObjectMeta: api.ObjectMeta{
			Name: objName,
		},
	}
	key := dummyObj.MakeKey("orchestration")
	hook := &orchHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
		defer cancelFunc()
		txn := kvs.NewTxn()
		kvs.Delete(ctx, key, nil)
		if test.existing != nil {
			// encrypt object as credentials are stored as secret
			if err := test.existing.ApplyStorageTransformer(ctx, true); err != nil {
				t.Fatalf("[%s] test failed, error encrypting password, Err: %v", test.name, err)
			}

			if err := kvs.Create(ctx, key, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with cluster, Err: %v", test.name, err)
			}
		}
		out, ok, err := hook.handleCredentialUpdate(ctx, kvs, txn, key, test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		AssertEquals(t, test.err, err, fmt.Sprintf("[%v] test failed", test.name))
		AssertEquals(t, test.out, out, fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
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

	type testCase struct {
		op  apiintf.APIOperType
		obj *orchestration.Orchestrator
		err error
	}

	testCases := []testCase{
		{apiintf.CreateOper, makeOrchObj("orch1", "10.1.1.1"), nil},                                                           // First object no conflicts
		{apiintf.CreateOper, makeOrchObj("orch2", "11.1.1.1"), nil},                                                           // Second object no conflicts
		{apiintf.CreateOper, makeOrchObj("orch3", "11.1.1.1"), fmt.Errorf("URI is already used by Orchestrator orch2")},       // Third object with conflict
		{apiintf.UpdateOper, makeOrchObj("orch1", "11.1.1.1"), fmt.Errorf("URI is already used by Orchestrator orch2")},       // first object update with conflict
		{apiintf.CreateOper, makeOrchObj("orch4", "{10.30.1.181:8989}"), fmt.Errorf("{10.30.1.181:8989} is not a valid URI")}, // object with bad uri
		{
			apiintf.CreateOper,
			&orchestration.Orchestrator{
				ObjectMeta: api.ObjectMeta{
					Name:            "cred-miss",
					ResourceVersion: "1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Orchestrator",
					APIVersion: "v1",
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "vc.pensando.io",
				},
			},
			fmt.Errorf("Orch cred-miss has credentials missing"),
		},
		{
			apiintf.CreateOper,
			&orchestration.Orchestrator{
				ObjectMeta: api.ObjectMeta{
					Name:            "user-miss",
					ResourceVersion: "1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Orchestrator",
					APIVersion: "v1",
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "vc.pensando.io",
					Credentials: &monitoring.ExternalCred{
						AuthType: monitoring.ExportAuthType_AUTHTYPE_USERNAMEPASSWORD.String(),
						Password: "pass",
					},
				},
			},
			fmt.Errorf("Credentials for orchestrator user-miss missing username"),
		},
		{
			apiintf.CreateOper,
			&orchestration.Orchestrator{
				ObjectMeta: api.ObjectMeta{
					Name:            "pass-miss",
					ResourceVersion: "1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Orchestrator",
					APIVersion: "v1",
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "vc.pensando.io",
					Credentials: &monitoring.ExternalCred{
						AuthType: monitoring.ExportAuthType_AUTHTYPE_USERNAMEPASSWORD.String(),
						UserName: "user",
					},
				},
			},
			fmt.Errorf("Credentials for orchestrator pass-miss missing password"),
		},
		{
			apiintf.CreateOper,
			&orchestration.Orchestrator{
				ObjectMeta: api.ObjectMeta{
					Name:            "extra-elem",
					ResourceVersion: "1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Orchestrator",
					APIVersion: "v1",
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "vc.pensando.io",
					Credentials: &monitoring.ExternalCred{
						AuthType:    monitoring.ExportAuthType_AUTHTYPE_USERNAMEPASSWORD.String(),
						UserName:    "user",
						Password:    "pass",
						BearerToken: "randomtoken",
					},
				},
			},
			fmt.Errorf("Credentials for orchestrator extra-elem has unnecessary fields passed"),
		},
		{
			apiintf.CreateOper,
			&orchestration.Orchestrator{
				ObjectMeta: api.ObjectMeta{
					Name:            "empty",
					ResourceVersion: "1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Orchestrator",
					APIVersion: "v1",
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "vc.pensando.io",
					Credentials: &monitoring.ExternalCred{
						AuthType: monitoring.ExportAuthType_AUTHTYPE_TOKEN.String(),
					},
				},
			},
			fmt.Errorf("Credentials for orchestrator empty missing token"),
		},
		{
			apiintf.CreateOper,
			&orchestration.Orchestrator{
				ObjectMeta: api.ObjectMeta{
					Name:            "wrong-elem-2",
					ResourceVersion: "1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Orchestrator",
					APIVersion: "v1",
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "vc.pensando.io",
					Credentials: &monitoring.ExternalCred{
						AuthType: monitoring.ExportAuthType_AUTHTYPE_TOKEN.String(),
						UserName: "user",
					},
				},
			},
			fmt.Errorf("Credentials for orchestrator wrong-elem-2 missing token"),
		},
		{
			apiintf.CreateOper,
			&orchestration.Orchestrator{
				ObjectMeta: api.ObjectMeta{
					Name:            "extra-elem",
					ResourceVersion: "1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Orchestrator",
					APIVersion: "v1",
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "vc.pensando.io",
					Credentials: &monitoring.ExternalCred{
						AuthType:    monitoring.ExportAuthType_AUTHTYPE_TOKEN.String(),
						BearerToken: "token",
						Password:    "pass",
					},
				},
			},
			fmt.Errorf("Credentials for orchestrator extra-elem has unnecessary fields passed"),
		},
		{
			apiintf.CreateOper,
			&orchestration.Orchestrator{
				ObjectMeta: api.ObjectMeta{
					Name:            "empty",
					ResourceVersion: "1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Orchestrator",
					APIVersion: "v1",
				},
				Spec: orchestration.OrchestratorSpec{
					URI: "vc.pensando.io",
					Credentials: &monitoring.ExternalCred{
						AuthType: monitoring.ExportAuthType_AUTHTYPE_CERTS.String(),
					},
				},
			},
			fmt.Errorf("Credentials for orchestrator empty missing fields"),
		},
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

func TestOrchManagedNamespaceUpdate(t *testing.T) {
	logConfig := &log.Config{
		Module:      "Orchestrator-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	hooks := &orchHooks{
		logger: l,
	}

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")

	uri := "vc:8989"

	nw := &network.Network{
		ObjectMeta: api.ObjectMeta{
			Name:   "nw-orch-test",
			Tenant: "default",
		},
		TypeMeta: api.TypeMeta{
			Kind: "network",
		},
		Spec: network.NetworkSpec{
			Type: network.NetworkType_Bridged.String(),
			Orchestrators: []*network.OrchestratorInfo{
				&network.OrchestratorInfo{
					Name:      "o1",
					Namespace: "used-namespace1",
				},
				&network.OrchestratorInfo{
					Name:      "o1",
					Namespace: "used-namespace2",
				},
			},
		},
	}

	ctx := context.TODO()
	// Create orch
	o1 := &orchestration.Orchestrator{
		ObjectMeta: api.ObjectMeta{
			Name:            "orchdc",
			ResourceVersion: "1",
			Tenant:          "default",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "Orchestrator",
			APIVersion: "v1",
		},
		Spec: orchestration.OrchestratorSpec{
			URI: uri,
			Credentials: &monitoring.ExternalCred{
				AuthType: "username-password",
				UserName: "user",
				Password: "pass",
			},
			ManageNamespaces: []string{"used-namespace1", "used-namespace2", "unused-namespace1"},
		},
	}

	key := o1.MakeKey(string(apiclient.GroupOrchestration))
	err = kv.Create(ctx, key, o1)
	defer kv.Delete(ctx, key, nil)
	AssertOk(t, err, "kv operation failed")

	// Create network
	err = kv.Create(ctx, nw.MakeKey(string(apiclient.GroupNetwork)), nw)
	AssertOk(t, err, "kv operation failed")

	// Add namespace
	o1.Spec.ManageNamespaces = []string{"used-namespace1", "used-namespace2", "unused-namespace1", "another-unused-namespace"}
	_, _, err = hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *o1)
	AssertOk(t, err, "namespace addition should have succeeded")

	// Remove a user namespace
	o1.Spec.ManageNamespaces = []string{"used-namespace2", "unused-namespace1"}
	_, _, err = hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *o1)
	Assert(t, err != nil, "managed namespace removal should have failed")

	// Remove unused namespace
	o1.Spec.ManageNamespaces = []string{"used-namespace1", "used-namespace2"}
	_, _, err = hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *o1)
	AssertOk(t, err, "manage namespace removal should have succeeded")

	// Update with "all_namespace"
	o1.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	_, _, err = hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *o1)
	AssertOk(t, err, "manage namespace update should have succeeded")

	// Update with "all_namespace"
	o1.Spec.ManageNamespaces = []string{utils.ManageAllDcs, "random-dc"}
	_, _, err = hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *o1)
	Assert(t, err != nil, "manage namespace update should have succeeded")

	// Update with empty list
	o1.Spec.ManageNamespaces = []string{}
	_, _, err = hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *o1)
	Assert(t, err != nil, "manage namespace update should have succeeded")

	// Delete the network correponding to used-namespace1
	err = kv.Delete(ctx, nw.MakeKey(string(apiclient.GroupNetwork)), nw)
	AssertOk(t, err, "kv operation failed")

	// Update with empty list
	o1.Spec.ManageNamespaces = []string{}
	_, _, err = hooks.validateOrchestrator(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *o1)
	AssertOk(t, err, "manage namespace update should have succeeded")

}
