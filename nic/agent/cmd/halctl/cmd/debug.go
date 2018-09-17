//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	traceLevel string
)

var debugCmd = &cobra.Command{
	Use:   "debug",
	Short: "set debug options",
	Long:  "set debug options",
}

var traceDebugCmd = &cobra.Command{
	Use:   "trace",
	Short: "set debug trace level",
	Long:  "set debug trace level",
	Run:   traceDebugCmdHandler,
}

var traceShowCmd = &cobra.Command{
	Use:   "trace",
	Short: "show trace level",
	Long:  "show trace level",
	Run:   traceShowCmdHandler,
}

var flushLogsDebugCmd = &cobra.Command{
	Use:   "flush",
	Short: "flush logs",
	Long:  "flush logs",
	Run:   flushLogsDebugCmdHandler,
}

func init() {
	rootCmd.AddCommand(debugCmd)
	debugCmd.AddCommand(traceDebugCmd)
	traceDebugCmd.AddCommand(flushLogsDebugCmd)
	showCmd.AddCommand(traceShowCmd)

	traceDebugCmd.Flags().StringVar(&traceLevel, "level", "none", "Specify trace level")
}

func traceShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.TraceGet(context.Background(), empty)
	if err != nil {
		log.Errorf("Getting Trace failed. %v", err)
	}

	// Print Trace Level
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		traceShowResp(resp)
	}
}

func traceDebugCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	var traceReqMsg *halproto.TraceRequestMsg

	if cmd.Flags().Changed("level") {
		if isTraceLevelValid(traceLevel) != true {
			fmt.Printf("Invalid argument\n")
			return
		}
		var req *halproto.TraceSpec
		// Set Trace
		req = &halproto.TraceSpec{
			TraceLevel: inputToTraceLevel(traceLevel),
		}
		traceReqMsg = &halproto.TraceRequestMsg{
			Request: []*halproto.TraceSpec{req},
		}
	} else {
		fmt.Printf("Argument required. Set level using '--level ...' flag\n")
		return
	}

	// HAL call
	respMsg, err := client.TraceUpdate(context.Background(), traceReqMsg)
	if err != nil {
		log.Errorf("Set trace level failed. %v", err)
	}

	// Print Trace Level
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		traceShowResp(resp)
	}
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

func inputToTraceLevel(level string) halproto.TraceLevel {
	switch level {
	case "none":
		return halproto.TraceLevel_TRACE_LEVEL_NONE
	case "error":
		return halproto.TraceLevel_TRACE_LEVEL_ERROR
	case "debug":
		return halproto.TraceLevel_TRACE_LEVEL_DEBUG
	default:
		return halproto.TraceLevel_TRACE_LEVEL_NONE
	}
}

func traceLevelToStr(level halproto.TraceLevel) string {
	switch level {
	case halproto.TraceLevel_TRACE_LEVEL_NONE:
		return "None"
	case halproto.TraceLevel_TRACE_LEVEL_ERROR:
		return "Error"
	case halproto.TraceLevel_TRACE_LEVEL_DEBUG:
		return "Debug"
	default:
		return "Invalid"
	}
}

func traceShowResp(resp *halproto.TraceResponse) {
	fmt.Printf("Trace level set to %-12s\n", resp.GetTraceLevel())
}

func flushLogsDebugCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.FlushLogs(context.Background(), empty)
	if err != nil {
		log.Errorf("Flushing logs failed. %v", err)
	}

	// Print Response
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		if cmd != nil {
			fmt.Println("Flushing logs succeeded")
		}
	}
}
