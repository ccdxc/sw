package state

import (
	"testing"

	"github.com/pensando/sw/api/generated/cluster"
	nmdProto "github.com/pensando/sw/nic/agent/protos/nmd"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//++++++++++++++++++++++++++++ Happy Path Test Cases ++++++++++++++++++++++++++++++++++++++++

func TestValidHostMode(t *testing.T) {
	t.Parallel()
	hostModeSpec := nmdProto.NaplesSpec{
		Mode: nmdProto.MgmtMode_HOST.String(),
	}

	err := isHostModeValid(hostModeSpec)
	AssertOk(t, err, "Valid host mode spec should not report an error")
}

func TestValidNetworkMode(t *testing.T) {
	t.Parallel()
	netModeSpec := nmdProto.NaplesSpec{
		Mode:        nmdProto.MgmtMode_NETWORK.String(),
		NetworkMode: nmdProto.NetworkMode_INBAND.String(),
		Controllers: []string{"4.2.2.2"},
		ID:          "testNaples",
	}

	err := isNetworkModeValid(netModeSpec)
	AssertOk(t, err, "Valid network mode spec should not report an error")
}

//++++++++++++++++++++++++++++ Corner Test Cases ++++++++++++++++++++++++++++++++++++++++

func TestInValidHostModeAndNetworkMode(t *testing.T) {
	t.Parallel()
	hostModeSpec := nmdProto.NaplesSpec{
		Mode:        nmdProto.MgmtMode_HOST.String(),
		NetworkMode: nmdProto.MgmtMode_NETWORK.String(),
	}

	err := isHostModeValid(hostModeSpec)
	Assert(t, err != nil, "Invalid host mode spec should report an error. Err: %v", err)
}

func TestInValidHostModeAndControllers(t *testing.T) {
	t.Parallel()
	hostModeSpec := nmdProto.NaplesSpec{
		Mode:        nmdProto.MgmtMode_HOST.String(),
		Controllers: []string{"42.42.42.42"},
	}

	err := isHostModeValid(hostModeSpec)
	Assert(t, err != nil, "Invalid host mode spec should report an error. Err: %v", err)
}

func TestInValidHostModeAndIPAddress(t *testing.T) {
	t.Parallel()
	hostModeSpec := nmdProto.NaplesSpec{
		Mode: nmdProto.MgmtMode_HOST.String(),
		IPConfig: &cluster.IPConfig{
			IPAddress: "42.42.42.42",
		},
	}

	err := isHostModeValid(hostModeSpec)
	Assert(t, err != nil, "Invalid host mode spec should report an error. Err: %v", err)
}

func TestInValidHostModeAndDefaultGW(t *testing.T) {
	t.Parallel()
	hostModeSpec := nmdProto.NaplesSpec{
		Mode: nmdProto.MgmtMode_HOST.String(),
		IPConfig: &cluster.IPConfig{
			DefaultGW: "42.42.42.42",
		},
	}

	err := isHostModeValid(hostModeSpec)
	Assert(t, err != nil, "Invalid host mode spec should report an error. Err: %v", err)
}

func TestInValidHostModeAndDNSServers(t *testing.T) {
	t.Parallel()
	hostModeSpec := nmdProto.NaplesSpec{
		Mode: nmdProto.MgmtMode_HOST.String(),
		IPConfig: &cluster.IPConfig{
			DNSServers: []string{"dnssec.pensando.io"},
		},
	}
	err := isHostModeValid(hostModeSpec)
	Assert(t, err != nil, "Invalid host mode spec should report an error. Err: %v", err)
}

func TestValidHostModeZeroValueIPConfig(t *testing.T) {
	t.Parallel()
	hostModeSpec := nmdProto.NaplesSpec{
		Mode:     nmdProto.MgmtMode_HOST.String(),
		IPConfig: &cluster.IPConfig{},
	}
	err := isHostModeValid(hostModeSpec)
	AssertOk(t, err, "Valid host mode spec should report an error. Err: %v", err)
}

func TestInValidNetworkModeEmptyMgmtNetwork(t *testing.T) {
	t.Parallel()
	netModeSpec := nmdProto.NaplesSpec{
		Mode:        nmdProto.MgmtMode_NETWORK.String(),
		Controllers: []string{"4.2.2.2"},
	}

	err := isNetworkModeValid(netModeSpec)
	Assert(t, err != nil, "Invalid network mode spec should report an error")
}

func TestInValidNetworkModeBadMgmtNetwork(t *testing.T) {
	t.Parallel()
	netModeSpec := nmdProto.NaplesSpec{
		Mode:        nmdProto.MgmtMode_NETWORK.String(),
		NetworkMode: "Bad Management Network",
		Controllers: []string{"4.2.2.2"},
	}

	err := isNetworkModeValid(netModeSpec)
	Assert(t, err != nil, "Invalid network mode spec should report an error")
}

func TestInValidNetworkModeWithNaplesProfile(t *testing.T) {
	t.Parallel()
	netModeSpec := nmdProto.NaplesSpec{
		Mode:          nmdProto.MgmtMode_NETWORK.String(),
		NetworkMode:   nmdProto.NetworkMode_INBAND.String(),
		NaplesProfile: "default",
		Controllers:   []string{"4.2.2.2"},
	}

	err := isNetworkModeValid(netModeSpec)
	Assert(t, err != nil, "Invalid network mode spec should report an error")
}
