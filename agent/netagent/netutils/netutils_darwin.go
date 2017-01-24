// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

//+build !test

package netutils

import (
	"fmt"
	"net"
)

func CreateVethPair(name1, name2 string) error {
	return nil
}

func DeleteVethPair(name1, name2 string) error {
	return nil
}

// MoveIntfToNamespace moves intf to namespace
func MoveIntfToNamespace(intfName, nsPath string) error {
	if nsPath == "/proc/self/ns/net" {
		return nil
	}

	return fmt.Errorf("Not Implemented")
}

// SetupIntfInNamespace configures interface in namespace
func SetupIntfInNamespace(intfName, nsIntfName, nsPath string, ipv4AddrMask, ipv6AddrMask *net.IPNet) error {
	if nsPath == "/proc/self/ns/net" {
		return nil
	}

	return fmt.Errorf("Not Implemented")
}
