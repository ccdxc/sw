package services

import (
	"os"
	"sync"
	"text/template"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
)

type ntpParams struct {
	NtpServer []string
}

const chronydConfigFile = `
{{range .NtpServer}}
server {{.}} iburst
initstepslew 10 {{.}}
{{end}}

makestep 1 3
rtcsync
local stratum 10
allow
`

type ntpService struct {
	externalNtpServers []string

	sync.Mutex
	isLeader  bool
	enabled   bool
	leader    string // non-leaders listen to leader for time
	leaderSvc types.LeaderService
	sysSvc    types.SystemdService
	openFile  func(string, int, os.FileMode) (*os.File, error)
}

// NtpOption fills the optional params
type NtpOption func(service *ntpService)

const (
	ntpLeaderKey = "ntp"
)

// WithOpenFileNtpOption to pass a specifc OpenFile implementation
func WithOpenFileNtpOption(openFile func(string, int, os.FileMode) (*os.File, error)) NtpOption {
	return func(n *ntpService) {
		n.openFile = openFile
	}
}

// WithLeaderSvcNtpOption to pass a specifc types.LeaderService implementation
func WithLeaderSvcNtpOption(leaderSvc types.LeaderService) NtpOption {
	return func(n *ntpService) {
		n.leaderSvc = leaderSvc
	}
}

// WithSystemdSvcNtpOption to pass a specifc types.SystemdService implementation
func WithSystemdSvcNtpOption(sysSvc types.SystemdService) NtpOption {
	return func(n *ntpService) {
		n.sysSvc = sysSvc
	}
}

// NewNtpService creates a new NTP service.
func NewNtpService(externalNtpServers []string, options ...NtpOption) types.NtpService {
	n := ntpService{
		externalNtpServers: make([]string, len(externalNtpServers)),
	}
	copy(n.externalNtpServers, externalNtpServers)
	for _, o := range options {
		if o != nil {
			o(&n)
		}
	}
	if n.openFile == nil {
		n.openFile = os.OpenFile
	}
	if n.leaderSvc == nil {
		hostname, _ := os.Hostname()
		n.leaderSvc = NewLeaderService(env.KVStore, ntpLeaderKey, hostname)
	}
	if n.sysSvc == nil {
		n.sysSvc = env.SystemdService
	}

	return &n
}

// NtpConfigFile writes config file with servers to synchronize the time to
func (n *ntpService) NtpConfigFile(servers []string) {
	tmpl, err := template.New("test").Parse(chronydConfigFile)
	if err != nil {
		panic(err)
	}
	nc := ntpParams{NtpServer: servers}

	f, err := n.openFile(globals.NtpConfigFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0600)
	if err != nil {
		log.Println("Unable to create ntp config file file: ", err)
		return
	}
	defer f.Close()
	err = tmpl.Execute(f, nc)
	if err != nil {
		log.Printf("error %v while writing to ntp config file", err)
		return
	}
}

func (n *ntpService) Start() {
	if n.enabled {
		return
	}

	n.leaderSvc.Register(n)

	n.leaderSvc.Start()
	n.sysSvc.Start()

	n.Lock()
	defer n.Unlock()
	if n.leaderSvc.IsLeader() {
		n.isLeader = true
	}
	n.enabled = true
	if n.isLeader {
		n.leaderConfig()
	}
}

func (n *ntpService) Stop() {
	n.leaderSvc.UnRegister(n)
	n.Lock()
	defer n.Unlock()
	n.enabled = false
	n.memberConfig()
}

func (n *ntpService) restartNtpd() {
	// use systemd to reload the NTP daemon running on this instance
	err := n.sysSvc.StartUnit("pen-ntpconfig.service")
	if err != nil {
		log.Printf("error %v while restarting ntp config file", err)
		return
	}
}

func (n *ntpService) leaderConfig() {
	n.NtpConfigFile(n.externalNtpServers)
	n.restartNtpd()
}

func (n *ntpService) memberConfig() {
	if n.leader != "" {
		n.NtpConfigFile([]string{n.leader})
		n.restartNtpd()
	}
}

func (n *ntpService) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	var err error
	switch e.Evt {
	case types.LeaderEventChange:
		n.Lock()
		defer n.Unlock()
		n.isLeader = false
		n.leader = e.Leader
		if n.enabled {
			n.memberConfig()
		}
	case types.LeaderEventWon:
		n.Lock()
		defer n.Unlock()
		n.isLeader = true
		if n.enabled {
			n.leaderConfig()
		}
	case types.LeaderEventLost:
		n.Lock()
		defer n.Unlock()
		n.isLeader = false
		n.leader = e.Leader
		if n.enabled {
			n.memberConfig()
		}
	}
	return err
}
