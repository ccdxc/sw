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
	numSamples    uint32
	sleepInterval uint32
	enableMsPcie  bool
	sLifID        uint64
	qType         string
	qNum          uint32
	qRange        string
	pollCount     uint32
)

var platShowCmd = &cobra.Command{
	Use:   "platform",
	Short: "show platform information",
	Long:  "show platform information",
}

var schedulerShowCmd = &cobra.Command{
	Use:   "scheduler",
	Short: "show scheduler information",
	Long:  "show scheduler information",
}

var schedulerStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show scheduler statistics",
	Long:  "show scheduler statistics",
	Run:   schedulerStatsShowCmdHandler,
}

var schedulerActiveQShowCmd = &cobra.Command{
	Use:   "active-queues",
	Short: "show scheduler active-queues",
	Long:  "show scheduler active-queues",
	Run:   schedulerActiveQShowCmdHandler,
}

var hbmLlcStatsShowCmd = &cobra.Command{
	Use:   "llc-stats",
	Short: "show hbm llc-stats",
	Long:  "show hbm llc-stats",
	Run:   llcStatsShowCmdHandler,
}

var platHbmShowCmd = &cobra.Command{
	Use:   "hbm",
	Short: "show hbm information",
	Long:  "show hbm information",
}

var platHbmBwShowCmd = &cobra.Command{
	Use:   "bandwidth",
	Short: "show hbm bandwidth information",
	Long:  "show hbm bandwidth information",
	Run:   platHbmBwShowCmdHandler,
}

func init() {
	showCmd.AddCommand(platShowCmd)
	platShowCmd.AddCommand(platHbmShowCmd)
	platShowCmd.AddCommand(schedulerShowCmd)
	schedulerShowCmd.AddCommand(schedulerStatsShowCmd)
	schedulerShowCmd.AddCommand(schedulerActiveQShowCmd)
	platHbmShowCmd.AddCommand(platHbmBwShowCmd)
	platHbmShowCmd.AddCommand(hbmLlcStatsShowCmd)

	platHbmBwShowCmd.Flags().Uint32Var(&numSamples, "num-samples", 1, "Specify number of samples")
	platHbmBwShowCmd.Flags().Uint32Var(&sleepInterval, "sleep-interval", 1, "Specify sleep interval in ns")
	platHbmBwShowCmd.Flags().BoolVar(&enableMsPcie, "enable-ms-pcie", false, "Dump MS and PCIE BW")
	schedulerActiveQShowCmd.Flags().Uint64Var(&sLifID, "lif-id", 0, "Specify Lif ID")
	schedulerActiveQShowCmd.Flags().StringVar(&qType, "queue-type", "none", "Specify queue type. Should be one of (none, admin, tx, rx, rdma-send, rdma-redv, cq, eq, nvme, storage, virtio-rx, virtio-tx)")
	schedulerActiveQShowCmd.Flags().Uint32Var(&qNum, "queue-num", 0, "Specify queue number")
	schedulerActiveQShowCmd.Flags().StringVar(&qRange, "queue-range", "", "Specify queue-range (inclusive) in comma separated list Ex: \"--queue-range=10-100,150-200\"")
	schedulerActiveQShowCmd.Flags().Uint32Var(&pollCount, "poll-count", 1, "Specify polling count")
	schedulerActiveQShowCmd.MarkFlagRequired("lif-id")
	schedulerActiveQShowCmd.MarkFlagRequired("queue-type")
}

func schedulerActiveQShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *halproto.SchedulerActiveQRequest

	if isQTypeInvalid(qType) {
		fmt.Printf("Invalid queue type\n")
		return
	}

	qTypeEnum := qTypeToEnum(qType)

	if cmd.Flags().Changed("queue-range") {
		var qStart []uint32
		var qEnd []uint32
		var start uint32
		var end uint32

		n, _ := fmt.Sscanf(qRange, "%d-%d%s", start, end, qRange)
		for n >= 2 {
			qStart = append(qStart, start)
			qEnd = append(qEnd, end)
			n, err = fmt.Sscanf(qRange, ",%d-%d%s", start, end, qRange)
		}

		if len(qStart) == 1 {
			req = &halproto.SchedulerActiveQRequest{
				LifId: sLifID,
				QType: qTypeEnum,
				NumOrRanges: &halproto.SchedulerActiveQRequest_QRanges{
					QRanges: &halproto.QRanges{
						Range: []*halproto.QRange{
							{QBeg: qStart[0], QEnd: qEnd[0]},
						},
					},
				},
				PollCount: pollCount,
			}
		} else if len(qStart) == 2 {
			req = &halproto.SchedulerActiveQRequest{
				LifId: sLifID,
				QType: qTypeEnum,
				NumOrRanges: &halproto.SchedulerActiveQRequest_QRanges{
					QRanges: &halproto.QRanges{
						Range: []*halproto.QRange{
							{QBeg: qStart[0], QEnd: qEnd[0]},
							{QBeg: qStart[1], QEnd: qEnd[1]},
						},
					},
				},
				PollCount: pollCount,
			}
		} else if len(qStart) == 3 {
			req = &halproto.SchedulerActiveQRequest{
				LifId: sLifID,
				QType: qTypeEnum,
				NumOrRanges: &halproto.SchedulerActiveQRequest_QRanges{
					QRanges: &halproto.QRanges{
						Range: []*halproto.QRange{
							{QBeg: qStart[0], QEnd: qEnd[0]},
							{QBeg: qStart[1], QEnd: qEnd[1]},
							{QBeg: qStart[2], QEnd: qEnd[2]},
						},
					},
				},
				PollCount: pollCount,
			}
		} else {
			fmt.Printf("Too many queue ranges. Max of 3 supported\n")
			return
		}
	} else if cmd.Flags().Changed("queue-num") {
		req = &halproto.SchedulerActiveQRequest{
			LifId: sLifID,
			QType: qTypeEnum,
			NumOrRanges: &halproto.SchedulerActiveQRequest_QNum{
				QNum: qNum,
			},
			PollCount: pollCount,
		}
	} else {
		fmt.Printf("Either queue-num or queue-range needs to be provided\n")
		return
	}

	fmt.Printf("%+v\n", req)

	reqMsg := &halproto.SchedulerActiveQRequestMsg{
		Request: []*halproto.SchedulerActiveQRequest{req},
	}

	// HAL Call
	respMsg, err := client.SchedulerActiveQGet(context.Background(), reqMsg)
	if err != nil {
		fmt.Printf("Scheduler stats get failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		schedulerActiveQPrintOneResp(resp)
	}
}

func qTypeToEnum(str string) halproto.LifQPurpose {
	switch str {
	case "none":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_NONE
	case "admin":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_ADMIN
	case "tx":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_TX
	case "rx":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_RX
	case "rdma-send":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_RDMA_SEND
	case "rdma-recv":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_RDMA_RECV
	case "cq":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_CQ
	case "eq":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_EQ
	case "nvme":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_NVME
	case "storage":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_STORAGE
	case "virtio-tx":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_VIRTIO_TX
	case "virtio-rx":
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_VIRTIO_RX
	default:
		return halproto.LifQPurpose_LIF_QUEUE_PURPOSE_NONE
	}
}

func isQTypeInvalid(str string) bool {
	switch str {
	case "none":
		return false
	case "admin":
		return false
	case "tx":
		return false
	case "rx":
		return false
	case "rdma-send":
		return false
	case "rdma-recv":
		return false
	case "cq":
		return false
	case "eq":
		return false
	case "nvme":
		return false
	case "storage":
		return false
	case "virtio-tx":
		return false
	case "virtio-rx":
		return false
	default:
		return true
	}
}

func schedulerActiveQPrintOneResp(resp *halproto.SchedulerActiveQResponse) {
	activeQ := resp.GetActiveQNum()

	for _, q := range activeQ {
		fmt.Printf("Queue %d is active\n", q)
	}
}

func schedulerStatsShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var empty *halproto.Empty

	// HAL Call
	respMsg, err := client.SchedulerStatsGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Scheduler stats get failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		schedulerStatsPrintOneResp(resp)
	}
}

func schedulerStatsPrintOneResp(resp *halproto.SchedulerStatsResponse) {
	fmt.Printf("Doorbell Set Count:     %d\n", resp.GetDoorbellSetCount())
	fmt.Printf("Doorbell Cleared Count: %d\n", resp.GetDoorbellClearCount())
	fmt.Printf("Rate-Limit Start Count: %d\n", resp.GetRatelimitStartCount())
	fmt.Printf("Rate-Limit Stop Count:  %d\n\n", resp.GetRatelimitStopCount())

	cosEntries := resp.GetCosEntry()

	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-14s%-10s\n", "Cos", "Doorbell Count", "Xon Status")
	fmt.Println(hdrLine)

	for _, entry := range cosEntries {
		fmt.Printf("Cos%-3d%-14d%-10t\n", entry.GetCos(), entry.GetDoorbellCount(), entry.GetXonStatus())
	}
}

func llcStatsShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.LlcGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Llc get failed. %v\n", err)
		return
	}

	llcGetPrintHeader()

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
		}
		llcGetPrintOneResp(resp)
	}
}

func llcGetPrintHeader() {
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 40)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-20s%-10s\n", "Channel", "Type", "Count")
	fmt.Println(hdrLine)
}

func llcGetPrintOneResp(resp *halproto.LlcGetResponse) {
	stats := resp.GetCount()
	str := strings.ToLower(strings.Replace(resp.GetType().String(), "LLC_COUNTER_", "", -1))
	str = strings.Replace(str, "_", "-", -1)
	for i := 0; i < 16; i++ {
		fmt.Printf("%-10d%-20s%-10d\n",
			i, str, stats[i])
	}
}

func platHbmBwShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewDebugClient(c.ClientConn)

	// HAL call
	req := &halproto.HbmBwGetRequest{
		NumSamples:    numSamples,
		SleepInterval: sleepInterval,
	}

	reqMsg := &halproto.HbmBwGetRequestMsg{
		Request: []*halproto.HbmBwGetRequest{req},
	}

	respMsg, err := client.HbmBwGet(context.Background(), reqMsg)
	if err != nil {
		fmt.Printf("Getting Platform HBM BW failed. %v\n", err)
		return
	}

	// Print Header
	hbmBwShowHeader(enableMsPcie)

	i := 0
	for _, resp := range respMsg.Response {
		hbmBwShowOneResp(resp, enableMsPcie)
		i++
		if i%5 == 0 {
			fmt.Printf("%-10d\n", resp.GetClkDiff())
		}
	}
}

func hbmBwShowHeader(enableMsPcie bool) {
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 140)
	fmt.Println(hdrLine)
	if enableMsPcie == true {
		fmt.Printf(
			"%-11s%-11s%-11s%-11s"+
				"%-11s%-11s%-11s%-11s"+
				"%-11s%-11s%-11s%-11s"+
				"%-11s%-11s%-11s%-11s"+
				"%-11s%-11s%-11s%-11s"+
				"%-11s\n",
			"TXD AvgRd", "TXD AvgWr", "TXD MaxRd", "TXD MaxWr",
			"RXD AvgRd", "RXD AvgWr", "RXD MaxRd", "RXD MaxWr",
			"PB AvgRd", "PB AvgWr", "PB MaxRd", "PB MaxWr",
			"MS AvgRd", "MS AvgWr", "MS MaxRd", "MS MaxWr",
			"PCIE AvgRd", "PCIE AvgWr", "PCIE MaxRd", "PCIE MaxWr",
			"ClockDiff")
	} else {
		fmt.Printf(
			"%-11s%-11s%-11s%-11s"+
				"%-11s%-11s%-11s%-11s"+
				"%-11s%-11s%-11s%-11s"+
				"%-11s\n",
			"TXD AvgRd", "TXD AvgWr", "TXD MaxRd", "TXD MaxWr",
			"RXD AvgRd", "RXD AvgWr", "RXD MaxRd", "RXD MaxWr",
			"PB AvgRd", "PB AvgWr", "PB MaxRd", "PB MaxWr",
			"ClockDiff")
	}

	fmt.Println(hdrLine)
}

func hbmBwShowOneResp(resp *halproto.HbmBwGetResponse, enableMsPcie bool) {
	var avgRead float64
	var avgWrite float64
	var maxRead float64
	var maxWrite float64

	blockType := resp.GetType()

	if enableMsPcie == false {
		if blockType == halproto.CapriBlock_CAPRI_BLOCK_MS ||
			blockType == halproto.CapriBlock_CAPRI_BLOCK_PCIE {
			return
		}
	}

	avgRead = 0
	avgWrite = 0
	maxRead = 0
	maxWrite = 0

	avg := resp.GetAverage()
	if avg != nil {
		avgRead = avg.GetRead()
		avgWrite = avg.GetWrite()
	}

	max := resp.GetMaximum()
	if max != nil {
		maxRead = max.GetRead()
		maxWrite = max.GetWrite()
	}

	fmt.Printf("%-11f%-11f%-11f%-11f",
		avgRead, avgWrite, maxRead, maxWrite)
}
