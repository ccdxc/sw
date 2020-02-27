//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bytes"
	"context"
	"fmt"
	"strings"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	lifID string
	ifID  string
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
	lifShowCmd.Flags().StringVar(&lifID, "id", "", "Specify Lif ID")
	ifShowCmd.Flags().StringVar(&ifID, "id", "", "Specify interface ID")
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

	if cmd == nil || cmd.Flags().Changed("id") == false {
		req = &pds.InterfaceGetRequest{
			Id: [][]byte{},
		}
	} else {
		req = &pds.InterfaceGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(ifID).Bytes()},
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
	hdrLine := strings.Repeat("-", 177)
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-14s%-11s%-11s%-40s%-6s%-40s%-18s%-14s%-20s\n",
		"IfIndex", "Interface", "AdminState", "OperState", "Port", "LifID",
		"VPC", "IPPrefix", "Encap", "MACAddress")
	fmt.Println(hdrLine)
}

func printIf(intf *pds.Interface) {
	spec := intf.GetSpec()
	status := intf.GetStatus()
	ifIndex := status.GetIfIndex()
	ifStr := ifIndexToPortIdStr(ifIndex)
	adminState := strings.Replace(spec.GetAdminStatus().String(),
		"IF_STATUS_", "", -1)
	adminState = strings.Replace(adminState, "_", "-", -1)
	operState := strings.Replace(status.GetOperStatus().String(),
		"IF_STATUS_", "", -1)
	operState = strings.Replace(operState, "_", "-", -1)
	portNum := ""
	vpc := "-"
	ipPrefix := "-"
	encap := "-"
	mac := "-"
	lifId := "-"
	switch spec.GetType() {
	case pds.IfType_IF_TYPE_UPLINK:
		lifId = fmt.Sprint(status.GetUplinkIfStatus().GetLifId())
		portNum = uuid.FromBytesOrNil(spec.GetUplinkSpec().GetPortId()).String()
	case pds.IfType_IF_TYPE_L3:
		portNum = uuid.FromBytesOrNil(spec.GetL3IfSpec().GetPortId()).String()
		vpc = uuid.FromBytesOrNil(spec.GetL3IfSpec().GetVpcId()).String()
		ipPrefix = utils.IPPrefixToStr(spec.GetL3IfSpec().GetPrefix())
		mac = utils.MactoStr(spec.GetL3IfSpec().GetMACAddress())
		encap = utils.EncapToString(spec.GetL3IfSpec().GetEncap())
	}
	fmt.Printf("0x%-12x%-14s%-11s%-11s%-40s%-6s%-6s%-18s%-14s%-20s\n",
		ifIndex, ifStr, adminState, operState, portNum, lifId,
		vpc, ipPrefix, encap, mac)
}

func lifGetNameFromKey(key []byte) string {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		return "-"
	}
	defer c.Close()

	invalidUuid := make([]byte, 16)
	if bytes.Equal(key, invalidUuid) {
		return "-"
	}

	req := &pds.LifGetRequest{
		Id: [][]byte{key},
	}

	client := pds.NewIfSvcClient(c)
	respMsg, err := client.LifGet(context.Background(), req)
	if err != nil {
		return "-"
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		return "-"
	}
	resp := respMsg.Response[0]
	return resp.GetStatus().GetName()
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

	if cmd == nil || cmd.Flags().Changed("id") == false {
		req = &pds.LifGetRequest{
			Id: [][]byte{},
		}
	} else {
		req = &pds.LifGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(lifID).Bytes()},
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
	hdrLine := strings.Repeat("-", 155)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-10s%-15s%-20s%-40s%-25s%-5s\n",
		"ID", "IfIndex", "Name", "MAC Address", "PinnedInterface",
		"Type", "State")
	fmt.Println(hdrLine)
}

func printLif(lif *pds.Lif) {
	spec := lif.GetSpec()
	status := lif.GetStatus()
	lifType := strings.Replace(spec.GetType().String(), "LIF_TYPE_", "", -1)
	lifType = strings.Replace(lifType, "_", "-", -1)
	state := strings.Replace(status.GetStatus().String(), "IF_STATUS_", "", -1)
	fmt.Printf("%-40s%-10x%-15s%-20s%-40s%-25s%-5s\n",
		uuid.FromBytesOrNil(spec.GetId()).String(), status.GetIfIndex(),
		status.GetName(), utils.MactoStr(spec.GetMacAddress()),
		uuid.FromBytesOrNil(spec.GetPinnedInterface()).String(), lifType, state)
}
