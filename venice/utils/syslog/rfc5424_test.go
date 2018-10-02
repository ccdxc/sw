package syslog

import (
	"fmt"
	"net"
	"os"
	"runtime"
	"strings"
	"testing"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/syslog/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

func TestSyslogNRfc5424(t *testing.T) {
	_, err := NewRfc5424("invalid", "", LogFtp, "host1", "app1") // invalid network
	Assert(t, err != nil && strings.Contains(err.Error(), "network invalid"), "expected network invalid error")

	_, err = NewRfc5424("tcp", "invalid", LogFtp, "host1", "app1") // invalid remote addr
	Assert(t, err != nil && strings.Contains(err.Error(), "address invalid"), "expected address invalid error")

	_, err = NewRfc5424("tcp", "", LogFtp, "host1", "app1") // missing remote addr
	Assert(t, err != nil && strings.Contains(err.Error(), "missing address"), "expected missing address error")

	s, err := NewRfc5424("", "", LogFtp, "host1", "app1") // empty network and raddr
	AssertOk(t, err, "failed to create new syslog writer")
	s.Close()

	s, err = NewRfc5424("", "", LogFtp, "", "app1") // empty hostname
	AssertOk(t, err, "failed to create new syslog writer")
	s.Close()

	s, err = NewRfc5424("", "", LogFtp, "", "") // empty app name
	AssertOk(t, err, "failed to create new syslog writer")
	s.Close()
}

func TestPrio(t *testing.T) {
	c := gomock.NewController(t)
	defer c.Finish()

	conn := mock.NewMockConn(c)
	pid := os.Getpid()

	s, err := NewRfc5424("", "", LogLocal1, "host1", "app1", WithTimeFn(func() string { return "2018-02-12T16:24:52-08:00" }), WithConn(conn))
	AssertOk(t, err, "failed to create new syslog")
	conn.EXPECT().Close().Return(nil).Times(1)
	s.Close()

	conn.EXPECT().Write([]byte(fmt.Sprintf("<142>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Info(&Message{"msg1", StrData{"id1": {"k1": "v1"}}, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<143>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Debug(&Message{"msg1", StrData{"id1": {"k1": "v1"}}, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<139>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Err(&Message{"msg1", StrData{"id1": {"k1": "v1"}}, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<138>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Crit(&Message{"msg1", StrData{"id1": {"k1": "v1"}}, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<136>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Emerg(&Message{"msg1", StrData{"id1": {"k1": "v1"}}, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<141>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Notice(&Message{"msg1", StrData{"id1": {"k1": "v1"}}, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<136>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Emerg(&Message{"msg1", nil, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<137>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Alert(&Message{"msg1", nil, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<138>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Crit(&Message{"msg1", nil, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<139>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Err(&Message{"msg1", nil, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<140>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Warning(&Message{"msg1", nil, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<141>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Notice(&Message{"msg1", nil, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<142>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Info(&Message{"msg1", nil, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<143>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Debug(&Message{"msg1", nil, "noop"})

	conn.EXPECT().Write([]byte(fmt.Sprintf("<143>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Debug(&Message{"msg1", nil, "noop\n"})

	conn.EXPECT().Close().Times(1)
	s.Close()
}

func TestAppName(t *testing.T) {
	c := gomock.NewController(t)
	defer c.Finish()

	conn := mock.NewMockConn(c)
	s, err := NewRfc5424("", "", LogLocal1, "h1", "", WithTimeFn(func() string { return "2018-02-12T16:24:52-08:00" }), WithConn(conn))
	AssertOk(t, err, "failed to create new syslog")
	conn.EXPECT().Close().Return(nil).Times(1)
	s.Close()

	pid := os.Getpid()
	conn.EXPECT().Write([]byte(fmt.Sprintf("<142>1 2018-02-12T16:24:52-08:00 h1 - %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Info(&Message{MsgID: "msg1", StructuredData: StrData{"id1": {"k1": "v1"}}, Msg: "noop"})
}

// TestSyslogRfc5424WithTCPServer tests syslog with dummy TCP server acting as the syslog server
func TestSyslogRfc5424WithTCPServer(t *testing.T) {
	listener, rCh, err := serviceutils.StartTCPServer("127.0.0.1:0")
	AssertOk(t, err, "failed to start TCP server")
	defer listener.Close()

	s, err := NewRfc5424("tcp", listener.Addr().String(), LogFtp, "host1", "app1")
	AssertOk(t, err, "failed to create new syslog")
	defer s.Close()

	testSyslogMessageDelivery(t, monitoring.MonitoringExportFormat_SYSLOG_RFC5424, s, rCh)
}

// TestSyslogRfc5424WithUDPServer tests syslog with dummy UDP server acting as the syslog server
func TestSyslogRfc5424WithUDPServer(t *testing.T) {
	pConn, rCh, err := serviceutils.StartUDPServer("127.0.0.1:0")
	AssertOk(t, err, "failed to start UDP server")
	defer pConn.Close()

	s, err := NewRfc5424("udp", pConn.LocalAddr().String(), LogFtp, "tag1", "app1")
	AssertOk(t, err, "failed to create new syslog")
	defer s.Close()

	testSyslogMessageDelivery(t, monitoring.MonitoringExportFormat_SYSLOG_RFC5424, s, rCh)
}

func TestMain(m *testing.M) {
	var sockPath string

	if runtime.GOOS == "darwin" {
		sockPath = "/var/run/syslog"
	} else {
		sockPath = "/dev/log"
	}

	if _, err := os.Stat(sockPath); err != nil {
		if os.IsNotExist(err) != true {
			fmt.Printf("failed to access syslog socket, err: %s \n", err)
			os.Exit(2)
		}
		ln, err := net.Listen("unix", sockPath)
		if err != nil {
			fmt.Printf("failed to setup syslog socket, err: %s \n", err)
			os.Exit(2)
		}
		defer ln.Close()
		defer os.Remove(sockPath)
	}
	os.Exit(m.Run())
}
