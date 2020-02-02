package utils

import (
	"regexp"
	"sort"
	"strconv"
	"strings"
)

// parses and returns one of the port number from `halctl show port status` output
func GetPortNum(portStatusOut string) string {
	temp := strings.Split(portStatusOut, "\n")
	temp = temp[3:] // remove header
	sort.Strings(temp)
	for _, port := range temp {
		if strings.TrimSpace(port) != "" {
			portDet := strings.Split(port, " ")
			if len(portDet) > 0 {
				return portDet[0] // port num
			}
		}
	}

	return ""
}

// macAddrToUint64 converts mac address string to hex uint64
func MacAddrToUint64(macAddr string) uint64 {
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(macAddr, "")
	hexAddr, _ := strconv.ParseUint(hex, 16, 64)
	return hexAddr
}
