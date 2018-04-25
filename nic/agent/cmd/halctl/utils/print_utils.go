package utils

import (
	"encoding/binary"
	"fmt"
	"net"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

// MactoStr converts a uint64 to a MAC string
func MactoStr(mac uint64) string {
	var bytes [6]byte

	bytes[0] = byte(mac & 0xFF)
	bytes[1] = byte((mac >> 8) & 0xFF)
	bytes[2] = byte((mac >> 16) & 0xFF)
	bytes[3] = byte((mac >> 24) & 0xFF)
	bytes[4] = byte((mac >> 32) & 0xFF)
	bytes[5] = byte((mac >> 40) & 0xFF)

	macStr := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0])

	return macStr
}

// VrfTypeToStr converts a VRF type to VRF string
func VrfTypeToStr(vrfType halproto.VrfType) string {
	switch vrfType {
	case halproto.VrfType_VRF_TYPE_INFRA:
		return "Infra"
	case halproto.VrfType_VRF_TYPE_CUSTOMER:
		return "Cust"
	default:
		return "Invalid"
	}
}

// TableKindToStr converts table type to string
func TableKindToStr(tType halproto.TableKind) string {
	switch tType {
	case halproto.TableKind_TABLE_INDEX:
		return "Index"
	case halproto.TableKind_TABLE_TCAM:
		return "Tcam"
	case halproto.TableKind_TABLE_HASH:
		return "Hash"
	case halproto.TableKind_TABLE_FLOW:
		return "Flow"
	case halproto.TableKind_TABLE_MET:
		return "Met"
	default:
		return "Invalid"
	}
}

// TnnlEncTypeToStr converts tunnel encap type to string
func TnnlEncTypeToStr(encType halproto.IfTunnelEncapType) string {
	switch encType {
	case halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_VXLAN:
		return "Vxlan"
	case halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_GRE:
		return "Gre"
	default:
		return "Invalid"
	}
}

// IPAddrToStr converts HAL proto IP address to string
func IPAddrToStr(ipAddr *halproto.IPAddress) string {
	if ipAddr.GetIpAf() == halproto.IPAddressFamily_IP_AF_INET {
		v4Addr := ipAddr.GetV4Addr()
		ip := make(net.IP, 4)
		binary.BigEndian.PutUint32(ip, v4Addr)
		return ip.String()
		/*
			v4Addr := ipAddr.GetV4Addr()
			v4Str := fmt.Sprintf("%d.%d.%d.%d", ((v4Addr >> 24) & 0xff), ((v4Addr >> 16) & 0xff),
				((v4Addr >> 8) & 0xff), (v4Addr & 0xff))
			return v4Str
		*/
	}
	v6Addr := ipAddr.GetV6Addr()
	ip := make(net.IP, 16)
	copy(ip, v6Addr)
	return ip.String()
	/*
		v6Addr := ipAddr.GetV6Addr()
		v6Str := fmt.Sprintf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
			v6Addr[0], v6Addr[1], v6Addr[2], v6Addr[3],
			v6Addr[4], v6Addr[5], v6Addr[6], v6Addr[7],
			v6Addr[8], v6Addr[9], v6Addr[10], v6Addr[11],
			v6Addr[12], v6Addr[13], v6Addr[14], v6Addr[15])
		return v6Str
	*/

}

// IPPrefixToStr converts prefix to string
func IPPrefixToStr(pfx *halproto.IPPrefix) string {
	return fmt.Sprintf("%s/%d", IPAddrToStr(pfx.GetAddress()), pfx.GetPrefixLen())
}
