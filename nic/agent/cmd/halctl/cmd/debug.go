//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	traceLevel string
	secProfID  uint32
	connTrack  string
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

var fwDebugCmd = &cobra.Command{
	Use:   "firewall",
	Short: "set firewall options",
	Long:  "set firewall options",
}

var secProfDebugCmd = &cobra.Command{
	Use:   "security-profile",
	Short: "set firewall security profile options",
	Long:  "set firewall security profile options",
	Run:   fwSecProfDebugCmdHandler,
}

var platDebugCmd = &cobra.Command{
	Use:   "platform",
	Short: "set platform options",
	Long:  "set platform options",
}

var platHbmDebugCmd = &cobra.Command{
	Use:   "hbm",
	Short: "set platform hbm options",
	Long:  "set platform hbm options",
}
var platLlcDebugCmd = &cobra.Command{
	Use:   "llc-setup",
	Short: "debug platform hbm llc-setup [cache-read|cache-write|scratchpad-access|cache-hit|cache-miss|partial-write|cache-maint-op|eviction|retry-needed|retry-access|disable]",
	Long:  "debug platform hbm llc-setup [cache-read|cache-write|scratchpad-access|cache-hit|cache-miss|partial-write|cache-maint-op|eviction|retry-needed|retry-access|disable]",
	Run:   llcSetupCmdHandler,
}

func init() {
	rootCmd.AddCommand(debugCmd)
	debugCmd.AddCommand(traceDebugCmd)
	debugCmd.AddCommand(fwDebugCmd)
	debugCmd.AddCommand(platDebugCmd)
	platDebugCmd.AddCommand(platHbmDebugCmd)
	platHbmDebugCmd.AddCommand(platLlcDebugCmd)
	traceDebugCmd.AddCommand(flushLogsDebugCmd)
	fwDebugCmd.AddCommand(secProfDebugCmd)
	showCmd.AddCommand(traceShowCmd)

	traceDebugCmd.Flags().StringVar(&traceLevel, "level", "none", "Specify trace level")
	secProfDebugCmd.Flags().Uint32Var(&secProfID, "id", 0, "Specify firewall security profile ID")
	secProfDebugCmd.Flags().StringVar(&connTrack, "conntrack", "off", "Turn connection tracking on/off")
	secProfDebugCmd.MarkFlagRequired("id")
	secProfDebugCmd.MarkFlagRequired("conntrack")
}

func traceShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.TraceGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting Trace failed. %v\n", err)
		return
	}

	// Print Trace Level
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		traceShowResp(resp)
	}
}

func traceDebugCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
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
		fmt.Printf("Set trace level failed. %v\n", err)
		return
	}

	// Print Trace Level
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
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
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.FlushLogs(context.Background(), empty)
	if err != nil {
		fmt.Printf("Flushing logs failed. %v\n", err)
		return
	}

	// Print Response
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		if cmd != nil {
			fmt.Println("Flushing logs succeeded")
		}
	}
}

func llcSetupCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	var llcType halproto.LlcCounterType

	if len(args) > 0 {
		if strings.Compare(args[0], "cache-read") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_CACHE_READ
		} else if strings.Compare(args[0], "cache-write") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_CACHE_WRITE
		} else if strings.Compare(args[0], "scratchpad-access") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_SCRATCHPAD_ACCESS
		} else if strings.Compare(args[0], "cache-hit") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_CACHE_HIT
		} else if strings.Compare(args[0], "cache-miss") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_CACHE_MISS
		} else if strings.Compare(args[0], "partial-write") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_PARTIAL_WRITE
		} else if strings.Compare(args[0], "cache-maint-op") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_CACHE_MAINT_OP
		} else if strings.Compare(args[0], "eviction") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_EVICTION
		} else if strings.Compare(args[0], "retry-needed") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_RETRY_NEEDED
		} else if strings.Compare(args[0], "retry-access") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_RETRY_ACCESS
		} else if strings.Compare(args[0], "disable") == 0 {
			llcType = halproto.LlcCounterType_LLC_COUNTER_CACHE_NONE
		} else {
			fmt.Printf("Invalid argument\n")
			return
		}
	} else {
		fmt.Printf("Command needs an argument. Refer to help string.\n")
		return
	}

	req := &halproto.LlcSetupRequest{
		Type: llcType,
	}
	reqMsg := &halproto.LlcSetupRequestMsg{
		Request: []*halproto.LlcSetupRequest{req},
	}

	// HAL call
	respMsg, err := client.LlcSetup(context.Background(), reqMsg)
	if err != nil {
		fmt.Printf("Llc setup failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
		} else {
			str := strings.ToLower(strings.Replace(llcType.String(), "LLC_COUNTER_", "", -1))
			str = strings.Replace(str, "_", "-", -1)
			if strings.Compare(str, "cache-none") == 0 {
				fmt.Printf("LLC tracking disabled\n")
			} else {
				fmt.Printf("LLC set to track %s\n", str)
			}
		}
		return
	}
}

func fwSecProfDebugCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewNwSecurityClient(c.ClientConn)

	var reqMsg *halproto.SecurityProfileRequestMsg

	if cmd.Flags().Changed("id") {
		if cmd.Flags().Changed("conntrack") {
			if isConnTrackValid(connTrack) != true {
				fmt.Printf("Invalid argument\n")
				return
			}
			var req *halproto.SecurityProfileSpec

			// Set conn tracking
			req = &halproto.SecurityProfileSpec{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle{
					KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
						ProfileId: secProfID,
					},
				},
				CnxnTrackingEn: inputToConnTrack(connTrack),
			}
			reqMsg = &halproto.SecurityProfileRequestMsg{
				Request: []*halproto.SecurityProfileSpec{req},
			}
		} else {
			fmt.Printf("Argument required. Turn connection tracking on/off using '--conntrack on/off' flag\n")
		}
	} else {
		fmt.Printf("Argument required. Set security profile ID using '--id ...' flag\n")
		return
	}

	// HAL call
	_, err = client.SecurityProfileUpdate(context.Background(), reqMsg)
	if err != nil {
		fmt.Printf("Set conn tracking failed. %v\n", err)
		return
	}

	// Get security profile
	var getReqMsg *halproto.SecurityProfileGetRequestMsg
	var getReq *halproto.SecurityProfileGetRequest

	getReq = &halproto.SecurityProfileGetRequest{
		KeyOrHandle: &halproto.SecurityProfileKeyHandle{
			KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
				ProfileId: secProfID,
			},
		},
	}

	getReqMsg = &halproto.SecurityProfileGetRequestMsg{
		Request: []*halproto.SecurityProfileGetRequest{getReq},
	}

	// HAL call
	getRespMsg, err := client.SecurityProfileGet(context.Background(), getReqMsg)
	if err != nil {
		fmt.Printf("Get conn tracking failed. %v\n", err)
		return
	}

	// Print Conn Tracking
	for _, getResp := range getRespMsg.Response {
		if getResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", getResp.ApiStatus)
			continue
		}
		fmt.Printf("Connection Tracking is %s\n", getConnTrack(getResp))
	}
}

func isConnTrackValid(str string) bool {
	switch str {
	case "on":
		return true
	case "off":
		return true
	default:
		return false
	}
}

func inputToConnTrack(str string) bool {
	switch str {
	case "on":
		return true
	case "off":
		return false
	}

	return false
}

func getConnTrack(resp *halproto.SecurityProfileGetResponse) string {
	switch resp.GetSpec().GetCnxnTrackingEn() {
	case true:
		return "on"
	case false:
		return "off"
	}

	return "off"
}
