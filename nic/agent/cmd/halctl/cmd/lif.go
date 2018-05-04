//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	lifID       uint64
	lifDetailID uint64
)

var lifShowCmd = &cobra.Command{
	Use:   "lif",
	Short: "lif",
	Long:  "shows lif",
	Run:   lifShowCmdHandler,
}

var lifDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "detail",
	Long:  "shows lif detail",
	Run:   lifDetailShowCmdHandler,
}

func init() {
	showCmd.AddCommand(lifShowCmd)
	lifShowCmd.AddCommand(lifDetailShowCmd)

	lifShowCmd.Flags().Uint64Var(&lifID, "id", 1, "Specify lif-id")
	lifDetailShowCmd.Flags().Uint64Var(&lifDetailID, "id", 1, "Specify lif-id")
}

func lifShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	var req *halproto.LifGetRequest
	if cmd.Flags().Changed("id") {
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
		log.Errorf("Getting Lif failed. %v", err)
	}
	fmt.Printf("Received %v of responses. err: %v\n", len(respMsg.Response), err)

	// Print Header
	lifShowHeader(cmd, args)

	// Print LIFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		lifShowOneResp(resp)
	}
	c.Close()
}

func lifDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	var req *halproto.LifGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific lif
		req = &halproto.LifGetRequest{
			KeyOrHandle: &halproto.LifKeyHandle{
				KeyOrHandle: &halproto.LifKeyHandle_LifId{
					LifId: lifDetailID,
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
		log.Errorf("Getting Lif failed. %v", err)
	}
	fmt.Printf("Received %v of responses. err: %v\n", len(respMsg.Response), err)

	// Print LIFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
	c.Close()
}
func lifShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "PktFilter", "VStrip", "VIns", "PUplink", "RdmaEn")
	fmt.Println(hdrLine)
}

func lifShowOneResp(resp *halproto.LifGetResponse) {
	fmt.Printf("%-10d%-10d%-10s%-10v%-10v%-10d%-10v\n",
		resp.GetSpec().GetKeyOrHandle().GetLifId(),
		resp.GetStatus().GetLifHandle(),
		pktfltrToStr(resp.GetSpec().GetPacketFilter()),
		resp.GetSpec().GetVlanStripEn(),
		resp.GetSpec().GetVlanInsertEn(),
		resp.GetSpec().GetPinnedUplinkIfKeyHandle().GetInterfaceId(),
		resp.GetSpec().GetEnableRdma())
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
