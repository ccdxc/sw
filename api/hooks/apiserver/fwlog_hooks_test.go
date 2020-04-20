package impl

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestValidateFwlogPolicy(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute*5)
	defer cancel()

	logConfig := &log.Config{
		Module:      "fwlog-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	s := &fwlogHooks{
		logger: l,
	}
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}

	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}
	txn := kvs.NewTxn()

	testFwlogSpecList := []struct {
		name   string
		policy monitoring.FwlogPolicy
		fail   bool
	}{
		{
			name: "invalid protocol",
			fail: true,
			policy: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "IP/1234",
						},
					},
				},
			},
		},

		{
			name: "valid policy",
			fail: false,
			policy: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
						},
					},
				},
			},
		},
	}

	for i := range testFwlogSpecList {
		_, ok, err := s.validateFwlogPolicy(ctx, kvs, txn, "", apiintf.CreateOper, false,
			testFwlogSpecList[i].policy)

		if testFwlogSpecList[i].fail == true {
			t.Logf(fmt.Sprintf("test [%v] returned %v", testFwlogSpecList[i].name, err))
			Assert(t, ok == false, "test [%v] returned %v", testFwlogSpecList[i].name, err)
		} else {
			t.Log(fmt.Sprintf("test [%v] returned %v", testFwlogSpecList[i].name, err))
			Assert(t, ok == true, "test [%v] returned %v", testFwlogSpecList[i].name, err)
		}
	}
}

func TestFwlogHandleCredentialUpdate(t *testing.T) {
	kind := string(monitoring.KindFwlogPolicy)
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		existing *monitoring.FwlogPolicy
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
			in: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{},
				},
			},
			out: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{},
				},
			},
			result: true,
			err:    fmt.Errorf("Invalid input type"),
		},
		{
			name: "missing existing obj",
			oper: apiintf.UpdateOper,
			in: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{},
				},
			},
			existing: nil,
			out: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{},
				},
			},
			result: true,
			err:    kvstore.NewKeyNotFoundError("/venice/config/monitoring/fwlogPolicy/default/default", 0),
		},
		{
			name: "populate existing credentials",
			oper: apiintf.UpdateOper,
			in: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{ // new collector
							Destination: "10.1.1.101",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "testPassword",
							},
						},
						{ // existing collector
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
						},
					},
				},
			},
			existing: &monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "testPassword",
							},
						},
					},
				},
			},
			out: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{ // new collector
							Destination: "10.1.1.101",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "testPassword",
							},
						},
						{ // existing collector
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "testPassword",
							},
						},
					},
				},
			},
			result: true,
			err:    nil,
		},
		{
			name: "Update existing",
			oper: apiintf.UpdateOper,
			in: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{ // new collector
							Destination: "10.1.1.101",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "testPassword",
							},
						},
						{ // existing collector
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "newPassword",
							},
						},
					},
				},
			},
			existing: &monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "testPassword",
							},
						},
					},
				},
			},
			out: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{ // new collector
							Destination: "10.1.1.101",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "testPassword",
							},
						},
						{ // existing collector
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
							Credentials: &monitoring.ExternalCred{
								Password: "newPassword",
							},
						},
					},
				},
			},
			result: true,
			err:    nil,
		},
	}

	logConfig := log.GetDefaultConfig("TestfwlogsHook-handleCredentials")
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
	dummyObj := &monitoring.FwlogPolicy{
		TypeMeta: api.TypeMeta{Kind: kind},
		ObjectMeta: api.ObjectMeta{
			Namespace: globals.DefaultNamespace,
			Name:      globals.DefaultTenant,
			Tenant:    globals.DefaultTenant,
		},
	}
	key := dummyObj.MakeKey("monitoring")
	hook := &fwlogHooks{
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
