package cmd

import (
	"fmt"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"

	"os"
)

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
	resp, err := utils.CmdSend("/var/run/fd_recv_sock", iovec, int(os.Stdout.Fd()))
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
