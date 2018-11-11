//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	enicID uint64
)

var enicShowCmd = &cobra.Command{
	Use:   "enic",
	Short: "show enic interface information",
	Long:  "show enic interface object information",
	Run: func(cmd *cobra.Command, args []string) {
		// cmd.Help()
		enicShowCmdHandler(cmd, args)
	},
	// Run:   enicShowCmdHandler,
}

var enicSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "show spec information about enic interface",
	Long:  "show spec information about enic interface object",
	Run:   enicShowCmdHandler,
}

var enicStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show status information about enic interface",
	Long:  "show status information about enic interface object",
	Run:   enicShowStatusCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(enicShowCmd)
	enicShowCmd.AddCommand(enicSpecShowCmd)
	enicShowCmd.AddCommand(enicStatusShowCmd)
	enicShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	enicShowCmd.Flags().Uint64Var(&enicID, "id", 1, "Specify if-id")
	enicSpecShowCmd.Flags().Uint64Var(&enicID, "id", 1, "Specify if-id")
	enicStatusShowCmd.Flags().Uint64Var(&enicID, "id", 1, "Specify if-id")
}

func handleEnicShowCmd(cmd *cobra.Command, spec bool, status bool) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	defer c.Close()

	var req *halproto.InterfaceGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific if
		req = &halproto.InterfaceGetRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: enicID,
				},
			},
		}
	} else {
		// Get all ifs
		req = &halproto.InterfaceGetRequest{}
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	// HAL call
	respMsg, err := client.InterfaceGet(context.Background(), ifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting if failed. %v\n", err)
		return
	}

	// Print Header
	if spec == true {
		enicShowHeader()
	} else if status == true {
		enicShowStatusHeader()
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		if spec == true {
			enicShowOneResp(resp)
		} else if status == true {
			enicShowStatusOneResp(resp)
		}
	}
}

func enicShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("yaml") {
		enicDetailShowCmdHandler(cmd, nil)
		return
	}

	handleEnicShowCmd(cmd, true, false)
}

func enicShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleEnicShowCmd(cmd, false, true)
}

func enicDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	defer c.Close()

	var req *halproto.InterfaceGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific if
		req = &halproto.InterfaceGetRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: enicID,
				},
			},
		}
	} else {
		// Get all ifs
		req = &halproto.InterfaceGetRequest{}
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	// HAL call
	respMsg, err := client.InterfaceGet(context.Background(), ifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting if failed. %v\n", err)
		return
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func enicShowStatusHeader() {
	fmt.Printf("\n")
	fmt.Printf("Handle:  IF's handle          Status:    IF's status\n")
	fmt.Printf("LportId: LPort ID             UplnkHndl: Uplink IF handle\n")
	fmt.Printf("MacVlanIdxHost:  Input prop. Mac Vlan table idx from host packets\n")
	fmt.Printf("MacVlanIdxNet:   Input prop. Mac Vlan table idx from network packets\n")
	fmt.Printf("NatL2SegClassic: Input prop. table idx for native l2seg. Classic mode\n")
	fmt.Printf("L2SegMmbrInfo:   L2seg membership info for Enic. Classic mode\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 104)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-16s%-16s%-16s%-16s\n",
		"Handle", "Status", "LportId", "UplnkHndl", "MacVlanIdxHost", "MacVlanIdxNet", "NatL2SegClassic", "L2SegMmbrInfo")
	fmt.Println(hdrLine)
}

func enicShowStatusOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_ENIC {
		return
	}

	fmt.Printf("%-10d%-10s%-10d%-10d%-16d%-16d%-16d",
		resp.GetStatus().GetIfHandle(),
		strings.ToLower(strings.Replace(resp.GetStatus().GetIfStatus().String(), "IF_STATUS_", "", -1)),
		resp.GetStatus().GetEnicInfo().GetEnicLportId(),
		resp.GetStatus().GetEnicInfo().GetUplinkIfHandle(),
		resp.GetStatus().GetEnicInfo().GetSmartEnicInfo().GetInpPropMacVlanIdxHost(),
		resp.GetStatus().GetEnicInfo().GetSmartEnicInfo().GetInpPropMacVlanIdxNet(),
		resp.GetStatus().GetEnicInfo().GetClassicEnicInfo().GetInpPropNatL2SegClassic())

	memberStr := ""
	first := true
	count := 0
	for _, member := range resp.GetStatus().GetEnicInfo().GetClassicEnicInfo().GetMembershipInfo() {
		if first == true {
			memberStr += fmt.Sprintf("%d/%d", member.GetL2SegmentKeyOrHandle().GetL2SegmentHandle(), member.GetInpPropIdx())
			first = false
		} else {
			memberStr += fmt.Sprintf(", %d/%d", member.GetL2SegmentKeyOrHandle().GetL2SegmentHandle(), member.GetInpPropIdx())
		}
		count++
		if count == 3 {
			count = 0
			memberStr += fmt.Sprintf("\n%-88s", " ")
		}
	}
	memberStr += "\n"
	fmt.Printf("%-16s", memberStr)
	fmt.Printf("\n")
}

func enicShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:     Interface ID         Handle: IF's handle\n")
	fmt.Printf("Ifype:  Interface type       EType:  Enic type\n")
	fmt.Printf("EL2seg: Enic's l2seg         Emac:   Enic's mac\n")
	fmt.Printf("Encap:  Enic's encap         ELif:   Enic's Lif\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 80)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-20s%-10s%-10s\n",
		"Id", "IfType", "EType", "EL2seg", "Emac", "Eencap", "ELif")
	fmt.Println(hdrLine)
}

func enicShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_ENIC {
		return
	}
	enicType := resp.GetSpec().GetIfEnicInfo().GetEnicType()
	fmt.Printf("%-10d%-10s%-10s",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		ifTypeToStr(ifType),
		enicTypeToStr(enicType))

	switch enicType {
	case halproto.IfEnicType_IF_ENIC_TYPE_USEG:
		fallthrough
	case halproto.IfEnicType_IF_ENIC_TYPE_PVLAN:
		fallthrough
	case halproto.IfEnicType_IF_ENIC_TYPE_DIRECT:
		fallthrough
	case halproto.IfEnicType_IF_ENIC_TYPE_GFT:
		macStr := utils.MactoStr(resp.GetSpec().GetIfEnicInfo().GetEnicInfo().GetMacAddress())
		fmt.Printf("%-10d%-20s%-10d%-10d",
			resp.GetSpec().GetIfEnicInfo().GetEnicInfo().GetL2SegmentKeyHandle().GetSegmentId(),
			macStr,
			resp.GetSpec().GetIfEnicInfo().GetEnicInfo().GetEncapVlanId(),
			resp.GetSpec().GetIfEnicInfo().GetLifKeyOrHandle().GetLifId())
	case halproto.IfEnicType_IF_ENIC_TYPE_CLASSIC:
		fmt.Printf("%-10d%-20s%-10s%-10d",
			resp.GetSpec().GetIfEnicInfo().GetClassicEnicInfo().GetNativeL2SegmentId(),
			"-",
			"-",
			resp.GetSpec().GetIfEnicInfo().GetLifKeyOrHandle().GetLifId())
	default:
		fmt.Printf("%-10s%-20s%-10s%-10s", "-", "-", "-", "-")
	}
	fmt.Printf("\n")

}

func enicTypeToStr(enicType halproto.IfEnicType) string {
	switch enicType {
	case halproto.IfEnicType_IF_ENIC_TYPE_USEG:
		return "Useg"
	case halproto.IfEnicType_IF_ENIC_TYPE_PVLAN:
		return "Pvlan"
	case halproto.IfEnicType_IF_ENIC_TYPE_DIRECT:
		return "Direct"
	case halproto.IfEnicType_IF_ENIC_TYPE_CLASSIC:
		return "Classic"
	case halproto.IfEnicType_IF_ENIC_TYPE_GFT:
		return "GFT"
	default:
		return "Invalid"
	}
}
