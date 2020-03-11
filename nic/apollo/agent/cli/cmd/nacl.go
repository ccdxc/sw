//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var tableShowCmd = &cobra.Command{
	Use:   "table",
	Short: "show table information",
	Long:  "show tableinformation",
}

var naclShowCmd = &cobra.Command{
	Use:   "nacl",
	Short: "show nacl table",
	Long:  "show nacl table",
	Run:   naclShowCmdHandler,
}

func init() {
	showCmd.AddCommand(tableShowCmd)
	tableShowCmd.AddCommand(naclShowCmd)
}

func naclShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var cmdCtxt *pds.CommandCtxt

	// dump nacl table
	cmdCtxt = &pds.CommandCtxt{
		Version: 1,
		Cmd:     pds.Command_CMD_NACL_DUMP,
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}
