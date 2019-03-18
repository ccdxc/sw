//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	routeID uint32
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
	routeShowCmd.Flags().Uint32VarP(&routeID, "route-id", "i", 0, "Specify Route ID")
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

	if cmd.Flags().Changed("yaml") == false {
		fmt.Printf("Only yaml output supported right now. Use --yaml option\n")
		return
	}

	client := pds.NewRouteSvcClient(c)

	var req *pds.RouteTableGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific Route
		req = &pds.RouteTableGetRequest{
			Id: []uint32{routeID},
		}
	} else {
		// Get all Routes
		req = &pds.RouteTableGetRequest{
			Id: []uint32{},
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
	for _, resp := range respMsg.Response {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}
