package exporters

import (
	"fmt"
	"strings"
	"sync"
	"testing"

	"github.com/google/uuid"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/syslog"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	syslogBufferLen = 30
	sLogConfig      = &log.Config{
		Module:      "syslog_exporter_test",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       true,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
		FileCfg:     log.FileConfig{},
	}
	sLogger = log.GetNewLogger(sLogConfig)
)

// TestSyslogEventsExporter tests the syslog events exporter
func TestSyslogEventsExporter(t *testing.T) {
	// start UDP servers to receive syslog messages
	pConn1, receivedMsgsAtUDPServer1, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn1.Close()

	tmp1 := strings.Split(pConn1.LocalAddr().String(), ":")
	pConn2, receivedMsgsAtUDPServer2, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn2.Close()
	tmp2 := strings.Split(pConn2.LocalAddr().String(), ":")

	// create syslog writers
	writer1, err := syslog.NewBsd("udp",
		fmt.Sprintf("127.0.0.1:%s", tmp1[len(tmp1)-1]),
		syslog.LogUser,
		"pen-events")
	AssertOk(t, err, "failed to create syslog BSD writer, err: %v", err)

	writer2, err := syslog.NewRfc5424("udp",
		fmt.Sprintf("127.0.0.1:%s", tmp2[len(tmp2)-1]),
		syslog.LogUser,
		utils.GetHostname(),
		"pen-events")
	AssertOk(t, err, "failed to create syslog BSD writer, err: %v", err)

	// create syslog exporter
	sExporter, err := NewSyslogExporter(t.Name(), syslogBufferLen, []syslog.Writer{writer1, writer2}, sLogger)
	AssertOk(t, err, "failed to create syslog exporter, err: %v", err)
	defer sExporter.Stop()

	// create mock events channel; in the real case, it will come from the dispatcher
	mockEventsChan := &mockEventChanImpl{
		result:  make(chan events.Batch, syslogBufferLen),
		stopped: make(chan struct{}),
	}

	mockOffsetTracker := &mockOffsetTrackerImpl{}
	sExporter.Start(mockEventsChan, mockOffsetTracker)
	Assert(t, sExporter.Name() == t.Name(), "expected exporter name: %s, got :%s", t.Name(), sExporter.Name())
	Assert(t, sExporter.ChLen() == syslogBufferLen, "expected channel length: %s, got: %s", syslogBufferLen, sExporter.ChLen())

	var wg sync.WaitGroup
	wg.Add(2)
	// ensure all the events are received by the syslog server - 1
	go func() {
		defer wg.Done()
		totalReceivedMessages := 0
		for {
			msg, _ := <-receivedMsgsAtUDPServer1
			if syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_BSD, msg) {
				totalReceivedMessages++
			}
			if totalReceivedMessages == 10 {
				return
			}
		}
	}()

	// ensure all the events are received by the syslog server - 2
	go func() {
		defer wg.Done()
		totalReceivedMessages := 0
		for {
			msg, _ := <-receivedMsgsAtUDPServer2
			if syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_RFC5424, msg) {
				totalReceivedMessages++
			}
			if totalReceivedMessages == 10 {
				return
			}
		}
	}()

	// send some events to the syslog exporter
	for i := 0; i < 10; i++ {
		temp := *dummyEvt
		temp.EventAttributes.Type = "TEST-1"
		temp.ObjectMeta.UUID = uuid.New().String()

		// send events to mock events chan
		mockEventsChan.result <- newMockBatch([]*evtsapi.Event{&temp}, 0)
	}

	wg.Wait()
}
