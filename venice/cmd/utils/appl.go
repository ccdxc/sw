package utils

import (
	"io/ioutil"
	"strings"
)

// IsRunningonVeniceAppl returns true when running on Venice Appliance
// 	In case of Appliance, we own the OS code as well and hence during upgrade
//	we upgrade OS in addition to venice application code.
func IsRunningOnVeniceAppl() bool {

	// We look for pen.venice= in /proc/cmdline
	// Since we use our custom bootloader on our appliance, presence of this implies appliance
	read, err := ioutil.ReadFile("/proc/cmdline")
	if err != nil {
		return false
	}

	if strings.Contains(string(read), "pen.venice=") {
		return true
	}
	return false
}
