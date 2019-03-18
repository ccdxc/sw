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
	subnetID uint32
)

var subnetShowCmd = &cobra.Command{
	Use:   "subnet",
	Short: "show Subnet information",
	Long:  "show Subnet object information",
	Run:   subnetShowCmdHandler,
}

func init() {
	showCmd.AddCommand(subnetShowCmd)
	subnetShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	subnetShowCmd.Flags().Uint32VarP(&subnetID, "id", "i", 0, "Specify Subnet ID")
}

func subnetShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSubnetSvcClient(c)

	var req *pds.SubnetGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific Subnet
		req = &pds.SubnetGetRequest{
			Id: []uint32{subnetID},
		}
	} else {
		// Get all Subnets
		req = &pds.SubnetGetRequest{
			Id: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.SubnetGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Subnet failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Subnets
	for _, resp := range respMsg.Response {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}
