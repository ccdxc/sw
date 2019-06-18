package impl

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestValidateEventPolicy(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute*5)
	defer cancel()

	logConfig := &log.Config{
		Module:      "Event-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	s := &eventHooks{
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

	testEventSpecList := []struct {
		name   string
		policy monitoring.EventPolicy
		fail   bool
	}{
		{
			name: "invalid DNS",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-DNS",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "test.pensando.iox",
							Transport:   "TCP/1234",
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "duplicate targets",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "duplicate-targets",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
						},
						{
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

		{
			name: "invalid destination (empty)",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-destination",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "",
							Transport:   "TCP/1234",
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid transport (empty)",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-transport",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "",
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid targets (empty)",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-targets",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid Transport, missing port",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-Transport-missing-port",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP",
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid Transport, missing protocol",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-Transport-missing-protocol",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "1234",
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid protocol, only TCP/UDP is supported",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-protocol",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "ICMP/1234",
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid port (bbbb)",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-port-bbbb",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/bbbb",
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid port (65536)",
			fail: true,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-port-65536",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/65536",
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "valid policy",
			fail: false,
			policy: monitoring.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "eventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "valid-policy",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
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
	}

	for i := range testEventSpecList {
		_, ok, err := s.validateSyslogEventPolicy(ctx, kvs, txn, "", apiintf.CreateOper, false,
			testEventSpecList[i].policy)

		if testEventSpecList[i].fail == true {
			t.Logf(fmt.Sprintf("test [%v] returned %v", testEventSpecList[i].name, ok))
			Assert(t, ok == false, "test [%v] returned %v", testEventSpecList[i].name, err)
		} else {
			t.Log(fmt.Sprintf("test [%v] returned %v", testEventSpecList[i].name, ok))
			Assert(t, ok == true, "test [%v] returned %v", testEventSpecList[i].name, err)
		}
	}
}
