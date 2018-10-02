package syslog

import (
	"fmt"
	"math/rand"
	"strings"
	"sync"
	"testing"

	"github.com/pensando/sw/api/generated/monitoring"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

func TestSyslogBsd(t *testing.T) {
	_, err := NewBsd("invalid", "", LogLocal0, "tag1")
	Assert(t, err != nil && strings.Contains(err.Error(), "network invalid"), "expected network invalid error")
	_, err = NewBsd("tcp", "invalid", LogLocal0, "tag1")
	Assert(t, err != nil && strings.Contains(err.Error(), "address invalid"), "expected address invalid error")

	s, err := NewBsd("", "", LogLocal0, "tag1")
	AssertOk(t, err, "failed to create new syslog")
	AssertOk(t, s.Info(&Message{Msg: "BSD style syslog"}), "failed to send syslog message")
	s.Close()
}

// TestSyslogBsdWithTCPServer tests syslog with dummy TCP server acting as the syslog server
func TestSyslogBsdWithTCPServer(t *testing.T) {
	listener, rCh, err := serviceutils.StartTCPServer("127.0.0.1:0")
	AssertOk(t, err, "failed to start TCP server")
	defer listener.Close()

	s, err := NewBsd("tcp", listener.Addr().String(), LogLocal0, "tag1")
	AssertOk(t, err, "failed to create new syslog")
	defer s.Close()

	testSyslogMessageDelivery(t, monitoring.MonitoringExportFormat_SYSLOG_BSD, s, rCh)
}

// TestSyslogBsdWithUDPServer tests syslog with dummy UDP server acting as the syslog server
func TestSyslogBsdWithUDPServer(t *testing.T) {
	pConn, rCh, err := serviceutils.StartUDPServer("127.0.0.1:0")
	AssertOk(t, err, "failed to start UDP server")
	defer pConn.Close()

	s, err := NewBsd("udp", pConn.LocalAddr().String(), LogLocal0, "tag1")
	AssertOk(t, err, "failed to create new syslog")
	defer s.Close()

	testSyslogMessageDelivery(t, monitoring.MonitoringExportFormat_SYSLOG_BSD, s, rCh)
}

func testSyslogMessageDelivery(t *testing.T, format monitoring.MonitoringExportFormat, s Writer, rCh chan string) {
	syslogMessages := struct {
		sync.RWMutex
		m map[string]struct{}
	}{
		m: map[string]struct{}{},
	}

	// add some more messages
	for i := 0; i < 10; i++ {
		syslogMessages.m[CreateAlphabetString(5)] = struct{}{}
	}

	for msg := range syslogMessages.m {
		switch rand.Int31n(10) {
		case 1:
			AssertOk(t, s.Emerg(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Emerg(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		case 2:
			AssertOk(t, s.Alert(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Alert(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		case 3:
			AssertOk(t, s.Crit(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Crit(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		case 4:
			AssertOk(t, s.Err(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Err(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		case 5:
			AssertOk(t, s.Warning(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Warning(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		case 6:
			AssertOk(t, s.Notice(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Notice(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		case 7:
			AssertOk(t, s.Info(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Info(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		case 8:
			AssertOk(t, s.Debug(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Debug(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		default:
			AssertOk(t, s.Info(&Message{Msg: msg, MsgID: msg, StructuredData: StrData{"id1": {"k1": "v1"}}}), "failed to send syslog message")
			AssertOk(t, s.Info(&Message{Msg: msg, MsgID: msg}), "failed to send syslog message")
		}
	}

	go func() {
		for {
			select {
			case receivedMsg, ok := <-rCh:
				if !ok {
					return
				}

				Assert(t, ValidateSyslogMessage(format, receivedMsg),
					"message did not match %v format, msg: %v", format, receivedMsg)
				syslogMessages.Lock()
				for sentMsg := range syslogMessages.m {
					if strings.Contains(receivedMsg, sentMsg) {
						delete(syslogMessages.m, sentMsg)
					}
				}
				syslogMessages.Unlock()
			}
		}
	}()

	AssertEventually(t,
		func() (bool, interface{}) {
			syslogMessages.RLock()
			defer syslogMessages.RUnlock()
			return len(syslogMessages.m) == 0, nil
		}, fmt.Sprintf("did not receive all the syslog messages sent, pending: %v", len(syslogMessages.m)), "20ms", "10s")
}
