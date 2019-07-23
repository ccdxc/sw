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
	lifID uint32
)

var lifShowCmd = &cobra.Command{
	Use:   "lif",
	Short: "show lif information",
	Long:  "show lif object information",
	Run:   lifShowCmdHandler,
}

func init() {
	showCmd.AddCommand(lifShowCmd)
	lifShowCmd.Flags().Uint32Var(&lifID, "id", 0, "Specify Lif ID")
}

func lifShowCmdHandler(cmd *cobra.Command, args []string) {
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

	var req *pds.LifGetRequest

	if cmd.Flags().Changed("id") == false {
		req = &pds.LifGetRequest{
			LifId: []uint32{},
		}
	} else {
		req = &pds.LifGetRequest{
			LifId: []uint32{lifID},
		}
	}

	client := pds.NewIfSvcClient(c)

	respMsg, err := client.LifGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Get Lif failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	printLifHeader()
	for _, resp := range respMsg.Response {
		printLif(resp)
	}
}

func printLifHeader() {
	hdrLine := strings.Repeat("-", 45)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-14s%-25s\n",
		"ID", "PinnedIfID", "LifType")
	fmt.Println(hdrLine)
}

func printLif(lif *pds.Lif) {
	spec := lif.GetSpec()
	lifType := strings.Replace(spec.GetType().String(), "LIF_TYPE_", "", -1)
	lifType = strings.Replace(lifType, "_", "-", -1)
	fmt.Printf("%-6d%-14s%-25s\n",
		spec.GetLifId(),
		ifIndexToPortIdStr(spec.GetPinnedInterfaceId()),
		lifType)
}
