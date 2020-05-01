//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"sort"
	"strings"

	"google.golang.org/grpc"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var ()

var systemBwStatsShowCmd = &cobra.Command{
	Use:   "bw",
	Short: "show system statistics bw",
	Long:  "show system statistics bw",
	Run:   systemBwStatsShowCmdHandler,
}

func init() {
	systemStatsShowCmd.AddCommand(systemBwStatsShowCmd)
}

func systemBwStatsShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	// Print Header
	bwShowHeader()

	// Uplink's BW
	bwUplinkShow(c)

	// LIF's BW
	bwLifShow(c)
}

func bwShowHeader() {
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 65)
	fmt.Printf("Bandwidth Statistics:\n")
	fmt.Printf("PPS: Packets per second     BPS: Bytes per second\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-15s%-7s%-17s | %-7s%-17s\n",
		"", "", "TX", "", "RX")
	fmt.Printf("%-15s%-12s%-12s | %-12s%-12s\n",
		"IF", "PPS", "BPS", "PPS", "BPS")
	fmt.Println(hdrLine)
}

func bwUplinkShow(c *grpc.ClientConn) {
	client := halproto.NewInterfaceClient(c)
	var req *halproto.InterfaceGetRequest
	req = &halproto.InterfaceGetRequest{}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	// HAL call
	respMsg, err := client.InterfaceGet(context.Background(), ifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting if failed. %v\n", err)
		return
	}

	m := make(map[uint64]*halproto.InterfaceGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		if resp.GetSpec().GetType() != halproto.IfType_IF_TYPE_UPLINK {
			continue
		}
		m[resp.GetSpec().GetKeyOrHandle().GetInterfaceId()] = resp
	}
	var keys []uint64
	for k := range m {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	for _, k := range keys {
		ifBwOneResp(m[k])
	}
}

func ifBwOneResp(resp *halproto.InterfaceGetResponse) {
	var txPps uint64
	var txBps uint64
	var rxPps uint64
	var rxBps uint64

	intfStr := ifRespToStr(resp)
	txPps = resp.GetStatus().GetUplinkInfo().GetTxPps()
	txBps = resp.GetStatus().GetUplinkInfo().GetTxBytesps()
	rxPps = resp.GetStatus().GetUplinkInfo().GetRxPps()
	rxBps = resp.GetStatus().GetUplinkInfo().GetRxBytesps()
	fmt.Printf("%-15s%-12d%-12d | %-12d%-12d\n",
		intfStr, txPps, txBps, rxPps, rxBps)
}

func bwLifShow(c *grpc.ClientConn) {
	client := halproto.NewInterfaceClient(c)
	var req *halproto.LifGetRequest
	req = &halproto.LifGetRequest{}
	lifGetReqMsg := &halproto.LifGetRequestMsg{
		Request: []*halproto.LifGetRequest{req},
	}

	// HAL call
	respMsg, err := client.LifGet(context.Background(), lifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Lif failed. %v\n", err)
		return
	}
	m := make(map[uint64]*halproto.LifGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		if (resp.GetSpec().GetType() == halproto.LifType_LIF_TYPE_NONE) ||
			(resp.GetSpec().GetType() == halproto.LifType_LIF_TYPE_SWM) {
			continue
		}
		m[resp.GetSpec().GetKeyOrHandle().GetLifId()] = resp
	}
	var keys []uint64
	for k := range m {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	for _, k := range keys {
		lifBwOneResp(m[k])
	}
}

func lifBwOneResp(resp *halproto.LifGetResponse) {
	var txPps uint64
	var txBps uint64
	var rxPps uint64
	var rxBps uint64

	intfStr := resp.GetSpec().GetName()
	stats := resp.GetStats()
	rxStats := stats.GetDataLifStats().GetRxStats()
	txStats := stats.GetDataLifStats().GetTxStats()

	txPps = txStats.GetPps()
	txBps = txStats.GetBytesps()
	rxPps = rxStats.GetPps()
	rxBps = rxStats.GetBytesps()
	fmt.Printf("%-15s%-12d%-12d | %-12d%-12d\n",
		intfStr, txPps, txBps, rxPps, rxBps)
}
