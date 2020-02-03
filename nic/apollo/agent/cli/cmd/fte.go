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

var fteShowCmd = &cobra.Command{
	Use:   "fte",
	Short: "show fte information",
	Long:  "show fte information",
}

var fteStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show fte statistics",
	Long:  "show fte statistics",
	Run:   fteStatsShowCmdHandler,
}

var fteClearCmd = &cobra.Command{
	Use:   "fte",
	Short: "clear fte information",
	Long:  "clear fte information",
}

var fteStatsClearCmd = &cobra.Command{
	Use:   "statistics",
	Short: "clear fte statistics",
	Long:  "clear fte statistics",
	Run:   fteStatsClearCmdHandler,
}

func init() {
	showCmd.AddCommand(fteShowCmd)
	fteShowCmd.AddCommand(fteStatsShowCmd)
	fteStatsShowCmd.Flags().Bool("api", false, "Show FTE API statistics")
	fteStatsShowCmd.Flags().Bool("table", false, "Show FTE table statistics")

	clearCmd.AddCommand(fteClearCmd)
	fteClearCmd.AddCommand(fteStatsClearCmd)
	fteStatsClearCmd.Flags().Bool("api", false, "Clear FTE API statistics")
	fteStatsClearCmd.Flags().Bool("table", false, "Clear FTE table statistics")
}

func fteStatsClearCmdHandler(cmd *cobra.Command, args []string) {
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

	apiStats := true
	tableStats := true

	if cmd != nil {
		if cmd.Flags().Changed("api") == false &&
			cmd.Flags().Changed("table") == false {
		} else if cmd.Flags().Changed("api") == false {
			apiStats = false
		} else if cmd.Flags().Changed("table") == false {
			tableStats = false
		}
	}

	client := pds.NewDebugSvcClient(c)

	req := &pds.FteStatsClearRequest{
		ApiStats:   apiStats,
		TableStats: tableStats,
	}

	// PDS call
	resp, err := client.FteStatsClear(context.Background(), req)
	if err != nil {
		fmt.Printf("FTE stats get failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	fmt.Printf("FTE stats cleared\n")
}

func fteStatsShowCmdHandler(cmd *cobra.Command, args []string) {
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

	apiStats := true
	tableStats := true

	if cmd != nil {
		if cmd.Flags().Changed("api") == false &&
			cmd.Flags().Changed("table") == false {
		} else if cmd.Flags().Changed("api") == false {
			apiStats = false
		} else if cmd.Flags().Changed("table") == false {
			tableStats = false
		}
	}
	client := pds.NewDebugSvcClient(c)

	var empty *pds.Empty

	// PDS call
	resp, err := client.FteStatsGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("FTE stats get failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	if apiStats {
		fteApiStatsPrintHeader()
		fteApiStatsPrintEntry(resp.GetApiStats())
	}

	if tableStats {
		fteTableStatsPrintHeader()
		fteTableStatsPrintEntry(resp.GetTableStats())
	}
}

func fteApiStatsPrintHeader() {
}

func fteApiStatsPrintEntry(stats *pds.FteApiStats) {
}

func fteTableStatsPrintHeader() {
}

func fteTableStatsPrintEntry(stats *pds.FteTableStats) {
}
