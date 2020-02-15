package cmd

import (
	"fmt"
	"net"
	"syscall"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"

	"os"
)

var CmdSocket string = "/var/run/cmd_server_sock"
var vppUdsPath string = "/run/vpp/pds.sock"

type PrintObject interface {
	PrintHeader()
	HandleObject([]byte) bool
}

// function to handle commands over unix domain sockets
// param[in] cmd     Command context to be sent
// return    cmdResp Command response
//           err     Error
func HandleCommand(cmdCtxt *pds.CommandCtxt) (*pds.CommandResponse, error) {
	// marshall cmdCtxt
	iovec, err := proto.Marshal(cmdCtxt)
	if err != nil {
		fmt.Printf("Marshall command failed with error %v\n", err)
		return nil, err
	}

	// send over UDS
	resp, err := utils.CmdSend(CmdSocket, iovec, int(os.Stdout.Fd()))
	if err != nil {
		fmt.Printf("Command send operation failed with error %v\n", err)
		return nil, err
	}

	// unmarshal response
	cmdResp := &pds.CommandResponse{}
	err = proto.Unmarshal(resp, cmdResp)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return nil, err
	}

	return cmdResp, nil
}

// function to handle show object command over unix domain sockets
// param[in] cmd     Command context to be sent
// return    err     Error
func HandleUdsShowObject(cmdCtxt *pds.CommandCtxt, i PrintObject) error {
	// marshall cmdCtxt
	iovec, err := proto.Marshal(cmdCtxt)
	if err != nil {
		fmt.Printf("Marshall command failed with error %v\n", err)
		return err
	}

	c, err := net.Dial("unixpacket", vppUdsPath)
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

	err = syscall.Sendmsg(socket, iovec, nil, nil, 0)
	if err != nil {
		fmt.Printf("Sendmsg failed with error %v\n", err)
		return err
	}

	i.PrintHeader()
	// read from the socket until the no more entries are received
	resp := make([]byte, 256)
	for {
		n, _, _, _, err := syscall.Recvmsg(socket, resp, nil, syscall.MSG_WAITALL)
		if err != nil {
			fmt.Printf("Recvmsg failed with error %v\n", err)
			return err
		}

		done := i.HandleObject(resp[:n])
		if done {
			// Last message
			return nil
		}
	}
	return nil
}
