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
)

var platShowCmd = &cobra.Command{
	Use:   "platform",
	Short: "show platform information",
	Long:  "show platform information",
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
	platHbmShowCmd.AddCommand(platHbmBwShowCmd)
	platHbmShowCmd.AddCommand(hbmLlcStatsShowCmd)

	platHbmBwShowCmd.Flags().Uint32Var(&numSamples, "num-samples", 1, "Specify number of samples")
	platHbmBwShowCmd.Flags().Uint32Var(&sleepInterval, "sleep-interval", 1, "Specify sleep interval in ns")
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
	hbmBwShowHeader()

	// Print LIFs
	for _, resp := range respMsg.Response {
		hbmBwShowOneResp(resp)
	}
}

func hbmBwShowHeader() {
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 110)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-20s%-20s%-20s%-20s%-20s\n",
		"BlockType", "AvgReadBW", "AvgWriteBW", "MaxReadBW", "MaxWriteBW", "ClockDiff")
	fmt.Println(hdrLine)
}

func hbmBwShowOneResp(resp *halproto.HbmBwGetResponse) {
	var avgRead uint64
	var avgWrite uint64
	var maxRead uint64
	var maxWrite uint64

	blockType := strings.Replace(resp.GetType().String(), "CAPRI_BLOCK_", "", -1)
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

	fmt.Printf("%-10s%-20d%-20d%-20d%-20d%-20d\n",
		blockType, avgRead, avgWrite,
		maxRead, maxWrite, resp.GetClkDiff())
}
