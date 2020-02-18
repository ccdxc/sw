package utils

import (
	"fmt"
)

// MACAddrStrtoUint64 converts string MAC address to uint64
func MACAddrStrToUint64(mac string) uint64 {
	var addr [6]uint64

	fmt.Sscanf(mac, "%x:%x:%x:%x:%x:%x", &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5])
	return ((addr[0] << 40) | (addr[1] << 32) | (addr[2] << 24) | (addr[3] << 16) | (addr[4] << 8) | (addr[5]))
}
