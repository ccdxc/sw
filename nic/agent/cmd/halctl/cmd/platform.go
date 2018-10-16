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

var platHbmShowCmd = &cobra.Command{
	Use:   "hbm",
	Short: "show hbm information",
	Long:  "show hbm information",
}

var platHbmBwShowCmd = &cobra.Command{
	Use:   "bw",
	Short: "show hbm bandwidth information",
	Long:  "show hbm bandwidth information",
	Run:   platHbmBwShowCmdHandler,
}

func init() {
	showCmd.AddCommand(platShowCmd)
	platShowCmd.AddCommand(platHbmShowCmd)
	platHbmShowCmd.AddCommand(platHbmBwShowCmd)

	platHbmBwShowCmd.Flags().Uint32Var(&numSamples, "num-samples", 1, "Specify number of samples")
	platHbmBwShowCmd.Flags().Uint32Var(&sleepInterval, "sleep-interval", 1, "Specify sleep interval in ns")
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
