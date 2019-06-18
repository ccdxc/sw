//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	meterID uint32
	statsID uint32
)

var meterShowCmd = &cobra.Command{
	Use:   "meter",
	Short: "show meter information",
	Long:  "show meter object information",
	Run:   meterShowCmdHandler,
}

var meterStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show meter statistics",
	Long:  "show meter statistics",
	Run:   meterShowStatsCmdHandler,
}

func init() {
	showCmd.AddCommand(meterShowCmd)
	meterShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	meterShowCmd.Flags().Uint32VarP(&meterID, "id", "i", 0, "Specify meter policy ID")

	meterShowCmd.AddCommand(meterStatsShowCmd)
	meterStatsShowCmd.Flags().Uint32VarP(&statsID, "meter-stats-index", "i", 0, "Specify meter stats index")
	meterStatsShowCmd.MarkFlagRequired("meter-stats-index")
}

func meterShowStatsCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewDebugSvcClient(c)

	req := &pds.MeterStatsGetRequest{
		StatsIndex: statsID,
	}

	// PDS call
	respMsg, err := client.MeterStatsGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting meter statistics failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	meterStatsPrintHeader()
	meterStatsPrintEntry(respMsg)
}

func meterStatsPrintHeader() {
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-12s%-12s\n",
		"ID", "TxBytes", "RxBytes")
	fmt.Println(hdrLine)
}

func meterStatsPrintEntry(resp *pds.MeterStatsGetResponse) {
	fmt.Printf("%-6d%-12d%-12d\n", statsID,
		resp.GetTxBytes(), resp.GetRxBytes())
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
		fmt.Printf("Getting meter failed. %v\n", err)
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
