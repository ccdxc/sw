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
	routeID      string
	routeTableID string
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
	routeShowCmd.Flags().StringVarP(&routeID, "route-id", "i", "", "Specify Route ID")
	routeShowCmd.Flags().StringVarP(&routeTableID, "route-table-id", "t", "", "Specify Route Table ID")
	routeShowCmd.Flags().Bool("summary", false, "Display number of objects")
}

func routeShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewRouteSvcClient(c)

	if cmd != nil && !cmd.Flags().Changed("route-table-id") && cmd.Flags().Changed("route-id") {
		fmt.Printf("Command arguments not provided correctly, route-table-id argument also required\n")
		return
	}

	if cmd != nil && cmd.Flags().Changed("route-id") {
		handleRouteGet(cmd, client)
	} else {
		handleRouteTableGet(cmd, client)
	}
}

func handleRouteTableGet(cmd *cobra.Command, client pds.RouteSvcClient) {

	var req *pds.RouteTableGetRequest

	if cmd != nil && cmd.Flags().Changed("route-table-id") {
		req = &pds.RouteTableGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(routeTableID).Bytes()},
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
		fmt.Printf("Getting Route Table failed, err %v\n", err)
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
		printRouteTableSummary(len(respMsg.Response))
	} else {
		printRouteTableHeader()
		for _, resp := range respMsg.Response {
			printRouteTable(resp)
		}
		printRouteTableSummary(len(respMsg.Response))
	}
}

func printRouteTableSummary(count int) {
	fmt.Printf("\nNo. of route tables : %d\n\n", count)
}

func printRouteTableHeader() {
	hdrLine := strings.Repeat("-", 158)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-10s%-40s%-20s%-8s%-40s\n%-40s%-10s%-40s%-20s%-8s%-40s\n",
		"Route Table ID", "Priority", "Route ID", "Prefix", "NextHop", "NextHop",
		"", "Enabled", "", "", "Type", "")
	fmt.Println(hdrLine)
}

func printRouteTable(rt *pds.RouteTable) {
	spec := rt.GetSpec()
	if spec == nil {
		return
	}

	routes := spec.GetRoutes()
	first := true
	priorityEn := spec.GetPriorityEn()
	priorityStr := "N"
	if priorityEn {
		priorityStr = "Y"
	}

	fmt.Printf("%-40s%-9s", uuid.FromBytesOrNil(spec.GetId()).String(),
		priorityStr)

	for _, route := range routes {
		if first != true {
			fmt.Printf("%-40s%-9s", "", "")
		}
		fmt.Printf("%-40s", uuid.FromBytesOrNil(route.GetId()).String())
		switch route.GetAttrs().GetNh().(type) {
		case *pds.RouteAttrs_NextHop:
			fmt.Printf("%-20s%-8s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"IP",
				utils.IPAddrToStr(route.GetAttrs().GetNextHop()))
			first = false
		case *pds.RouteAttrs_NexthopId:
			fmt.Printf("%-20s%-8s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"ID",
				uuid.FromBytesOrNil(route.GetAttrs().GetNexthopId()).String())
			first = false
		case *pds.RouteAttrs_NexthopGroupId:
			fmt.Printf("%-20s%-8s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"ECMP",
				uuid.FromBytesOrNil(route.GetAttrs().GetNexthopGroupId()).String())
			first = false
		case *pds.RouteAttrs_VPCId:
			fmt.Printf("%-20s%-8s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"VPC",
				uuid.FromBytesOrNil(route.GetAttrs().GetVPCId()).String())
			first = false
		case *pds.RouteAttrs_TunnelId:
			fmt.Printf("%-20s%-8s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"TEP",
				uuid.FromBytesOrNil(route.GetAttrs().GetTunnelId()).String())
			first = false
		case *pds.RouteAttrs_VnicId:
			fmt.Printf("%-20s%-8s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"VNIC",
				uuid.FromBytesOrNil(route.GetAttrs().GetVnicId()).String())
			first = false
		default:
			fmt.Printf("%-20s%-8s%-40s\n",
				utils.IPPrefixToStr(route.GetAttrs().GetPrefix()),
				"-", "-")
			first = false
		}
	}
}

func handleRouteGet(cmd *cobra.Command, client pds.RouteSvcClient) {

	var req *pds.RouteGetRequest
	req = &pds.RouteGetRequest{
		Id: []*pds.RouteId{
			&pds.RouteId{
				Id:           uuid.FromStringOrNil(routeID).Bytes(),
				RouteTableId: uuid.FromStringOrNil(routeTableID).Bytes(),
			},
		},
	}
	// PDS call
	respMsg, err := client.RouteGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Route failed, err %v\n", err)
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
	} else {
		for _, resp := range respMsg.Response {
			printRoute(resp)
		}
	}
}

func printRoute(route *pds.Route) {
	spec := route.GetSpec()
	if spec == nil {
		return
	}

	attr := spec.GetAttrs()
	if attr == nil {
		return
	}

	fmt.Printf("%-30s : %s\n", "Route Id", uuid.FromBytesOrNil(spec.GetId()).String())
	fmt.Printf("%-30s : %s\n", "Route Table Id", uuid.FromBytesOrNil(spec.GetRouteTableId()).String())
	fmt.Printf("%-30s : %s\n", "Prefix", utils.IPPrefixToStr(attr.GetPrefix()))
	fmt.Printf("%-30s : %d\n", "Priority", attr.GetPriority())

	switch spec.GetAttrs().GetNh().(type) {
	case *pds.RouteAttrs_NextHop:

		fmt.Printf("%-30s : %s\n", "Nexthop Type", "IP address")
		fmt.Printf("%-30s : %s\n", "Nexthop",
			utils.IPAddrToStr(attr.GetNextHop()))
	case *pds.RouteAttrs_NexthopId:
		fmt.Printf("%-30s : %s\n", "Nexthop Type", "Nexthop ID")
		fmt.Printf("%-30s : %s\n", "Nexthop",
			uuid.FromBytesOrNil(attr.GetNexthopId()).String())
	case *pds.RouteAttrs_NexthopGroupId:
		fmt.Printf("%-30s : %s\n", "Nexthop Type", "ECMP")
		fmt.Printf("%-30s : %s\n", "Nexthop",
			uuid.FromBytesOrNil(attr.GetNexthopGroupId()).String())
	case *pds.RouteAttrs_VPCId:
		fmt.Printf("%-30s : %s\n", "Nexthop Type", "VPC")
		fmt.Printf("%-30s : %s\n", "Nexthop",
			uuid.FromBytesOrNil(attr.GetVPCId()).String())
	case *pds.RouteAttrs_TunnelId:
		fmt.Printf("%-30s : %s\n", "Nexthop Type", "TEP")
		fmt.Printf("%-30s : %s\n", "Nexthop",
			uuid.FromBytesOrNil(attr.GetTunnelId()).String())
	case *pds.RouteAttrs_VnicId:
		fmt.Printf("%-30s : %s\n", "Nexthop Type", "VNIC")
		fmt.Printf("%-30s : %s\n", "Nexthop",
			uuid.FromBytesOrNil(attr.GetVnicId()).String())
	default:
		fmt.Printf("%-30s : %s\n", "Nexthop Type", "-")
		fmt.Printf("%-30s : %s\n", "Nexthop", "-")
	}
	fmt.Printf("%-30s : %s\n", "Source NAT Action",
		strings.Replace(attr.GetNatAction().GetSrcNatAction().String(), "NAT_ACTION_", "", -1))
	fmt.Printf("%-30s : %s\n", "Destination NAT IP",
		utils.IPAddrToStr(attr.GetNatAction().GetDstNatIP()))
	fmt.Printf("%-30s : %t\n", "Meter Enable", attr.GetMeterEn())
}
