//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	// "math/rand"
	"os"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var systemShowCmd = &cobra.Command{
	Use:   "system",
	Short: "show system information",
	Long:  "show system information",
}

var systemStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show system statistics [ingress-drop | egress-drop | fte | table | api | pb | all] (Default: all)",
	Long:  "show system statistics [ingress-drop | egress-drop | fte | table | api | pb | all] (Default: all)",
	Run:   systemStatsShowCmdHandler,
}

var threadShowCmd = &cobra.Command{
	Use:   "thread",
	Short: "show system threads",
	Long:  "show system threads",
	Run:   threadShowCmdHandler,
}

var threadDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show system threads information",
	Long:  "show system threads information",
	Run:   threadDetailShowCmdHandler,
}

func init() {
	showCmd.AddCommand(systemShowCmd)
	systemShowCmd.AddCommand(systemStatsShowCmd)
	systemShowCmd.AddCommand(threadShowCmd)
	threadShowCmd.AddCommand(threadDetailShowCmd)
}

func systemStatsShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Errorf("Could not connect to the HAL. Is HAL Running?")
		os.Exit(1)
	}
	client := halproto.NewSystemClient(c.ClientConn)

	// Check the args
	ingressDrop := false
	egressDrop := false
	table := false
	fte := false
	api := false
	pb := false

	if len(args) > 0 {
		if strings.Compare(args[0], "ingress-drop") == 0 {
			ingressDrop = true
		} else if strings.Compare(args[0], "egress-drop") == 0 {
			egressDrop = true
		} else if strings.Compare(args[0], "table") == 0 {
			table = true
		} else if strings.Compare(args[0], "fte") == 0 {
			fte = true
		} else if strings.Compare(args[0], "api") == 0 {
			api = true
		} else if strings.Compare(args[0], "pb") == 0 {
			pb = true
		} else if strings.Compare(args[0], "all") == 0 {
			ingressDrop = true
			egressDrop = true
			table = true
			fte = true
			api = true
			pb = true
		} else {
			fmt.Printf("Invalid argument\n")
			return
		}
	} else {
		ingressDrop = true
		egressDrop = true
		table = true
		fte = true
		api = true
		pb = true
	}

	// HAL call
	var empty *halproto.Empty
	resp, err := client.SystemGet(context.Background(), empty)
	if err != nil {
		log.Errorf("Getting System Stats failed. %v", err)
		return
	}

	if resp.GetApiStatus() != halproto.ApiStatus_API_STATUS_OK {
		log.Errorf("HAL Returned non OK status. %v", resp.GetApiStatus())
		return
	}

	if ingressDrop {
		// Print Header
		fmt.Println("\nSystem Drop Stats:")
		systemDropStatsShowHeader()

		// Print System Drop Stats
		for _, entry := range resp.GetStats().GetDropStats().DropEntries {
			systemDropStatsShowEntry(entry)
		}
	}

	if egressDrop {
		// Print Header
		fmt.Println("\nSystem Egress Drop Stats:")
		systemDropStatsShowHeader()

		// Print System Egress Drop Stats
		for _, entry := range resp.GetStats().GetEgressDropStats().DropEntries {
			systemEgressDropStatsShowEntry(entry)
		}
	}

	if table {
		// Print Header
		fmt.Println("\nSystem Table Stats:")
		systemTableStatsShowHeader()

		// Print Table Stats
		for _, entry := range resp.GetStats().GetTableStats().TableStats {
			systemTableStatsShowEntry(entry)
		}
	}

	if api {
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

	if fte {
		fmt.Println("\nFTE Stats:")
		fteStatsShow(resp.GetStats())

		fmt.Println("\nSession Summary Stats:")
		sessionSummaryStatsShow(resp.GetStats())
	}

	if pb {
		var dmaIn uint32
		var dmaOut uint32
		var ingIn uint32
		var ingOut uint32
		var egrIn uint32
		var egrOut uint32
		uplinkIn := []uint32{0, 0, 0, 0, 0, 0, 0, 0, 0}
		uplinkOut := []uint32{0, 0, 0, 0, 0, 0, 0, 0, 0}

		for _, entry := range resp.GetStats().GetPacketBufferStats().PortStats {
			if entry.GetPacketBufferPort().GetPortType() == halproto.PacketBufferPortType_PACKET_BUFFER_PORT_TYPE_DMA {
				dmaIn = entry.GetBufferStats().GetSopCountIn()
				dmaOut = entry.GetBufferStats().GetSopCountOut()
			} else if entry.GetPacketBufferPort().GetPortType() == halproto.PacketBufferPortType_PACKET_BUFFER_PORT_TYPE_P4IG {
				ingIn = entry.GetBufferStats().GetSopCountIn()
				ingOut = entry.GetBufferStats().GetSopCountOut()
			} else if entry.GetPacketBufferPort().GetPortType() == halproto.PacketBufferPortType_PACKET_BUFFER_PORT_TYPE_P4EG {
				egrIn = entry.GetBufferStats().GetSopCountIn()
				egrOut = entry.GetBufferStats().GetSopCountOut()
			} else if entry.GetPacketBufferPort().GetPortType() == halproto.PacketBufferPortType_PACKET_BUFFER_PORT_TYPE_UPLINK {
				uplinkIn[entry.GetPacketBufferPort().GetPortNum()] = entry.GetBufferStats().GetSopCountIn()
				uplinkOut[entry.GetPacketBufferPort().GetPortNum()] = entry.GetBufferStats().GetSopCountOut()
			}
		}
		pbStatsShow(dmaIn, dmaOut,
			ingIn, ingOut,
			egrIn, egrOut,
			uplinkIn, uplinkOut)
	}
}

func pbStatsShow(dmaIn uint32, dmaOut uint32,
	ingIn uint32, ingOut uint32,
	egrIn uint32, egrOut uint32,
	portIn []uint32, portOut []uint32) {

	// Randomized values
	// dmaIn = uint32(rand.Intn(65535))
	// dmaOut = uint32(rand.Intn(65535))
	// ingIn = uint32(rand.Intn(65535))
	// ingOut = uint32(rand.Intn(65535))
	// egrIn = uint32(rand.Intn(65535))
	// egrOut = uint32(rand.Intn(65535))
	// for i := 0; i < 9; i++ {
	// 	portIn[i] = uint32(rand.Intn(65535))
	// 	portOut[i] = uint32(rand.Intn(65535))
	// }

	fmt.Println()

	fmt.Println("Packet Buffer (PB) Stats:")

	// P4+ block
	fmt.Print(" ")
	fmt.Print(strings.Repeat(" ", 5))
	fmt.Println(strings.Repeat("-", 129))
	// 2nd line
	fmt.Print(strings.Repeat(" ", 5))
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 28))
	fmt.Print("TX-DMA")
	fmt.Print(strings.Repeat(" ", 29))
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 28))
	fmt.Print("RX-DMA")
	fmt.Print(strings.Repeat(" ", 28))
	fmt.Printf("%c\n", 65372) // Vert. bar

	// 3rd line
	fmt.Print(" ")
	fmt.Print(strings.Repeat("-", 4))
	fmt.Print(" ")
	fmt.Print(strings.Repeat("-", 129))
	fmt.Print(" ")
	fmt.Println(strings.Repeat("-", 4))

	// 4th line
	printDMAStats(dmaIn, dmaOut)
	// 5th line
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 3))
	blankSpaces(127)
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%c\n", 65372) // Vert. bar
	// 6th line
	printP4OutStats(ingOut, egrOut)
	// 7th line
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(" 4 ")
	blankSpaces(127)
	fmt.Print(" 4 ")
	fmt.Printf("%c\n", 65372) // Vert. bar
	// 8th line
	printPacketBuffer()
	// 9th line
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(" N ")
	blankSpaces(127)
	fmt.Print(" G ")
	fmt.Printf("%c\n", 65372) // Vert. bar
	// 10th line
	printP4InStats(ingIn, egrIn)
	// 11th line
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 3))
	blankSpaces(127)
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%c\n", 65372) // Vert. bar

	printOddUplinkStats(portIn, portOut)

	// Hor line
	fmt.Print(" ")
	fmt.Print(strings.Repeat("-", 4))
	fmt.Print(" ")
	fmt.Print(strings.Repeat("-", 12))

	// Print dotted line for uplinks
	for i := 0; i < 9; i++ {
		fmt.Printf("%c", 9679) // Circle
		fmt.Print(strings.Repeat("-", 12))
	}
	fmt.Print(" ")
	fmt.Println(strings.Repeat("-", 4))

	printUplinkNum()

	printEvenUplinkStats(portIn, portOut)

	fmt.Println()
	fmt.Println()
}

func printOddUplinkStats(portIn []uint32, portOut []uint32) {
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 18))
	fmt.Printf("%5d", portIn[1])
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%-5d", portOut[1])

	for i := 3; i < 9; i = i + 2 {
		fmt.Print(strings.Repeat(" ", 13))
		fmt.Printf("%5d", portIn[i])
		fmt.Print(strings.Repeat(" ", 3))
		fmt.Printf("%-5d", portOut[i])
	}

	fmt.Print(strings.Repeat(" ", 18))
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%c\n", 65372) // Vert. bar
}

func printEvenUplinkStats(portIn []uint32, portOut []uint32) {
	fmt.Print(" ")
	fmt.Print(strings.Repeat(" ", 5))
	fmt.Print(strings.Repeat(" ", 6))
	fmt.Printf("%5d", portIn[0])
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%-5d", portOut[0])

	for i := 2; i < 10; i = i + 2 {
		fmt.Print(strings.Repeat(" ", 13))
		fmt.Printf("%5d", portIn[i])
		fmt.Print(strings.Repeat(" ", 3))
		fmt.Printf("%-5d", portOut[i])
	}
}

func printUplinkNum() {
	fmt.Print(" ")
	fmt.Print(strings.Repeat(" ", 5))
	spaceLine := strings.Repeat(" ", 9)
	fmt.Print(spaceLine)

	fmt.Printf("%c  0 %c", 11014, 11015) // Arrows
	for i := 1; i < 9; i++ {
		fmt.Print(strings.Repeat(" ", 7))
		fmt.Printf("%c  %d %c", 11014, i, 11015) // Arrows
	}
	fmt.Println()
}

func printBlock(ingIn uint32, ingOut uint32, egrIn uint32, egrOut uint32) {
	// ----- First Line ------
	fmt.Printf("%c", 65372) // Vert. bar
	// Indent
	spaceLine := strings.Repeat(" ", 30)
	fmt.Print(spaceLine)

	// Ingress top bar
	horLine := strings.Repeat("-", 10)
	fmt.Print(horLine)

	spaceLine = strings.Repeat(" ", 40)
	fmt.Print(spaceLine)

	// Egress top bar
	horLine = strings.Repeat("-", 10)
	fmt.Print(horLine)

	// Spaces after egress
	spaceLine = strings.Repeat(" ", 37)
	fmt.Print(spaceLine)

	fmt.Printf("%c\n", 65372) // Vert. bar

	// ----- Second Line ------
	fmt.Printf("%c", 65372) // Vert. bar
	// Indent
	spaceLine = strings.Repeat(" ", 21)
	fmt.Print(spaceLine)

	fmt.Printf("%5d ", ingIn)

	fmt.Printf("%c ", 10145) // Right arrow

	// Ingress left bar
	fmt.Printf("%c", 65372) // Vert. bar

	// Ingress spaces
	fmt.Printf("Ingress ")

	// Ingress right bar
	fmt.Printf("%c", 65372) // Vert. bar

	fmt.Printf("%c", 10145) // Right arrow

	fmt.Printf(" %-5d", ingOut)

	spaceLine = strings.Repeat(" ", 23)
	fmt.Print(spaceLine)

	fmt.Printf("%5d ", egrIn)

	fmt.Printf("%c ", 10145) // Right arrow

	// Egress left bar
	fmt.Printf("%c", 65372) // Vert. bar

	// Egress spaces
	fmt.Printf(" Egress ")

	// Egress right bar
	fmt.Printf("%c", 65372) // Vert. bar

	fmt.Printf("%c", 10145) // Right arrow

	fmt.Printf(" %-5d", egrOut)

	// Spaces after egress
	spaceLine = strings.Repeat(" ", 29)
	fmt.Print(spaceLine)

	fmt.Printf("%c\n", 65372) // Vert. bar

	// ----- Third Line ------
	fmt.Printf("%c", 65372) // Vert. bar
	// Indent
	spaceLine = strings.Repeat(" ", 30)
	fmt.Print(spaceLine)

	// Ingress top bar
	horLine = strings.Repeat("-", 10)
	fmt.Print(horLine)

	spaceLine = strings.Repeat(" ", 40)
	fmt.Print(spaceLine)

	// Egress top bar
	horLine = strings.Repeat("-", 10)
	fmt.Print(horLine)

	// Spaces after egress
	spaceLine = strings.Repeat(" ", 37)
	fmt.Print(spaceLine)

	fmt.Printf("%c\n", 65372) // Vert. bar

}

func printPacketBuffer() {
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(" I ")
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 58))
	fmt.Print("Packet Buffer")
	fmt.Print(strings.Repeat(" ", 56))
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(" E ")
	fmt.Printf("%c\n", 65372) // Vert. bar
}

func printP4InStats(ingIn uint32, egrIn uint32) {
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(" G ")
	fmt.Printf("%c", 65372)              // Vert. bar
	fmt.Printf(" %c %-5d", 10145, ingIn) // -> arrow
	fmt.Print(strings.Repeat(" ", 111))
	fmt.Printf("%5d %c ", egrIn, 11013) // <- arrow
	fmt.Printf("%c", 65372)             // Vert. bar
	fmt.Print(" R ")
	fmt.Printf("%c\n", 65372) // Vert. bar
}

func printP4OutStats(ingOut uint32, egrOut uint32) {
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(" P ")
	fmt.Printf("%c", 65372)               // Vert. bar
	fmt.Printf(" %c %-5d", 11013, ingOut) // <- arrow
	fmt.Print(strings.Repeat(" ", 111))
	fmt.Printf("%5d %c ", egrOut, 10145) // -> arrow
	fmt.Printf("%c", 65372)              // Vert. bar
	fmt.Print(" P ")
	fmt.Printf("%c\n", 65372) // Vert. bar
}

func printDMAStats(dmaIn uint32, dmaOut uint32) {
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 25))
	fmt.Printf("%5d %c", dmaIn, 11015)
	fmt.Print(strings.Repeat(" ", 58))
	fmt.Printf("%5d %c", dmaOut, 11014)
	fmt.Print(strings.Repeat(" ", 30))
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%c\n", 65372) // Vert. bar
}

func blankSpaces(num int) {
	fmt.Printf("%c", 65372) // Vert. bar
	blankLine := strings.Repeat(" ", num)
	fmt.Print(blankLine)
	fmt.Printf("%c", 65372) // Vert. bar
}

func blankLine(num int) {
	fmt.Printf("%c", 65372) // Vert. bar
	blankLine := strings.Repeat(" ", num)
	fmt.Print(blankLine)
	fmt.Printf("%c\n", 65372) // Vert. bar
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

func threadShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Errorf("Could not connect to the HAL. Is HAL Running?")
		os.Exit(1)
	}
	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.ThreadGet(context.Background(), empty)
	if err != nil {
		log.Errorf("Thread get failed. %v", err)
	}

	threadShowHeader()

	// Print Response
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		threadShowEntry(resp)
	}
}

func threadShowHeader() {
	hdrLine := strings.Repeat("-", 150)
	fmt.Println(hdrLine)
	fmt.Printf("%-20s%-10s%-12s%-14s%-14s%-10s%-20s%-10s%-21s%-10s%-12s\n",
		"Name", "Id", "PThreadId", "CtrlCoreMask", "DataCoreMask", "Prio", "SchedPolicy", "Running", "Role", "CoreMask", "LastHb(ms)")
	fmt.Println(hdrLine)
}

func threadShowEntry(resp *halproto.ThreadResponse) {
	spec := resp.GetSpec()
	status := resp.GetStatus()

	fmt.Printf("%-20s%-10d%-12d%-#14x%-#14x%-10d%-20s%-10t%-21s%-#10x%-12d\n",
		spec.GetName(), spec.GetId(), spec.GetPthreadId(),
		resp.GetControlCoreMask(), resp.GetDataCoreMask(),
		spec.GetPrio(), spec.GetSchedPolicy().String(),
		spec.GetRunning(), spec.GetRole().String(),
		spec.GetCoreMask(), status.GetLastHb()/1000)
}

func threadDetailShow(ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Errorf("Could not connect to the HAL. Is HAL Running?")
		os.Exit(1)
	}
	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.ThreadGet(context.Background(), empty)
	if err != nil {
		log.Errorf("Thread get failed. %v", err)
	}

	// Print Response
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				log.Errorf("Failed to write to file %s, err : %v",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func threadDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	threadDetailShow(nil)
}
