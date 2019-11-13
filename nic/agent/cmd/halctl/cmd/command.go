package cmd

import (
	"fmt"

	"github.com/golang/protobuf/proto"
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"

	"os"
)

// CmdSocket unix domain socket path
var CmdSocket = "/var/run/cmd_server_sock_sysmon"

// HandleCommand function to handle commands over unix domain sockets
// param[in] cmdCtxt Command context to be sent
// return    cmdResp Command response
//           error   Error if any, else nil
func HandleCommand(cmdCtxt *halproto.CommandCtxt) (*halproto.CommandResponse, error) {
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
	cmdResp := &halproto.CommandResponse{}
	err = proto.Unmarshal(resp, cmdResp)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return nil, err
	}

	return cmdResp, nil
}

func interruptClearCmdHandler(cmd *cobra.Command, args []string) {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the sysmon. Is sysmon Running?\n")
		return
	}
	defer c.Close()

	var cmdCtxt *halproto.CommandCtxt

	cmdCtxt = &halproto.CommandCtxt{
		Version: 1,
		Cmd:     halproto.Command_CMD_CLEAR_INTR,
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if cmdResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func interruptShowCmdHandler(cmd *cobra.Command, args []string) {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the sysmon. Is sysmon Running?\n")
		return
	}
	defer c.Close()

	var cmdCtxt *halproto.CommandCtxt

	cmdCtxt = &halproto.CommandCtxt{
		Version: 1,
		Cmd:     halproto.Command_CMD_DUMP_INTR,
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if cmdResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

var interruptShowCmd = &cobra.Command{
	Use:   "interrupts",
	Short: "show interrupt details",
	Long:  "show interrupt details information",
	Run:   interruptShowCmdHandler,
}

var interruptClearCmd = &cobra.Command{
	Use:   "interrupts",
	Short: "clear interrupt details",
	Long:  "clear interrupt details information",
	Run:   interruptClearCmdHandler,
}

func init() {
	showCmd.AddCommand(interruptShowCmd)
	clearCmd.AddCommand(interruptClearCmd)
}
