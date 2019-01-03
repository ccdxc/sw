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
	tcpProxyQid            uint32
	tcpProxySessionSrcIP   string
	tcpProxySessionDstIP   string
	tcpProxySessionSrcPort uint32
	tcpProxySessionDstPort uint32
)

var tcpProxyShowCmd = &cobra.Command{
	Use:   "tcp-proxy",
	Short: "show tcp-proxy information",
	Long:  "show tcp-proxy information",
	Run:   tcpProxyShowCmdHandler,
}

var tcpProxySessionShowCmd = &cobra.Command{
	Use:   "session",
	Short: "show tcp-proxy sessions",
	Long:  "show tcp-proxy sessions",
	Run:   tcpProxySessionShowCmdHandler,
}

var tcpProxyStatisticsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show tcp-proxy statistics",
	Long:  "show tcp-proxy statistics",
	Run:   tcpProxyStatisticsShowCmdHandler,
}

var tcpProxyCbShowCmd = &cobra.Command{
	Use:   "cbs",
	Short: "show tcp-proxy control blocks",
	Long:  "show tcp-proxy control blocks",
	Run:   tcpProxyCbShowCmdHandler,
}

func init() {
	showCmd.AddCommand(tcpProxyShowCmd)
	tcpProxyShowCmd.AddCommand(tcpProxySessionShowCmd)
	tcpProxyShowCmd.AddCommand(tcpProxyStatisticsShowCmd)
	tcpProxyShowCmd.AddCommand(tcpProxyCbShowCmd)
	tcpProxyShowCmd.Flags().Uint32Var(&tcpProxyQid, "qid", 1, "Specify qid")
	tcpProxyCbShowCmd.Flags().Uint32Var(&tcpProxyQid, "qid", 1, "Specify qid")
	tcpProxyStatisticsShowCmd.Flags().Uint32Var(&tcpProxyQid, "qid", 1, "Specify qid")
	tcpProxySessionShowCmd.Flags().StringVar(&tcpProxySessionSrcIP, "srcip",
		"0.0.0.0", "Specify session src ip")
	tcpProxySessionShowCmd.Flags().StringVar(&tcpProxySessionDstIP, "dstip",
		"0.0.0.0", "Specify session dst ip")
	tcpProxySessionShowCmd.Flags().Uint32Var(&tcpProxySessionSrcPort, "srcport",
		0, "Specify session src port")
	tcpProxySessionShowCmd.Flags().Uint32Var(&tcpProxySessionDstPort, "dstport",
		0, "Specify session dst port")
}

func tcpProxyShowCmdHandler(cmd *cobra.Command, args []string) {
	showCbs := true
	showStats := true
	doTCPProxyCbShowCmd(cmd, args, showCbs, showStats)
}

func tcpProxyCbShowCmdHandler(cmd *cobra.Command, args []string) {
	showCbs := true
	showStats := false
	doTCPProxyCbShowCmd(cmd, args, showCbs, showStats)
}

func tcpProxyStatisticsShowCmdHandler(cmd *cobra.Command, args []string) {
	showCbs := false
	showStats := true
	if cmd.Flags().Changed("qid") {
		doTCPProxyCbShowCmd(cmd, args, showCbs, showStats)
	} else {
		doTCPProxyGlobalStatsShowCmd(cmd, args)
	}
}

func doTCPProxyGlobalStatsShowCmd(cmd *cobra.Command, args []string) {

	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewTcpProxyClient(c.ClientConn)

	defer c.Close()

	req := &halproto.TcpProxyGlobalStatsGetRequest{}

	globalStatsReqMsg := &halproto.TcpProxyGlobalStatsGetRequestMsg{
		Request: []*halproto.TcpProxyGlobalStatsGetRequest{req},
	}

	respMsg, err := client.TcpProxyGlobalStatsGet(context.Background(),
		globalStatsReqMsg)
	if err != nil {
		fmt.Printf("TcpProxyGlobalStatsGet failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		fmt.Printf("HAL Returned error status. %v\n", respMsg.ApiStatus)
		os.Exit(1)
	}

	fmt.Printf("TCP Global Statistics\n")
	fmt.Printf("%s\n", strings.Repeat("-", 60))
	fmt.Printf("%-30s : %-6d\n", "Rnmdr_full", respMsg.GlobalStats.RnmdrFull)
	fmt.Printf("%-30s : %-6d\n", "InvalidSesqDescr", respMsg.GlobalStats.InvalidSesqDescr)
	fmt.Printf("%-30s : %-6d\n", "InvalidRetxSesqDescr",
		respMsg.GlobalStats.InvalidRetxSesqDescr)
	fmt.Printf("%-30s : %-6d\n", "RetxPartialAck", respMsg.GlobalStats.RetxPartialAck)
	fmt.Printf("%-30s : %-6d\n", "RetxNopSchedule", respMsg.GlobalStats.RetxNopSchedule)
	fmt.Printf("%-30s : %-6d\n", "GcFull", respMsg.GlobalStats.GcFull)
	fmt.Printf("%-30s : %-6d\n", "TlsGcFull", respMsg.GlobalStats.TlsGcFull)
}

func tcpProxySessionShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewTcpProxyClient(c.ClientConn)

	var sessionGetReqMsg *halproto.TcpProxySessionGetRequestMsg
	var req *halproto.TcpProxySessionGetRequest
	if cmd.Flags().Changed("srcip") || cmd.Flags().Changed("dstip") ||
		cmd.Flags().Changed("srcport") || cmd.Flags().Changed("dstport") {
		req = &halproto.TcpProxySessionGetRequest{
			SessionFilter: &halproto.TcpProxySessionFilter{
				SrcIp: &halproto.IPAddress{
					IpAf: halproto.IPAddressFamily_IP_AF_INET,
					V4OrV6: &halproto.IPAddress_V4Addr{
						V4Addr: IPAddrStrtoUint32(tcpProxySessionSrcIP),
					},
				},
				DstIp: &halproto.IPAddress{
					IpAf: halproto.IPAddressFamily_IP_AF_INET,
					V4OrV6: &halproto.IPAddress_V4Addr{
						V4Addr: IPAddrStrtoUint32(tcpProxySessionDstIP),
					},
				},
				SrcPort:  tcpProxySessionSrcPort,
				DstPort:  tcpProxySessionDstPort,
				MatchAll: false,
			},
		}
	} else {
		req = &halproto.TcpProxySessionGetRequest{
			SessionFilter: &halproto.TcpProxySessionFilter{
				MatchAll: true,
			},
		}
	}

	sessionGetReqMsg = &halproto.TcpProxySessionGetRequestMsg{
		Request: []*halproto.TcpProxySessionGetRequest{req},
	}

	// HAL call
	respMsg, err := client.TcpProxySessionGet(context.Background(), sessionGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Session failed. %v\n", err)
		return
	}

	// Print Sessions
	fmt.Printf("Active TCP sessions:\n")

	flowIndx := 1
	if len(respMsg.Response) != 0 {
		fmt.Printf("%s\n", strings.Repeat("-", 96))
		fmt.Printf("%-12s%-16s%-16s%-12s%-12s%-12s%-12s%-12s\n",
			"Flow", "Source IP", "Dest IP", "Source Port", "Dest Port",
			"Queue Id1", "Queue Id2", "Flow Type")
		fmt.Printf("%s\n", strings.Repeat("-", 96))
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}

		fmt.Printf("%-12d", flowIndx)
		fmt.Printf("%-16s", utils.IPAddrToStr(resp.TcpproxyFlow.SrcIp))
		fmt.Printf("%-16s", utils.IPAddrToStr(resp.TcpproxyFlow.DstIp))
		fmt.Printf("%-12d", resp.TcpproxyFlow.Sport)
		fmt.Printf("%-12d", resp.TcpproxyFlow.Dport)
		fmt.Printf("%-12d", resp.TcpproxyFlow.Qid1)
		fmt.Printf("%-12d", resp.TcpproxyFlow.Qid2)
		fmt.Printf("%-12d\n", resp.TcpproxyFlow.FlowType)
		flowIndx++
	}

	if flowIndx > 1 {
		fmt.Printf("%s\n", strings.Repeat("-", 96))
		fmt.Printf("Found %d flows.\n\n", flowIndx-1)
	}
}

func doTCPProxyCbShowCmd(cmd *cobra.Command, args []string, showCbs bool,
	showStats bool) {

	if !cmd.Flags().Changed("qid") {
		fmt.Printf("Please specify a queue id(qid)...\n")
		return
	}

	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewInternalClient(c.ClientConn)

	defer c.Close()

	req := &halproto.TcpCbGetRequest{
		KeyOrHandle: &halproto.TcpCbKeyHandle{
			KeyOrHandle: &halproto.TcpCbKeyHandle_TcpcbId{
				TcpcbId: tcpProxyQid,
			},
		},
	}

	tcpCbGetReqMsg := &halproto.TcpCbGetRequestMsg{
		Request: []*halproto.TcpCbGetRequest{req},
	}

	respMsg, err := client.TcpCbGet(context.Background(), tcpCbGetReqMsg)
	if err != nil {
		fmt.Printf("TcpCbGet failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned error status. %v\n", resp.ApiStatus)
			continue
		}

		if showCbs {
			fmt.Printf("%s\n", strings.Repeat("-", 80))
			fmt.Printf("\nTCP Control block(TCB)\n")
			fmt.Printf("%s\n", strings.Repeat("-", 80))
			showTCPCb(resp)
			fmt.Printf("%s\n\n", strings.Repeat("-", 80))
		}

		if showStats {
			fmt.Printf("%s\n", strings.Repeat("-", 80))
			fmt.Printf("\nTCP Statistics\n")
			fmt.Printf("%s\n", strings.Repeat("-", 80))
			showTCPStats(resp)
			fmt.Printf("%s\n\n", strings.Repeat("-", 80))
		}
	}
}

func showTCPCb(resp *halproto.TcpCbGetResponse) {

	spec := resp.GetSpec()

	fmt.Printf("%-30s : %-6d\n", "rcv_nxt", spec.RcvNxt)
	fmt.Printf("%-30s : %-6d\n", "snd_nxt", spec.SndNxt)
	fmt.Printf("%-30s : %-6d\n", "snd_una", spec.SndUna)
	fmt.Printf("%-30s : %-6d\n", "rcv_tsval", spec.RcvTsval)
	fmt.Printf("%-30s : %-6d\n", "ts_recent", spec.TsRecent)
	fmt.Printf("%-30s : %-6X\n", "serq_base", spec.SerqBase)
	fmt.Printf("%-30s : %-6X\n", "debug_dol", spec.DebugDol)
	fmt.Printf("%-30s : %-6X\n", "sesq_base", spec.SesqBase)
	fmt.Printf("%-30s : %-6d\n", "snd_wnd", spec.SndWnd)
	fmt.Printf("%-30s : %-6d\n", "snd_cwnd", spec.SndCwnd)
	fmt.Printf("%-30s : %-6d\n", "rcv_mss", spec.RcvMss)
	fmt.Printf("%-30s : %-6d\n", "source_port", spec.SourcePort)
	fmt.Printf("%-30s : %-6d\n", "dest_port", spec.DestPort)
	fmt.Printf("%-30s : %-6d\n", "state", spec.State)
	fmt.Printf("%-30s : %-6d\n", "source_lif", spec.SourceLif)
	fmt.Printf("%-30s : %-6X\n", "debug_dol_tx", spec.DebugDolTx)
	fmt.Printf("%-30s : %-6d\n", "header_len", spec.HeaderLen)
	fmt.Printf("%-30s : %-6t\n", "pending_ack_send", spec.PendingAckSend)
	fmt.Printf("%-30s : %-6d\n", "retx_snd_una", spec.RetxSndUna)
	fmt.Printf("%-30s : %-6d\n", "other_qid", spec.OtherQid)
	fmt.Printf("%-30s : %-6d\n", "rto_backoff", spec.RtoBackoff)
}

func showTCPStats(resp *halproto.TcpCbGetResponse) {

	stats := resp.GetStats()

	fmt.Printf("%-30s : %-6d\n", "pkts_rcvd", stats.PktsRcvd)
	fmt.Printf("%-30s : %-6d\n", "bytes_rcvd", stats.BytesRcvd)
	fmt.Printf("%-30s : %-6d\n", "bytes_acked", stats.BytesAcked)
	fmt.Printf("%-30s : %-6d\n", "pkts_sent", stats.PktsSent)
	fmt.Printf("%-30s : %-6d\n", "bytes_sent", stats.BytesSent)
	fmt.Printf("%-30s : %-6d\n", "slow_path_cnt", stats.SlowPathCnt)
	fmt.Printf("%-30s : %-6d\n", "serq_full_cnt", stats.SerqFullCnt)
	fmt.Printf("%-30s : %-6d\n", "ooo_cnt", stats.OooCnt)
	fmt.Printf("%-30s : %-6d\n", "sesq_pi", stats.SesqPi)
	fmt.Printf("%-30s : %-6d\n", "sesq_ci", stats.SesqCi)
	fmt.Printf("%-30s : %-6d\n", "sesq_retx_ci", stats.SesqRetxCi)
	fmt.Printf("%-30s : %-6d\n", "sesq_tx_ci", stats.SesqTxCi)
	fmt.Printf("%-30s : %-6d\n", "send_ack_pi", stats.SendAckPi)
	fmt.Printf("%-30s : %-6d\n", "send_ack_ci", stats.SendAckCi)
	fmt.Printf("%-30s : %-6d\n", "fast_timer_pi", stats.FastTimerPi)
	fmt.Printf("%-30s : %-6d\n", "fast_timer_ci", stats.FastTimerCi)
	fmt.Printf("%-30s : %-6d\n", "del_ack_pi", stats.DelAckPi)
	fmt.Printf("%-30s : %-6d\n", "del_ack_ci", stats.DelAckCi)
	fmt.Printf("%-30s : %-6d\n", "asesq_pi", stats.AsesqPi)
	fmt.Printf("%-30s : %-6d\n", "asesq_ci", stats.AsesqCi)
	fmt.Printf("%-30s : %-6d\n", "asesq_retx_ci", stats.AsesqRetxCi)
	fmt.Printf("%-30s : %-6d\n", "pending_tx_pi", stats.PendingTxPi)
	fmt.Printf("%-30s : %-6d\n", "pending_tx_ci", stats.PendingTxCi)
	fmt.Printf("%-30s : %-6d\n", "fast_retrans_pi", stats.FastRetransPi)
	fmt.Printf("%-30s : %-6d\n", "fast_retrans_ci", stats.FastRetransCi)
	fmt.Printf("%-30s : %-6d\n", "clean_retx_pi", stats.CleanRetxPi)
	fmt.Printf("%-30s : %-6d\n", "clean_retx_ci", stats.CleanRetxCi)
	fmt.Printf("%-30s : %-6d\n", "packets_out", stats.PacketsOut)
	fmt.Printf("%-30s : %-6d\n", "tx_ring_pi", stats.TxRingPi)
	fmt.Printf("%-30s : %-6d\n", "partial_ack_cnt", stats.PartialAckCnt)
}
