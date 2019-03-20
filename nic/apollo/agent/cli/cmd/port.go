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
	portID uint32
)

var portShowCmd = &cobra.Command{
	Use:   "port",
	Short: "show port information",
	Long:  "show port object information",
}

var portStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show port statistics",
	Long:  "show port statistics",
	Run:   portShowCmdHandler,
}

func init() {
	showCmd.AddCommand(portShowCmd)
	portShowCmd.AddCommand(portStatsShowCmd)
	portStatsShowCmd.Flags().Uint32VarP(&portID, "id", "i", 0, "Specify Port ID")
}

func portShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewPortSvcClient(c)

	var req *pds.PortGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific Port
		req = &pds.PortGetRequest{
			PortId: []uint32{portID},
		}
	} else {
		// Get all Ports
		req = &pds.PortGetRequest{
			PortId: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.PortGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Port failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	printPortStatsHeader()

	// Print Ports
	for _, resp := range respMsg.Response {
		printPortStats(resp)
	}
}

func printPortStatsHeader() {
	hdrLine := strings.Repeat("-", 37)
	fmt.Println(hdrLine)
	fmt.Printf("%-7s%-25s%-5s\n", "PortId", "Field", "Count")
	fmt.Println(hdrLine)
}

func printPortStats(resp *pds.Port) {
	first := true
	macStats := resp.GetStats().GetMacStats()
	mgmtMacStats := resp.GetStats().GetMgmtMacStats()

	fmt.Printf("%-7d", resp.GetSpec().GetPortId())
	for _, s := range macStats {
		if first == false {
			fmt.Printf("%-7s", "")
		}
		fmt.Printf("%-25s%-5d\n",
			strings.Replace(s.GetType().String(), "_", " ", -1),
			s.GetCount())
		first = false
	}

	first = true
	for _, s := range mgmtMacStats {
		if first == false {
			fmt.Printf("%-7s", "")
		}
		str := strings.Replace(s.GetType().String(), "MGMT_MAC_", "", -1)
		str = strings.Replace(str, "_", " ", -1)
		fmt.Printf("%-25s%-5d\n", str, s.GetCount())
		first = false
	}
}
