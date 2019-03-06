package iotakit

import (
	"sort"
	"strings"
)

// parses and returns one of the port number from `halctl show port status` output
func getPortNum(portStatusOut string) string {
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
