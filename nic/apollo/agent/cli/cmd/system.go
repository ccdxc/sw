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
	llcTypeStr string
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

var llcDebugCmd = &cobra.Command{
	Use:   "llc-cache",
	Short: "debug system llc-cache",
	Long:  "debug system llc-cache",
	Run:   llcSetupCmdHandler,
}

var llcShowCmd = &cobra.Command{
	Use:   "llc-stats",
	Short: "show system llc-stats",
	Long:  "show system llc-stats",
	Run:   llcShowCmdHandler,
}

var tableShowCmd = &cobra.Command{
	Use:   "table-stats",
	Short: "show system table-stats",
	Long:  "show system table-stats",
	Run:   tableShowCmdHandler,
}

func init() {
	debugCmd.AddCommand(systemDebugCmd)
	systemDebugCmd.Flags().Uint32VarP(&clockFreq, "clock-frequency", "c", 0, "Specify clock-frequency (Allowed: 833, 900, 957, 1033, 1100)")
	systemDebugCmd.MarkFlagRequired("clock-frequency")

	showCmd.AddCommand(systemShowCmd)
	systemShowCmd.Flags().Bool("power", false, "Show system power information")
	systemShowCmd.Flags().Bool("temperature", false, "Show system power information")

	debugCmd.AddCommand(traceDebugCmd)
	traceDebugCmd.Flags().StringVar(&traceLevel, "level", "none", "Specify trace level (Allowed: none, error, warn, info, debug, verbose)")

	debugCmd.AddCommand(llcDebugCmd)
	llcDebugCmd.Flags().StringVar(&llcTypeStr, "type", "none", "Specify LLC Cache type (Allowed: cache-read,cache-write,scratchpad-access,cache-hit,cache-miss,partial-write,cache-maint-op,eviction,retry-needed,retry-access,disable)")
	llcDebugCmd.MarkFlagRequired("type")

	systemShowCmd.AddCommand(llcShowCmd)
	systemShowCmd.AddCommand(tableShowCmd)
}

func tableShowCmdHandler(cmd *cobra.Command, args []string) {
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
		fmt.Printf("Table stats get failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	tableStatsPrintResp(resp.GetResponse())
}

func tableStatsPrintHeader() {
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-20s%-10s\n", "Type", "Count")
	fmt.Println(hdrLine)
}

func tableStatsPrintResp(stats []*pds.TableStatsResponse) {
	for _, resp := range stats {
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

func llcShowCmdHandler(cmd *cobra.Command, args []string) {
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
	resp, err := client.LlcStatsGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("LLC get failed. %v\n", err)
		return
	}

	llcGetPrintHeader()

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	llcGetPrintResp(resp)
}

func llcGetPrintHeader() {
	hdrLine := strings.Repeat("-", 40)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-20s%-10s\n", "Channel", "Type", "Count")
	fmt.Println(hdrLine)
}

func llcGetPrintResp(resp *pds.LlcStatsGetResponse) {
	stats := resp.GetStats()
	count := stats.GetCount()
	str := strings.ToLower(strings.Replace(stats.GetType().String(), "LLC_COUNTER_", "", -1))
	str = strings.Replace(str, "_", "-", -1)
	for i := 0; i < 16; i++ {
		fmt.Printf("%-10d%-20s%-10d\n",
			i, str, count[i])
	}
}

func llcSetupCmdHandler(cmd *cobra.Command, args []string) {
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

	var llcType pds.LlcCounterType

	if cmd.Flags().Changed("type") {
		if strings.Compare(llcTypeStr, "cache-read") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_CACHE_READ
		} else if strings.Compare(llcTypeStr, "cache-write") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_CACHE_WRITE
		} else if strings.Compare(llcTypeStr, "scratchpad-access") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_SCRATCHPAD_ACCESS
		} else if strings.Compare(llcTypeStr, "cache-hit") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_CACHE_HIT
		} else if strings.Compare(llcTypeStr, "cache-miss") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_CACHE_MISS
		} else if strings.Compare(llcTypeStr, "partial-write") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_PARTIAL_WRITE
		} else if strings.Compare(llcTypeStr, "cache-maint-op") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_CACHE_MAINT_OP
		} else if strings.Compare(llcTypeStr, "eviction") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_EVICTION
		} else if strings.Compare(llcTypeStr, "retry-needed") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_RETRY_NEEDED
		} else if strings.Compare(llcTypeStr, "retry-access") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_RETRY_ACCESS
		} else if strings.Compare(llcTypeStr, "none") == 0 {
			llcType = pds.LlcCounterType_LLC_COUNTER_CACHE_NONE
		} else {
			fmt.Printf("Invalid argument\n")
			return
		}
	} else {
		fmt.Printf("Command needs an argument. Refer to help string.\n")
		return
	}

	req := &pds.LlcSetupRequest{
		Type: llcType,
	}

	// PDS call
	resp, err := client.LlcSetup(context.Background(), req)
	if err != nil {
		fmt.Printf("LLC setup failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	str := strings.ToLower(strings.Replace(llcType.String(), "LLC_COUNTER_", "", -1))
	str = strings.Replace(str, "_", "-", -1)
	if strings.Compare(str, "cache-none") == 0 {
		fmt.Printf("LLC tracking disabled\n")
	} else {
		fmt.Printf("LLC set to track %s\n", str)
	}
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
	case "warn":
		return true
	case "info":
		return true
	case "verbose":
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
	case "warn":
		return pds.TraceLevel_TRACE_LEVEL_WARN
	case "info":
		return pds.TraceLevel_TRACE_LEVEL_INFO
	case "verbose":
		return pds.TraceLevel_TRACE_LEVEL_VERBOSE
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
