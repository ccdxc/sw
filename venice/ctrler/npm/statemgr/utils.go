package statemgr

import (
	"encoding/binary"
	"fmt"
	"net"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func cloneRD(in *network.RouteDistinguisher) *netproto.RouteDistinguisher {
	if in == nil {
		return nil
	}
	return &netproto.RouteDistinguisher{
		Type:          in.Type,
		AdminValue:    in.AdminValue,
		AssignedValue: in.AssignedValue,
	}
}

// ParseToIPAddress parses a string to a netproto IPaddress
func ParseToIPAddress(in string) (*netproto.IPAddress, error) {
	ip := net.ParseIP(in)
	if ip == nil {
		return nil, fmt.Errorf("could not parse ip")
	}

	// We only support v4 addresses for now.
	if ip.To4() == nil {
		return nil, fmt.Errorf("non IPv4 address. Not supported")
	}
	return &netproto.IPAddress{
		Type:      netproto.IPAF_IP_AF_INET,
		V4Address: binary.BigEndian.Uint32(ip.To4()),
	}, nil
}

// ParseToIPPrefix parses a string to a netproto IPPrefix
func ParseToIPPrefix(in string) (*netproto.IPPrefix, error) {
	ip, ipn, err := net.ParseCIDR(in)
	if err != nil {
		return nil, fmt.Errorf("could not parse ip prefix")
	}

	// We only support v4 addresses for now.
	if ip.To4() == nil {
		return nil, fmt.Errorf("non IPv4 address. Not supported")
	}

	len, _ := ipn.Mask.Size()
	return &netproto.IPPrefix{
		PrefixLen: uint32(len),
		Address: netproto.IPAddress{
			Type:      netproto.IPAF_IP_AF_INET,
			V4Address: binary.BigEndian.Uint32(ip.To4()),
		},
	}, nil
}
