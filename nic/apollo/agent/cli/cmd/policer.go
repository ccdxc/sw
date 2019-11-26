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
	policerID uint32
)

var policerShowCmd = &cobra.Command{
	Use:   "policer",
	Short: "show  policer information",
	Long:  "show policer object information",
	Run:   policerShowCmdHandler,
}

func init() {
	showCmd.AddCommand(policerShowCmd)
	policerShowCmd.Flags().Uint32VarP(&policerID, "id", "i", 0, "Specify policer ID")
}

func policerShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewPolicerSvcClient(c)

	var req *pds.PolicerGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific policer
		req = &pds.PolicerGetRequest{
			Id: []uint32{policerID},
		}
	} else {
		// Get all Mirror sessions
		req = &pds.PolicerGetRequest{
			Id: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.PolicerGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting mirror session failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print policer
	printPolicerHeader()
	for _, resp := range respMsg.Response {
		printPolicer(resp)
	}
}

func printPolicerHeader() {
	hdrLine := strings.Repeat("-", 60)
	fmt.Println(hdrLine)
	fmt.Printf("%-5s%-10s%-5s%-10s%-10s%-10s%-10s\n",
		"ID", "Direction", "Type", "PPS/BPS", "Burst", "AcceptCnt", "DropCnt")
	fmt.Println(hdrLine)
}

func printPolicer(p *pds.Policer) {
	spec := p.GetSpec()
	stats := p.GetStats()

	typeStr := ""
	var burst uint64
	var count uint64
	if spec.GetPPSPolicer() != nil {
		typeStr += "PPS"
		count = spec.GetPPSPolicer().GetPacketsPerSecond()
		burst = spec.GetPPSPolicer().GetBurst()
	} else if spec.GetBPSPolicer() != nil {
		typeStr += "BPS"
		count = spec.GetBPSPolicer().GetBytesPerSecond()
		burst = spec.GetBPSPolicer().GetBurst()
	}
	fmt.Printf("%-5d%-10s%-5s%-10d%-10d%-10d%-10d\n",
		spec.GetId(),
		strings.Replace(spec.GetDirection().String(), "POLICER_DIR_", "", -1),
		typeStr, count, burst,
		stats.GetAccept(), stats.GetDrop())
}
