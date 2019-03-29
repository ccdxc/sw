//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	clockFreq  uint32
	traceLevel string
)

var systemDebugCmd = &cobra.Command{
	Use:   "system",
	Short: "set sytem debug information",
	Long:  "set system debug information",
	Run:   systemDebugCmdHandler,
}

var systemShowCmd = &cobra.Command{
	Use:   "system",
	Short: "show system information",
	Long:  "show system information",
	Run:   systemShowCmdHandler,
}

var traceDebugCmd = &cobra.Command{
	Use:   "trace",
	Short: "set debug trace level",
	Long:  "set debug trace level",
	Run:   traceDebugCmdHandler,
}

func init() {
	debugCmd.AddCommand(systemDebugCmd)
	systemDebugCmd.Flags().Uint32VarP(&clockFreq, "clock-frequency", "c", 0, "Specify clock-frequency (Allowed: 833, 900, 957, 1033, 1100)")
	systemDebugCmd.MarkFlagRequired("clock-frequency")

	showCmd.AddCommand(systemShowCmd)
	systemShowCmd.Flags().Bool("power", false, "Show system power information")
	systemShowCmd.Flags().Bool("temperature", false, "Show system power information")

	debugCmd.AddCommand(traceDebugCmd)
	traceDebugCmd.Flags().StringVar(&traceLevel, "level", "none", "Specify trace level (Allowed: none, error, debug)")
}

func traceDebugCmdHandler(cmd *cobra.Command, args []string) {
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

	var traceReq *pds.TraceRequest

	if cmd.Flags().Changed("level") {
		if isTraceLevelValid(traceLevel) != true {
			fmt.Printf("Invalid argument\n")
			return
		}
		traceReq = &pds.TraceRequest{
			TraceLevel: inputToTraceLevel(traceLevel),
		}
	} else {
		fmt.Printf("Argument required. Set level using '--level ...' flag\n")
		return
	}

	// HAL call
	resp, err := client.TraceUpdate(context.Background(), traceReq)
	if err != nil {
		fmt.Printf("Set trace level failed. %v\n", err)
		return
	}

	// Print Trace Level
	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	fmt.Printf("Trace level set to %-12s\n", resp.GetTraceLevel())
}

func isTraceLevelValid(level string) bool {
	switch level {
	case "none":
		return true
	case "error":
		return true
	case "debug":
		return true
	default:
		return false
	}
}

func inputToTraceLevel(level string) pds.TraceLevel {
	switch level {
	case "none":
		return pds.TraceLevel_TRACE_LEVEL_NONE
	case "error":
		return pds.TraceLevel_TRACE_LEVEL_ERROR
	case "debug":
		return pds.TraceLevel_TRACE_LEVEL_DEBUG
	default:
		return pds.TraceLevel_TRACE_LEVEL_NONE
	}
}

func systemShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	power := false
	temp := false

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewDebugSvcClient(c)

	if cmd.Flags().Changed("power") {
		power = true
	}
	if cmd.Flags().Changed("temperature") {
		temp = true
	}
	if cmd.Flags().Changed("power") == false &&
		cmd.Flags().Changed("temperature") == false {
		temp = true
		power = true
	}

	if power {
		systemPowerShow(client)
	}

	if temp {
		systemTemperatureShow(client)
	}
}

func systemPowerShow(client pds.DebugSvcClient) {
	var empty *pds.Empty

	// PDS call
	resp, err := client.SystemPowerGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("System power get failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	printPowerHeader()
	fmt.Printf("%-10s%-10d\n", "Pin", resp.GetPin())
	fmt.Printf("%-10s%-10d\n", "Pout1", resp.GetPout1())
	fmt.Printf("%-10s%-10d\n", "Pout2", resp.GetPout2())
}

func printPowerHeader() {
	hdrLine := strings.Repeat("-", 20)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s\n", "Type", "Power(W)")
	fmt.Println(hdrLine)
}

func systemTemperatureShow(client pds.DebugSvcClient) {
	var empty *pds.Empty

	// PDS call
	resp, err := client.SystemTemperatureGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("System temperature get failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	printTempHeader()
	fmt.Printf("%-20s%-10d\n", "Die Temperature", resp.GetDieTemp())
	fmt.Printf("%-20s%-10d\n", "Local Temperature", resp.GetLocalTemp())
	fmt.Printf("%-20s%-10d\n", "HBM Temperature", resp.GetHbmTemp())
}

func printTempHeader() {
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-20s%-10s\n", "Type", "Temp(C)")
	fmt.Println(hdrLine)
}

func systemDebugCmdHandler(cmd *cobra.Command, args []string) {
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

	if cmd.Flags().Changed("clock-frequency") {
		systemClockFrequencySet(client)
	}
}

func systemClockFrequencySet(client pds.DebugSvcClient) {
	req := &pds.ClockFrequencyRequest{
		ClockFrequency: clockFreq,
	}

	// PDS call
	resp, err := client.ClockFrequencyUpdate(context.Background(), req)
	if err != nil {
		fmt.Printf("Clock-frequency update failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	fmt.Printf("Clock-frequency set to %d\n", clockFreq)
}
