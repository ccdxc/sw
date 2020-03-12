package syslog

import (
	"context"
	"fmt"
	"net"
	"reflect"
	"strings"
	"testing"

	"github.com/pensando/sw/api/generated/monitoring"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCreateSyslogWriter(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	addr, _, err := Server(ctx, "127.0.0.1:", monitoring.MonitoringExportFormat_SYSLOG_BSD.String(), "udp")
	AssertOk(t, err, "failed to create syslog BSD server")
	s := strings.Split(addr, ":")
	tests := []struct {
		name            string
		syslogExportCfg *monitoring.SyslogExportConfig
		targets         []*monitoring.ExportConfig
		out             map[string]*WriterConfig
		err             error
	}{
		{
			name: "successful test",
			syslogExportCfg: &monitoring.SyslogExportConfig{
				FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
				Prefix:           "tag1",
			},
			targets: []*monitoring.ExportConfig{
				{
					Destination: s[0],
					Transport:   fmt.Sprintf("%s/%s", "UDP", s[1]),
				},
			},
			out: map[string]*WriterConfig{
				fmt.Sprintf("%s:%s/%s", s[0], "UDP", s[1]): {
					Format:     monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Network:    "udp",
					RemoteAddr: addr,
					Tag:        "tag1",
					Priority:   Priority(monitoring.SyslogFacility_LOG_USER),
					Writer:     nil,
				},
			},
		},
	}
	for _, test := range tests {
		writerMap, err := CreateSyslogWriters(ctx, "tag1", t.Name(), test.syslogExportCfg, monitoring.MonitoringExportFormat_SYSLOG_BSD.String(), test.targets)
		Assert(t, err == test.err, "unexpected error creating syslog writers")
		for key, expW := range test.out {
			w, ok := writerMap[key]
			Assert(t, ok, fmt.Sprintf("unable to find writer with key %v", key))
			Assert(t, w.Writer != nil, "syslog writer shouldn't be nil")
			w.Writer = nil
			Assert(t, reflect.DeepEqual(expW, w), "expected writer: %#v, got: %#v", expW, w)
		}
	}
}

func TestValidateTransport(t *testing.T) {
	tests := []struct {
		name      string
		transport string
		err       error
	}{
		{
			name:      "incorrect format",
			transport: "UDP",
			err:       fmt.Errorf("transport should be in protocol/port format"),
		},
		{
			name:      "incorrect protocol",
			transport: "http/443",
			err:       fmt.Errorf("invalid protocol in %s\n Accepted protocols: TCP, UDP", "http/443"),
		},
		{
			name:      "incorrect port",
			transport: "udp/sdf",
			err:       fmt.Errorf("invalid port in %s", "udp/sdf"),
		},
		{
			name:      "valid transport",
			transport: "udp/123",
			err:       nil,
		},
		{
			name:      "out of bound port",
			transport: "udp/69000",
			err:       fmt.Errorf("invalid port (> %d) in %s", ^uint16(0), "udp/69000"),
		},
	}
	for _, test := range tests {
		err := ValidateTransport(test.transport)
		Assert(t, reflect.DeepEqual(err, test.err), "unexpected error, expected %v, got %v", test.err, err)
	}
}

func TestValidateDestination(t *testing.T) {
	tests := []struct {
		name        string
		destination string
		err         error
	}{
		{
			name:        "empty destination",
			destination: "",
			err:         fmt.Errorf("destination cannot be empty"),
		},
		{
			name:        "incorrect destination",
			destination: "incorrect-destination",
			err: fmt.Errorf("failed to resolve destination {%s}, err: %v", "incorrect-destination", func() error {
				_, err := net.LookupHost("incorrect-destination")
				return err
			}()),
		},
		{
			name:        "valid destination",
			destination: "127.0.0.1",
			err:         nil,
		},
	}
	for _, test := range tests {
		err := ValidateDestination(test.destination)
		Assert(t, reflect.DeepEqual(err, test.err), "unexpected error, expected %v, got %v", test.err, err)
	}
}
