//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package utils

import (
	"encoding/binary"
	"fmt"
	"net"
	"syscall"
	"unsafe"
)

func FdSend(udsPath string, cid int64, fds ...int) error {
	c, err := net.Dial("unix", udsPath)
	if err != nil {
		fmt.Printf("Could not connect to unix domain socket\n")
		return err
	}
	defer c.Close()

	udsConn := c.(*net.UnixConn)
	udsFile, err := udsConn.File()
	if err != nil {
		return err
	}
	socket := int(udsFile.Fd())
	defer udsFile.Close()

	var p []byte
	if cid != 0 {
		p = make([]byte, unsafe.Sizeof(cid))
		ucid := uint64(cid)
		binary.LittleEndian.PutUint64(p, ucid)
	} else {
		p = nil
	}

	rights := syscall.UnixRights(fds...)
	return syscall.Sendmsg(socket, p, rights, nil, 0)
}
