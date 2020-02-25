package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

var tsShowCmd = &cobra.Command{
	Use:   "techsupport",
	Short: "collect technical support information",
	Long:  "collect technical support information",
	Run:   tsShowCmdHandler,
}

// default location of the output file
var (
	outDir  = "/tmp/hal_techsupport"
	outFile = "hal-cmds.txt"
)

func init() {
	showCmd.AddCommand(tsShowCmd)
	tsShowCmd.Flags().StringVar(&outDir, "out", outDir, "output directory")
}

func tsShowCmdHandler(cmd *cobra.Command, args []string) {
	outFile = outDir + "/" + outFile

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	fmt.Printf("Capturing techsupport information\n\n")
	fmt.Printf("Flush HAL logs\n\n")
	flushLogsDebugCmdHandler(nil, nil)
	fmt.Printf("Lif information\n\n")
	handlelifDetailShowCmd(nil, nil)
	fmt.Printf("Interface information\n\n")
	handleIfDetailShowCmd(nil, nil)
	fmt.Printf("Security profile information\n\n")
	handleNwsecProfShowCmd(nil, nil)
	//Move this to stream and enable
	//fmt.Printf("Security policy information\n\n")
	//handleNwsecPolicyDetailShowCmd(nil, nil)
	fmt.Printf("VRF information\n\n")
	handleVrfDetailShowCmd(nil, nil)
	fmt.Printf("L2 segment information\n\n")
	handlel2segDetailShowCmd(nil, nil)
	fmt.Printf("Multicast Information\n\n")
	handleMulticastDetailShowCmd(nil, nil)
	fmt.Printf("Network information\n\n")
	handleNwDetailShowCmd(nil, nil)
	fmt.Printf("Endpoint information\n\n")
	handleEpDetailShowCmd(nil, nil)
	fmt.Printf("Port information\n\n")
	handlePortDetailShowCmd(nil, nil)
	//fmt.Printf("Session information\n\n")
	//handleSessionDetailShowCmd(nil, nil)
	fmt.Printf("Mirror Session Information\n\n")
	mirrorShowCmdHandler(nil, nil)
	fmt.Printf("Collector Information\n\n")
	collectorShowCmdHandler(nil, nil)
	fmt.Printf("Flow-Monitor Rule Information\n\n")
	flowMonitorShowCmdHandler(nil, nil)
	fmt.Printf("ACL information\n\n")
	handleACLShowCmd(nil, nil)
	fmt.Printf("QoS information\n\n")
	handleQosShowCmd(nil, nil)
	fmt.Printf("Copp information\n\n")
	handleCoppShowCmd(nil, nil)
	fmt.Printf("Memory Mtrack information\n\n")
	mtrackShowCmdHandler(nil, nil)
	fmt.Printf("Memory slab information\n\n")
	slabDetailShowCmdHandler(nil, nil)
	fmt.Printf("Thread information\n\n")
	threadDetailShow(nil)
	fmt.Printf("System information\n\n")
	handleSystemDetailShowCmd(nil, nil)
	fmt.Printf("System Qos Credits\n\n")
	systemQueueCreditsShowCmdHandler(nil, nil)
	fmt.Printf("System Statistics Pb detail\n\n")
	systemPbDetailStatsShowCmdHandler(nil, nil)
	fmt.Printf("System Statistics Qos Class Queues\n\n")
	qosQueuesCmdHandler(nil, nil)
	fmt.Printf("System Statistics Qos Class Thresholds\n\n")
	qosThresholdsCmdHandler(nil, nil)
	fmt.Printf("Table Information\n\n")
	tableInfoShowCmdHandler(nil, nil)
	fmt.Printf("\nTable Dump logs\n\n")
	tableDumpShowCmdHandler(nil, nil)
	fmt.Printf("\nInterrupt Dump\n\n")
	interruptShowCmdHandler(nil, nil)
	fmt.Printf("\nSystem Drop Stats\n\n")
	systemDropStatsShowCmdHandler(nil, nil)

	fmt.Printf("Flush HAL logs\n\n")
	flushLogsDebugCmdHandler(nil, nil)
}
