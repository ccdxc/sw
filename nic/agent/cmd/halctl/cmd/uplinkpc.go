package cmd

import (
	"context"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	uplinkPcID uint64
)

var uplinkPcShowCmd = &cobra.Command{
	Use:   "uplink-pc",
	Short: "uplink-pc",
	Long:  "shows uplink-pc",
	Run:   uplinkPcShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(uplinkPcShowCmd)

	uplinkPcShowCmd.Flags().Uint64Var(&uplinkPcID, "id", 1, "Specify if-id")
}

func uplinkPcShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	var req *halproto.InterfaceGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific if
		req = &halproto.InterfaceGetRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: uplinkPcID,
				},
			},
		}
	} else {
		// Get all ifs
		req = &halproto.InterfaceGetRequest{}
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	// HAL call
	respMsg, err := client.InterfaceGet(context.Background(), ifGetReqMsg)
	if err != nil {
		log.Errorf("Getting if failed. %v", err)
	}

	// Print Header
	uplinkPcShowHeader(cmd, args)

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		uplinkPcShowOneResp(resp)
	}
	c.Close()
}

func uplinkPcShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "IfType", "NatL2seg", "NL2Segs", "LportId", "HwLifId", "UpIdx", "NMbrs", "Mbrs")
	fmt.Println(hdrLine)
}

func uplinkPcShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK_PC {
		return
	}
	mbrs := resp.GetSpec().GetIfUplinkPcInfo().GetMemberIfKeyHandle()
	fmt.Printf("%-10d%-10d%-10s%-10d%-10d%-10d%-10d%-10d%-10d",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		resp.GetStatus().GetIfHandle(),
		ifTypeToStr(ifType),
		resp.GetSpec().GetIfUplinkPcInfo().GetNativeL2SegmentId(),
		resp.GetStatus().GetUplinkInfo().GetNumL2Segs(),
		resp.GetStatus().GetUplinkInfo().GetUplinkLportId(),
		resp.GetStatus().GetUplinkInfo().GetHwLifId(),
		resp.GetStatus().GetUplinkInfo().GetUplinkIdx(),
		len(mbrs))

	for _, mbr := range mbrs {
		fmt.Printf("%-5d", mbr.GetInterfaceId())
	}
	fmt.Printf("\n")
}
