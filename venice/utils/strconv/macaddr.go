package strconv

import (
	"net"
)

const hexDigit = "0123456789abcdef"

// MacString returns a "." formatted MAC address string with format  xxx.yyyy.zzzz
func MacString(in net.HardwareAddr) string {
	if len(in) == 0 {
		return ""
	}
	buf := make([]byte, 0, len(in)*3-1)
	for i, b := range in {
		if i > 0 && i%2 == 0 {
			buf = append(buf, '.')
		}
		buf = append(buf, hexDigit[b>>4])
		buf = append(buf, hexDigit[b&0xF])
	}
	return string(buf)
}

// ParseMacAddr parses mac address as a string and normalizes it to a "." formatted MAC address
//   of the format xxx.yyyy.zzzz
func ParseMacAddr(in string) (string, error) {
	mac, err := net.ParseMAC(in)
	if err != nil {
		return "", err
	}
	return MacString(mac), nil
}
