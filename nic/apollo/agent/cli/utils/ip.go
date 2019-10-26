package utils

import (
	"fmt"
	"net"

	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

// IPAddrStrtoUint32 converts string IP address to uint32
func IPAddrStrtoUint32(ip string) uint32 {
	var addr [4]uint32
	fmt.Sscanf(ip, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3])
	return ((addr[0] << 24) + (addr[1] << 16) + (addr[2] << 8) + (addr[3]))
}

// IPAddrStrToPDSIPAddr converts string ip address to pds native IPAddress Type
func IPAddrStrToPDSIPAddr(ip string) *pds.IPAddress {
	netIP := net.ParseIP(ip)
	isV6 := (netIP.To4() == nil)

	var ipAddr *pds.IPAddress
	if isV6 {
		ipAddr = &pds.IPAddress{
			Af: pds.IPAF_IP_AF_INET6,
			V4OrV6: &pds.IPAddress_V6Addr{
				V6Addr: netIP,
			},
		}
	} else {
		ipAddr = &pds.IPAddress{
			Af: pds.IPAF_IP_AF_INET,
			V4OrV6: &pds.IPAddress_V4Addr{
				V4Addr: IPAddrStrtoUint32(ip),
			},
		}
	}
	return ipAddr
}
