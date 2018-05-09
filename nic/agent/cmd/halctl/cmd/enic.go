//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	enicID       uint64
	enicDetailID uint64
)

var enicShowCmd = &cobra.Command{
	Use:   "enic",
	Short: "show enic interface information",
	Long:  "show enic interface object information",
	Run:   enicShowCmdHandler,
}

var enicDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed information about enic interface",
	Long:  "show detailed information about enic interface object",
	Run:   enicDetailShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(enicShowCmd)
	enicShowCmd.AddCommand(enicDetailShowCmd)

	enicShowCmd.Flags().Uint64Var(&enicID, "id", 1, "Specify if-id")
	enicDetailShowCmd.Flags().Uint64Var(&enicDetailID, "id", 1, "Specify if-id")
}

func enicShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

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
		log.Errorf("Getting if failed. %v", err)
	}

	// Print Header
	enicShowHeader(cmd, args)

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		enicShowOneResp(resp)
	}
	c.Close()
}

func enicDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	var req *halproto.InterfaceGetRequest
	if cmd.Flags().Changed("detId") {
		// Get specific if
		req = &halproto.InterfaceGetRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: enicDetailID,
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
		log.Errorf("Getting if failed. %v", err)
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
	c.Close()
}

func enicShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("Id:     Interface ID         Handle: IF's handle\n")
	fmt.Printf("Ifype:  Interface type       EType:  Enic type\n")
	fmt.Printf("EL2seg: Enic's l2seg         Emac:   Enic's mac\n")
	fmt.Printf("Encap:  Enic's encap         ELif:   Enic's Lif\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 90)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-20s%-10s%-10s\n",
		"Id", "Handle", "IfType", "EType", "EL2seg", "Emac", "Eencap", "ELif")
	fmt.Println(hdrLine)
}

func enicShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_ENIC {
		return
	}
	enicType := resp.GetSpec().GetIfEnicInfo().GetEnicType()
	fmt.Printf("%-10d%-10d%-10s%-10s",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		resp.GetStatus().GetIfHandle(),
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
			resp.GetSpec().GetIfEnicInfo().GetClassicEnicInfo().GetNativeL2SegmentHandle(),
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
