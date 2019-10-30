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
	ifID  uint32
)

var lifShowCmd = &cobra.Command{
	Use:   "lif",
	Short: "show lif information",
	Long:  "show lif object information",
	Run:   lifShowCmdHandler,
}

var ifShowCmd = &cobra.Command{
	Use:   "interface",
	Short: "show interface information",
	Long:  "show interface object information",
	Run:   ifShowCmdHandler,
}

func init() {
	showCmd.AddCommand(lifShowCmd)
	showCmd.AddCommand(ifShowCmd)
	lifShowCmd.Flags().Uint32Var(&lifID, "id", 0, "Specify Lif ID")
	ifShowCmd.Flags().Uint32Var(&ifID, "id", 0, "Specify interface ID")
}

func ifShowCmdHandler(cmd *cobra.Command, args []string) {
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

	var req *pds.InterfaceGetRequest

	if cmd.Flags().Changed("id") == false {
		req = &pds.InterfaceGetRequest{
			Id: []uint32{},
		}
	} else {
		req = &pds.InterfaceGetRequest{
			Id: []uint32{ifID},
		}
	}

	client := pds.NewIfSvcClient(c)

	respMsg, err := client.InterfaceGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Get Lif failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	printIfHeader()
	for _, resp := range respMsg.Response {
		printIf(resp)
	}
}

func printIfHeader() {
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-14s%-10s%-11s%-8s%-6s%-18s%-14s%-20s\n",
		"ID", "Type", "AdminState", "Port",
		"VPC", "IPPrefix", "Encap", "MACAddress")
	fmt.Println(hdrLine)
}

func printIf(intf *pds.Interface) {
	spec := intf.GetSpec()
	ifIndex := spec.GetId()
	ifType := strings.Replace(spec.GetType().String(), "IF_TYPE_", "", -1)
	ifType = strings.Replace(ifType, "_", "-", -1)
	adminState := strings.Replace(spec.GetAdminStatus().String(),
		"IF_STATUS_", "", -1)
	adminState = strings.Replace(adminState, "_", "-", -1)
	var portNum uint32
	vpc := "-"
	ipPrefix := "-"
	encap := "-"
	mac := "-"
	switch ifType {
	case "UPLINK":
		portNum = spec.GetUplinkSpec().GetPortId()
	case "L3":
		portNum = spec.GetL3IfSpec().GetPortId()
		vpc = fmt.Sprint(spec.GetL3IfSpec().GetVpcId())
		ipPrefix = utils.IPPrefixToStr(spec.GetL3IfSpec().GetPrefix())
		mac = utils.MactoStr(spec.GetL3IfSpec().GetMACAddress())
		encap = utils.EncapToString(spec.GetL3IfSpec().GetEncap())
	}
	fmt.Printf("0x%-12x%-10s%-11s%-8d%-6s%-18s%-14s%-20s\n",
		ifIndex, ifType, adminState, portNum,
		vpc, ipPrefix, encap, mac)
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
	hdrLine := strings.Repeat("-", 62)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-15s%-16s%-25s\n",
		"ID", "Name", "PinnedInterface", "Type")
	fmt.Println(hdrLine)
}

func printLif(lif *pds.Lif) {
	spec := lif.GetSpec()
	lifType := strings.Replace(spec.GetType().String(), "LIF_TYPE_", "", -1)
	lifType = strings.Replace(lifType, "_", "-", -1)
	fmt.Printf("%-6d%-15s%-16s%-25s\n",
		spec.GetLifId(), "",
		ifIndexToPortIdStr(spec.GetPinnedInterfaceId()),
		lifType)
}
