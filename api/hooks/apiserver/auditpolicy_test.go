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

func TestValidateAuditPolicy(t *testing.T) {
	tests := []struct {
		name       string
		in         interface{}
		ignoreSpec bool
		errs       []error
	}{
		{
			name: "invalid input object",
			in: struct {
				Test string
			}{"testing"},
			errs: []error{fmt.Errorf("invalid object %T instead of AuditPolicy", struct{ Test string }{"testing"})},
		},
		{
			name: "Missing syslog config",
			in: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuditPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "MissingSyslogConfig",
				},
				Spec: monitoring.AuditPolicySpec{},
			},
			errs: nil,
		},
		{
			name: "valid audit policy",
			in: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindAuditPolicy),
				},
				ObjectMeta: api.ObjectMeta{
					Name:      "audit-policy-1",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.AuditPolicySpec{
					Syslog: &monitoring.SyslogAuditor{
						Enabled: true,
						Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{

							{
								Destination: "192.168.1.10",
								Transport:   fmt.Sprintf("%s/%s", "UDP", "1234"),
							},
						},
						SyslogConfig: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							Prefix:           "pen-audit-events",
						},
					},
				},
			},
			errs: nil,
		},
		{
			name: "ignore spec",
			in: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindAuditPolicy),
				},
				ObjectMeta: api.ObjectMeta{
					Name:      "audit-policy-1",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.AuditPolicySpec{
					Syslog: &monitoring.SyslogAuditor{
						Enabled: true,
						Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{

							{
								Destination: "192.168.1.10",
								Transport:   fmt.Sprintf("%s/%s", "UDP", "1234"),
							},
						},
						SyslogConfig: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							Prefix:           "pen-audit-events",
						},
					},
				},
			},
			ignoreSpec: true,
			errs:       nil,
		},
	}
	r := auditPolicyHooks{}
	logConfig := log.GetDefaultConfig("TestAuditPolicyHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		errs := r.validateAuditPolicy(test.in, "", false, test.ignoreSpec)
		SortErrors(errs)
		SortErrors(test.errs)
		Assert(t, len(errs) == len(test.errs), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs, errs))
		for i, err := range errs {
			Assert(t, err.Error() == test.errs[i].Error(), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs[i], errs[i]))
		}
	}
}

func TestAuditPolicyHandleCredentialUpdate(t *testing.T) {
	kind := string(monitoring.KindAuditPolicy)
	objName := globals.DefaultTenant
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		existing *monitoring.AuditPolicy
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
			in: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{},
			},
			out: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{},
			},
			result: true,
			err:    fmt.Errorf("Invalid input type"),
		},
		{
			name: "missing existing obj",
			oper: apiintf.UpdateOper,
			in: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{},
			},
			existing: nil,
			out: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{},
			},
			result: true,
			err:    kvstore.NewKeyNotFoundError("/venice/config/monitoring/audit-policy/default/Singleton", 0),
		},
		{
			name: "populate existing credentials",
			oper: apiintf.UpdateOper,
			in: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{
					Syslog: &monitoring.SyslogAuditor{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
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
						SyslogConfig: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
			existing: &monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{
					Syslog: &monitoring.SyslogAuditor{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "TCP/1234",
								Credentials: &monitoring.ExternalCred{
									Password: "testPassword",
								},
							},
						},
						SyslogConfig: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
			out: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{
					Syslog: &monitoring.SyslogAuditor{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
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
						SyslogConfig: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
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
			in: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{
					Syslog: &monitoring.SyslogAuditor{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
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
						SyslogConfig: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
			existing: &monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{
					Syslog: &monitoring.SyslogAuditor{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "TCP/1234",
								Credentials: &monitoring.ExternalCred{
									Password: "testPassword",
								},
							},
						},
						SyslogConfig: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
			out: monitoring.AuditPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      objName,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.AuditPolicySpec{
					Syslog: &monitoring.SyslogAuditor{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
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
						SyslogConfig: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
			result: true,
			err:    nil,
		},
	}

	logConfig := log.GetDefaultConfig("TestEventsHook-handleCredentials")
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
	dummyObj := &monitoring.AuditPolicy{
		TypeMeta: api.TypeMeta{Kind: kind},
		ObjectMeta: api.ObjectMeta{
			Namespace: globals.DefaultNamespace,
			Name:      objName,
			Tenant:    globals.DefaultTenant,
		},
	}
	key := dummyObj.MakeKey("monitoring")
	hook := &auditPolicyHooks{
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
