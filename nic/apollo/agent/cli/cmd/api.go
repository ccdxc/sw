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

var apiShowCmd = &cobra.Command{
	Use:   "api-engine",
	Short: "show API engine statistics",
	Long:  "show API engine statistics",
	Run:   apiShowCmdHandler,
}

var storeShowCmd = &cobra.Command{
	Use:   "store",
	Short: "show PDS store statistics",
	Long:  "show PDS store statistics",
	Run:   storeShowCmdHandler,
}

func init() {
	systemStatsShowCmd.AddCommand(apiShowCmd)
	systemStatsShowCmd.AddCommand(storeShowCmd)
}

func storeShowCmdHandler(cmd *cobra.Command, args []string) {
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

	// dump API counters
	cmdCtxt := &pds.CommandCtxt{
		Version: 1,
		Cmd:     pds.Command_CMD_STORE_STATS_DUMP,
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func apiShowCmdHandler(cmd *cobra.Command, args []string) {
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

	// dump API counters
	cmdCtxt := &pds.CommandCtxt{
		Version: 1,
		Cmd:     pds.Command_CMD_API_ENGINE_STATS_DUMP,
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}
