//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bytes"
	"context"
	"fmt"
	"os"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	clockFreq    uint32
	armClockFreq uint32
	traceLevel   string
	llcTypeStr   string
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

var systemStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show system statistics",
	Long:  "show system statistics",
}

var queueStatsCmd = &cobra.Command{
	Use:   "queue",
	Short: "show system statistics queue",
	Long:  "show system statistics queue",
	Run:   systemQueueStatsCmdHandler,
}

var systemQueueCreditsShowCmd = &cobra.Command{
	Use:   "queue-credits",
	Short: "show system statistics packet-buffer queue-credits",
	Long:  "show system statistics packet-buffer queue-credits",
	Run:   systemQueueCreditsShowCmdHandler,
}

var traceDebugCmd = &cobra.Command{
	Use:   "trace",
	Short: "set debug trace level",
	Long:  "set debug trace level",
	Run:   traceDebugCmdHandler,
}

var memoryDebugCmd = &cobra.Command{
	Use:   "memory",
	Short: "debug memory",
	Long:  "debug memory",
	Run:   memoryDebugCmdHandler,
}

var llcDebugCmd = &cobra.Command{
	Use:   "llc-cache",
	Short: "debug system llc-cache",
	Long:  "debug system llc-cache",
	Run:   llcSetupCmdHandler,
}

var llcShowCmd = &cobra.Command{
	Use:   "llc",
	Short: "show last level cache statistics",
	Long:  "show last level cache statistics",
	Run:   llcShowCmdHandler,
}

var pbShowCmd = &cobra.Command{
	Use:   "packet-buffer",
	Short: "show packet buffer statistics",
	Long:  "show packet buffer statistics",
	Run:   pbShowCmdHandler,
}

var dropShowCmd = &cobra.Command{
	Use:   "drop",
	Short: "show system drop statistics",
	Long:  "show system drop statistics",
	Run:   dropShowCmdHandler,
}

var pbDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show packet buffer statistics detail",
	Long:  "show packet buffer statistics detail",
	Run:   pbDetailShowCmdHandler,
}

func init() {
	debugCmd.AddCommand(systemDebugCmd)
	systemDebugCmd.Flags().Uint32VarP(&clockFreq, "clock-frequency", "c", 0, "Specify clock-frequency (Allowed: 833, 900, 957, 1033, 1100)")
	systemDebugCmd.Flags().Uint32VarP(&armClockFreq, "arm-clock-frequency", "a", 0, "Specify arm-clock-frequency (Allowed: 1667, 2200)")

	showCmd.AddCommand(systemShowCmd)
	systemShowCmd.Flags().Bool("power", false, "Show system power information")
	systemShowCmd.Flags().Bool("temperature", false, "Show system power information")
	systemShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	systemShowCmd.AddCommand(queueStatsCmd)
	queueStatsCmd.Flags().Bool("input", false, "Show system input queue-statistics")
	queueStatsCmd.Flags().Bool("output", false, "Show system input queue-statistics")
	queueStatsCmd.Flags().Bool("yaml", true, "Output in yaml")

	debugCmd.AddCommand(traceDebugCmd)
	traceDebugCmd.Flags().StringVar(&traceLevel, "level", "none", "Specify trace level (Allowed: none, error, warn, info, debug, verbose)")
	traceDebugCmd.Flags().Bool("flush", false, "Flush logs")

	debugCmd.AddCommand(llcDebugCmd)
	llcDebugCmd.Flags().StringVar(&llcTypeStr, "type", "none", "Specify LLC Cache type (Allowed: cache-read,cache-write,scratchpad-access,cache-hit,cache-miss,partial-write,cache-maint-op,eviction,retry-needed,retry-access,disable)")
	llcDebugCmd.MarkFlagRequired("type")

	debugCmd.AddCommand(memoryDebugCmd)
	memoryDebugCmd.Flags().Bool("memory-trim", false, "Reclaim free memory from malloc")
	memoryDebugCmd.MarkFlagRequired("memory-trim")

	systemShowCmd.AddCommand(systemStatsShowCmd)
	systemStatsShowCmd.AddCommand(llcShowCmd)
	llcShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	systemStatsShowCmd.AddCommand(pbShowCmd)
	pbShowCmd.Flags().Bool("yaml", true, "Output in yaml")

	pbShowCmd.AddCommand(systemQueueCreditsShowCmd)
	systemQueueCreditsShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	pbShowCmd.AddCommand(pbDetailShowCmd)
	pbDetailShowCmd.Flags().Bool("yaml", true, "Output in yaml")

	systemStatsShowCmd.AddCommand(dropShowCmd)
	dropShowCmd.Flags().Bool("yaml", true, "Output in yaml")
}

func dropShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewDeviceSvcClient(c)

	var req *pds.Empty

	// PDS call
	resp, err := client.DeviceGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting drop statistics failed. %v\n", err)
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
		printDropStats(resp)
	}
}

func printDropStatsHeader() {
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-50s%-12s\n",
		"Reason", "Drop Count")
	fmt.Println(hdrLine)
}

func printDropStats(resp *pds.DeviceGetResponse) {
	statistics := resp.GetResponse().GetStats()
	ingress := statistics.GetIngress()
	egress := statistics.GetEgress()

	fmt.Printf("Ingress drop statistics\n")
	printDropStatsHeader()
	for _, entry := range ingress {
		fmt.Printf("%-50s%-12d\n",
			entry.GetName(),
			entry.GetCount())
	}
	fmt.Printf("\nEgress drop statistics\n")
	printDropStatsHeader()
	for _, entry := range egress {
		fmt.Printf("%-50s%-12d\n",
			entry.GetName(),
			entry.GetCount())
	}
}

func systemQueueCreditsShowCmdHandler(cmd *cobra.Command, args []string) {
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
	resp, err := client.QueueCreditsGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Queue credits get failed. %v\n", err)
		return
	}

	if resp.GetApiStatus() != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.GetApiStatus())
		return
	}

	if cmd != nil && cmd.Flags().Changed("yaml") {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else {
		queueHeaderPrint()

		qVal := [16]uint32{}
		qVal2 := [16]uint32{}
		var str string

		for _, portCredit := range resp.GetPortQueueCredit() {
			fmt.Printf("%-5d|", portCredit.GetPort())
			for _, queueCredit := range portCredit.GetQueueCredit() {
				qIndex := queueCredit.GetQueue()
				if qIndex < 16 {
					qVal[qIndex] = queueCredit.GetCredit()
				} else {
					qVal2[qIndex-16] = queueCredit.GetCredit()
				}
			}
			str = fmt.Sprintf("%-9v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-9v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}
	}
}

func queueHeaderPrint() {
	hdrLine := strings.Repeat("-", 166)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d\n"+
		"%-6s%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d\n",
		"     |", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		"     |", 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31)
	fmt.Println(hdrLine)
}

func memoryDebugCmdHandler(cmd *cobra.Command, args []string) {
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
	_, err = client.MemoryTrim(context.Background(), empty)
	if err != nil {
		fmt.Printf("Memory trim failed. %v\n", err)
		return
	}

	fmt.Printf("Memory trim succeeded\n")
}

func systemQueueStatsCmdHandler(cmd *cobra.Command, args []string) {
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
	resp, err := client.PbStatsGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("PB statistics get failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	input := false
	output := false

	if cmd != nil && (cmd.Flags().Changed("input") ||
		cmd.Flags().Changed("ouput")) {
		if cmd.Flags().Changed("input") {
			input = true
		}
		if cmd.Flags().Changed("output") {
			output = true
		}
	} else {
		input = true
		output = true
	}

	if cmd != nil && cmd.Flags().Changed("yaml") {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else {
		systemQueueStatsPrint(resp.GetPbStats(), input, output)
	}
}

func systemQueueStatsPrint(resp *pds.PacketBufferStats, input bool, output bool) {
	portStats := resp.GetPortStats()
	var str string

	if input == true {
		fmt.Printf("Buffer Occupancy:\n")
		queueHeaderPrint()
		for _, port := range portStats {
			qVal := [16]uint32{}
			qVal2 := [16]uint32{}
			if port.GetPacketBufferPort().GetPortType() == 3 {
				fmt.Printf("%-5d|", port.GetPacketBufferPort().GetPortNum())
			} else {
				fmt.Printf("%-5s|", strings.Replace(port.GetPacketBufferPort().GetPortType().String(), "PACKET_BUFFER_PORT_TYPE_", "", -1))
			}
			for _, input := range port.GetQosQueueStats().GetInputQueueStats() {
				qIndex := input.GetInputQueueIdx()
				if qIndex < 16 {
					qVal[qIndex] = input.GetBufferOccupancy()
				} else {
					qVal2[qIndex-16] = input.GetBufferOccupancy()
				}
			}
			str = fmt.Sprintf("%-9v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-9v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}

		fmt.Printf("Peak Occupancy:\n")
		queueHeaderPrint()
		for _, port := range portStats {
			qVal := [16]uint32{}
			qVal2 := [16]uint32{}
			if port.GetPacketBufferPort().GetPortType() == 3 {
				fmt.Printf("%-5d|", port.GetPacketBufferPort().GetPortNum())
			} else {
				fmt.Printf("%-5s|", strings.Replace(port.GetPacketBufferPort().GetPortType().String(), "PACKET_BUFFER_PORT_TYPE_", "", -1))
			}
			for _, input := range port.GetQosQueueStats().GetInputQueueStats() {
				qIndex := input.GetInputQueueIdx()
				if qIndex < 16 {
					qVal[qIndex] = input.GetPeakOccupancy()
				} else {
					qVal2[qIndex-16] = input.GetPeakOccupancy()
				}
			}
			str = fmt.Sprintf("%-9v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-9v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}

		fmt.Printf("Input Queue Port Monitor:\n")
		queueHeaderPrint()
		for _, port := range portStats {
			qVal := [16]uint64{}
			qVal2 := [16]uint64{}
			if port.GetPacketBufferPort().GetPortType() == 3 {
				fmt.Printf("%-5d|", port.GetPacketBufferPort().GetPortNum())
			} else {
				fmt.Printf("%-5s|", strings.Replace(port.GetPacketBufferPort().GetPortType().String(), "PACKET_BUFFER_PORT_TYPE_", "", -1))
			}
			for _, input := range port.GetQosQueueStats().GetInputQueueStats() {
				qIndex := input.GetInputQueueIdx()
				if qIndex < 16 {
					qVal[qIndex] = input.GetPortMonitor()
				} else {
					qVal2[qIndex-16] = input.GetPortMonitor()
				}
			}
			str = fmt.Sprintf("%-9v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-9v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}
	}

	if output == true {
		fmt.Printf("Queue Depth:\n")
		queueHeaderPrint()
		for _, port := range portStats {
			qVal := [16]uint32{}
			qVal2 := [16]uint32{}
			if port.GetPacketBufferPort().GetPortType() == 3 {
				fmt.Printf("%-5d|", port.GetPacketBufferPort().GetPortNum())
			} else {
				fmt.Printf("%-5s|", strings.Replace(port.GetPacketBufferPort().GetPortType().String(), "PACKET_BUFFER_PORT_TYPE_", "", -1))
			}
			for _, output := range port.GetQosQueueStats().GetOutputQueueStats() {
				qIndex := output.GetOutputQueueIdx()
				if qIndex < 16 {
					qVal[qIndex] = output.GetQueueDepth()
				} else {
					qVal2[qIndex-16] = output.GetQueueDepth()
				}
			}
			str = fmt.Sprintf("%-9v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-9v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}

		fmt.Printf("Output Queue Port Monitor:\n")
		queueHeaderPrint()
		for _, port := range portStats {
			qVal := [16]uint64{}
			qVal2 := [16]uint64{}
			if port.GetPacketBufferPort().GetPortType() == 3 {
				fmt.Printf("%-5d|", port.GetPacketBufferPort().GetPortNum())
			} else {
				fmt.Printf("%-5s|", strings.Replace(port.GetPacketBufferPort().GetPortType().String(), "PACKET_BUFFER_PORT_TYPE_", "", -1))
			}
			for _, output := range port.GetQosQueueStats().GetOutputQueueStats() {
				qIndex := output.GetOutputQueueIdx()
				if qIndex < 16 {
					qVal[qIndex] = output.GetPortMonitor()
				} else {
					qVal2[qIndex-16] = output.GetPortMonitor()
				}
			}
			str = fmt.Sprintf("%-9v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-9v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}
	}
}

func pbShowCmdHandler(cmd *cobra.Command, args []string) {
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
	resp, err := client.PbStatsGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("PB statistics get failed. %v\n", err)
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
		var dmaIn uint32
		var dmaOut uint32
		var ingIn uint32
		var ingOut uint32
		var egrIn uint32
		var egrOut uint32
		uplinkIn := []uint32{0, 0, 0, 0, 0, 0, 0, 0, 0}
		uplinkOut := []uint32{0, 0, 0, 0, 0, 0, 0, 0, 0}

		for _, entry := range resp.GetPbStats().GetPortStats() {
			if entry.GetPacketBufferPort().GetPortType() == pds.PacketBufferPortType_PACKET_BUFFER_PORT_TYPE_DMA {
				dmaIn = entry.GetBufferStats().GetSopCountIn()
				dmaOut = entry.GetBufferStats().GetSopCountOut()
			} else if entry.GetPacketBufferPort().GetPortType() == pds.PacketBufferPortType_PACKET_BUFFER_PORT_TYPE_P4IG {
				ingIn = entry.GetBufferStats().GetSopCountIn()
				ingOut = entry.GetBufferStats().GetSopCountOut()
			} else if entry.GetPacketBufferPort().GetPortType() == pds.PacketBufferPortType_PACKET_BUFFER_PORT_TYPE_P4EG {
				egrIn = entry.GetBufferStats().GetSopCountIn()
				egrOut = entry.GetBufferStats().GetSopCountOut()
			} else if entry.GetPacketBufferPort().GetPortType() == pds.PacketBufferPortType_PACKET_BUFFER_PORT_TYPE_UPLINK {
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

func leftArrow() uint32 {
	return 8592
	//return 11013
}

func rightArrow() uint32 {
	return 8594
	// return 10145
}

func upArrow() uint32 {
	return 8595
	// return 11014
}

func downArrow() uint32 {
	return 8593
	// return 11015
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
	fmt.Print(strings.Repeat(" ", 10))

	fmt.Printf("%c 0 %c", downArrow(), upArrow()) // Up arrow,  Down arrow
	for i := 1; i < 9; i++ {
		fmt.Print(strings.Repeat(" ", 8))
		fmt.Printf("%c %d %c", downArrow(), i, upArrow()) // Up arrow,  Down arrow
	}
	fmt.Println()
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
	fmt.Printf("%c", 65372)                     // Vert. bar
	fmt.Printf(" %c %-5d", rightArrow(), ingIn) // -> arrow
	fmt.Print(strings.Repeat(" ", 111))
	fmt.Printf("%5d %c ", egrIn, leftArrow()) // <- arrow
	fmt.Printf("%c", 65372)                   // Vert. bar
	fmt.Print(" R ")
	fmt.Printf("%c\n", 65372) // Vert. bar
}

func printP4OutStats(ingOut uint32, egrOut uint32) {
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(" P ")
	fmt.Printf("%c", 65372)                     // Vert. bar
	fmt.Printf(" %c %-5d", leftArrow(), ingOut) // <- arrow
	fmt.Print(strings.Repeat(" ", 111))
	fmt.Printf("%5d %c ", egrOut, rightArrow()) // -> arrow
	fmt.Printf("%c", 65372)                     // Vert. bar
	fmt.Print(" P ")
	fmt.Printf("%c\n", 65372) // Vert. bar
}

func printDMAStats(dmaIn uint32, dmaOut uint32) {
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 3))
	fmt.Printf("%c", 65372) // Vert. bar
	fmt.Print(strings.Repeat(" ", 25))
	fmt.Printf("%5d %c", dmaIn, upArrow()) // Down arrow
	fmt.Print(strings.Repeat(" ", 58))
	fmt.Printf("%5d %c", dmaOut, downArrow()) // Up arrow
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
		llcGetPrintHeader()
		llcGetPrintResp(resp)
	}
}

func llcGetPrintHeader() {
	hdrLine := strings.Repeat("-", 40)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-20s%-10s\n", "Channel", "Type", "Count")
	fmt.Println(hdrLine)
}

func llcGetPrintResp(resp *pds.LlcStatsGetResponse) {
	statistics := resp.GetStats()
	count := statistics.GetCount()
	str := strings.ToLower(strings.Replace(statistics.GetType().String(), "LLC_COUNTER_", "", -1))
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

	if cmd != nil && cmd.Flags().Changed("type") {
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

	if cmd != nil && cmd.Flags().Changed("level") {
		if isTraceLevelValid(traceLevel) != true {
			fmt.Printf("Invalid argument\n")
			return
		}
		traceReq = &pds.TraceRequest{
			TraceLevel: inputToTraceLevel(traceLevel),
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
	} else if cmd == nil || cmd.Flags().Changed("flush") {
		var empty *pds.Empty

		// PDS call
		_, err := client.TraceFlush(context.Background(), empty)
		if err != nil {
			fmt.Printf("Flush logs failed. %v\n", err)
			return
		}

		fmt.Printf("Flush logs succeeded\n")
	} else {
		fmt.Printf("Argument required. Use --help for valid arguments\n")
		return
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
	yamlOutput := false

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewDebugSvcClient(c)

	if cmd != nil && cmd.Flags().Changed("power") {
		power = true
	}
	if cmd != nil && cmd.Flags().Changed("temperature") {
		temp = true
	}
	if cmd != nil && cmd.Flags().Changed("yaml") {
		yamlOutput = true
	}

	if cmd == nil || (cmd.Flags().Changed("power") == false &&
		cmd.Flags().Changed("temperature") == false) {
		temp = true
		power = true
	}

	if power {
		systemPowerShow(client, yamlOutput)
	}

	if temp {
		systemTemperatureShow(client, yamlOutput)
	}
}

func systemPowerShow(client pds.DebugSvcClient, yamlOutput bool) {
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

	if yamlOutput {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else {
		printPowerHeader()
		fmt.Printf("%-10s%-10d\n", "Pin", resp.GetPin())
		fmt.Printf("%-10s%-10d\n", "Pout1", resp.GetPout1())
		fmt.Printf("%-10s%-10d\n", "Pout2", resp.GetPout2())
	}
}

func printPowerHeader() {
	hdrLine := strings.Repeat("-", 20)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s\n", "Type", "Power(W)")
	fmt.Println(hdrLine)
}

func systemTemperatureShow(client pds.DebugSvcClient, yamlOutput bool) {
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

	if yamlOutput {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else {
		printTempHeader()
		fmt.Printf("%-20s%-10d\n", "Die Temperature", resp.GetDieTemp())
		fmt.Printf("%-20s%-10d\n", "Local Temperature", resp.GetLocalTemp())
		fmt.Printf("%-20s%-10d\n", "HBM Temperature", resp.GetHbmTemp())
	}
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

	if cmd != nil && cmd.Flags().Changed("clock-frequency") {
		systemClockFrequencySet(client)
	}

	if cmd != nil && cmd.Flags().Changed("arm-clock-frequency") {
		systemArmClockFrequencySet(client)
	}
}

func systemClockFrequencySet(client pds.DebugSvcClient) {
	req := &pds.ClockFrequencyRequest{
		ClockFrequency:    clockFreq,
		ArmClockFrequency: 0,
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

func systemArmClockFrequencySet(client pds.DebugSvcClient) {
	req := &pds.ClockFrequencyRequest{
		ClockFrequency:    0,
		ArmClockFrequency: armClockFreq,
	}

	// PDS call
	resp, err := client.ClockFrequencyUpdate(context.Background(), req)
	if err != nil {
		fmt.Printf("ARM Clock-frequency update failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	fmt.Printf("ARM Clock-frequency set to %dMhz. Please reboot for changes to take effect\n", armClockFreq)
}

// InputQueueInfo defines the input queue details
type InputQueueInfo struct {
	queueIndex      uint32
	bufferOccupancy uint32
	peakOccupancy   uint32
	portMonitor     uint64
	valid           bool
}

// OutputQueueInfo defines the output queue details
type OutputQueueInfo struct {
	queueIndex  uint32
	queueDepth  uint32
	portMonitor uint64
	valid       bool
}

func pbOqCountersShowInternal(outputQueueInfo [][]OutputQueueInfo, startQ uint32, endQ uint32) {
	var dashes bytes.Buffer

	fmt.Printf("%-9s", "Queue | ")
	dashes.WriteString(strings.Repeat("-", 9))
	for iQ := startQ; iQ <= endQ; iQ++ {
		fmt.Printf("%-18d | ", iQ)
		dashes.WriteString(strings.Repeat("-", 21))
	}
	fmt.Printf("\n" + dashes.String() + "\n")
	dashes.Reset()

	fmt.Printf("%-9s", "Port  | ")
	dashes.WriteString(strings.Repeat("-", 9))
	for iQ := startQ; iQ <= endQ; iQ++ {
		fmt.Printf("%-8s%-10s | ", "depth", "port_mon")
		dashes.WriteString(strings.Repeat("-", 21))
	}
	fmt.Printf("\n" + dashes.String() + "\n")

	for pbPort := 0; pbPort < 12; pbPort++ {
		fmt.Printf("%-9d", pbPort)
		for iQ := startQ; iQ <= endQ; iQ++ {
			if outputQueueInfo[pbPort][iQ].valid == true {
				fmt.Printf("%-8d%-10d | ",
					outputQueueInfo[pbPort][iQ].queueDepth,
					outputQueueInfo[pbPort][iQ].portMonitor)
			} else {
				fmt.Printf("%-18s | ", "-")
			}
		}
		fmt.Printf("\n")
	}
	fmt.Printf(dashes.String() + "\n")
}

func pbOqCountersShow(outputQueueInfo [][]OutputQueueInfo) {
	fmt.Printf("Output Queue Depth and Port monitor\n")
	fmt.Printf("-----------------------------------\n")
	pbOqCountersShowInternal(outputQueueInfo, 0, 7)
	fmt.Printf("\n")
	pbOqCountersShowInternal(outputQueueInfo, 8, 15)
	fmt.Printf("\n")
	pbOqCountersShowInternal(outputQueueInfo, 16, 23)
	fmt.Printf("\n")
	pbOqCountersShowInternal(outputQueueInfo, 24, 31)
	fmt.Printf("\n")
}

func pbIqPortMonShowInternal(inputQueueInfo [][]InputQueueInfo, startQ uint32, endQ uint32) {
	var dashes bytes.Buffer

	fmt.Printf("%-9s", "Queue | ")
	dashes.WriteString(strings.Repeat("-", 9))
	for iQ := startQ; iQ <= endQ; iQ++ {
		fmt.Printf("%-10d | ", iQ)
		dashes.WriteString(strings.Repeat("-", 13))
	}
	fmt.Printf("\n" + dashes.String() + "\n")
	dashes.Reset()

	fmt.Printf("%-9s", "Port  | ")
	dashes.WriteString(strings.Repeat("-", 9))
	for iQ := startQ; iQ <= endQ; iQ++ {
		fmt.Printf("%-10s | ", "port_mon")
		dashes.WriteString(strings.Repeat("-", 13))
	}
	fmt.Printf("\n" + dashes.String() + "\n")

	for pbPort := 0; pbPort < 12; pbPort++ {
		fmt.Printf("%-9d", pbPort)
		for iQ := startQ; iQ <= endQ; iQ++ {
			if inputQueueInfo[pbPort][iQ].valid == true {
				fmt.Printf("%-10d | ",
					inputQueueInfo[pbPort][iQ].portMonitor)
			} else {
				fmt.Printf("%-10s | ", "-")
			}
		}
		fmt.Printf("\n")
	}
	fmt.Printf(dashes.String() + "\n")
}

func pbOccupancyCounterShowInternal(inputQueueInfo [][]InputQueueInfo, startQ uint32, endQ uint32) {
	var dashes bytes.Buffer

	fmt.Printf("%-9s", "Queue | ")
	dashes.WriteString(strings.Repeat("-", 9))
	for iQ := startQ; iQ <= endQ; iQ++ {
		fmt.Printf("%-16d | ", iQ)
		dashes.WriteString(strings.Repeat("-", 19))
	}
	fmt.Printf("\n" + dashes.String() + "\n")
	dashes.Reset()

	fmt.Printf("%-9s", "Port  | ")
	dashes.WriteString(strings.Repeat("-", 9))
	for iQ := startQ; iQ <= endQ; iQ++ {
		fmt.Printf("%-8s%-8s | ", "occ", "peak")
		dashes.WriteString(strings.Repeat("-", 19))
	}
	fmt.Printf("\n" + dashes.String() + "\n")

	for pbPort := 0; pbPort < 12; pbPort++ {
		fmt.Printf("%-9d", pbPort)
		for iQ := startQ; iQ <= endQ; iQ++ {
			if inputQueueInfo[pbPort][iQ].valid == true {
				fmt.Printf("%-8d%-8d | ",
					inputQueueInfo[pbPort][iQ].bufferOccupancy,
					inputQueueInfo[pbPort][iQ].peakOccupancy)
			} else {
				fmt.Printf("%-8s%-8s | ", "-", "-")
			}
		}
		fmt.Printf("\n")
	}
	fmt.Printf(dashes.String() + "\n")
}

func pbOccupancyCountersShow(inputQueueInfo [][]InputQueueInfo) {
	fmt.Printf("Input Queue Occupancy, Peak\n")
	fmt.Printf("--------------------------------------------\n")
	pbOccupancyCounterShowInternal(inputQueueInfo, 0, 7)
	fmt.Printf("\n")
	pbOccupancyCounterShowInternal(inputQueueInfo, 8, 15)
	fmt.Printf("\n")
	pbOccupancyCounterShowInternal(inputQueueInfo, 16, 23)
	fmt.Printf("\n")
	pbOccupancyCounterShowInternal(inputQueueInfo, 24, 31)
	fmt.Printf("\n")

	fmt.Printf("Input Queue Port monitor\n")
	fmt.Printf("--------------------------------------------\n")
	pbIqPortMonShowInternal(inputQueueInfo, 0, 7)
	fmt.Printf("\n")
	pbIqPortMonShowInternal(inputQueueInfo, 8, 15)
	fmt.Printf("\n")
	pbIqPortMonShowInternal(inputQueueInfo, 16, 23)
	fmt.Printf("\n")
	pbIqPortMonShowInternal(inputQueueInfo, 24, 31)
	fmt.Printf("\n")
}

func pbOccupancyOqCountersPopulate(portStats *pds.PacketBufferPortStats,
	inputQueueInfo [][]InputQueueInfo,
	outputQueueInfo [][]OutputQueueInfo) {
	portNum := portStats.GetPacketBufferPort().GetPortNum()

	for _, inputQueueStats := range portStats.GetQosQueueStats().GetInputQueueStats() {
		inputQueueIndex := inputQueueStats.GetInputQueueIdx()
		bufferOccupancy := inputQueueStats.GetBufferOccupancy()
		peakOccupancy := inputQueueStats.GetPeakOccupancy()
		portMonitor := inputQueueStats.GetPortMonitor()
		inputQueueInfo[portNum][inputQueueIndex].valid = true
		inputQueueInfo[portNum][inputQueueIndex].bufferOccupancy = bufferOccupancy
		inputQueueInfo[portNum][inputQueueIndex].peakOccupancy = peakOccupancy
		inputQueueInfo[portNum][inputQueueIndex].portMonitor = portMonitor
	}
	for _, outputQueueStats := range portStats.GetQosQueueStats().GetOutputQueueStats() {
		outputQueueIndex := outputQueueStats.GetOutputQueueIdx()
		outputQueueDepth := outputQueueStats.GetQueueDepth()
		portMonitor := outputQueueStats.GetPortMonitor()
		outputQueueInfo[portNum][outputQueueIndex].valid = true
		outputQueueInfo[portNum][outputQueueIndex].queueDepth = outputQueueDepth
		outputQueueInfo[portNum][outputQueueIndex].portMonitor = portMonitor
	}
}

func pbDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := pds.NewDebugSvcClient(c)

	var empty *pds.Empty

	// PDS call
	resp, err := client.PbStatsGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("PB statistics get failed. %v\n", err)
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
		inputQueueInfo := make([][]InputQueueInfo, 12)
		for i := range inputQueueInfo {
			inputQueueInfo[i] = make([]InputQueueInfo, 32)
		}

		outputQueueInfo := make([][]OutputQueueInfo, 12)
		for i := range outputQueueInfo {
			outputQueueInfo[i] = make([]OutputQueueInfo, 32)
		}

		for _, entry := range resp.GetPbStats().GetPortStats() {
			pbOccupancyOqCountersPopulate(entry, inputQueueInfo, outputQueueInfo)
		}

		pbOccupancyCountersShow(inputQueueInfo)
		pbOqCountersShow(outputQueueInfo)
	}
}
