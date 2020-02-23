//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"sort"
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
	Run:   uplinkShowSpecCmdHandler,
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
	client := halproto.NewInterfaceClient(c)

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
		uplinkShowSpecHeader()
	} else if status == true {
		uplinkShowStatusHeader()
	} else {
		uplinkShowHeader()
	}

	// Print IFs
	m := make(map[uint64]*halproto.InterfaceGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		m[resp.GetSpec().GetKeyOrHandle().GetInterfaceId()] = resp
	}
	var keys []uint64
	for k := range m {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	for _, k := range keys {
		if spec == true {
			uplinkShowSpecOneResp(m[k])
		} else if status == true {
			uplinkShowStatusOneResp(m[k])
		} else {
			uplinkShowOneResp(m[k])
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

	handleUplinkShowCmd(cmd, false, false)
}

func uplinkShowSpecCmdHandler(cmd *cobra.Command, args []string) {
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
	client := halproto.NewInterfaceClient(c)

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
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
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
	fmt.Printf("Id:       Interface ID                        Port:      Front Panel Port Number\n")
	fmt.Printf("Admin:    Admin Status                        Oper:      Oper Status\n")
	fmt.Printf("LportId:  Lport                               HwLifId:   Lif Id\n")
	fmt.Printf("UpIdx:    Uplink Idx                          HwPortNum: Asic Port Number\n")
	fmt.Printf("NatL2seg: Native L2seg                        #L2Segs:   Number of L2segs\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-16s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Port", "Admin", "Oper", "LportId", "HwLifId", "UpIdx", "HwPortNum", "NatL2seg", "#L2Segs")
	fmt.Println(hdrLine)
}

func uplinkShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK {
		return
	}
	fmt.Printf("%-16s%-10s%-10s%-10s%-10d%-10d%-10d%-10d%-10d%-10d\n",
		utils.IfIndexToStr(uint32(resp.GetSpec().GetKeyOrHandle().GetInterfaceId())),
		utils.IfIndexToStr(uint32(resp.GetSpec().GetIfUplinkInfo().GetPortNum())),
		strings.ToLower(strings.Replace(resp.GetSpec().GetAdminStatus().String(), "IF_STATUS_", "", -1)),
		strings.ToLower(strings.Replace(resp.GetStatus().GetIfStatus().String(), "IF_STATUS_", "", -1)),
		resp.GetStatus().GetUplinkInfo().GetUplinkLportId(),
		resp.GetStatus().GetUplinkInfo().GetHwLifId(),
		resp.GetStatus().GetUplinkInfo().GetUplinkIdx(),
		resp.GetStatus().GetUplinkInfo().GetHwPortNum(),
		resp.GetSpec().GetIfUplinkInfo().GetNativeL2SegmentId(),
		resp.GetStatus().GetUplinkInfo().GetNumL2Segs())
}

func uplinkShowSpecHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:       Interface ID                        UpPort:   Uplink's Port\n")
	fmt.Printf("UpNL2seg: Uplink's Native L2seg\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-16s%-10s%-10s\n",
		"Id", "Port", "NatL2seg")
	fmt.Println(hdrLine)
}

func uplinkShowSpecOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK {
		return
	}
	fmt.Printf("%-16s%-10s%-10d\n",
		utils.IfIndexToStr(uint32(resp.GetSpec().GetKeyOrHandle().GetInterfaceId())),
		utils.IfIndexToStr(uint32(resp.GetSpec().GetIfUplinkInfo().GetPortNum())),
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
	fmt.Printf("%-16s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "Status", "LportId", "HwLifId", "UpIdx", "HwPortNum", "#L2Segs")
	fmt.Println(hdrLine)
}

func uplinkShowStatusOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK {
		return
	}
	fmt.Printf("%-16s%-10d%-10s%-10d%-10d%-10d%-10d%-10d\n",
		utils.IfIndexToStr(uint32(resp.GetSpec().GetKeyOrHandle().GetInterfaceId())),
		resp.GetStatus().GetIfHandle(),
		strings.ToLower(strings.Replace(resp.GetStatus().GetIfStatus().String(), "IF_STATUS_", "", -1)),
		resp.GetStatus().GetUplinkInfo().GetUplinkLportId(),
		resp.GetStatus().GetUplinkInfo().GetHwLifId(),
		resp.GetStatus().GetUplinkInfo().GetUplinkIdx(),
		resp.GetStatus().GetUplinkInfo().GetHwPortNum(),
		resp.GetStatus().GetUplinkInfo().GetNumL2Segs())
}
