package ipif

import (
	"context"
	"net"
	"sync"
	"time"

	"github.com/pensando/netlink"

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
	StopDHCPConfig()
	GetInterfaceIPs() []InterfaceIP
	GetStaticRoutes() []StaticRoute
}

// IPClient helps to set the IP address of the management interfaces
type IPClient struct {
	primaryIntf         netlink.Link
	secondaryInterfaces []netlink.Link
	nmd                 api.NmdAPI
	dhcpState           *DHCPState
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
	nmd                     api.NmdAPI
	pipeline                string
	PrimaryIntfClient       *dhcp.Client
	SecondaryIntfClients    map[int]*dhcp.Client
	IPNet                   net.IPNet
	GwIP                    net.IP
	VeniceIPs               map[string]bool
	InterfaceIPs            []InterfaceIP
	StaticRoutes            []StaticRoute
	LeaseDuration           time.Duration
	AckPacket               dhcp4.Packet
	CurState                string
	DhcpCtx, RenewCtx       context.Context
	DhcpCancel, RenewCancel context.CancelFunc
	Hostname                string
}

// InterfaceIP captures the ip address info for the uplink interfaces supplied by the dhcp server
type InterfaceIP struct {
	IfID      int8
	PrefixLen int8
	IPAddress net.IP
	GwIP      net.IP
}

// StaticRoute captures the static route info for the Distributed Service Card
type StaticRoute struct {
	DestAddr      net.IP
	DestPrefixLen uint32
	NextHopAddr   net.IP
}
