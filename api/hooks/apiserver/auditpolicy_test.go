package impl

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
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
