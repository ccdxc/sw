package ipif

import (
	"net"
	"sync"
	"time"

	"github.com/vishvananda/netlink"

	"github.com/krolaw/dhcp4"
	"github.com/pensando/dhcp-client"

	"github.com/pensando/sw/nic/agent/nmd/api"
)

// IPClientIf Implements NMD IP Client
type IPClientIf interface {
	DoStaticConfig() (string, error)
	DoDHCPConfig() error
	DoNTPSync() error
	GetIPClientIntf() string
	GetDHCPState() string
}

// IPClient helps to set the IP address of the management interfaces
type IPClient struct {
	intf      netlink.Link
	nmd       api.NmdAPI
	dhcpState *DHCPState
}

type dhcpState int

const (
	none dhcpState = iota
	dhcpSent
	dhcpTimedout
	missingVendorAttributes
	dhcpDone
)

func (s dhcpState) String() string {
	return [...]string{"none", "dhcpSent", "dhcpTimedout", "missingVendorAttributes", "dhcpDone"}[s]
}

// DHCPState captures all dhcp state information
type DHCPState struct {
	sync.Mutex
	nmd           api.NmdAPI
	Client        *dhcp.Client
	IPNet         net.IPNet
	GwIP          net.IP
	VeniceIPs     map[string]bool
	LeaseDuration time.Duration
	AckPacket     dhcp4.Packet
	CurState      string
}
