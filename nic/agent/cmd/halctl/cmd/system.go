//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var systemShowCmd = &cobra.Command{
	Use:   "system",
	Short: "show system information",
	Long:  "show system information",
}

var apiStatsShowCmd = &cobra.Command{
	Use:   "api-stats",
	Short: "show system api-stats",
	Long:  "show system api-stats",
	Run:   apiStatsShowCmdHandler,
}

var systemStatsShowCmd = &cobra.Command{
	Use:   "stats",
	Short: "show system stats",
	Long:  "show system stats",
	Run:   systemStatsShowCmdHandler,
}

func init() {
	showCmd.AddCommand(systemShowCmd)
	systemShowCmd.AddCommand(apiStatsShowCmd)
	systemShowCmd.AddCommand(systemStatsShowCmd)
}

func apiStatsShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewSystemClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.ApiStatsGet(context.Background(), empty)
	if err != nil {
		log.Errorf("Getting API Stats failed. %v", err)
		return
	}

	// Print Header
	apiStatsShowHeader()

	// Print API Stats
	for _, entry := range respMsg.ApiEntries {
		apiStatsEntryShow(entry)
	}
}

func apiStatsShowHeader() {
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-55s%-12s%-12s%-12s\n",
		"API", "Num Calls", "Num Success", "Num Fail")
	fmt.Println(hdrLine)
}

func apiStatsEntryShow(entry *halproto.ApiStatsEntry) {
	fmt.Printf("%-55s%-12d%-12d%-12d\n",
		entry.GetApiType().String(),
		entry.GetNumApiCall(),
		entry.GetNumApiSuccess(),
		entry.GetNumApiFail())
}

func systemStatsShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewSystemClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	resp, err := client.SystemGet(context.Background(), empty)
	if err != nil {
		log.Errorf("Getting System Stats failed. %v", err)
		return
	}

	if resp.GetApiStatus() != halproto.ApiStatus_API_STATUS_OK {
		log.Errorf("HAL Returned non OK status. %v", resp.GetApiStatus())
		return
	}

	// Print Header
	fmt.Println("System Drop Stats:")
	systemDropStatsShowHeader()

	// Print System Drop Stats
	for _, entry := range resp.GetStats().GetDropStats().DropEntries {
		systemDropStatsShowEntry(entry)
	}

	// Print Header
	fmt.Println("System Egress Drop Stats:")
	systemDropStatsShowHeader()

	// Print System Egress Drop Stats
	for _, entry := range resp.GetStats().GetEgressDropStats().DropEntries {
		systemEgressDropStatsShowEntry(entry)
	}

	// Print Header
	fmt.Println("System Table Stats:")
	systemTableStatsShowHeader()

	// Print Table Stats
	for _, entry := range resp.GetStats().GetTableStats().TableStats {
		systemTableStatsShowEntry(entry)
	}

	fmt.Println("FTE Stats:")
	fteStatsShow(resp.GetStats())

	fmt.Println("Session Summary Stats:")
	sessionSummaryStatsShow(resp.GetStats())
}

func systemDropStatsShowHeader() {
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-50s%-12s\n",
		"Reason", "Drop Count")
	fmt.Println(hdrLine)
}

func dropReasonToString(reasons *halproto.DropReasons) string {
	if reasons.GetDropMalformedPkt() {
		return "Drop Malformed Pkt"
	}
	if reasons.GetDropInputMapping() {
		return "Drop Input Mapping"
	}
	if reasons.GetDropInputMappingDejavu() {
		return "Drop Input Mapping Deja Vu"
	}
	if reasons.GetDropFlowHit() {
		return "Drop Flow Hit"
	}
	if reasons.GetDropFlowMiss() {
		return "Drop Flow Miss"
	}
	if reasons.GetDropNacl() {
		return "Drop Nacl"
	}
	if reasons.GetDropIpsg() {
		return "Drop IP SG"
	}
	if reasons.GetDropIpNormalization() {
		return "Drop IP Normalization"
	}
	if reasons.GetDropTcpNormalization() {
		return "Drop TCP Normalization"
	}
	if reasons.GetDropTcpRstWithInvalidAckNum() {
		return "Drop TCP RST with Invalid ACK Num"
	}
	if reasons.GetDropTcpNonSynFirstPkt() {
		return "Drop TCP Non-SYN First Pkt"
	}
	if reasons.GetDropIcmpNormalization() {
		return "Drop ICMP Normalization"
	}
	if reasons.GetDropInputPropertiesMiss() {
		return "Drop Input Properties Miss"
	}
	if reasons.GetDropTcpOutOfWindow() {
		return "Drop TCP Out-of-window"
	}
	if reasons.GetDropTcpSplitHandshake() {
		return "Drop TCP Split Handshake"
	}
	if reasons.GetDropTcpWinZeroDrop() {
		return "Drop TCP Window Zero"
	}
	if reasons.GetDropTcpDataAfterFin() {
		return "Drop TCP Data after FIN"
	}
	if reasons.GetDropTcpNonRstPktAfterRst() {
		return "Drop TCP non-RST Pkt after RST"
	}
	if reasons.GetDropTcpInvalidResponderFirstPkt() {
		return "Drop TCP Invalid Responder First Pkt"
	}
	if reasons.GetDropTcpUnexpectedPkt() {
		return "Drop TCP Unexpected Pkt"
	}
	if reasons.GetDropSrcLifMismatch() {
		return "Drop Src Lif Mismatch"
	}
	if reasons.GetDropParserIcrcError() {
		return "Drop Parser ICRC Error"
	}
	if reasons.GetDropParseLenError() {
		return "Drop Parse Len Error"
	}
	if reasons.GetDropHardwareError() {
		return "Drop Hardware Error"
	}
	return "Invalid"
}

func systemDropStatsShowEntry(entry *halproto.DropStatsEntry) {
	str := dropReasonToString(entry.GetReasons())
	if strings.Compare(str, "Invalid") == 0 {
		return
	}
	fmt.Printf("%-50s%-12d\n",
		str,
		entry.GetDropCount())
}

func egressDropReasonToString(reasons *halproto.EgressDropReasons) string {
	if reasons.GetDropOutputMapping() {
		return "Drop Output Mapping"
	}
	if reasons.GetDropPruneSrcPort() {
		return "Drop Prune Src Port"
	}
	if reasons.GetDropMirror() {
		return "Drop Mirror"
	}
	if reasons.GetDropPolicer() {
		return "Drop Policer"
	}
	if reasons.GetDropCopp() {
		return "Drop Copp"
	}
	if reasons.GetDropChecksumErr() {
		return "Drop Checksum Error"
	}
	return "Invalid"
}

func systemEgressDropStatsShowEntry(entry *halproto.EgressDropStatsEntry) {
	str := egressDropReasonToString(entry.GetReasons())
	if strings.Compare(str, "Invalid") == 0 {
		return
	}
	fmt.Printf("%-50s%-12d\n",
		str,
		entry.GetDropCount())
}

func systemTableStatsShowHeader() {
	hdrLine := strings.Repeat("-", 155)
	fmt.Println(hdrLine)
	fmt.Printf("%-22s%-30s%-12s%-12s%-12s%-16s%-12s%-12s%-12s%-12s\n",
		"Type", "Name", "Size", "Oflow Size", "Entries",
		"Oflow Entries", "Inserts", "Insert Err", "Deletes", "Delete Err")
	fmt.Println(hdrLine)
}

func systemTableStatsShowEntry(entry *halproto.TableStatsEntry) {
	fmt.Printf("%-22s%-30s%-12d%-12d%-12d%-16d%-12d%-12d%-12d%-12d\n",
		entry.GetTableType().String(),
		entry.GetTableName(),
		entry.GetTableSize(),
		entry.GetOverflowTableSize(),
		entry.GetEntriesInUse(),
		entry.GetOverflowEntriesInUse(),
		entry.GetNumInserts(),
		entry.GetNumInsertErrors(),
		entry.GetNumDeletes(),
		entry.GetNumDeleteErrors())
}

func fteStatsShow(stats *halproto.Stats) {
	var ftestats *halproto.FTEStats

	ftestats = stats.GetFteStats()

	fmt.Printf("\n%s%-15d\n", "Connection per-second		:", ftestats.GetConnPerSecond())
	fmt.Printf("%s%-15d\n", "Flow-miss Packets		:", ftestats.GetFlowMissPkts())
	fmt.Printf("%s%-15d\n", "Redir Packets			:", ftestats.GetRedirPkts())
	fmt.Printf("%s%-15d\n", "Cflow Packets			:", ftestats.GetCflowPkts())
	fmt.Printf("%s%-15d\n", "TCP Close Packets		:", ftestats.GetTcpClosePkts())
	fmt.Printf("%s%-15d\n", "TLS Proxy Packets		:", ftestats.GetTlsProxyPkts())
	fmt.Printf("%s%-15d\n", "Softq Reqs			:", ftestats.GetSoftqReqs())
	fmt.Printf("%s%-15d\n", "Queued Tx Packets		:", ftestats.GetQueuedTxPkts())
	fmt.Printf("\n%s\n", "FTE Error Count:")
	hdrLine := strings.Repeat("-", 56)
	fmt.Println(hdrLine)
	fmt.Printf("%25s%25s\n", "Error Type", "Drop Count")
	fmt.Println(hdrLine)
	for _, fteerr := range ftestats.FteErrors {
		if fteerr.GetCount() != 0 {
			fmt.Printf("%-25s%-25d\n", fteerr.GetFteError(), fteerr.GetCount())
		}
	}

	fmt.Printf("%s\n", "FTE Feature Stats:")
	hdrLines := strings.Repeat("-", 100)
	fmt.Println(hdrLines)
	fmt.Printf("%25s%25s%25s%25s\n", "Feature Name", "Drop Count", "Drop Reason", "Drops per-reason")
	fmt.Println(hdrLines)
	for _, featurestats := range ftestats.FeatureStats {
		fmt.Printf("%25s%25d", featurestats.GetFeatureName(), featurestats.GetDropPkts())
		for _, fteerr := range featurestats.DropReason {
			if fteerr.GetCount() != 0 {
				fmt.Printf("%25s%25d\n", fteerr.GetFteError(), fteerr.GetCount())
			}
		}
		fmt.Printf("\n")
	}
	fmt.Printf("\n")
}

func sessionSummaryStatsShow(stats *halproto.Stats) {
	var sessstats *halproto.SessionSummaryStats

	sessstats = stats.GetSessionStats()

	hdrLine := strings.Repeat("-", 56)
	fmt.Println(hdrLine)
	fmt.Printf("%25s%25s\n", "Session Type", "Count")
	fmt.Println(hdrLine)
	fmt.Printf("%25s%25d\n", "L2", sessstats.GetL2Sessions())
	fmt.Printf("%25s%25d\n", "TCP", sessstats.GetTcpSessions())
	fmt.Printf("%25s%25d\n", "UDP", sessstats.GetUdpSessions())
	fmt.Printf("%25s%25d\n", "ICMP", sessstats.GetIcmpSessions())
	fmt.Printf("%25s%25d\n", "DROP", sessstats.GetDropSessions())
}
