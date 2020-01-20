package services

import (
	"io/ioutil"
	"os"
	"strings"
	"testing"

	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type mockFile struct {
	t           *testing.T
	tmpFileName string
}

type ntpTest struct {
	s types.SystemdService
	n *ntpService
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

func setupNtp(t *testing.T, extServers, quorumNodes []string, nodeID string) *ntpTest {
	s := NewSystemdService(WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	f := mockFile{t: t}
	n := NewNtpService(extServers, quorumNodes, nodeID, WithSystemdSvcNtpOption(s), WithOpenFileNtpOption(f.mockFileOpen))
	return &ntpTest{s: s, f: &f, n: n.(*ntpService)}
}

func cleanupNtp(ntpT *ntpTest) {
	ntpT.n.Stop()
	os.Remove(ntpT.f.tmpFileName)
}

func checkNtpNodeConfig(t *testing.T, ntpT *ntpTest) {
	content, err := ioutil.ReadFile(ntpT.f.tmpFileName)
	conf := string(content[:])
	AssertOk(t, err, "error reading temp file %s", ntpT.f.tmpFileName)
	Assert(t, strings.Contains(conf, "server "+ntpT.n.leader), "did not find leader in node config %s", conf)
	Assert(t, strings.Count(conf, "server ") == 1, "expected exactly 1 servers in node config %v", conf)
}

func checkNtpMasterConfig(t *testing.T, ntpT *ntpTest) {
	content, err := ioutil.ReadFile(ntpT.f.tmpFileName)
	conf := string(content[:])
	AssertOk(t, err, "error reading temp file %s", ntpT.f.tmpFileName)
	for _, s := range ntpT.n.externalNtpServers {
		Assert(t, strings.Contains(conf, s), "did not find external server %s in master node config: %v", s, conf)
	}
	for _, n := range ntpT.n.quorumNodes {
		Assert(t, !strings.Contains(conf, n), "found unexpected quorum node %s in master node config: %v", n, conf)
	}
}

func TestNtpServiceGiveupLeadership(t *testing.T) {
	extServers := []string{"cmd-ntp-test-ext-srv1", "cmd-ntp-test-ext-srv2"}
	quorumNodes := []string{"cmd-ntp-test-quorum1", "cmd-ntp-test-quorum2"}

	ntpT := setupNtp(t, extServers, quorumNodes, quorumNodes[0])
	defer cleanupNtp(ntpT)
	ntpT.n.Start()

	ntpT.n.UpdateNtpConfig(quorumNodes[0])
	checkNtpMasterConfig(t, ntpT)

	ntpT.n.UpdateNtpConfig(quorumNodes[1])
	checkNtpNodeConfig(t, ntpT)

	ntpT.n.UpdateNtpConfig(quorumNodes[0])
	checkNtpMasterConfig(t, ntpT)
}

func TestNtpServiceCheckIfServersUpdated(t *testing.T) {
	extServers := []string{"cmd-ntp-test-ext-srv1", "cmd-ntp-test-ext-srv2"}
	quorumNodes := []string{"cmd-ntp-test-quorum1", "cmd-ntp-test-quorum2"}

	ntpT := setupNtp(t, extServers, quorumNodes, quorumNodes[0])
	defer cleanupNtp(ntpT)
	ntpT.n.Start()

	ntpT.n.UpdateNtpConfig(quorumNodes[0])
	checkNtpMasterConfig(t, ntpT)

	extServers = []string{"cmd-ntp-test-ext-srv3", "cmd-ntp-test-ext-srv4"}
	ntpT.n.UpdateServerList(extServers)
	checkNtpMasterConfig(t, ntpT)
}
