//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"sort"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	lifID       uint64
	lifSpecID   uint64
	lifStatusID uint64
)

var lifShowCmd = &cobra.Command{
	Use:   "lif",
	Short: "show logical interface (lif) information",
	Long:  "shows logical interface (lif) object information",
	Run:   lifShowCmdHandler,
}

var lifShowSpecCmd = &cobra.Command{
	Use:   "spec",
	Short: "show logical interface (lif) spec information",
	Long:  "shows logical interface (lif) object spec information",
	Run:   lifShowSpecCmdHandler,
}

var lifShowStatusCmd = &cobra.Command{
	Use:   "status",
	Short: "show logical interface (lif) status information",
	Long:  "shows logical interface (lif) object status information",
	Run:   lifShowStatusCmdHandler,
}

var lifClearStatsCmd = &cobra.Command{
	Use:   "lif-stats",
	Short: "clear lif stats",
	Long:  "clear lif stats",
	Run:   lifClearStatsCmdHandler,
}

func init() {
	showCmd.AddCommand(lifShowCmd)
	lifShowCmd.AddCommand(lifShowSpecCmd)
	lifShowCmd.AddCommand(lifShowStatusCmd)
	clearCmd.AddCommand(lifClearStatsCmd)

	lifShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	lifShowCmd.Flags().Uint64Var(&lifID, "id", 1, "Specify lif-id")
	lifShowSpecCmd.Flags().Uint64Var(&lifSpecID, "id", 1, "Specify lif-id")
	lifShowStatusCmd.Flags().Uint64Var(&lifStatusID, "id", 1, "Specify lif-id")
	lifClearStatsCmd.Flags().Uint64Var(&lifID, "id", 0, "Specify lif-id")
}

func handleLifShowCmd(cmd *cobra.Command, id uint64, spec bool, status bool) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c)

	defer c.Close()

	var req *halproto.LifGetRequest
	if id != 0 {
		// Get specific lif
		req = &halproto.LifGetRequest{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: id,
				},
			},
		}
	} else {
		// Get all Lifs
		req = &halproto.LifGetRequest{}
	}

	lifGetReqMsg := &halproto.LifGetRequestMsg{
		Request: []*halproto.LifGetRequest{req},
	}

	// HAL call
	respMsg, err := client.LifGet(context.Background(), lifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Lif failed. %v\n", err)
		return
	}

	// Print Header
	if spec == true {
		lifShowSpecHeader()
	} else if status == true {
		lifShowStatusHeader()
	}

	// Print LIFs
	m := make(map[uint64]*halproto.LifGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
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
		if spec == true {
			lifShowSpecOneResp(m[k])
		} else if status == true {
			lifShowStatusOneResp(m[k])
		}
	}
}

func lifShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		lifDetailShowCmdHandler(cmd, args)
		return
	}

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("id") {
		handleLifShowCmd(cmd, lifID, true, false)
	} else {
		handleLifShowCmd(cmd, 0, true, false)
	}
}

func lifShowSpecCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("id") {
		handleLifShowCmd(cmd, lifSpecID, true, false)
	} else {
		handleLifShowCmd(cmd, 0, true, false)
	}
}

func lifShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("id") {
		handleLifShowCmd(cmd, lifStatusID, false, true)
	} else {
		handleLifShowCmd(cmd, 0, false, true)
	}
}

func handlelifDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c)

	defer c.Close()

	var req *halproto.LifGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific lif
		req = &halproto.LifGetRequest{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: lifID,
				},
			},
		}
	} else {
		// Get all Lifs
		req = &halproto.LifGetRequest{}
	}
	lifGetReqMsg := &halproto.LifGetRequestMsg{
		Request: []*halproto.LifGetRequest{req},
	}

	// HAL call
	respMsg, err := client.LifGet(context.Background(), lifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Lif failed. %v\n", err)
		return
	}

	// Print LIFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func lifDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handlelifDetailShowCmd(cmd, nil)
}

func lifShowSpecHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:          Lif Id                   Name:        Lif Name\n")
	fmt.Printf("Type:        Lif Type                 PktFilter:   Packet Filters (AM, BC, PR)\n")
	fmt.Printf("PUplink:     Pinned Uplink IF Id      \n")
	fmt.Printf("Flags:       VS: Vlan Strip, VI: Vlan Insert, RE: RDMA Enable, RS: RDMA Sniffer\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 110)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-20s%-25s%-10s%-15s%-15s\n",
		"Id", "Name", "Type", "PktFilter", "PUplink", "Flags")
	fmt.Println(hdrLine)
}

func lifShowSpecOneResp(resp *halproto.LifGetResponse) {
	ifIDStr := utils.IfIndexToStr(uint32(resp.GetSpec().GetPinnedUplinkIfKeyHandle().GetInterfaceId()))
	typeStr := strings.Replace(resp.GetSpec().GetType().String(), "LIF_TYPE_", "", -1)
	typeStr = strings.ToLower(strings.Replace(typeStr, "_", "-", -1))
	fmt.Printf("%-10d%-20s%-25s%-10s%-15s%-15s\n",
		resp.GetSpec().GetKeyOrHandle().GetLifId(),
		strings.ToLower(resp.GetSpec().GetName()),
		typeStr,
		pktfltrToStr(resp.GetSpec().GetPacketFilter()),
		ifIDStr,
		lifFlagsToStr(resp))
}

func lifShowStatusHeader() {
	fmt.Printf("\n")
	fmt.Printf("Handle:  Lif Handle         HW ID:   Lif Hardware ID\n")
	fmt.Printf("Status:  Lif Status\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s\n",
		"Handle", "HW ID", "Status")
	fmt.Println(hdrLine)
}

func lifShowStatusOneResp(resp *halproto.LifGetResponse) {
	status := resp.GetStatus()
	lifStatus := status.GetLifStatus().String()
	lifStatus = strings.ToLower(strings.Replace(lifStatus, "IF_STATUS_", "", -1))

	fmt.Printf("%-10d%-10d%-10s\n",
		status.GetLifHandle(),
		status.GetHwLifId(),
		lifStatus)
}

func lifFlagsToStr(resp *halproto.LifGetResponse) string {
	var str string

	if resp.GetSpec().GetVlanStripEn() {
		str += "VS,"
	}
	if resp.GetSpec().GetVlanInsertEn() {
		str += "VI,"
	}
	if resp.GetSpec().GetEnableRdma() {
		str += "RE,"
	}
	if resp.GetSpec().GetRdmaSniffEn() {
		str += "RS,"
	}

	sz := len(str)
	if sz > 0 && str[sz-1] == ',' {
		str = str[:sz-1]
	} else {
		str += "-"
	}

	return str
}

func pktfltrToStr(fltrType *halproto.PktFilter) string {

	var str string

	if !fltrType.GetReceiveBroadcast() &&
		!fltrType.GetReceiveAllMulticast() &&
		!fltrType.GetReceivePromiscuous() {
		return "-"
	}

	if fltrType.GetReceiveBroadcast() {
		str += "BC,"
	}

	if fltrType.GetReceiveAllMulticast() {
		str += "AM,"
	}

	if fltrType.GetReceivePromiscuous() {
		str += "PR"
	}

	sz := len(str)
	if sz > 0 && str[sz-1] == ',' {
		str = str[:sz-1]
	}

	return str
}

func lifIDGetName(id uint64) string {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c)

	defer c.Close()

	var req *halproto.LifGetRequest
	// Get specific lif
	req = &halproto.LifGetRequest{
		KeyOrHandle: &halproto.LifKeyHandle{
			KeyOrHandle: &halproto.LifKeyHandle_LifId{
				LifId: id,
			},
		},
	}

	lifGetReqMsg := &halproto.LifGetRequestMsg{
		Request: []*halproto.LifGetRequest{req},
	}

	// HAL call
	respMsg, err := client.LifGet(context.Background(), lifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Lif failed. %v\n", err)
		return "Invalid"
	}

	// Traverse LIFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		return resp.GetSpec().GetName()
	}
	return "Invalid"
}

func lifClearStatsCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c)

	defer c.Close()

	var req *halproto.LifGetRequest
	if lifID != 0 {
		// Get specific lif
		req = &halproto.LifGetRequest{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: lifID,
				},
			},
		}
	} else {
		// Get all Lifs
		req = &halproto.LifGetRequest{}
	}

	lifGetReqMsg := &halproto.LifGetRequestMsg{
		Request: []*halproto.LifGetRequest{req},
	}

	// HAL call
	respMsg, err := client.LifGet(context.Background(), lifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Lif failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		resp.GetSpec().StatsReset = true
		lifReqMsg := &halproto.LifRequestMsg{
			Request: []*halproto.LifSpec{resp.GetSpec()},
		}

		updateRespMsg, err := client.LifUpdate(context.Background(), lifReqMsg)
		if err != nil {
			fmt.Printf("Update Lif failed. %v\n", err)
			continue
		}

		for _, updateResp := range updateRespMsg.Response {
			if updateResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", updateResp.ApiStatus)
				continue
			}
		}
	}
}
