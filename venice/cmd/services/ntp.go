package services

import (
	"bytes"
	"io"
	"net"
	"os"
	"reflect"
	"sort"
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
{{end}}

makestep 1 0
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
func (n *ntpService) NtpConfigFile(serverNames []string) {
	tmpl, err := template.New("test").Parse(chronydConfigFile)
	if err != nil {
		panic(err)
	}
	// if we are given a DNS name, we should expand it because it could be a pool name
	// like pool.ntp.org and chronyc does not support adding pools, so it would treat it
	// as a single server, which is not good for accuracy and redundancy.
	var servers []string
	for _, n := range serverNames {
		addrs, err := net.LookupHost(n)
		if err == nil {
			servers = append(servers, addrs...)
		} else {
			servers = append(servers, n)
		}
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
	log.Infof("Updated NTP config files: %v", buf.String())
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
	} else {
		n.memberConfig()
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
	// Leader only syncs with external servers, if any.
	// it also advertise its clock to non-leaders but does not sync from non-leaders
	// otherwise we can end up in a "no majority" case if clocks are significantly different
	ntpServers = append(ntpServers, n.externalNtpServers...)
	n.NtpConfigFile(ntpServers)
}

func (n *ntpService) memberConfig() {
	// Members only sync with the leader
	if n.leader == "" {
		// not much we can do in this case
		// pass other quorum nodes so we can stay in sync if they are reachable
		log.Errorf("Error generating NTP config for non-leader node: leader is empty")
		n.NtpConfigFile(n.quorumNodes)
		return
	}
	n.NtpConfigFile([]string{n.leader})
}

func (n *ntpService) UpdateNtpConfig(leader string) {
	n.Lock()
	defer n.Unlock()
	n.leader = leader
	if n.isLeader() {
		n.leaderConfig()
	} else {
		n.memberConfig()
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
