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
	tunnelID uint32
)

var tunnelShowCmd = &cobra.Command{
	Use:   "tunnel",
	Short: "show Tunnel information",
	Long:  "show Tunnel object information",
	Run:   tunnelShowCmdHandler,
}

func init() {
	showCmd.AddCommand(tunnelShowCmd)
	tunnelShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	tunnelShowCmd.Flags().Uint32VarP(&tunnelID, "id", "i", 0, "Specify Tunnel ID")
}

func tunnelShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewTunnelSvcClient(c)

	var req *pds.TunnelGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific Tunnel
		req = &pds.TunnelGetRequest{
			Id: []uint32{tunnelID},
		}
	} else {
		// Get all Tunnels
		req = &pds.TunnelGetRequest{
			Id: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.TunnelGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Tunnel failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Tunnels
	for _, resp := range respMsg.Response {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}
