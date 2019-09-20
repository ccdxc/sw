package services

import (
	"bytes"
	"io"
	"os"
	"reflect"
	"sort"
	"strings"
	"sync"
	"text/template"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
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
rtconutc
local stratum 10
allow
`

type ntpService struct {
	externalNtpServers []string
	quorumNodes        []string
	sync.Mutex
	enabled  bool
	leader   string // non-leaders listen to leader for time
	sysSvc   types.SystemdService
	openFile func(string, int, os.FileMode) (*os.File, error)
	nodeID   string
	ntpConf  string
}

// NtpOption fills the optional params
type NtpOption func(service *ntpService)

// WithOpenFileNtpOption to pass a specifc OpenFile implementation
func WithOpenFileNtpOption(openFile func(string, int, os.FileMode) (*os.File, error)) NtpOption {
	return func(n *ntpService) {
		n.openFile = openFile
	}
}

// WithSystemdSvcNtpOption to pass a specifc types.SystemdService implementation
func WithSystemdSvcNtpOption(sysSvc types.SystemdService) NtpOption {
	return func(n *ntpService) {
		n.sysSvc = sysSvc
	}
}

// NewNtpService creates a new NTP service.
func NewNtpService(externalNtpServers []string, envQuorumNodes []string, nodeID string, options ...NtpOption) types.NtpService {
	n := ntpService{
		externalNtpServers: make([]string, len(externalNtpServers)),
	}
	sort.Strings(externalNtpServers)
	copy(n.externalNtpServers, externalNtpServers)
	for _, o := range options {
		if o != nil {
			o(&n)
		}
	}
	if n.openFile == nil {
		n.openFile = os.OpenFile
	}
	if n.sysSvc == nil {
		n.sysSvc = env.SystemdService
	}
	n.nodeID = nodeID
	n.quorumNodes = utils.GetOtherQuorumNodes(envQuorumNodes, nodeID)
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
		log.Errorln("Unable to create ntp config file: ", err)
		return
	}
	defer f.Close()
	var buf bytes.Buffer
	w := io.MultiWriter(f, &buf)
	err = tmpl.Execute(w, nc)
	if err != nil {
		log.Errorf("error %v while writing to ntp config file", err)
		return
	}
	n.ntpConf = buf.String()
	n.restartNtpd()
}

func (n *ntpService) Start() {
	if n.enabled {
		return
	}

	n.sysSvc.Start()
	n.Lock()
	defer n.Unlock()
	n.enabled = true
	if n.isLeader() {
		n.leaderConfig()
	}
}

func (n *ntpService) Stop() {
	n.Lock()
	defer n.Unlock()
	n.enabled = false
	n.memberConfig()
}

// UpdateServerList updates NTP Servers list
func (n *ntpService) UpdateServerList(externalNtpServers []string) {
	var newNtpServers []string
	newNtpServers = append(newNtpServers, externalNtpServers...)
	sort.Strings(newNtpServers)
	// Compare to find out if the NTP Servers list has been updated
	// If so, change config file and restart NTP service on the node
	n.Lock()
	defer n.Unlock()
	if reflect.DeepEqual(n.externalNtpServers, newNtpServers) == false {
		n.externalNtpServers = newNtpServers
		if n.isLeader() {
			n.leaderConfig()
		}
	}
	return
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
	ntpServers := make([]string, 0)
	ntpServers = append(ntpServers, n.externalNtpServers...)
	ntpServers = append(ntpServers, n.quorumNodes...)
	n.NtpConfigFile(ntpServers)
}

func (n *ntpService) memberConfig() {
	n.NtpConfigFile(n.quorumNodes)
}

func (n *ntpService) UpdateNtpConfig(leader string) {
	n.Lock()
	defer n.Unlock()
	// check if ntp conf contains external servers
	var isLeaderConf bool
	for _, server := range n.externalNtpServers {
		if strings.Contains(n.ntpConf, server) {
			isLeaderConf = true
			break
		}
	}
	// if node was not a leader and new leader is not this node then no need to update config
	if n.ntpConf != "" && !isLeaderConf && !n.isLeader() && leader != n.nodeID {
		n.leader = leader
		return
	}
	n.leader = leader
	if !n.isLeader() {
		n.memberConfig()
	} else {
		n.leaderConfig()
	}
}

func (n *ntpService) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	log.Infof("got leader event %#v, type %v, leader %v, node %v", e, e.Evt, e.Leader, n.nodeID)
	n.UpdateNtpConfig(e.Leader)
	return nil
}

func (n *ntpService) isLeader() bool {
	return n.nodeID == n.leader
}
