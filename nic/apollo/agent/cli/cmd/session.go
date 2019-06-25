//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	sessionStatsID string
)

var sessionStatsShowCmd = &cobra.Command{
	Use:   "session-stats",
	Short: "show session statistics",
	Long:  "show session statistics",
	Run:   sessionShowStatsCmdHandler,
}

func init() {
	showCmd.AddCommand(sessionStatsShowCmd)
	sessionStatsShowCmd.Flags().StringVarP(&sessionStatsID, "session-stats-index", "i", "", "Specify session stats index. Ex: 1-20 or 10")
	sessionStatsShowCmd.MarkFlagRequired("session-stats-index")
}

func sessionShowStatsCmdHandler(cmd *cobra.Command, args []string) {
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

	var statsIDLow uint32
	var statsIDHigh uint32

	n, _ := fmt.Sscanf(sessionStatsID, "%d-%d", &statsIDLow, &statsIDHigh)
	if n != 2 {
		n, _ = fmt.Sscanf(sessionStatsID, "%d", &statsIDLow)
		if n != 1 {
			fmt.Printf("Invalid session statistics index provided. Refer to help string\n")
			return
		}
		statsIDHigh = statsIDLow
	}

	if statsIDLow > statsIDHigh {
		fmt.Printf("Invalid session statistics index provided. Refer to help string")
		return
	}

	req := &pds.SessionStatsGetRequest{
		StatsIndexLow:  statsIDLow,
		StatsIndexHigh: statsIDHigh,
	}

	// PDS call
	respMsg, err := client.SessionStatsGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting session statistics failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	sessionStatsPrintHeader()
	sessionStatsPrintEntry(respMsg)
}

func sessionStatsPrintHeader() {
	hdrLine := strings.Repeat("-", 86)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-20s%-20s%-20s%-20s\n",
		"ID", "InitiatorPkts", "InitiatorBytes", "ResponderPkts", "ResponderBytes")
	fmt.Println(hdrLine)
}

func sessionStatsPrintEntry(resp *pds.SessionStatsGetResponse) {
	for _, stats := range resp.GetStats() {
		fmt.Printf("%-6d%-20d%-20d%-20d%-20d\n",
			stats.GetStatsIndex(),
			stats.GetInitiatorFlowPkts(),
			stats.GetInitiatorFlowBytes(),
			stats.GetResponderFlowPkts(),
			stats.GetResponderFlowBytes())
	}
}
