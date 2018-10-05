//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"net"
	"os"
	"reflect"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	srcLport        uint32
	dstLport        uint32
	dropReason      uint32
	flowLookupDir   uint32
	flowLookupType  uint32
	flowLookupVrf   uint32
	flowLookupSrc   string
	flowLookupDst   string
	flowLookupProto uint32
	flowLookupSport uint32
	flowLookupDport uint32
	ethDmac         uint64
	fromCPU         bool
	isEgress        bool
	isDisable       bool
)

var fteSpanDebugCmd = &cobra.Command{
	Use:   "fte-span",
	Short: "set fte span",
	Long:  "set fte span",
	Run:   fteSpanDebugCmdHandler,
}

var fteSpanDebugDisableCmd = &cobra.Command{
	Use:   "disable",
	Short: "set fte span",
	Long:  "set fte span",
	Run:   fteSpanDebugDisableCmdHandler,
}

var fteSpanShowCmd = &cobra.Command{
	Use:   "fte-span",
	Short: "shows fte span",
	Long:  "shows fte span",
	Run:   fteSpanShowCmdHandler,
}

func init() {
	debugCmd.AddCommand(fteSpanDebugCmd)
	showCmd.AddCommand(fteSpanShowCmd)
	fteSpanDebugCmd.AddCommand(fteSpanDebugDisableCmd)

	fteSpanDebugCmd.Flags().Uint32Var(&srcLport, "slport", 0, "Specify src_lport")
	fteSpanDebugCmd.Flags().Uint32Var(&dstLport, "dlport", 0, "Specify dst_lport")
	fteSpanDebugCmd.Flags().Uint32Var(&dropReason, "drop", 0, "Specify P4 drop bitmap. Matches exact drops. Not one of")
	fteSpanDebugCmd.Flags().Uint32Var(&flowLookupDir, "dir", 0, "Specify flow lookup dir. 0: From Host, 1: From Uplink")
	fteSpanDebugCmd.Flags().Uint32Var(&flowLookupType, "type", 0, "Specify flow type. 0: None, 1: Mac, 2: v4, 3: v6, 4: From VM Bounce, 5: To VM Bounce")
	fteSpanDebugCmd.Flags().Uint32Var(&flowLookupVrf, "vrf", 0, "Specify vrf (flow lookup id)")
	fteSpanDebugCmd.Flags().StringVar(&flowLookupSrc, "src", "0.0.0.0", "Specify flow source")
	fteSpanDebugCmd.Flags().StringVar(&flowLookupDst, "dst", "0.0.0.0", "Specify flow destination")
	fteSpanDebugCmd.Flags().Uint32Var(&flowLookupProto, "proto", 0, "Specify flow proto")
	fteSpanDebugCmd.Flags().Uint32Var(&flowLookupSport, "sport", 0, "Specify flow sport")
	fteSpanDebugCmd.Flags().Uint32Var(&flowLookupDport, "dport", 0, "Specify flow dport")
	fteSpanDebugCmd.Flags().Uint64Var(&ethDmac, "dmac", 0, "Specify ethernet dmac")
	fteSpanDebugCmd.Flags().BoolVar(&fromCPU, "from-cpu", false, "Specify from cpu flag")
	fteSpanDebugCmd.Flags().BoolVar(&isEgress, "is-egress", false, "Enable egress span")
}

func fteSpanShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty
	respMsg, err := client.FteSpanGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting if failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b) + "\n")
		fmt.Println("---")
	}
}

func fteSpanDebugDisableCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c.ClientConn)

	req := &halproto.FteSpanRequest{
		Selector: 0,
	}
	fteSpanReqMsg := &halproto.FteSpanRequestMsg{
		Request: []*halproto.FteSpanRequest{req},
	}

	// HAL call
	respMsg, err := client.FteSpanUpdate(context.Background(), fteSpanReqMsg)
	if err != nil {
		fmt.Printf("FTE Span update failed. %v\n", err)
		return
	}

	// Print Tables: For now its only one at a time
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
	}

	fmt.Println("Success: FTE Span UnInstalled.")

}

func fteSpanDebugCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c.ClientConn)

	var sel uint32
	if cmd.Flags().Changed("slport") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_SRC_LPORT))
	}
	if cmd.Flags().Changed("dlport") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_DST_LPORT))
	}
	if cmd.Flags().Changed("drop") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_DROP_REASON))
	}
	if cmd.Flags().Changed("dir") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FLOW_LKUP_DIR))
	}
	if cmd.Flags().Changed("type") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FLOW_LKUP_TYPE))
	}
	if cmd.Flags().Changed("vrf") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FLOW_LKUP_VRF))
	}
	var flowSrcProto *halproto.IPAddress
	if cmd.Flags().Changed("src") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FLOW_LKUP_SRC))
		ip := net.ParseIP(flowLookupSrc)
		if ip == nil {
			fmt.Printf("Wrong IP address format for source: %s", flowLookupSrc)
			os.Exit(1)
		}
		if ip.To4() != nil {
			flowSrcProto = &halproto.IPAddress{
				IpAf: halproto.IPAddressFamily_IP_AF_INET,
				V4OrV6: &halproto.IPAddress_V4Addr{
					V4Addr: utils.IP2Int(ip),
				},
			}
		}
		// TODO: Handle IPv6
	}
	var flowDstProto *halproto.IPAddress
	if cmd.Flags().Changed("dst") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FLOW_LKUP_DST))
		ip := net.ParseIP(flowLookupDst)
		if ip == nil {
			fmt.Printf("Wrong IP address format for destination: %s", flowLookupDst)
			os.Exit(1)
		}
		if ip.To4() != nil {
			flowDstProto = &halproto.IPAddress{
				IpAf: halproto.IPAddressFamily_IP_AF_INET,
				V4OrV6: &halproto.IPAddress_V4Addr{
					V4Addr: utils.IP2Int(ip),
				},
			}
		}
		// TODO: Handle IPv6
	}
	if cmd.Flags().Changed("proto") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FLOW_LKUP_PROTO))
	}
	if cmd.Flags().Changed("sport") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FLOW_LKUP_SPORT))
	}
	if cmd.Flags().Changed("dport") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FLOW_LKUP_DPORT))
	}
	if cmd.Flags().Changed("dmac") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_ETH_DMAC))
	}
	if cmd.Flags().Changed("from-cpu") {
		sel |= (1 << (uint)(halproto.FTESpanMatchSelector_FROM_CPU))
	}

	req := &halproto.FteSpanRequest{
		Selector:      sel,
		SrcLport:      srcLport,
		DstLport:      dstLport,
		DropReason:    dropReason,
		FlowLkupDir:   flowLookupDir,
		FlowLkupType:  flowLookupType,
		FlowLkupVrf:   flowLookupVrf,
		FlowLkupSrc:   flowSrcProto,
		FlowLkupDst:   flowDstProto,
		FlowLkupProto: flowLookupProto,
		FlowLkupSport: flowLookupSport,
		FlowLkupDport: flowLookupDport,
		EthDmac:       ethDmac,
		FromCpu:       fromCPU,
		IsEgress:      isEgress,
	}
	fteSpanReqMsg := &halproto.FteSpanRequestMsg{
		Request: []*halproto.FteSpanRequest{req},
	}

	// HAL call
	respMsg, err := client.FteSpanUpdate(context.Background(), fteSpanReqMsg)
	if err != nil {
		fmt.Printf("FTE Span update failed. %v\n", err)
		return
	}

	// Print Tables: For now its only one at a time
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
	}

	fmt.Println("Success: FTE Span Installed.")

}
