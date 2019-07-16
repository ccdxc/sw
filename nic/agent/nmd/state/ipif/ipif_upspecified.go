// +build !linux

package ipif

import (
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/nic/agent/nmd/api"
)

// NewIPClient returns the instance of IPClient for non-linux platforms
func NewIPClient(nmd api.NmdAPI, intf string) (*IPClient, error) {
	var dhcpState DHCPState
	mgmtIntf, _ := netlink.LinkByName(intf)
	ipClient := IPClient{
		nmd:       nmd,
		intf:      mgmtIntf,
		dhcpState: &dhcpState,
	}
	return &ipClient, nil
}

// DoStaticConfig performs static IPAddress/Default GW configuration. Stubbed out for darwin
func (c *IPClient) DoStaticConfig() (string, error) {
	// Look up interface

	return "a.b.c.d", nil
}

// DoDHCPConfig performs dhcp configuration. Stubbed out for darwin
func (c *IPClient) DoDHCPConfig() error {
	return nil
}

// DoNTPSync runs a ntp sync with venice co-ordinates acting as the server
func (c *IPClient) DoNTPSync() error {
	return nil
}

// GetIPClientIntf returns the interface name
func (c *IPClient) GetIPClientIntf() string {
	return "mock"
}

// GetDHCPState get current dhcp state
func (c *IPClient) GetDHCPState() string {
	return "none"
}
