package cmd

import (
	"context"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var routeShowCmd = &cobra.Command{
	Use:   "route",
	Short: "route",
	Long:  "shows route",
	Run:   routeShowCmdHandler,
}

func init() {
	showCmd.AddCommand(routeShowCmd)
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
