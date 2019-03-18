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

var deviceShowCmd = &cobra.Command{
	Use:   "device",
	Short: "show Device information",
	Long:  "show Device object information",
	Run:   deviceShowCmdHandler,
}

func init() {
	showCmd.AddCommand(deviceShowCmd)
	deviceShowCmd.Flags().Bool("yaml", false, "Output in yaml")
}

func deviceShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewDeviceSvcClient(c)

	var req *pds.Empty

	// PDS call
	resp, err := client.DeviceGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Device failed. %v\n", err)
		return
	}

	// Print Device
	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	respType := reflect.ValueOf(resp)
	b, _ := yaml.Marshal(respType.Interface())
	fmt.Println(string(b))
	fmt.Println("---")
}
