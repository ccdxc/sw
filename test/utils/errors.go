// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package utils

import (
	"net"
	"os"
	"syscall"
)

// IsConnRefusedError returns true if the supplied error is a TCP "connection refused" error
func IsConnRefusedError(err error) bool {
	if err == nil {
		return false
	}
	neterr, ok := err.(*net.OpError)
	if !ok {
		return false
	}
	osErr, ok := neterr.Err.(*os.SyscallError)
	if !ok {
		return false
	}
	return osErr.Err == syscall.ECONNREFUSED
}
