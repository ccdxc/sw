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
	vnicID uint32
)

var vnicShowCmd = &cobra.Command{
	Use:   "vnic",
	Short: "show Vnic information",
	Long:  "show Vnic object information",
	Run:   vnicShowCmdHandler,
}

func init() {
	showCmd.AddCommand(vnicShowCmd)
	vnicShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	vnicShowCmd.Flags().Uint32VarP(&vnicID, "id", "i", 0, "Specify Vnic ID")
}

func vnicShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewVnicSvcClient(c)

	var req *pds.VnicGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific Vnic
		req = &pds.VnicGetRequest{
			VnicId: []uint32{vnicID},
		}
	} else {
		// Get all Vnics
		req = &pds.VnicGetRequest{
			VnicId: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.VnicGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Vnic failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Vnics
	for _, resp := range respMsg.Response {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}
