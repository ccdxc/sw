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
	uplinkID uint64
)

var uplinkShowCmd = &cobra.Command{
	Use:   "uplink",
	Short: "show uplink interface information",
	Long:  "show uplink interface object information",
	Run:   uplinkShowCmdHandler,
}

var uplinkSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "spec",
	Long:  "shows uplink spec",
	Run:   uplinkShowCmdHandler,
}

var uplinkStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "status",
	Long:  "shows uplink status",
	Run:   uplinkShowStatusCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(uplinkShowCmd)
	uplinkShowCmd.AddCommand(uplinkSpecShowCmd)
	uplinkShowCmd.AddCommand(uplinkStatusShowCmd)

	uplinkShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	uplinkShowCmd.Flags().Uint64Var(&uplinkID, "id", 1, "Specify if-id")
	uplinkSpecShowCmd.Flags().Uint64Var(&uplinkID, "id", 1, "Specify if-id")
	uplinkStatusShowCmd.Flags().Uint64Var(&uplinkID, "id", 1, "Specify if-id")
}

func handleUplinkShowCmd(cmd *cobra.Command, spec bool, status bool) {
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
					InterfaceId: uplinkID,
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
		uplinkShowHeader()
	} else if status == true {
		uplinkShowStatusHeader()
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		if spec == true {
			uplinkShowOneResp(resp)
		} else if status == true {
			uplinkShowStatusOneResp(resp)
		}
	}
}

func uplinkShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		uplinkDetailShowCmdHandler(cmd, args)
		return
	}

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleUplinkShowCmd(cmd, true, false)
}

func uplinkShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleUplinkShowCmd(cmd, false, true)
}

func uplinkDetailShowCmdHandler(cmd *cobra.Command, args []string) {
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
					InterfaceId: uplinkID,
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

func uplinkShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:       Interface ID                        Ifype:    Interface type\n")
	fmt.Printf("UpPort:   Uplink's Port                       UpNL2seg: Uplink's Native L2seg\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 40)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s\n",
		"Id", "IfType", "UpPort", "UpNL2seg")
	fmt.Println(hdrLine)
}

func uplinkShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK {
		return
	}
	fmt.Printf("%-10d%-10s%-10d%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		ifTypeToStr(ifType),
		resp.GetSpec().GetIfUplinkInfo().GetPortNum(),
		resp.GetSpec().GetIfUplinkInfo().GetNativeL2SegmentId())
}

func uplinkShowStatusHeader() {
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

func uplinkShowStatusOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK {
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
