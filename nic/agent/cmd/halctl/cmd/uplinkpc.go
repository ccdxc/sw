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
	uplinkPcID       uint64
	uplinkPcDetailID uint64
)

var uplinkPcShowCmd = &cobra.Command{
	Use:   "uplink-pc",
	Short: "show uplink-pc interface information",
	Long:  "show uplink-pc interface object information",
	Run:   uplinkPcShowCmdHandler,
}

var uplinkPcSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "show spec information about uplink-pc interface",
	Long:  "show spec information about uplink-pc interface object",
	Run:   uplinkPcShowCmdHandler,
}

var uplinkPcStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show status information about uplink-pc interface",
	Long:  "show status information about uplink-pc interface object",
	Run:   uplinkPcShowStatusCmdHandler,
}

var uplinkPcDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed information about uplink-pc interface",
	Long:  "show detailed information about uplink-pc interface object",
	Run:   uplinkPcDetailShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(uplinkPcShowCmd)
	uplinkPcShowCmd.AddCommand(uplinkPcSpecShowCmd)
	uplinkPcShowCmd.AddCommand(uplinkPcStatusShowCmd)
	uplinkPcShowCmd.AddCommand(uplinkPcDetailShowCmd)

	uplinkPcShowCmd.Flags().Uint64Var(&uplinkPcID, "id", 1, "Specify if-id")
	uplinkPcSpecShowCmd.Flags().Uint64Var(&uplinkPcID, "id", 1, "Specify if-id")
	uplinkPcStatusShowCmd.Flags().Uint64Var(&uplinkPcID, "id", 1, "Specify if-id")
	uplinkPcDetailShowCmd.Flags().Uint64Var(&uplinkPcDetailID, "id", 1, "Specify if-id")
}

func handleUplinkPcShowCmd(cmd *cobra.Command, spec bool, status bool) {
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
					InterfaceId: uplinkPcID,
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
		uplinkPcShowHeader()
	} else if status == true {
		uplinkPcShowStatusHeader()
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		if spec == true {
			uplinkPcShowOneResp(resp)
		} else if status == true {
			uplinkPcShowStatusOneResp(resp)
		}
	}
}

func uplinkPcShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleUplinkPcShowCmd(cmd, true, false)
}

func uplinkPcShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleUplinkPcShowCmd(cmd, false, true)
}

func uplinkPcDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *halproto.InterfaceGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific if
		req = &halproto.InterfaceGetRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: uplinkPcDetailID,
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

func uplinkPcShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:       Interface ID                        Ifype: Interface type\n")
	fmt.Printf("UpNL2seg: Uplink's Native L2seg               #Mbrs: Num. of member uplinks\n")
	fmt.Printf("Mbrs:     Uplink Members\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 50)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "IfType", "NatL2seg", "NMbrs", "Mbrs")
	fmt.Println(hdrLine)
}

func uplinkPcShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK_PC {
		return
	}
	mbrs := resp.GetSpec().GetIfUplinkPcInfo().GetMemberIfKeyHandle()
	fmt.Printf("%-10d%-10s%-10d%-10d",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		ifTypeToStr(ifType),
		resp.GetSpec().GetIfUplinkPcInfo().GetNativeL2SegmentId(),
		len(mbrs))

	for _, mbr := range mbrs {
		fmt.Printf("%-5d", mbr.GetInterfaceId())
	}
	if len(mbrs) == 0 {
		fmt.Printf("%-5s", "-")
	}
	fmt.Printf("\n")
}

func uplinkPcShowStatusHeader() {
	fmt.Printf("\n")
	fmt.Printf("Handle:   IF's handle                       Status:    IF's status\n")
	fmt.Printf("LportId:  Lport Id                          HwLifId:   Hw Lif ID\n")
	fmt.Printf("UpIdx:    Uplink Idx used in L2seg          HwPortNum: Hw Port Number\n")
	fmt.Printf("#L2Segs:  Number of L2segs\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Handle", "Status", "LportId", "HwLifId", "UpIdx", "HwPortNum", "#L2Segs")
	fmt.Println(hdrLine)
}

func uplinkPcShowStatusOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK_PC {
		return
	}
	fmt.Printf("%-10d%-10s%-10d%-10d%-10d%-10d%-10d\n",
		resp.GetStatus().GetIfHandle(),
		strings.ToLower(strings.Replace(resp.GetStatus().GetIfStatus().String(), "IF_STATUS_", "", -1)),
		resp.GetStatus().GetUplinkInfo().GetUplinkLportId(),
		resp.GetStatus().GetUplinkInfo().GetHwLifId(),
		resp.GetStatus().GetUplinkInfo().GetUplinkIdx(),
		resp.GetStatus().GetUplinkInfo().GetHwPortNum(),
		resp.GetStatus().GetUplinkInfo().GetNumL2Segs())
}
