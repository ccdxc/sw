// +build !linux

package ipif

import (
	"github.com/pensando/netlink"

	"github.com/pensando/sw/nic/agent/nmd/api"
)

// NewIPClient returns the instance of IPClient for non-linux platforms
func NewIPClient(nmd api.NmdAPI, intf string, pipeline string) (*IPClient, error) {
	var dhcpState DHCPState
	mgmtIntf, _ := netlink.LinkByName(intf)
	ipClient := IPClient{
		nmd:         nmd,
		primaryIntf: mgmtIntf,
		dhcpState:   &dhcpState,
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

// StopDHCPConfig cancels any active DHCP goroutines
func (c *IPClient) StopDHCPConfig() {
}

// GetStaticRoutes returns the set of static routes received via dhcp
func (c *IPClient) GetStaticRoutes() []StaticRoute {
	return []StaticRoute{}
}

// GetInterfaceIPs returns the set of interface IP information received via DHCP
func (c *IPClient) GetInterfaceIPs() []InterfaceIP {
	return []InterfaceIP{}
}
