package exporter

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	mockapi "github.com/pensando/sw/api/mock"
	eapiclient "github.com/pensando/sw/venice/ctrler/evtsmgr/apiclient"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/syslog"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

// tests the alert exporter
func TestExporter(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	mAPI := mockapi.NewMockServices(ctrl)
	mMonitoring := &mockMonitoringV1{}
	mAlertDestination := mockapi.NewMockMonitoringV1AlertDestinationInterface(ctrl)
	mMonitoring.mAlertDestination = mAlertDestination

	memDb := memdb.NewMemDb()
	exporter := NewAlertExporter(memDb, &eapiclient.ConfigWatcher{}, log.GetNewLogger(log.GetDefaultConfig("alert_exporter_test")))
	defer exporter.Stop()
	stop := make(chan struct{})

	// UDP server - 1
	pConn1, receivedMsgsAtUDPServer1, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn1.Close()
	tmp1 := strings.Split(pConn1.LocalAddr().String(), ":")

	// UDP server - 2
	pConn2, receivedMsgsAtUDPServer2, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn2.Close()
	tmp2 := strings.Split(pConn2.LocalAddr().String(), ":")

	alertDestUUID := uuid.NewV1().String()
	alertDestBSDSyslog := policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, alertDestUUID,
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_vname[int32(monitoring.MonitoringExportFormat_SYSLOG_BSD)],
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("udp/%s", tmp1[len(tmp1)-1]),
				},
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("udp/%s", tmp2[len(tmp2)-1]),
				},
			},
			Config: &monitoring.SyslogExportConfig{
				Prefix:           "pen-events-exporter-test",
				FacilityOverride: monitoring.SyslogFacility_vname[int32(monitoring.SyslogFacility_LOG_SYSLOG)],
			},
		})
	alertDestBSDSyslog.ObjectMeta.ModTime = api.Timestamp{}
	alertDestBSDSyslog.ObjectMeta.CreationTime = api.Timestamp{}
	err = memDb.AddObject(alertDestBSDSyslog)
	AssertOk(t, err, "Add object failed %v", err)

	alert := policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5), monitoring.AlertState_OPEN, "test-alert1", nil, nil, nil)

	totalExports := struct {
		sync.Mutex
		count int
	}{}
	totalReceived := struct {
		sync.Mutex
		count map[monitoring.MonitoringExportFormat]int
	}{count: map[monitoring.MonitoringExportFormat]int{}}

	go func() { // keep exporting alert every 60ms

		for {
			select {
			case <-stop:
				return
			case <-time.After(60 * time.Millisecond):
				ad := memDb.GetAlertDestination(alertDestBSDSyslog.GetObjectMeta().GetName())
				if ad != nil && ad.Spec.SyslogExport != nil {
					numTargets := len(ad.Spec.SyslogExport.Targets)
					go func() {
						getC := mAlertDestination.EXPECT().Get(gomock.Any(), alertDestBSDSyslog.GetObjectMeta()).Return(alertDestBSDSyslog, nil).MinTimes(0).MaxTimes(1)
						mAlertDestination.EXPECT().Update(gomock.Any(), alertDestBSDSyslog).Return(alertDestBSDSyslog, nil).MinTimes(0).MaxTimes(1).After(getC)
						mAPI.EXPECT().MonitoringV1().Return(mMonitoring).MinTimes(0).MaxTimes(2)
					}()

					if exporter.Export([]string{alertDestUUID}, alert) != nil {
						continue
					}

					totalExports.Lock()
					totalExports.count += numTargets
					totalExports.Unlock()
				}
			}
		}
	}()

	go func() { // check the messages from dummy UDP server
		for {
			select {
			case msg, ok := <-receivedMsgsAtUDPServer1:
				if !ok {
					return
				}

				totalReceived.Lock()
				if syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_BSD, msg) {
					totalReceived.count[monitoring.MonitoringExportFormat_SYSLOG_BSD]++
				} else if syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_RFC5424, msg) {
					totalReceived.count[monitoring.MonitoringExportFormat_SYSLOG_RFC5424]++
				}
				totalReceived.Unlock()
			}
		}
	}()

	go func() { // check the messages from dummy UDP server
		for {
			select {
			case msg, ok := <-receivedMsgsAtUDPServer2:
				if !ok {
					return
				}

				totalReceived.Lock()
				if syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_BSD, msg) {
					totalReceived.count[monitoring.MonitoringExportFormat_SYSLOG_BSD]++
				} else if syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_RFC5424, msg) {
					totalReceived.count[monitoring.MonitoringExportFormat_SYSLOG_RFC5424]++
				}
				totalReceived.Unlock()
			}
		}
	}()

	time.Sleep(1 * time.Second)

	// update the export from BSD to RFC
	alertDestBSDSyslog = policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, alertDestUUID,
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_vname[int32(monitoring.MonitoringExportFormat_SYSLOG_RFC5424)],
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("udp/%s", tmp1[len(tmp1)-1]),
				},
			},
		})
	alertDestBSDSyslog.ObjectMeta.ModTime = api.Timestamp{}
	alertDestBSDSyslog.ObjectMeta.CreationTime = api.Timestamp{}
	err = memDb.UpdateObject(alertDestBSDSyslog)
	AssertOk(t, err, "Update object failed %v", err)

	time.Sleep(1 * time.Second)

	err = memDb.DeleteObject(alertDestBSDSyslog) // delete alert destination
	AssertOk(t, err, "Delete object failed %v", err)

	close(stop)

	AssertEventually(t,
		func() (bool, interface{}) {
			totalReceived.Lock()
			defer totalReceived.Unlock()

			totalExports.Lock()
			defer totalExports.Unlock()

			totalR := totalReceived.count[monitoring.MonitoringExportFormat_SYSLOG_BSD] + totalReceived.count[monitoring.MonitoringExportFormat_SYSLOG_RFC5424]
			if totalExports.count == totalR {
				return true, nil
			}

			return false, fmt.Sprintf("expected: %v, received: %v", totalExports.count, totalR)
		}, "did not receive all the syslog messages sent", "20ms", "10s")

	totalReceived.Lock()
	defer totalReceived.Unlock()
	Assert(t, totalReceived.count[monitoring.MonitoringExportFormat_SYSLOG_BSD] > 0, "did not receive any BSD format syslog message")
	Assert(t, totalReceived.count[monitoring.MonitoringExportFormat_SYSLOG_RFC5424] > 0, "did not receive any RFC5424 format syslog message")
}

type mockMonitoringV1 struct {
	mAlertDestination monitoring.MonitoringV1AlertDestinationInterface
}

func (m *mockMonitoringV1) EventPolicy() monitoring.MonitoringV1EventPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) FwlogPolicy() monitoring.MonitoringV1FwlogPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) FlowExportPolicy() monitoring.MonitoringV1FlowExportPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) Alert() monitoring.MonitoringV1AlertInterface {
	return nil
}
func (m *mockMonitoringV1) AlertPolicy() monitoring.MonitoringV1AlertPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) StatsAlertPolicy() monitoring.MonitoringV1StatsAlertPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) AlertDestination() monitoring.MonitoringV1AlertDestinationInterface {
	return m.mAlertDestination
}
func (m *mockMonitoringV1) MirrorSession() monitoring.MonitoringV1MirrorSessionInterface {
	return nil
}
func (m *mockMonitoringV1) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}
func (m *mockMonitoringV1) TroubleshootingSession() monitoring.MonitoringV1TroubleshootingSessionInterface {
	return nil
}
func (m *mockMonitoringV1) TechSupportRequest() monitoring.MonitoringV1TechSupportRequestInterface {
	return nil
}
func (m *mockMonitoringV1) ArchiveRequest() monitoring.MonitoringV1ArchiveRequestInterface {
	return nil
}
func (m *mockMonitoringV1) AuditPolicy() monitoring.MonitoringV1AuditPolicyInterface {
	return nil
}
