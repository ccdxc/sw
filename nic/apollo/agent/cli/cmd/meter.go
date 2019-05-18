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
	meterID uint32
)

var meterShowCmd = &cobra.Command{
	Use:   "meter",
	Short: "show Meter information",
	Long:  "show Meter object information",
	Run:   meterShowCmdHandler,
}

func init() {
	showCmd.AddCommand(meterShowCmd)
	meterShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	meterShowCmd.Flags().Uint32VarP(&meterID, "id", "i", 0, "Specify meter policy ID")
}

func meterShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewMeterSvcClient(c)

	if cmd.Flags().Changed("yaml") == false {
		fmt.Printf("Only yaml output is supported. Use --yaml flag\n")
		return
	}

	var req *pds.MeterGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific Meter
		req = &pds.MeterGetRequest{
			Id: []uint32{meterID},
		}
	} else {
		// Get all Meters
		req = &pds.MeterGetRequest{
			Id: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.MeterGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Meter failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Vnics
	if cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	}
}
