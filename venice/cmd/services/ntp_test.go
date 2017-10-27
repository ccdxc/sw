package services

import (
	"io/ioutil"
	"os"
	"strings"
	"testing"

	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
)

type mockFile struct {
	t           *testing.T
	tmpFileName string
}

type ntpTest struct {
	l *mock.LeaderService
	s types.SystemdService
	n types.NtpService
	f *mockFile
}

func (m *mockFile) mockFileOpen(desiredFilename string, flag int, perms os.FileMode) (*os.File, error) {
	if m.tmpFileName != "" {
		return os.OpenFile(m.tmpFileName, flag, perms)
	}
	tmpfile, err := ioutil.TempFile("", "ntpTest")
	if err != nil {
		m.t.Fatalf("error %v failed creating tmpfile %v", err, m.tmpFileName)
		return nil, nil
	}
	m.tmpFileName = tmpfile.Name()
	return tmpfile, nil
}

func setupNtp(t *testing.T) *ntpTest {
	l := mock.NewLeaderService(t.Name())
	s := NewSystemdService(WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	f := mockFile{t: t}
	n := NewNtpService([]string{"server1", "server2"}, WithLeaderSvcNtpOption(l), WithSystemdSvcNtpOption(s), WithOpenFileNtpOption(f.mockFileOpen))
	return &ntpTest{l: l, s: s, f: &f, n: n}
}

func cleanupNtp(ntpT *ntpTest) {
	ntpT.n.Stop()
	os.Remove(ntpT.f.tmpFileName)
}

func checkNtpNodeConfig(t *testing.T, ntpT *ntpTest) {
	content, err := ioutil.ReadFile(ntpT.f.tmpFileName)
	if err != nil {
		t.Errorf("error %v reading temp file", err)
		return
	}
	if strings.Count(string(content[:]), "server ") != 1 {
		t.Errorf("didnt find 1 servers in config. found %v", content)
		return
	}
	if strings.Index(string(content[:]), "server "+t.Name()+"something iburst") == -1 {
		t.Errorf("something not present in config. found %v", string(content[:]))
		return
	}
}

func checkNtpMasterConfig(t *testing.T, ntpT *ntpTest) {
	content, err := ioutil.ReadFile(ntpT.f.tmpFileName)
	if err != nil {
		t.Errorf("error %v reading temp file %v", err, ntpT.f.tmpFileName)
		return
	}
	if strings.Count(string(content[:]), "server ") != 2 {
		t.Errorf("didnt find 2  servers in config. found %v", content)
		return
	}
	if strings.Index(string(content[:]), "server server1 iburst") == -1 {
		t.Errorf("server1 not present in config. found %v", content)
		return
	}
	if strings.Index(string(content[:]), "server server2 iburst") == -1 {
		t.Errorf("server2 not present in config. found %v", content)
		return
	}
}
func TestNtpServiceGiveupLeadership(t *testing.T) {
	t.Parallel()
	ntpT := setupNtp(t)

	ntpT.s.Start()
	ntpT.l.Start()
	ntpT.n.Start()

	checkNtpMasterConfig(t, ntpT)

	ntpT.l.GiveupLeadership()
	checkNtpNodeConfig(t, ntpT)

	cleanupNtp(ntpT)
}
