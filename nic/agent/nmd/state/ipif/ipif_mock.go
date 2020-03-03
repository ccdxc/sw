package ipif

import (
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/nic/agent/nmd/api"
)

// MockIPClient implements mock IP client
type MockIPClient IPClient

// NewMockIPClient returns the instance of IPClient for non-linux platforms
func NewMockIPClient(nmd api.NmdAPI, intf string) (*IPClient, error) {
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
func (c *MockIPClient) DoStaticConfig() (string, error) {
	// Look up interface

	return "a.b.c.d", nil
}

// DoDHCPConfig performs dhcp configuration. Stubbed out for darwin
func (c *MockIPClient) DoDHCPConfig() error {
	return nil
}

// DoNTPSync runs a ntp sync with venice co-ordinates acting as the server
func (c *MockIPClient) DoNTPSync() error {
	return nil
}

// GetIPClientIntf returns the interface name
func (c *MockIPClient) GetIPClientIntf() string {
	return "mock"
}

// GetDHCPState get current dhcp state
func (c *MockIPClient) GetDHCPState() string {
	return "none"
}

// StopDHCPConfig cancels any active DHCP goroutines
func (c *MockIPClient) StopDHCPConfig() {
}
