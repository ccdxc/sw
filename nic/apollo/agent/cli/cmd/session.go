//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"io"
	"reflect"
	"strings"

	"github.com/gogo/protobuf/types"

	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/spf13/cobra"
)

type myFlowMsg struct {
	msg *pds.FlowMsg
}

var (
	sessionStatsID string
	sessionVpcID   uint32
	sessionSrcIP   string
	sessionDstIP   string
	sessionSrcPort uint32
	sessionDstPort uint32
	sessionIPProto uint32
	flowVpcID      uint32
	flowSrcIP      string
	flowDstIP      string
	flowSrcPort    uint32
	flowDstPort    uint32
	flowIPProto    uint32
)

var sessionStatsShowCmd = &cobra.Command{
	Use:   "session-stats",
	Short: "show session statistics",
	Long:  "show session statistics",
	Run:   sessionShowStatsCmdHandler,
}

var flowShowCmd = &cobra.Command{
	Use:   "flow",
	Short: "show flow information",
	Long:  "show flow object information",
	Run:   flowShowCmdHandler,
}

var flowStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show flow statistics",
	Long:  "show flow statistics",
	Run:   flowStatsShowCmdHandler,
}

var sessionShowCmd = &cobra.Command{
	Use:   "session",
	Short: "show session information",
	Long:  "show session object information",
	Run:   sessionShowCmdHandler,
}

var flowClearCmd = &cobra.Command{
	Use:   "flow",
	Short: "clear flow information",
	Long:  "clear flow object information",
	Run:   flowClearCmdHandler,
}

var sessionClearCmd = &cobra.Command{
	Use:   "session",
	Short: "clear session information",
	Long:  "clear session object information",
	Run:   sessionClearCmdHandler,
}

func init() {
	showCmd.AddCommand(sessionStatsShowCmd)
	sessionStatsShowCmd.Flags().StringVarP(&sessionStatsID, "session-stats-index", "i", "", "Specify session stats index. Ex: 1-20 or 10")
	sessionStatsShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	sessionStatsShowCmd.MarkFlagRequired("session-stats-index")

	showCmd.AddCommand(sessionShowCmd)
	sessionShowCmd.Flags().Uint32Var(&sessionVpcID, "vpcid", 0, "Specify VPC ID (default is 0)")
	sessionShowCmd.Flags().StringVar(&sessionSrcIP, "srcip", "0.0.0.0", "Specify session src ip")
	sessionShowCmd.Flags().StringVar(&sessionDstIP, "dstip", "0.0.0.0", "Specify session dst ip")
	sessionShowCmd.Flags().Uint32Var(&sessionSrcPort, "srcport", 0, "Specify session src port")
	sessionShowCmd.Flags().Uint32Var(&sessionDstPort, "dstport", 0, "Specify session dst port")
	sessionShowCmd.Flags().Uint32Var(&sessionIPProto, "ipproto", 0, "Specify session IP proto")
	sessionShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	sessionShowCmd.Flags().Bool("summary", false, "Display number of objects")

	showCmd.AddCommand(flowShowCmd)
	flowShowCmd.Flags().Uint32Var(&flowVpcID, "vpcid", 0, "Specify VPC ID (default is 0)")
	flowShowCmd.Flags().StringVar(&flowSrcIP, "srcip", "0.0.0.0", "Specify flow src ip")
	flowShowCmd.Flags().StringVar(&flowDstIP, "dstip", "0.0.0.0", "Specify flow dst ip")
	flowShowCmd.Flags().Uint32Var(&flowSrcPort, "srcport", 0, "Specify flow src port")
	flowShowCmd.Flags().Uint32Var(&flowDstPort, "dstport", 0, "Specify flow dst port")
	flowShowCmd.Flags().Uint32Var(&flowIPProto, "ipproto", 0, "Specify flow IP proto")
	flowShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	flowShowCmd.Flags().Bool("summary", false, "Display number of objects")

	flowShowCmd.AddCommand(flowStatsShowCmd)
	flowStatsShowCmd.Flags().Bool("yaml", true, "Output in yaml")

	clearCmd.AddCommand(sessionClearCmd)
	sessionClearCmd.Flags().Uint32Var(&sessionVpcID, "vpcid", 0, "Specify VPC ID (default is 0)")
	sessionClearCmd.Flags().StringVar(&sessionSrcIP, "srcip", "0.0.0.0", "Specify session src ip")
	sessionClearCmd.Flags().StringVar(&sessionDstIP, "dstip", "0.0.0.0", "Specify session dst ip")
	sessionClearCmd.Flags().Uint32Var(&sessionSrcPort, "srcport", 0, "Specify session src port")
	sessionClearCmd.Flags().Uint32Var(&sessionDstPort, "dstport", 0, "Specify session dst port")
	sessionClearCmd.Flags().Uint32Var(&sessionIPProto, "ipproto", 0, "Specify session IP proto")

	clearCmd.AddCommand(flowClearCmd)
	flowClearCmd.Flags().Uint32Var(&flowVpcID, "vpcid", 0, "Specify VPC ID (default is 0)")
	flowClearCmd.Flags().StringVar(&flowSrcIP, "srcip", "0.0.0.0", "Specify flow src ip")
	flowClearCmd.Flags().StringVar(&flowDstIP, "dstip", "0.0.0.0", "Specify flow dst ip")
	flowClearCmd.Flags().Uint32Var(&flowSrcPort, "srcport", 0, "Specify flow src port")
	flowClearCmd.Flags().Uint32Var(&flowDstPort, "dstport", 0, "Specify flow dst port")
	flowClearCmd.Flags().Uint32Var(&flowIPProto, "ipproto", 0, "Specify flow IP proto")
}

func sessionClearCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSessionSvcClient(c)

	if cmd.Flags().Changed("vpcid") == false {
		sessionVpcID = 0
	}

	if cmd.Flags().Changed("srcip") == false {
		sessionSrcIP = "0.0.0.0"
	}

	if cmd.Flags().Changed("dstip") == false {
		sessionDstIP = "0.0.0.0"
	}

	if cmd.Flags().Changed("srcport") == false {
		sessionSrcPort = 0
	}

	if cmd.Flags().Changed("dstport") == false {
		sessionDstPort = 0
	}

	if cmd.Flags().Changed("ipproto") == false {
		sessionIPProto = 0
	}

	req := &pds.SessionClearRequest{
		Filter: &pds.SessionFilter{
			Vpc:     sessionVpcID,
			SrcAddr: utils.IPAddrStrToPDSIPAddr(sessionSrcIP),
			DstAddr: utils.IPAddrStrToPDSIPAddr(sessionDstIP),
			SrcPort: sessionSrcPort,
			DstPort: sessionDstPort,
			IPProto: sessionIPProto,
		},
	}

	// PDS call
	respMsg, err := client.SessionClear(context.Background(), req)
	if err != nil {
		fmt.Printf("Clearing sessions failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	fmt.Printf("Clearing sessions succeeded\n")
}

func flowClearCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSessionSvcClient(c)

	if cmd.Flags().Changed("vpcid") != cmd.Flags().Changed("srcip") !=
		cmd.Flags().Changed("dstip") != cmd.Flags().Changed("srcport") !=
		cmd.Flags().Changed("dstport") != cmd.Flags().Changed("ipproto") {
		fmt.Printf("Only specifying all filters or none is supported\n")
		return
	}

	req := &pds.FlowClearRequest{
		Filter: &pds.FlowFilter{
			Vpc:     flowVpcID,
			SrcAddr: utils.IPAddrStrToPDSIPAddr(flowSrcIP),
			DstAddr: utils.IPAddrStrToPDSIPAddr(flowDstIP),
			SrcPort: flowSrcPort,
			DstPort: flowDstPort,
			IPProto: flowIPProto,
		},
	}

	// PDS call
	respMsg, err := client.FlowClear(context.Background(), req)
	if err != nil {
		fmt.Printf("Clearing flows failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	fmt.Printf("Clearing flows succeeded\n")
}

func flowShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd != nil && cmd.Flags().Changed("vpcid") != cmd.Flags().Changed("srcip") !=
		cmd.Flags().Changed("dstip") != cmd.Flags().Changed("srcport") !=
		cmd.Flags().Changed("dstport") != cmd.Flags().Changed("ipproto") {
		fmt.Printf("Only specifying all filters or none is supported\n")
		return
	}

	// If one of the filters is set, then all of them are set, so just checking
	// for one
	filter := cmd != nil && cmd.Flags().Changed("srcip")
	yamlOutput := cmd != nil && cmd.Flags().Changed("yaml")
	summaryOutput := cmd != nil && cmd.Flags().Changed("summary")

	// If a filter is specified, use GRPC, otherwise use UDS to get the flow
	// data
	if filter {
		// Connect to PDS
		c, err := utils.CreateNewGRPCClient()
		if err != nil {
			fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
			return
		}
		defer c.Close()

		client := pds.NewSessionSvcClient(c)

		var empty *pds.Empty

		// PDS call
		stream, err := client.FlowGet(context.Background(), empty)
		if err != nil {
			fmt.Printf("Getting flows failed. %v\n", err)
			return
		}

		if yamlOutput == false {
			flowPrintHeader()
		}

		count := 0

		for {
			respMsg, err := stream.Recv()
			if err == io.EOF {
				break
			}
			if err != nil {
				fmt.Printf("Getting flow stream failure. %v\n", err)
				return
			}

			if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
				return
			}
			for _, flow := range respMsg.GetFlow() {
				// Print flows
				if flowMatchFilter(cmd, flow) {
					if yamlOutput {
						respType := reflect.ValueOf(flow)
						b, _ := yaml.Marshal(respType.Interface())
						fmt.Println(string(b))
						fmt.Println("---")
					} else if !summaryOutput {
						flowPrintEntry(flow)
					}
				}
			}
			count += len(respMsg.Flow)
		}
		flowPrintSummary(count)
	} else {
		flow := myFlowMsg{}
		msg := pds.FlowMsg{}
		flow.msg = &msg
		err := HandleUdsShowObject(pds.Command_CMD_FLOW_DUMP, flow)
		if err != nil {
			fmt.Printf("Error %v\n", err)
		}
	}
}

func flowMatchFilter(cmd *cobra.Command, flow *pds.Flow) bool {
	key := flow.GetKey().GetIPFlowKey()

	if cmd.Flags().Changed("vpcid") == false {
		if flow.GetVpc() != flowVpcID {
			return false
		}
	}

	if cmd.Flags().Changed("srcip") == false {
		if strings.Compare(utils.IPAddrToStr(key.GetSrcIP()), flowSrcIP) != 0 {
			return false
		}
	}

	if cmd.Flags().Changed("dstip") == false {
		if strings.Compare(utils.IPAddrToStr(key.GetDstIP()), flowDstIP) != 0 {
			return false
		}
	}

	if cmd.Flags().Changed("srcport") == false {
		if key.GetL4Info().GetTcpUdpInfo().GetSrcPort() != flowSrcPort {
			return false
		}
	}

	if cmd.Flags().Changed("dstport") == false {
		if key.GetL4Info().GetTcpUdpInfo().GetDstPort() != flowDstPort {
			return false
		}
	}

	if cmd.Flags().Changed("ipproto") == false {
		if key.GetIPProtocol() != flowIPProto {
			return false
		}
	}

	return true
}

func flowPrintSummary(count int) {
	fmt.Printf("\nNo. of flows : %d\n\n", count)
}

func flowPrintHeader() {
	hdrLine := strings.Repeat("-", 131)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-40s%-40s%-8s%-8s%-8s%-5s%-11s%-5s\n",
		"VPCId", "SrcAddr", "DstAddr", "SrcPort",
		"DstPort", "IPProto", "Role", "SessionIdx", "Epoch")
	fmt.Println(hdrLine)
}

func flowPrintEntry(flow *pds.Flow) {
	key := flow.GetKey().GetIPFlowKey()
	fmt.Printf("%-6d%-40s%-40s",
		flow.GetVpc(),
		utils.IPAddrToStr(key.GetSrcIP()),
		utils.IPAddrToStr(key.GetDstIP()))
	ipproto := key.GetIPProtocol()
	if (ipproto == 6) || (ipproto == 17) {
		fmt.Printf("%-8d%-8d",
			key.GetL4Info().GetTcpUdpInfo().GetSrcPort(),
			key.GetL4Info().GetTcpUdpInfo().GetDstPort())
		if ipproto == 6 {
			fmt.Printf("%-8s", "TCP")
		} else {
			fmt.Printf("%-8s", "UDP")
		}
	} else {
		fmt.Printf("%-8s%-8s", "-", "-")
		if ipproto == 1 {
			fmt.Printf("%-8s", "ICMP")
		} else {
			fmt.Printf("%-8d", ipproto)
		}
	}
	role := flow.GetFlowRole()
	if role == 0 {
		fmt.Printf("I    ")
	} else {
		fmt.Printf("R    ")
	}
	fmt.Printf("%-11d%-5d\n", flow.GetSessionIdx(), flow.GetEpoch())
}

func sessionShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSessionSvcClient(c)

	var empty *pds.Empty

	yamlOutput := (cmd != nil) && cmd.Flags().Changed("yaml")
	summaryOutput := (cmd != nil) && cmd.Flags().Changed("summary")

	// PDS call
	stream, err := client.SessionGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting sessions failed. %v\n", err)
		return
	}

	if yamlOutput == false {
		sessionPrintHeader()
	}

	count := 0

	for {
		respMsg, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Printf("Getting session stream failure. %v\n", err)
		}

		if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
			continue
		}
		// Print sessions
		for _, session := range respMsg.GetSession() {
			if yamlOutput {
				respType := reflect.ValueOf(session)
				b, _ := yaml.Marshal(respType.Interface())
				fmt.Println(string(b))
				fmt.Println("---")
			} else if !summaryOutput {
				sessionPrintEntry(session)
			}
		}
		count += len(respMsg.Session)
	}

	sessionPrintSummary(count)
}

func sessionPrintSummary(count int) {
	fmt.Printf("\nNo. of sessions : %d\n\n", count)
}

func sessionPrintHeader() {
}

func sessionPrintEntry(session *pds.Session) {
}

func sessionShowStatsCmdHandler(cmd *cobra.Command, args []string) {
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

	yamlOutput := (cmd != nil) && cmd.Flags().Changed("yaml")
	client := pds.NewDebugSvcClient(c)

	var statsIDLow uint32
	var statsIDHigh uint32

	n, _ := fmt.Sscanf(sessionStatsID, "%d-%d", &statsIDLow, &statsIDHigh)
	if n != 2 {
		n, _ = fmt.Sscanf(sessionStatsID, "%d", &statsIDLow)
		if n != 1 {
			fmt.Printf("Invalid session statistics index provided. Refer to help string\n")
			return
		}
		statsIDHigh = statsIDLow
	}

	if statsIDLow > statsIDHigh {
		fmt.Printf("Invalid session statistics index provided. Refer to help string")
		return
	}

	req := &pds.SessionStatsGetRequest{
		StatsIndexLow:  statsIDLow,
		StatsIndexHigh: statsIDHigh,
	}

	// PDS call
	respMsg, err := client.SessionStatsGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting session statistics failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	if yamlOutput {
		respType := reflect.ValueOf(respMsg)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else {
		sessionStatsPrintHeader()
		sessionStatsPrintEntry(respMsg)
	}
}

func sessionStatsPrintHeader() {
	hdrLine := strings.Repeat("-", 86)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-20s%-20s%-20s%-20s\n",
		"ID", "InitiatorPkts", "InitiatorBytes", "ResponderPkts", "ResponderBytes")
	fmt.Println(hdrLine)
}

func sessionStatsPrintEntry(resp *pds.SessionStatsGetResponse) {
	for _, stats := range resp.GetStats() {
		fmt.Printf("%-6d%-20d%-20d%-20d%-20d\n",
			stats.GetStatsIndex(),
			stats.GetInitiatorFlowPkts(),
			stats.GetInitiatorFlowBytes(),
			stats.GetResponderFlowPkts(),
			stats.GetResponderFlowBytes())
	}
}

func flowStatsShowCmdHandler(cmd *cobra.Command, args []string) {
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

	yamlOutput := (cmd != nil) && cmd.Flags().Changed("yaml")
	client := pds.NewDebugSvcClient(c)

	var empty *pds.Empty

	// PDS call
	respMsg, err := client.FlowStatsSummaryGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting flow statistics failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	if yamlOutput {
		respType := reflect.ValueOf(respMsg)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else {
		flowStatsPrint(respMsg)
	}
}

func flowStatsPrint(resp *pds.FlowStatsSummaryResponse) {
	fmt.Printf("%-30s: %d\n", "TCP over IPv4 Sessions", resp.GetNumTCPv4Sessions())
	fmt.Printf("%-30s: %d\n", "UDP over IPv4 Sessions", resp.GetNumUDPv4Sessions())
	fmt.Printf("%-30s: %d\n", "ICMP over IPv4 Sessions", resp.GetNumICMPv4Sessions())
	fmt.Printf("%-30s: %d\n", "Other IPv4 Sessions", resp.GetNumOtherIPv4Sessions())
	fmt.Printf("%-30s: %d\n", "TCP over IPv6 Sessions", resp.GetNumTCPv6Sessions())
	fmt.Printf("%-30s: %d\n", "UDP over IPv6 Sessions", resp.GetNumUDPv6Sessions())
	fmt.Printf("%-30s: %d\n", "ICMP over IPv6 Sessions", resp.GetNumICMPv6Sessions())
	fmt.Printf("%-30s: %d\n", "Other IPv6 Sessions", resp.GetNumOtherIPv6Sessions())
	fmt.Printf("%-30s: %d\n", "L2 Sessions", resp.GetNumL2Sessions())
	fmt.Printf("%-30s: %d\n", "Session Errors", resp.GetNumSessionErrors())
}

// PrintObject interface
func (flowMsg myFlowMsg) PrintHeader() {
	flowPrintHeader()
}

// PrintObject interface
func (flowMsg myFlowMsg) PrintSummary(count int) {
	flowPrintSummary(count)
}

// PrintObject interface
func (flowMsg myFlowMsg) HandleObject(data *types.Any) (done bool) {
	err := types.UnmarshalAny(data, flowMsg.msg)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		done = true
		return
	}
	if flowMsg.msg.FlowEntryCount == 0 {
		// Last message
		done = true
		return
	}
	flowPrintEntry(flowMsg.msg.FlowEntry)

	done = false
	return
}
