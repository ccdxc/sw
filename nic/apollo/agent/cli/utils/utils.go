//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package utils

import (
	"fmt"
	"net"
	"syscall"
)

func CmdSend(udsPath string, cmd []byte, fds ...int) ([]byte, error) {
	c, err := net.Dial("unix", udsPath)
	if err != nil {
		fmt.Printf("Could not connect to unix domain socket\n")
		return nil, err
	}
	defer c.Close()

	udsConn := c.(*net.UnixConn)
	udsFile, err := udsConn.File()
	if err != nil {
		return nil, err
	}
	socket := int(udsFile.Fd())
	defer udsFile.Close()

	var rights []byte
	rights = nil
	if fds[0] >= 0 {
		rights = syscall.UnixRights(fds...)
	}
	err = syscall.Sendmsg(socket, cmd, rights, nil, 0)
	if err != nil {
		fmt.Printf("Sendmsg failed with error %v\n", err)
		return nil, err
	}

	// wait for response
	resp := make([]byte, 20480)
	n, _, _, _, err := syscall.Recvmsg(socket, resp, nil, 0)
	if err != nil {
		fmt.Printf("Recvmsg failed with error %v\n", err)
		return nil, err
	}

	return resp[:n], nil
}
