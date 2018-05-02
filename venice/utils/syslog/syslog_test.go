package syslog

import (
	"fmt"
	"os"
	"testing"

	"github.com/golang/mock/gomock"

	"net"

	mock "github.com/pensando/sw/venice/utils/syslog/mock"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

func TestSyslogNRfc5424WithNoHostname(t *testing.T) {
	s, err := NewRfc5424(LogFtp, "", "app1")
	tu.AssertOk(t, err, "failed to create new syslog")
	defer s.conn.Close()
}

func TestSyslogNRfc5424(t *testing.T) {
	s, err := NewRfc5424(LogFtp, "host1", "app1")
	tu.AssertOk(t, err, "failed to create new syslog")
	defer s.conn.Close()
}

func TestSyslogBsd(t *testing.T) {
	s, err := NewBsd(LogLocal2, "tag1")
	tu.AssertOk(t, err, "failed to create new syslog")
	defer s.Close()
	s.Info("bsd style syslog")
}

func TestPrio(t *testing.T) {
	s, err := NewRfc5424(LogLocal1, "host1", "app1")
	tu.AssertOk(t, err, "failed to create new syslog")
	s.conn.Close()

	c := gomock.NewController(t)
	defer c.Finish()
	s.getTime = func() string { return "2018-02-12T16:24:52-08:00" }

	conn := mock.NewMockConn(c)
	s.conn = conn

	pid := os.Getpid()
	conn.EXPECT().Write([]byte(fmt.Sprintf("<142>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Info("msg1", StrData{"id1": {"k1": "v1"}}, "noop")
	conn.EXPECT().Write([]byte(fmt.Sprintf("<143>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Debug("msg1", StrData{"id1": {"k1": "v1"}}, "noop")
	conn.EXPECT().Write([]byte(fmt.Sprintf("<139>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Err("msg1", StrData{"id1": {"k1": "v1"}}, "noop")

	conn.EXPECT().Write([]byte(fmt.Sprintf("<138>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Crit("msg1", StrData{"id1": {"k1": "v1"}}, "noop")

	conn.EXPECT().Write([]byte(fmt.Sprintf("<136>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Emerg("msg1", StrData{"id1": {"k1": "v1"}}, "noop")

	conn.EXPECT().Write([]byte(fmt.Sprintf("<141>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Notice("msg1", StrData{"id1": {"k1": "v1"}}, "noop")

	conn.EXPECT().Write([]byte(fmt.Sprintf("<137>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Alert("msg1", nil, "noop")

	conn.EXPECT().Write([]byte(fmt.Sprintf("<140>1 2018-02-12T16:24:52-08:00 host1 app1 %d msg1 - noop\n", pid))).Return(5, nil).Times(1)
	s.Warning("msg1", nil, "noop")

	conn.EXPECT().Close().Times(1)
	s.Close()
}

func TestAppName(t *testing.T) {
	s, err := NewRfc5424(LogLocal1, "h1", "")
	tu.AssertOk(t, err, "failed to create new syslog")
	s.conn.Close()

	c := gomock.NewController(t)
	defer c.Finish()
	s.getTime = func() string {
		return "2018-02-12T16:24:52-08:00"
	}

	conn := mock.NewMockConn(c)
	s.conn = conn

	pid := os.Getpid()
	conn.EXPECT().Write([]byte(fmt.Sprintf("<142>1 2018-02-12T16:24:52-08:00 h1 - %d msg1 [id1 k1=\"v1\"] noop\n", pid))).Return(5, nil).Times(1)
	s.Info("msg1", StrData{"id1": {"k1": "v1"}}, "noop")
}

func TestMain(m *testing.M) {
	sockPath := "/dev/log"

	if _, err := os.Stat(sockPath); err != nil {
		if os.IsNotExist(err) != true {
			fmt.Printf("+++failed to access syslog socket, err: %s \n", err)
			os.Exit(2)
		}
		ln, err := net.Listen("unix", sockPath)
		if err != nil {
			fmt.Printf("+++failed to setup syslog socket, err: %s \n", err)
			os.Exit(2)
		}
		defer ln.Close()
		defer os.Remove(sockPath)
	}
	os.Exit(m.Run())
}
