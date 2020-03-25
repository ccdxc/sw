//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
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

var tableShowCmd = &cobra.Command{
	Use:   "table",
	Short: "show table information",
	Long:  "show table information",
}

var tableStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show table statistics",
	Long:  "show table statistics",
	Run:   tableStatsShowCmdHandler,
}

var naclShowCmd = &cobra.Command{
	Use:   "nacl",
	Short: "show nacl table",
	Long:  "show nacl table",
	Run:   naclShowCmdHandler,
}

func init() {
	showCmd.AddCommand(tableShowCmd)
	tableShowCmd.AddCommand(tableStatsShowCmd)
	tableStatsShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	tableShowCmd.AddCommand(naclShowCmd)
}

func naclShowCmdHandler(cmd *cobra.Command, args []string) {
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

	var cmdCtxt *pds.CommandCtxt

	// dump nacl table
	cmdCtxt = &pds.CommandCtxt{
		Version: 1,
		Cmd:     pds.Command_CMD_NACL_DUMP,
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return
	}
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func tableStatsShowCmdHandler(cmd *cobra.Command, args []string) {
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

	var empty *pds.Empty

	// PDS call
	resp, err := client.TableStatsGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Table statistics get failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	if cmd != nil && cmd.Flags().Changed("yaml") {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else {
		tableStatsPrintResp(resp.GetResponse())
	}
}

func tableStatsPrintHeader() {
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-20s%-10s\n", "Type", "Count")
	fmt.Println(hdrLine)
}

func tableStatsPrintResp(statistics []*pds.TableStatsResponse) {
	for _, resp := range statistics {
		fmt.Printf("Table Name: %s\n", resp.GetTableName())
		tableStatsPrintHeader()
		for _, entry := range resp.GetApiStats().GetEntry() {
			typeStr := strings.Replace(entry.GetType().String(), "TABLE_API_STATS_", "", -1)
			typeStr = strings.Replace(typeStr, "_", " ", -1)
			fmt.Printf("%-20s%-10d\n", typeStr, entry.GetCount())
		}
		tableStatsPrintHeader()
		for _, entry := range resp.GetTableStats().GetEntry() {
			typeStr := strings.Replace(entry.GetType().String(), "TABLE_STATS_", "", -1)
			typeStr = strings.Replace(typeStr, "_", " ", -1)
			fmt.Printf("%-20s%-10d\n", typeStr, entry.GetCount())
		}
		fmt.Printf("\n")
	}
}
