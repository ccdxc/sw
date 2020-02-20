package syslog

import (
	"context"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestBSDServer(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	for _, proto := range []string{"udp", "tcp"} {
		addr, ch, err := Server(ctx, "127.0.0.1:", monitoring.MonitoringExportFormat_SYSLOG_BSD.String(), proto)
		w, err := NewBsd(proto, addr, LogLocal0, t.Name(), "tag1")
		AssertOk(t, err, "failed to create BSD client for "+proto)
		m := &Message{
			MsgID: t.Name(),
			Msg:   "Test Message",
		}
		err = w.Info(m)
		AssertOk(t, err, "failed to write syslog")

		AssertEventually(t, func() (bool, interface{}) {
			select {
			case msg := <-ch:
				log.Infof("received syslog %+v", msg)
				return msg["content"] == m.Msg+"\n", msg
			case <-time.After(time.Second):
				return false, "failed to receive syslog"
			}
		}, "failed to receive message", "1s", "60s")
	}
}

func TestRFC5424Server(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	for _, proto := range []string{"udp", "tcp"} {
		addr, ch, err := Server(ctx, "127.0.0.1:", monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(), proto)
		w, err := NewRfc5424(proto, addr, LogLocal0, "localhost", t.Name())
		AssertOk(t, err, "failed to create RFC5424 client for "+proto)
		m := &Message{
			MsgID: t.Name(),
			Msg:   "Test Message",
		}
		err = w.Info(m)
		AssertOk(t, err, "failed to write syslog")

		AssertEventually(t, func() (bool, interface{}) {
			select {
			case msg := <-ch:
				log.Infof("received syslog %+v", msg)
				return msg["message"] == m.Msg+"\n", msg
			case <-time.After(time.Second):
				return false, "failed to receive syslog"
			}
		}, "failed to receive message", "1s", "60s")
	}
}

func TestParseErr(t *testing.T) {
	_, err := parseRfc3164(nil)
	Assert(t, err != nil, "parser test failed")
	_, err = parseRfc5424(nil)
	Assert(t, err != nil, "parser test failed")
	_, err = parseSyslog(monitoring.MonitoringExportFormat_SYSLOG_BSD.String(), nil)
	Assert(t, err != nil, "parser test failed")
	_, err = parseSyslog(monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(), nil)
	Assert(t, err != nil, "parser test failed")
}

func TestServerErr(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	_, _, err := Server(ctx, "127.0.0.1:", monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(), "ipv4")
	Assert(t, err != nil, "syslog server didn't fail for invalid protocol")
}
