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

var routeShowCmd = &cobra.Command{
	Use:   "route",
	Short: "show route objects",
	Long:  "show route object infromation",
	Run:   routeShowCmdHandler,
}

var routeDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed route information",
	Long:  "shows detailed information about route objects",
	Run:   routeDetailShowCmdHandler,
}

func init() {
	showCmd.AddCommand(routeShowCmd)
	routeShowCmd.AddCommand(routeDetailShowCmd)
}

func routeShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	var req *halproto.RouteGetRequest
	// Get all Routes
	req = &halproto.RouteGetRequest{}
	routeGetReqMsg := &halproto.RouteGetRequestMsg{
		Request: []*halproto.RouteGetRequest{req},
	}

	// HAL call
	respMsg, err := client.RouteGet(context.Background(), routeGetReqMsg)
	if err != nil {
		log.Errorf("Getting Route failed. %v", err)
	}

	// Print Header
	routeShowHeader()

	// Print Routes
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
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
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	var req *halproto.RouteGetRequest
	// Get all Routes
	req = &halproto.RouteGetRequest{}
	routeGetReqMsg := &halproto.RouteGetRequestMsg{
		Request: []*halproto.RouteGetRequest{req},
	}

	// HAL call
	respMsg, err := client.RouteGet(context.Background(), routeGetReqMsg)
	if err != nil {
		log.Errorf("Getting Route failed. %v", err)
	}

	// Print Routes
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
