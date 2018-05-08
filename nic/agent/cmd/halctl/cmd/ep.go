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

var epShowCmd = &cobra.Command{
	Use:   "endpoint",
	Short: "show endpoint information",
	Long:  "show endpoint object information",
	Run:   epShowCmdHandler,
}

var epPdShowCmd = &cobra.Command{
	Use:   "pd",
	Short: "pd",
	Long:  "shows ep pd",
	Run:   epPdShowCmdHandler,
}

var epDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed endpoint information",
	Long:  "show detailed information about endpoint objects",
	Run:   epDetailShowCmdHandler,
}

func init() {
	showCmd.AddCommand(epShowCmd)
	epShowCmd.AddCommand(epPdShowCmd)
	epShowCmd.AddCommand(epDetailShowCmd)
}

func epShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewEndpointClient(c.ClientConn)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		log.Errorf("Getting Endpoint failed. %v", err)
	}

	// Print Header
	epShowHeader(cmd, args)

	// Print VRFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		epShowOneResp(resp)
	}
}

func epPdShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewEndpointClient(c.ClientConn)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		log.Errorf("Getting Endpoint failed. %v", err)
	}

	// Print Header
	epPdShowHeader(cmd, args)

	// Print EPs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		epPdShowOneResp(resp)
	}
}

func epDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewEndpointClient(c.ClientConn)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		log.Errorf("Getting Endpoint failed. %v", err)
	}

	// Print EPs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func epShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("EPHandle:  EP's Handle                       L2SegID: EP's L2seg ID\n")
	fmt.Printf("Mac:       EP's Mac                          IfId:    IF on which EP was learnt\n")
	fmt.Printf("IsLocal:   EP's location                     #IPs:    EP's IPs\n")
	fmt.Printf("IPs:       EP's IPs\n")
	hdrLine := strings.Repeat("-", 120)
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-12s%-24s%-10s%-10s%-10s%-20s\n",
		"EPHandle", "L2SegID", "Mac", "IfId", "IsLocal", "#IPs", "IPs")
	fmt.Println(hdrLine)
}

func epShowOneResp(resp *halproto.EndpointGetResponse) {
	epAddr := resp.GetStatus().GetIpAddress()
	ipStr := utils.IPAddrToStr(epAddr[0].GetIpAddress())
	if len(epAddr) > 0 {
		for i := 1; i < len(epAddr); i++ {
			ipStr += ", "
			ipStr += utils.IPAddrToStr(epAddr[i].GetIpAddress())
		}
	}
	macStr := utils.MactoStr(resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetMacAddress())
	fmt.Printf("%-12d%-12d%-24s%-10d%-10t%-10d%-20s\n",
		resp.GetStatus().GetEndpointHandle(),
		resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetL2SegmentKeyHandle().GetSegmentId(),
		macStr,
		resp.GetSpec().GetEndpointAttrs().GetInterfaceKeyHandle().GetInterfaceId(),
		resp.GetStatus().GetIsEndpointLocal(),
		len(resp.GetStatus().GetIpAddress()),
		ipStr)
}

func epPdShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("EPHandle:  EP's Handle                       L2SegID: EP's L2seg ID\n")
	fmt.Printf("MacTblIdx: Registered MAC table Idx.         RwTblIdx: Rewrite table Idx.\n")
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-12s%-12s%-36s\n",
		"EPHandle", "L2SegID", "MacTblIdx", "RwTblIdx")
	fmt.Println(hdrLine)
}

func epPdShowOneResp(resp *halproto.EndpointGetResponse) {
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

	fmt.Printf("%-12d%-12d%-12s%-36s\n",
		resp.GetStatus().GetEndpointHandle(),
		resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetL2SegmentKeyHandle().GetSegmentId(),
		regMacTblStr,
		rwTblStr)
}
