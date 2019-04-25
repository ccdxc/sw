//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var flowClearCmd = &cobra.Command{
	Use:   "flow",
	Short: "clear flows",
	Long:  "clear flows",
	Run:   flowClearCmdHandler,
}

func init() {
	clearCmd.AddCommand(flowClearCmd)
}

func flowClearCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewDebugSvcClient(c)

	// PDS call
	var empty *pds.Empty
	_, err = client.FlowClear(context.Background(), empty)
	if err != nil {
		fmt.Printf("Clear flows failed. %v\n", err)
		return
	}

	fmt.Printf("Clear flows succeeded\n")
}
