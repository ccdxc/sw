//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var clearCmd = &cobra.Command{
	Use:   "clear",
	Short: "Clear Commands",
	Long:  "Clear Commands",
}

var systemClearCmd = &cobra.Command{
	Use:   "system",
	Short: "Clear system information",
	Long:  "Clear system information",
}

var systemStatsClearCmd = &cobra.Command{
	Use:   "statistics",
	Short: "clear system statistics [fte | pb | all] (Default: all)",
	Long:  "clear system statistics [fte | pb | all] (Default: all)",
	Run:   systemStatsClearCmdHandler,
}

func init() {
	rootCmd.AddCommand(clearCmd)
	clearCmd.AddCommand(systemClearCmd)
	systemClearCmd.AddCommand(systemStatsClearCmd)

	clearCmd.AddCommand(platformClearCmd)
	platformClearCmd.AddCommand(platformHbmClearCmd)
	platformHbmClearCmd.AddCommand(platformHbmLlcStatsClearCmd)
}

func systemStatsClearCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewSystemClient(c.ClientConn)

	// Check the args
	fte := false
	pb := false

	if len(args) > 0 {
		if strings.Compare(args[0], "fte") == 0 {
			fte = true
		} else if strings.Compare(args[0], "pb") == 0 {
			pb = true
		} else if strings.Compare(args[0], "all") == 0 {
			fte = true
			pb = true
		} else {
			fmt.Printf("Invalid argument\n")
			return
		}
	} else {
		fte = true
		pb = true
	}

	var empty *halproto.Empty

	if fte {
		_, err := client.ClearFteStats(context.Background(), empty)
		if err != nil {
			fmt.Printf("Clearing FTE Stats failed. %v\n", err)
			return
		}
	}

	if pb {
		_, err := client.ClearPbStats(context.Background(), empty)
		if err != nil {
			fmt.Printf("Clearing Pb Stats failed. %v\n", err)
			return
		}
	}
}

var platformClearCmd = &cobra.Command{
	Use:   "platform",
	Short: "clear platform hbm llc-stats",
	Long:  "clear platform hmb llc-stats",
}

var platformHbmClearCmd = &cobra.Command{
	Use:   "hbm",
	Short: "clear platform hbm llc-stats",
	Long:  "clear platform hmb llc-stats",
}

var platformHbmLlcStatsClearCmd = &cobra.Command{
	Use:   "llc-stats",
	Short: "clear platform hbm llc-stats",
	Long:  "clear platform hmb llc-stats",
	Run:   llcStatsClearCmdHandler,
}

func llcStatsClearCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var empty *halproto.Empty

	// HAL call
	_, err = client.LlcClear(context.Background(), empty)
	if err != nil {
		fmt.Printf("Llc clear failed. %v\n", err)
		return
	}
}
