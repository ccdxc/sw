package utils

import (
	"fmt"

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
