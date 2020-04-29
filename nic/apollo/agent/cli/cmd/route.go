//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	routeID string
)

var routeShowCmd = &cobra.Command{
	Use:   "route",
	Short: "show Route information",
	Long:  "show Route object information",
	Run:   routeShowCmdHandler,
}

func init() {
	showCmd.AddCommand(routeShowCmd)
	routeShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	routeShowCmd.Flags().Bool("summary", false, "Display number of objects")
	routeShowCmd.Flags().StringVarP(&routeID, "route-id", "i", "", "Specify Route ID")
}

func routeShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewRouteSvcClient(c)

	var req *pds.RouteTableGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific Route
		req = &pds.RouteTableGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(routeID).Bytes()},
		}
	} else {
		// Get all Routes
		req = &pds.RouteTableGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.RouteTableGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Route failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Routes
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printRouteSummary(len(respMsg.Response))
	} else {
		printRouteHeader()
		for _, resp := range respMsg.Response {
			printRoute(resp)
		}
		printRouteSummary(len(respMsg.Response))
	}
}

func printRouteSummary(count int) {
	fmt.Printf("\nNo. of routes : %d\n\n", count)
}

func printRouteHeader() {
	hdrLine := strings.Repeat("-", 112)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-20s%-12s%-40s\n",
		"ID", "Prefix", "NextHopType", "NextHop")
	fmt.Println(hdrLine)
}

func printRoute(rt *pds.RouteTable) {
	spec := rt.GetSpec()
	routes := spec.GetRoutes()
	first := true

	fmt.Printf("%-40s", uuid.FromBytesOrNil(spec.GetId()).String())

	for _, route := range routes {
		if first != true {
			fmt.Printf("%-40s", "")
		}
		switch route.GetAttrs().GetNh().(type) {
		case *pds.RouteAttrs_NextHop:
			fmt.Printf("%-20s%-12s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"IP",
				utils.IPAddrToStr(route.GetAttrs().GetNextHop()))
			first = false
		case *pds.RouteAttrs_NexthopId:
			fmt.Printf("%-20s%-12s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"ID",
				uuid.FromBytesOrNil(route.GetAttrs().GetNexthopId()).String())
			first = false
		case *pds.RouteAttrs_NexthopGroupId:
			fmt.Printf("%-20s%-12s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"ECMP",
				uuid.FromBytesOrNil(route.GetAttrs().GetNexthopGroupId()).String())
			first = false
		case *pds.RouteAttrs_VPCId:
			fmt.Printf("%-20s%-12s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"VPC",
				uuid.FromBytesOrNil(route.GetAttrs().GetVPCId()).String())
			first = false
		case *pds.RouteAttrs_TunnelId:
			fmt.Printf("%-20s%-12s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"TEP",
				uuid.FromBytesOrNil(route.GetAttrs().GetTunnelId()).String())
			first = false
		case *pds.RouteAttrs_VnicId:
			fmt.Printf("%-20s%-12s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"VNIC",
				uuid.FromBytesOrNil(route.GetAttrs().GetVnicId()).String())
			first = false
		default:
			fmt.Printf("%-20s%-12s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"-", "-")
			first = false
		}
	}
}
