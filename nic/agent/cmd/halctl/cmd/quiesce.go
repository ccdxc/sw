//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var quiesceCmd = &cobra.Command{
	Use:   "quiesce",
	Short: "quiesce start/stop",
	Long:  "quiesce start/stop",
}

var quiesceStartCmd = &cobra.Command{
	Use:   "start",
	Short: "quiesce start",
	Long:  "start quiescing the dataplane",
	Run:   quiesceStartCmdHandler,
}

var quiesceStopCmd = &cobra.Command{
	Use:   "stop",
	Short: "quiesce stop",
	Long:  "stop quiescing the dataplane",
	Run:   quiesceStopCmdHandler,
}

func init() {
	debugCmd.AddCommand(quiesceCmd)
	quiesceCmd.AddCommand(quiesceStartCmd)
	quiesceCmd.AddCommand(quiesceStopCmd)
}

func quiesceStartCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInternalClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var empty *halproto.EmptyRequest

	// HAL call
	_, err = client.QuiesceStart(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting Table Metadata failed. %v\n", err)
		return
	}
}

func quiesceStopCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInternalClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var empty *halproto.EmptyRequest

	// HAL call
	_, err = client.QuiesceStop(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting Table Metadata failed. %v\n", err)
		return
	}
}
