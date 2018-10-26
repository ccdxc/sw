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
	Short: "clear system statistics [fte | fte-txrx | table | pb | all] (Default: all)",
	Long:  "clear system statistics [fte | fte-txrx | table | pb | all] (Default: all)",
	Run:   systemStatsClearCmdHandler,
}

var systemDropStatsClearCmd = &cobra.Command{
	Use:   "drop",
	Short: "clear system statistics drop [ingress | egress | pb | all] (Default: all)",
	Long:  "clear system statistics drop [ingress | egress | pb | all] (Default: all)",
	Run:   systemDropStatsClearCmdHandler,
}

func init() {
	rootCmd.AddCommand(clearCmd)
	clearCmd.AddCommand(systemClearCmd)
	systemClearCmd.AddCommand(systemStatsClearCmd)
	systemStatsClearCmd.AddCommand(systemDropStatsClearCmd)

	clearCmd.AddCommand(platformClearCmd)
	platformClearCmd.AddCommand(platformHbmClearCmd)
	platformHbmClearCmd.AddCommand(platformHbmLlcStatsClearCmd)
}

func systemDropStatsClearCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewSystemClient(c.ClientConn)

	// Check the args
	ingressDrop := false
	egressDrop := false
	pbDrop := false

	if len(args) > 0 {
		if strings.Compare(args[0], "ingress") == 0 {
			ingressDrop = true
		} else if strings.Compare(args[0], "egress") == 0 {
			egressDrop = true
		} else if strings.Compare(args[0], "pb") == 0 {
			pbDrop = true
		} else if strings.Compare(args[0], "all") == 0 {
			ingressDrop = true
			egressDrop = true
			pbDrop = true
		} else {
			fmt.Printf("Invalid argument\n")
			return
		}
	} else {
		ingressDrop = true
		egressDrop = true
		pbDrop = true
	}

	var empty *halproto.Empty

	if ingressDrop {
		_, err := client.ClearIngressDropStats(context.Background(), empty)
		if err != nil {
			fmt.Printf("Clearing Ingress Drop Stats failed. %v\n", err)
			return
		}
	}

	if egressDrop {
		_, err := client.ClearEgressDropStats(context.Background(), empty)
		if err != nil {
			fmt.Printf("Clearing Egress Drop Stats failed. %v\n", err)
			return
		}
	}

	if pbDrop {
		_, err := client.ClearPbDropStats(context.Background(), empty)
		if err != nil {
			fmt.Printf("Clearing Pb Drop Stats failed. %v\n", err)
			return
		}
	}
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
	table := false
	fte := false
	pb := false
	fteTxRx := false

	if len(args) > 0 {
		if strings.Compare(args[0], "table") == 0 {
			table = true
		} else if strings.Compare(args[0], "fte") == 0 {
			fte = true
		} else if strings.Compare(args[0], "pb") == 0 {
			pb = true
		} else if strings.Compare(args[0], "fte-txrx") == 0 {
			fteTxRx = true
		} else if strings.Compare(args[0], "all") == 0 {
			table = true
			fte = true
			pb = true
			fteTxRx = true
		} else {
			fmt.Printf("Invalid argument\n")
			return
		}
	} else {
		table = true
		fte = true
		pb = true
		fteTxRx = true
	}

	var empty *halproto.Empty

	if table {
		_, err := client.ClearTableStats(context.Background(), empty)
		if err != nil {
			fmt.Printf("Clearing Table Stats failed. %v\n", err)
			return
		}
	}

	if fte {
		_, err := client.ClearFteStats(context.Background(), empty)
		if err != nil {
			fmt.Printf("Clearing FTE Stats failed. %v\n", err)
			return
		}
	}

	if fteTxRx {
		_, err := client.ClearFteTxRxStats(context.Background(), empty)
		if err != nil {
			fmt.Printf("Clearing FTE Tx-Rx Stats failed. %v\n", err)
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
