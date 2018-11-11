//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
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

func init() {
	showCmd.AddCommand(lifShowCmd)
	lifShowCmd.AddCommand(lifShowSpecCmd)
	lifShowCmd.AddCommand(lifShowStatusCmd)

	lifShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	lifShowCmd.Flags().Uint64Var(&lifID, "id", 1, "Specify lif-id")
	lifShowSpecCmd.Flags().Uint64Var(&lifSpecID, "id", 1, "Specify lif-id")
	lifShowStatusCmd.Flags().Uint64Var(&lifStatusID, "id", 1, "Specify lif-id")
}

func handleLifShowCmd(cmd *cobra.Command, id uint64, spec bool, status bool) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

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
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		if spec == true {
			lifShowSpecOneResp(resp)
		} else if status == true {
			lifShowStatusOneResp(resp)
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
	client := halproto.NewInterfaceClient(c.ClientConn)

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
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
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
	fmt.Printf("Id:          Lif Id                   PktFilter:   Packet Filters (ALL-MC, BC, Prom)\n")
	fmt.Printf("VStrip:      Vlan Strip Enable        VIns:        Vlan Insert Enable\n")
	fmt.Printf("PUplink:     Pinned Uplink IF Id      RdmaEn:      RDMA Enable\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "PktFilter", "VStrip", "VIns", "PUplink", "RdmaEn")
	fmt.Println(hdrLine)
}

func lifShowSpecOneResp(resp *halproto.LifGetResponse) {
	fmt.Printf("%-10d%-10s%-10v%-10v%-10d%-10v\n",
		resp.GetSpec().GetKeyOrHandle().GetLifId(),
		pktfltrToStr(resp.GetSpec().GetPacketFilter()),
		resp.GetSpec().GetVlanStripEn(),
		resp.GetSpec().GetVlanInsertEn(),
		resp.GetSpec().GetPinnedUplinkIfKeyHandle().GetInterfaceId(),
		resp.GetSpec().GetEnableRdma())
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

func pktfltrToStr(fltrType *halproto.PktFilter) string {

	var str string

	if !fltrType.GetReceiveBroadcast() &&
		!fltrType.GetReceiveAllMulticast() &&
		!fltrType.GetReceivePromiscuous() {
		return "None"
	}

	if fltrType.GetReceiveBroadcast() {
		str += "BC,"
	}

	if fltrType.GetReceiveAllMulticast() {
		str += "All-MC,"
	}

	if fltrType.GetReceivePromiscuous() {
		str += "Prom"
	}

	sz := len(str)
	if sz > 0 && str[sz-1] == ',' {
		str = str[:sz-1]
	}

	return str
}
