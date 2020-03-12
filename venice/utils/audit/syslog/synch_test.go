package syslog

import (
	"fmt"
	"net"
	"reflect"
	"strings"
	"sync"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/audit"
	"github.com/pensando/sw/venice/utils/audit/mock"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/syslog"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	ti tInfo
)

type tInfo struct {
	logger     log.Logger
	udpServers []*udpInfo
	getter     audit.PolicyGetter
}

type udpInfo struct {
	port     string
	pconn    net.PacketConn
	recvChan chan string
}

func setup() error {
	config := log.GetDefaultConfig("audit")
	config.Filter = log.AllowAllFilter
	ti.logger = log.GetNewLogger(config)
	for i := 0; i < 2; i++ {
		srv, err := startUDPServer()
		if err != nil {
			return err
		}
		ti.udpServers = append(ti.udpServers, srv)
	}
	policies := []*monitoring.AuditPolicy{
		{
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
							Destination: "127.0.0.1",
							Transport:   fmt.Sprintf("%s/%s", "UDP", ti.udpServers[0].port),
						},
						{
							Destination: "127.0.0.1",
							Transport:   fmt.Sprintf("%s/%s", "UDP", ti.udpServers[1].port),
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						Prefix:           "pen-audit-events",
					},
				},
			},
		},
		{
			TypeMeta: api.TypeMeta{
				Kind: string(monitoring.KindAuditPolicy),
			},
			ObjectMeta: api.ObjectMeta{
				Name:      "audit-policy-2",
				Tenant:    "testtenant",
				Namespace: globals.DefaultNamespace,
			},
			Spec: monitoring.AuditPolicySpec{
				Syslog: &monitoring.SyslogAuditor{
					Enabled: true,
					Format:  monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "127.0.0.1",
							Transport:   fmt.Sprintf("%s/%s", "UDP", ti.udpServers[0].port),
						},
						{
							Destination: "127.0.0.1",
							Transport:   fmt.Sprintf("%s/%s", "UDP", ti.udpServers[1].port),
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						Prefix:           "pen-audit-events",
					},
				},
			},
		},
		{
			TypeMeta: api.TypeMeta{
				Kind: string(monitoring.KindAuditPolicy),
			},
			ObjectMeta: api.ObjectMeta{
				Name:      "audit-policy-incorrect-port",
				Tenant:    "testtenant1",
				Namespace: globals.DefaultNamespace,
			},
			Spec: monitoring.AuditPolicySpec{
				Syslog: &monitoring.SyslogAuditor{
					Enabled: true,
					Format:  monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "127.0.0.1",
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
		{
			TypeMeta: api.TypeMeta{
				Kind: string(monitoring.KindAuditPolicy),
			},
			ObjectMeta: api.ObjectMeta{
				Name:      "audit-policy-3",
				Tenant:    mock.ErrorTenant,
				Namespace: globals.DefaultNamespace,
			},
			Spec: monitoring.AuditPolicySpec{
				Syslog: &monitoring.SyslogAuditor{
					Enabled: true,
					Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "127.0.0.1",
							Transport:   fmt.Sprintf("%s/%s", "UDP", ti.udpServers[0].port),
						},
						{
							Destination: "127.0.0.1",
							Transport:   fmt.Sprintf("%s/%s", "UDP", ti.udpServers[1].port),
						},
					},
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						Prefix:           "pen-audit-events",
					},
				},
			},
		},
	}
	ti.getter = mock.NewPolicyGetter(policies)
	return nil
}

func shutdown() {
	for _, srv := range ti.udpServers {
		srv.pconn.Close()
	}
	if ti.getter != nil {
		ti.getter.Stop()
	}
}

func startUDPServer() (*udpInfo, error) {
	pconn, recvChan, err := serviceutils.StartUDPServer(":0")
	if err != nil {
		return nil, err
	}
	tmp := strings.Split(pconn.LocalAddr().String(), ":")
	port := tmp[len(tmp)-1]
	return &udpInfo{
		port:     port,
		pconn:    pconn,
		recvChan: recvChan,
	}, nil
}

func validateSyslogMsgs(t *testing.T) bool {
	var rfcMessages, bsdMessages int
	var cases []reflect.SelectCase
	for _, srv := range ti.udpServers {
		cases = append(cases, reflect.SelectCase{
			Dir:  reflect.SelectRecv,
			Chan: reflect.ValueOf(srv.recvChan),
		})
	}
	cases = append(cases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(time.After(6 * time.Second)),
	})
	// wait for events
	for {
		chosen, value, ok := reflect.Select(cases)
		if !ok {
			return ok
		}
		if chosen == len(cases)-1 { // 6s timeout
			t.Logf("BSD Msgs: %v, RFC Msgs: %v", bsdMessages, rfcMessages)
			if bsdMessages > 0 || rfcMessages > 0 {
				return true
			}
			return false
		}
		msg, ok := value.Interface().(string)
		if !ok {
			return ok
		}
		if syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_BSD, msg) {
			bsdMessages++
		} else if syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_RFC5424, msg) {
			rfcMessages++
		} else {
			return false
		}
	}
}

func TestProcessEvents(t *testing.T) {
	err := setup()
	defer shutdown()
	AssertOk(t, err, "failed to create udp servers")
	auditor := NewSynchAuditor(nil, ti.logger, WithPolicyGetter(ti.getter), WithReportError(true))
	err = auditor.Run()
	AssertOk(t, err, "failed to initialize auditor")
	defer auditor.Shutdown()
	tn := time.Now()
	ts, _ := types.TimestampProto(tn)
	evts := []*auditapi.AuditEvent{
		{
			TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
			ObjectMeta: api.ObjectMeta{Name: "auditevent-1", UUID: uuid.NewV4().String(), Tenant: globals.DefaultTenant, CreationTime: api.Timestamp{Timestamp: *ts}},
			EventAttributes: auditapi.EventAttributes{
				Level:       auditapi.Level_RequestResponse.String(),
				Stage:       auditapi.Stage_RequestProcessing.String(),
				User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
				Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
				ClientIPs:   []string{"192.168.75.133"},
				Action:      auth.Permission_Create.String(),
				Outcome:     auditapi.Outcome_Success.String(),
				GatewayNode: "node2",
				GatewayIP:   "192.168.75.139",
				Data:        make(map[string]string),
			},
		},
		{
			TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
			ObjectMeta: api.ObjectMeta{Name: "auditevent-2", UUID: uuid.NewV4().String(), Tenant: "testtenant", CreationTime: api.Timestamp{Timestamp: *ts}},
			EventAttributes: auditapi.EventAttributes{
				Level:       auditapi.Level_RequestResponse.String(),
				Stage:       auditapi.Stage_RequestProcessing.String(),
				User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
				Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
				ClientIPs:   []string{"192.168.75.133"},
				Action:      auth.Permission_Create.String(),
				Outcome:     auditapi.Outcome_Success.String(),
				GatewayNode: "node2",
				GatewayIP:   "192.168.75.139",
				Data:        make(map[string]string),
			},
		},
	}
	time.Sleep(3 * time.Second)
	var waitgrp sync.WaitGroup
	for j := 0; j < 2; j++ {
		for i := 0; i < 90; i++ {
			waitgrp.Add(1)
			go func() {
				defer waitgrp.Done()
				err := auditor.ProcessEvents(evts...)
				AssertOk(t, err, "failed to process audit events")
			}()
		}
		waitgrp.Wait()
		Assert(t, validateSyslogMsgs(t), "syslog message validation failed")
		if j == 0 {
			ti.getter.Stop() // stop sending events on watch channel after first pass so that reconnectWriters() timer gets a chance to kick in
		}
	}
	ti.getter.Start() // start sending events on watch channel
}

func TestProcessEventsErrors(t *testing.T) {
	err := setup()
	defer shutdown()
	AssertOk(t, err, "failed to create udp servers")
	auditor := NewSynchAuditor(nil, ti.logger, WithPolicyGetter(ti.getter), WithReportError(true))
	err = auditor.Run()
	AssertOk(t, err, "failed to initialize auditor")
	defer auditor.Shutdown()
	tn := time.Now()
	ts, _ := types.TimestampProto(tn)
	evts := []*auditapi.AuditEvent{
		{
			TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
			ObjectMeta: api.ObjectMeta{Name: "auditevent-1", UUID: uuid.NewV4().String(), Tenant: mock.ErrorTenant, CreationTime: api.Timestamp{Timestamp: *ts}},
			EventAttributes: auditapi.EventAttributes{
				Level:       auditapi.Level_RequestResponse.String(),
				Stage:       auditapi.Stage_RequestProcessing.String(),
				User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
				Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
				ClientIPs:   []string{"192.168.75.133"},
				Action:      auth.Permission_Create.String(),
				Outcome:     auditapi.Outcome_Success.String(),
				GatewayNode: "node2",
				GatewayIP:   "192.168.75.139",
				Data:        make(map[string]string),
			},
		},
	}
	time.Sleep(3 * time.Second)
	err = auditor.ProcessEvents(evts...)
	Assert(t, err.Error() == "simulate error in GetAuditPolicy", fmt.Sprintf("unexpected error in processing audit events: %v", err))
}

func TestSyslogServerInaccessible(t *testing.T) {
	err := setup()
	defer shutdown()
	AssertOk(t, err, "failed to create udp servers")
	auditor := NewSynchAuditor(nil, ti.logger, WithPolicyGetter(ti.getter), WithReportError(true))
	err = auditor.Run()
	AssertOk(t, err, "failed to initialize auditor")
	defer auditor.Shutdown()
	evts := []*auditapi.AuditEvent{
		{
			TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
			ObjectMeta: api.ObjectMeta{Name: "auditevent-1", UUID: uuid.NewV4().String(), Tenant: "testtenant1"},
			EventAttributes: auditapi.EventAttributes{
				Level:       auditapi.Level_RequestResponse.String(),
				Stage:       auditapi.Stage_RequestProcessing.String(),
				User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
				Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
				ClientIPs:   []string{"192.168.75.133"},
				Action:      auth.Permission_Create.String(),
				Outcome:     auditapi.Outcome_Success.String(),
				GatewayNode: "node2",
				GatewayIP:   "192.168.75.139",
				Data:        make(map[string]string),
			},
		},
	}
	time.Sleep(3 * time.Second)
	var duration time.Duration
	for i := 0; i < 10000; i++ {
		tn := time.Now()
		err = auditor.ProcessEvents(evts...)
		duration += time.Now().Sub(tn)
		AssertOk(t, err, "unexpected error logging to syslog")
	}
	t.Logf("processing event took: %v", duration/10000)
}
