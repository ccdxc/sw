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

var (
	clockFreq uint32
)

var systemDebugCmd = &cobra.Command{
	Use:   "system",
	Short: "set sytem debug information",
	Long:  "set system debug information",
	Run:   systemDebugCmdHandler,
}

func init() {
	debugCmd.AddCommand(systemDebugCmd)
	systemDebugCmd.Flags().Uint32VarP(&clockFreq, "clock-frequency", "c", 0, "Specify clock-frequency (Allowed: 833, 1100)")
	systemDebugCmd.MarkFlagRequired("clock-frequency")
}

func systemDebugCmdHandler(cmd *cobra.Command, args []string) {
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

	if cmd.Flags().Changed("clock-frequency") {
		systemClockFrequencySet(client)
	}
}

func systemClockFrequencySet(client pds.DebugSvcClient) {
	req := &pds.ClockFrequencyRequest{
		ClockFrequency: clockFreq,
	}

	// PDS call
	respMsg, err := client.ClockFrequencyUpdate(context.Background(), req)
	if err != nil {
		fmt.Printf("Clock-frequency update failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	fmt.Printf("Clock-frequency set to %d\n", clockFreq)
}
