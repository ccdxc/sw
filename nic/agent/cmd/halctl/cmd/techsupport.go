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
	fmt.Printf("\nSystem mode\n\n")
	systemModeShowCmdHandler(nil, nil)
	fmt.Printf("\nSystem Inband Info\n\n")
	systemInbMgmtShowCmdHandler(nil, nil)
	fmt.Printf("\nSystem Feature Profile\n\n")
	systemFeatProfileShowCmdHandler(nil, nil)
	fmt.Printf("\nLif information\n\n")
	handlelifDetailShowCmd(nil, nil)
	fmt.Printf("\nInterface information\n\n")
	handleIfDetailShowCmd(nil, nil)
	fmt.Printf("\nInterface brief\n\n")
	ifShowCmdHandler(nil, nil)
	fmt.Printf("\nSecurity profile information\n\n")
	handleNwsecProfShowCmd(nil, nil)
	//Move this to stream and enable
	//fmt.Printf("Security policy information\n\n")
	//handleNwsecPolicyDetailShowCmd(nil, nil)
	fmt.Printf("\nVRF information\n\n")
	handleVrfDetailShowCmd(nil, nil)
	fmt.Printf("\nL2 segment information\n\n")
	handlel2segDetailShowCmd(nil, nil)
	fmt.Printf("\nMulticast Information\n\n")
	handleMulticastDetailShowCmd(nil, nil)
	fmt.Printf("\nNetwork information\n\n")
	handleNwDetailShowCmd(nil, nil)
	fmt.Printf("\nEndpoint information\n\n")
	handleEpDetailShowCmd(nil, nil)
	fmt.Printf("\nEndpoint summary\n\n")
	epShowCmdHandler(nil, nil)
	fmt.Printf("\nPort information\n\n")
	handlePortDetailShowCmd(nil, nil)
	fmt.Printf("Session information\n\n")
	sessionShowCmdHandler(nil, nil)
	fmt.Printf("\nMirror Session Information\n\n")
	mirrorShowCmdHandler(nil, nil)
	fmt.Printf("\nCollector Information\n\n")
	collectorShowCmdHandler(nil, nil)
	fmt.Printf("\nFlow-Monitor Rule Information\n\n")
	flowMonitorShowCmdHandler(nil, nil)
	fmt.Printf("\nACL information\n\n")
	handleACLShowCmd(nil, nil)
	fmt.Printf("\nQoS information\n\n")
	handleQosShowCmd(nil, nil)
	fmt.Printf("\nCopp information\n\n")
	handleCoppShowCmd(nil, nil)
	fmt.Printf("\nMemory Mtrack information\n\n")
	mtrackShowCmdHandler(nil, nil)
	fmt.Printf("\nMemory slab information\n\n")
	slabDetailShowCmdHandler(nil, nil)
	fmt.Printf("\nThread information\n\n")
	threadDetailShow(nil)
	fmt.Printf("\nSystem information\n\n")
	handleSystemDetailShowCmd(nil, nil)
	fmt.Printf("\nSystem Qos Credits\n\n")
	systemQueueCreditsShowCmdHandler(nil, nil)
	fmt.Printf("\nSystem Statistics Pb detail\n\n")
	systemPbDetailStatsShowCmdHandler(nil, nil)
	fmt.Printf("\nSystem Statistics Qos Class Queues\n\n")
	qosQueuesCmdHandler(nil, nil)
	fmt.Printf("\nSystem Statistics Qos Class Thresholds\n\n")
	qosThresholdsCmdHandler(nil, nil)
	fmt.Printf("\nTable Information\n\n")
	tableInfoShowCmdHandler(nil, nil)
	fmt.Printf("\nTable Dump logs\n\n")
	tableDumpShowCmdHandler(nil, nil)
	fmt.Printf("\nInterrupt Dump\n\n")
	interruptShowCmdHandler(nil, nil)
	fmt.Printf("\nSystem Drop Stats\n\n")
	systemDropStatsShowCmdHandler(nil, nil)
	fmt.Printf("\nvMotion Dump\n\n")
	vmotionDebugCmdHandler(nil, nil)

	fmt.Printf("Flush HAL logs\n\n")
	flushLogsDebugCmdHandler(nil, nil)
}
