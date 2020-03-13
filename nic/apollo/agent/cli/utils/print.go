package utils

import (
	"encoding/binary"
	"fmt"
	"net"
	"strings"

	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

// IPPrototoStr converts ip proto to string
func IPPrototoStr(proto uint32) string {
	switch proto {
	case 1:
		return "ICMP"
	case 6:
		return "TCP"
	case 17:
		return "UDP"
	default:
		return fmt.Sprint(proto)
	}
}

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

// IPAddrToStr converts PDS proto IP address to string
func IPAddrToStr(ipAddr *pds.IPAddress) string {
	if ipAddr.GetAf() == pds.IPAF_IP_AF_INET {
		v4Addr := ipAddr.GetV4Addr()
		ip := make(net.IP, 4)
		binary.BigEndian.PutUint32(ip, v4Addr)
		return ip.String()
	} else if ipAddr.GetAf() == pds.IPAF_IP_AF_INET6 {
		v6Addr := ipAddr.GetV6Addr()
		ip := make(net.IP, 16)
		copy(ip, v6Addr)
		return ip.String()
	} else {
		return "-"
	}

}

// Uint32IPAddrtoStr converts uint32 IP to string
func Uint32IPAddrtoStr(addr uint32) string {
	ip := make(net.IP, 4)
	binary.BigEndian.PutUint32(ip, addr)
	return ip.String()
}

// ByteIPv6AddrtoStr converts byte array IPv6 to string
func ByteIPv6AddrtoStr(addr []byte) string {
	ip := make(net.IP, 16)
	copy(ip, addr)
	return ip.String()
}

func IPv4PrefixToStr(pfx *pds.IPv4Prefix) string {
	return fmt.Sprintf("%s/%d", Uint32IPAddrtoStr(pfx.GetAddr()), pfx.GetLen())
}

func IPv6PrefixToStr(pfx *pds.IPv6Prefix) string {
	return fmt.Sprintf("%s/%d", ByteIPv6AddrtoStr(pfx.GetAddr()), pfx.GetLen())
}

// IPPrefixToStr converts prefix to string
func IPPrefixToStr(pfx *pds.IPPrefix) string {
	pfxStr := IPAddrToStr(pfx.GetAddr())
	if pfxStr == "-" {
		return "-/-"
	}
	return fmt.Sprintf("%s/%d", pfxStr, pfx.GetLen())
}

// EncapToString converts encap to string
func EncapToString(encap *pds.Encap) string {
	encapType := encap.GetType()
	encapStr := strings.Replace(encapType.String(), "ENCAP_TYPE_", "", -1)
	switch encapType {
	case pds.EncapType_ENCAP_TYPE_DOT1Q:
		encapStr += fmt.Sprintf("/%d", encap.GetValue().GetVlanId())
	case pds.EncapType_ENCAP_TYPE_MPLSoUDP:
		encapStr += fmt.Sprintf("/%d", encap.GetValue().GetMPLSTag())
	case pds.EncapType_ENCAP_TYPE_VXLAN:
		encapStr += fmt.Sprintf("/%d", encap.GetValue().GetVnid())
	default:
	}
	return encapStr
}

func BoolToString(val bool) string {
	if val {
		return "T"
	}
	return "F"
}
