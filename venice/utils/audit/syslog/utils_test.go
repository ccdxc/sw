package syslog

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api/generated/monitoring"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestValidateSyslogAuditorConfig(t *testing.T) {
	tests := []struct {
		name string
		in   *monitoring.SyslogAuditor
		errs []error
	}{
		{
			name: "no targets in Syslog config",
			in: &monitoring.SyslogAuditor{
				Enabled: true,
				Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
				Targets: []*monitoring.ExportConfig{},
				SyslogConfig: &monitoring.SyslogExportConfig{
					FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					Prefix:           "pen-audit-events",
				},
			},
			errs: []error{fmt.Errorf("no syslog servers specified")},
		},
		{
			name: "valid syslog auditor config",
			in: &monitoring.SyslogAuditor{
				Enabled: true,
				Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
				Targets: []*monitoring.ExportConfig{

					{
						Destination: "192.168.10.1",
						Transport:   fmt.Sprintf("%s/%s", "UDP", "1234"),
					},
				},
				SyslogConfig: &monitoring.SyslogExportConfig{
					FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					Prefix:           "pen-audit-events",
				},
			},
			errs: []error{},
		},
		{
			name: "invalid target",
			in: &monitoring.SyslogAuditor{
				Enabled: true,
				Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
				Targets: []*monitoring.ExportConfig{

					{
						Destination: "192.168.10.1",
						Transport:   "UDP/abc",
					},
				},
				SyslogConfig: &monitoring.SyslogExportConfig{
					FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					Prefix:           "pen-audit-events",
				},
			},
			errs: []error{fmt.Errorf("invalid port in %s", "UDP/abc")},
		},
	}

	for _, test := range tests {
		errs := ValidateSyslogAuditorConfig(test.in)
		SortErrors(errs)
		SortErrors(test.errs)
		Assert(t, len(errs) == len(test.errs), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs, errs))
		for i, err := range errs {
			Assert(t, err.Error() == test.errs[i].Error(), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs[i], errs[i]))
		}
	}
}
