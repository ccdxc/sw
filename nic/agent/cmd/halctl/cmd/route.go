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

var routeShowCmd = &cobra.Command{
	Use:   "route",
	Short: "show route objects",
	Long:  "show route object infromation",
	Run:   routeShowCmdHandler,
}

func init() {
	showCmd.AddCommand(routeShowCmd)

	routeShowCmd.Flags().Bool("yaml", false, "Output in yaml")
}

func routeShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		routeDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *halproto.RouteGetRequest
	// Get all Routes
	req = &halproto.RouteGetRequest{}
	routeGetReqMsg := &halproto.RouteGetRequestMsg{
		Request: []*halproto.RouteGetRequest{req},
	}

	// HAL call
	respMsg, err := client.RouteGet(context.Background(), routeGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Route failed. %v\n", err)
		return
	}

	// Print Header
	routeShowHeader()

	// Print Routes
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		routeShowOneResp(resp)
	}
}

func routeDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *halproto.RouteGetRequest
	// Get all Routes
	req = &halproto.RouteGetRequest{}
	routeGetReqMsg := &halproto.RouteGetRequestMsg{
		Request: []*halproto.RouteGetRequest{req},
	}

	// HAL call
	respMsg, err := client.RouteGet(context.Background(), routeGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Route failed. %v\n", err)
		return
	}

	// Print Routes
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

func routeShowHeader() {
	hdrLine := strings.Repeat("-", 50)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-20s%-10s%-10s\n",
		"VrfId", "Prefix", "Handle", "NHHandle")
	fmt.Println(hdrLine)
}

func routeShowOneResp(resp *halproto.RouteGetResponse) {
	fmt.Printf("%-10d%-20s%-10d%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetRouteKey().GetVrfKeyHandle().GetVrfId(),
		utils.IPPrefixToStr(resp.GetSpec().GetKeyOrHandle().GetRouteKey().GetIpPrefix()),
		resp.GetStatus().GetRouteHandle(),
		resp.GetSpec().GetNhKeyOrHandle().GetNexthopHandle())
}
