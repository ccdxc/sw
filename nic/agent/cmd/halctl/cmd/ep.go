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

var epShowCmd = &cobra.Command{
	Use:   "endpoint",
	Short: "show endpoint information",
	Long:  "show endpoint object information",
	Run:   epShowCmdHandler,
}

var epShowBriefCmd = &cobra.Command{
	Use:   "brief",
	Short: "show endpoint information",
	Long:  "show endpoint object information",
	Run:   epShowBriefCmdHandler,
}

var epShowSpecCmd = &cobra.Command{
	Use:   "spec",
	Short: "show endpoint spec information",
	Long:  "show endpoint object spec information",
	Run:   epShowCmdHandler,
}

var epShowStatusCmd = &cobra.Command{
	Use:   "status",
	Short: "show endpoint status information",
	Long:  "show status information about endpoint objects",
	Run:   epStatusShowCmdHandler,
}

var filterShowCmd = &cobra.Command{
	Use:   "filter",
	Short: "show filter object information",
	Long:  "show filter object information",
	Run:   filterShowCmdHandler,
}

func init() {
	showCmd.AddCommand(epShowCmd)
	epShowCmd.AddCommand(epShowBriefCmd)
	epShowCmd.AddCommand(epShowSpecCmd)
	epShowCmd.AddCommand(epShowStatusCmd)
	epShowCmd.Flags().Bool("yaml", false, "Output in yaml")

	showCmd.AddCommand(filterShowCmd)
	filterShowCmd.Flags().Bool("yaml", false, "Output in yaml")
}

func filterShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := halproto.NewEndpointClient(c)

	// Get filter
	req := &halproto.FilterGetRequest{}
	filterGetReqMsg := &halproto.FilterGetRequestMsg{
		Request: []*halproto.FilterGetRequest{req},
	}

	// HAL call
	respMsg, err := client.FilterGet(context.Background(), filterGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Filter failed. %v\n", err)
		return
	}

	if !(cmd.Flags().Changed("yaml")) {
		fmt.Printf("Only --yaml option supported\n")
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b) + "\n")
		fmt.Println("---")
	}
}

func epShowBriefCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		return
	}
	defer c.Close()

	client := halproto.NewEndpointClient(c)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Endpoint failed. %v\n", err)
		return
	}

	// Print Header
	epShowBriefHeader(cmd, args)

	ifIDToStr := ifGetAllStr()

	// Print endpoints
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		epShowBriefOneResp(resp, ifIDToStr)
	}
}

func epShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		if len(args) > 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
		handleEpDetailShowCmd(cmd, nil)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		if strings.Compare(args[0], "spec") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

	client := halproto.NewEndpointClient(c)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Endpoint failed. %v\n", err)
		return
	}

	// Print Header
	epShowHeader(cmd, args)

	// Print endpoints
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		epShowOneResp(resp)
	}
}

func epStatusShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := halproto.NewEndpointClient(c)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Endpoint failed. %v\n", err)
		return
	}

	// Print Header
	epStatusShowHeader(cmd, args)

	// Print EPs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}

		uplinkIf := resp.GetStatus().GetEnicPinnedUplinkIfKeyHandle()
		var uplink uint64
		if uplinkIf != nil {
			uplink = uplinkIf.GetIfHandle()
		} else {
			uplink = 0
		}
		uplinkStr := "-"
		// Retrieve uplink interface from handle
		if uplink != 0 {
			ifReq := &halproto.InterfaceGetRequest{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_IfHandle{
						IfHandle: uplink,
					},
				},
			}
			ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
				Request: []*halproto.InterfaceGetRequest{ifReq},
			}

			// HAL call
			hClient := halproto.NewInterfaceClient(c)
			hRespMsg, err := hClient.InterfaceGet(context.Background(), ifGetReqMsg)
			if err != nil {
				fmt.Printf("Getting if failed. %v\n", err)
				return
			}

			for _, hResp := range hRespMsg.Response {
				if hResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
					fmt.Printf("Operation failed with %v error\n", hResp.ApiStatus)
					continue
				}
				uplinkStr = fmt.Sprintf("uplink-%d", hResp.GetSpec().GetKeyOrHandle().GetInterfaceId())
			}
		}

		epStatusShowOneResp(uplinkStr, resp)
	}
}

func handleEpDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		return
	}
	defer c.Close()

	client := halproto.NewEndpointClient(c)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Endpoint failed. %v\n", err)
		return
	}

	// Print EPs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func epDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleEpDetailShowCmd(cmd, nil)
}

func epShowBriefHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 50)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-24s%-32s\n",
		"Vlan", "Mac", "Interface")
	fmt.Println(hdrLine)
}

func epShowBriefOneResp(resp *halproto.EndpointGetResponse, ifIDToStr map[uint64]string) {
	macStr := utils.MactoStr(resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetMacAddress())

	var ifID uint64
	ifID = resp.GetSpec().GetEndpointAttrs().GetInterfaceKeyHandle().GetInterfaceId()
	ifIDStr := ifIDToStr[ifID]
	l2segID := resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetL2SegmentKeyHandle().GetSegmentId()
	encap := l2segIDGetWireEncap(l2segID)
	if encap == 8192 {
		encap = 0
	}

	fmt.Printf("%-10d%-24s%-32s\n",
		encap, macStr, ifIDStr)
}

func epShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("Handle       : Endpoint's Handle                       L2SegID : Endpoint's L2seg ID\n")
	fmt.Printf("Mac          : Endpoint's Mac                          IfId    : Interface on which EP was learnt\n")
	fmt.Printf("IsLocal      : Endpoint's location                     Vlan    : Endpoint's Vlan\n")
	fmt.Printf("#IPs         : Endpoint's IPs                          IPs     : Endpoint's IPs\n")
	hdrLine := strings.Repeat("-", 120)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-21s%-15s%-10s%-5s%-5s%-20s\n",
		"Handle", "L2SegID", "Mac", "IfId", "IsLocal", "Vlan", "#IPs", "IPs")
	fmt.Println(hdrLine)
}

func epShowOneResp(resp *halproto.EndpointGetResponse) {
	var ipStr string

	epAddr := resp.GetStatus().GetIpAddress()
	if (epAddr != nil) && (len(epAddr) > 0) {
		ipStr = utils.IPAddrToStr(epAddr[0].GetIpAddress())
		for i := 1; i < len(epAddr); i++ {
			ipStr += ", "
			ipStr += utils.IPAddrToStr(epAddr[i].GetIpAddress())
		}
	}

	macStr := utils.MactoStr(resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetMacAddress())

	var ifID []uint64
	var ifIDStr []string

	ifID = append(ifID, resp.GetSpec().GetEndpointAttrs().GetInterfaceKeyHandle().GetInterfaceId())
	ifIDStr = append(ifIDStr, "-")
	_, ifIDStr = ifGetStrFromID(ifID)

	fmt.Printf("%-10d%-10d%-21s%-15s%-10t%-5d%-5d%-20s\n",
		resp.GetStatus().GetKeyOrHandle().GetEndpointHandle(),
		resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetL2SegmentKeyHandle().GetSegmentId(),
		macStr,
		ifIDStr[0],
		resp.GetStatus().GetIsEndpointLocal(),
		resp.GetSpec().GetEndpointAttrs().GetUsegVlan(),
		len(resp.GetStatus().GetIpAddress()),
		ipStr)
}

func epStatusShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("Handle       : Endpoint's Handle                L2SegID     : Endpoint's L2seg ID\n")
	fmt.Printf("MacTblIdx    : Registered MAC table Index       RwTblIdx    : Rewrite table Index\n")
	fmt.Printf("EnicUplinkID : Enic Uplink If ID\n")
	hdrLine := strings.Repeat("-", 85)
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-13s%-12s%-12s%-36s\n",
		"Handle", "EnicUplinkID", "L2SegID", "MacTblIdx", "RwTblIdx")
	fmt.Println(hdrLine)
}

func epStatusShowOneResp(uplinkStr string, resp *halproto.EndpointGetResponse) {
	epd := resp.GetStatus().GetEpdStatus()

	rwTblStr := ""
	first := true
	rwTblIdx := epd.GetRwTblIdx()

	for _, val := range rwTblIdx {
		if first == true {
			first = false
			rwTblStr += fmt.Sprintf("%d", val)
		} else {
			rwTblStr += fmt.Sprintf(", %d", val)
		}
	}

	if len(rwTblStr) == 0 {
		rwTblStr += "-"
	}

	regMacTblIdx := epd.GetRegMacTblIdx()
	regMacTblStr := ""
	if regMacTblIdx == 0xFFFFFFFF {
		regMacTblStr = "-"
	} else {
		regMacTblStr = fmt.Sprintf("%d", regMacTblIdx)
	}

	fmt.Printf("%-12d%-13s%-12d%-12s%-36s\n",
		resp.GetStatus().GetKeyOrHandle().GetEndpointHandle(),
		uplinkStr,
		resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetL2SegmentKeyHandle().GetSegmentId(),
		regMacTblStr,
		rwTblStr)
}
