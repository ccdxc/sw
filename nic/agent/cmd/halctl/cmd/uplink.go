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
	uplinkID uint64
)

var uplinkShowCmd = &cobra.Command{
	Use:   "uplink",
	Short: "uplink",
	Long:  "shows uplink",
	Run:   uplinkShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(uplinkShowCmd)

	uplinkShowCmd.Flags().Uint64Var(&uplinkID, "id", 1, "Specify if-id")
}

func uplinkShowCmdHandler(cmd *cobra.Command, args []string) {

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
					InterfaceId: uplinkID,
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
	uplinkShowHeader(cmd, args)

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		uplinkShowOneResp(resp)
	}
	c.Close()
}

func uplinkShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 90)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "IfType", "UpPort", "UpNL2seg", "NL2Segs", "LportId", "HwLifId", "UpIdx")
	fmt.Println(hdrLine)
}

func uplinkShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_UPLINK {
		return
	}
	fmt.Printf("%-10d%-10d%-10s%-10d%-10d%-10d%-10d%-10d%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		resp.GetStatus().GetIfHandle(),
		ifTypeToStr(ifType),
		resp.GetSpec().GetIfUplinkInfo().GetPortNum(),
		resp.GetSpec().GetIfUplinkInfo().GetNativeL2SegmentId(),
		resp.GetStatus().GetUplinkInfo().GetNumL2Segs(),
		resp.GetStatus().GetUplinkInfo().GetUplinkLportId(),
		resp.GetStatus().GetUplinkInfo().GetHwLifId(),
		resp.GetStatus().GetUplinkInfo().GetUplinkIdx())
}
