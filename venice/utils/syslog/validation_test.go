package syslog

import (
	"testing"

	"github.com/pensando/sw/api/generated/monitoring"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestValidateSyslogMessageFormat(t *testing.T) {
	tcs := []struct {
		msg             string
		format          monitoring.MonitoringExportFormat
		expectedSuccess bool
	}{
		{
			"<134>Nov 19 10:55:07 hostname tag1[74544]: test2",
			monitoring.MonitoringExportFormat_SYSLOG_BSD,
			true,
		},
		{
			"<134>Nov 19 10:55:07 hostname tag1[56789]: test2",
			monitoring.MonitoringExportFormat_SYSLOG_BSD,
			true,
		},
		{ // invalid priority
			"<invalid134>Nov 19 10:55:07 hostname tag1[74544]: test2",
			monitoring.MonitoringExportFormat_SYSLOG_BSD,
			false,
		},
		{ // invalid PID
			"<134>Nov 19 10:55:07 hostname tag1[74544invalid]: test2",
			monitoring.MonitoringExportFormat_SYSLOG_BSD,
			false,
		},
		{ // empty message
			"<134>2018-09-21T17:45:16-07:00 hostname tag1[74544invalid]: ",
			monitoring.MonitoringExportFormat_SYSLOG_BSD,
			false,
		},
		{
			"<94>1 2018-09-24T14:16:21-07:00 host1 app1 78990 msgid [id1 k1=\"v1\"] test1",
			monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			true,
		},
		{
			"<94>1 2018-11-19T21:05:36Z host1 app1 78990 msgid [id1 k1=\"v1\"] test1",
			monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			true,
		},
		{ // invalid version
			"<94>invalid 2018-09-24T14:16:21-07:00 host1 app1 78990 msgid [id1 k1=\"v1\"] test1",
			monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			false,
		},
		{ // invalid PID
			"<94>1 2018-09-24T14:16:21-07:00 host1 app1 7899invalid0 msgid [id1 k1=\"v1\"] test1",
			monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			false},
		{ // invalid priority
			"<invalid94>invalid 2018-09-24T14:16:21-07:00 host1 app1 78990 msgid [id1 k1=\"v1\"] test1",
			monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			false,
		},
		{
			"<10>1 2018-09-24T15:58:17-07:00 host1 pen-events 83256 7f28c637-60a4-4d3f-9e0d-e5dc2027dfdf " +
				"[status.reason policy-id=\"d2301def-6f9d-480b-af7a-43b0595d4218\"][status.source node-name=\"test-node\" component=\"feef8dcd-4a42-4266-80f8-e8936167497c\"]" +
				"[status.object-ref name=\"MOJPa\" uri=\"\" tenant=\"default\" namespace=\"default\" kind=\"Node\"][type kind=\"Alert\"][meta creation-time=\"2018-09-24 22:58:17.700839782 +0000 UTC\" mod-time=\"2018-09-24 22:58:17.700839782 +0000 UTC\" " +
				"name=\"7f28c637-60a4-4d3f-9e0d-e5dc2027dfdf\" uuid=\"7f28c637-60a4-4d3f-9e0d-e5dc2027dfdf\" tenant=\"default\" namespace=\"default\"]+" +
				"[spec state=\"OPEN\"][status severity=\"CRITICAL\" message=\"DUMMYEVENT-1-CRITICAL\" event-uri=\"/events/v1/events/3ec2586e-c3d4-4efd-8ea1-b83bb8610be2\"] " +
				"DUMMYEVENT-1-CRITICAL",
			monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			true,
		},
		{
			"invalid message format",
			monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			false,
		},
		{
			"invalid message format",
			monitoring.MonitoringExportFormat_SYSLOG_BSD,
			false,
		},
		{
			"",
			monitoring.MonitoringExportFormat_SYSLOG_BSD,
			false,
		},
		{ // invalid format
			"",
			monitoring.MonitoringExportFormat(5),
			false,
		},
	}

	for i, tc := range tcs {
		match := ValidateSyslogMessage(tc.format, tc.msg)
		Assert(t, match == tc.expectedSuccess, "tc #%v(%s) failed: expected: %v, got: %v", i, tc.msg, tc.expectedSuccess, match)
	}
}
