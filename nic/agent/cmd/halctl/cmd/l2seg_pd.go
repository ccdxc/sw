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
	pdL2segID uint64
	pdL2segBr bool
)

var l2segPdShowCmd = &cobra.Command{
	Use:   "pd",
	Short: "pd",
	Long:  "shows l2seg pd",
	Run:   l2segPdShowCmdHandler,
}

func init() {
	l2segShowCmd.AddCommand(l2segPdShowCmd)

	l2segPdShowCmd.Flags().Uint64Var(&pdL2segID, "id", 1, "Specify l2seg id")
	l2segPdShowCmd.Flags().BoolVar(&pdL2segBr, "brief", false, "Display briefly")
}

func l2segPdShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewL2SegmentClient(c.ClientConn)

	var req *halproto.L2SegmentGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.L2SegmentGetRequest{
			KeyOrHandle: &halproto.L2SegmentKeyHandle{
				KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
					SegmentId: l2segID,
				},
			},
		}
	} else {
		// Get all L2segs
		req = &halproto.L2SegmentGetRequest{}
	}
	l2segGetReqMsg := &halproto.L2SegmentGetRequestMsg{
		Request: []*halproto.L2SegmentGetRequest{req},
	}

	// HAL call
	respMsg, err := client.L2SegmentGet(context.Background(), l2segGetReqMsg)
	if err != nil {
		log.Errorf("Getting L2Seg failed. %v", err)
	}

	// Print Header
	l2segPdShowHeader(cmd, args)

	// Print VRFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		l2segPdShowOneResp(resp)
	}
	c.Close()
}

func l2segPdShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-12s%-12s%-12s\n",
		"Id", "Handle", "vrfId", "HwId", "LookupId", "CPUVlan", "InpPropCPU", "InpProp.1q", "InpPropPr")
	fmt.Println(hdrLine)
}

func l2segPdShowOneResp(resp *halproto.L2SegmentGetResponse) {
	if resp.GetStatus().GetEpdInfo() != nil {
		l2segEPdShowOneResp(resp)
	} else {
		fmt.Printf("No PD")
	}
}

func l2segEPdShowOneResp(resp *halproto.L2SegmentGetResponse) {
	epdStatus := resp.GetStatus().GetEpdInfo()
	inpPropIdxStr := ""
	inpPropIdxPrTagStr := ""

	/*
		inpPropIdx := epdStatus.GetInpPropIdx()
		inpPropIdxPrTag := epdStatus.GetInpPropIdxPrTag()
		first := true

		for idx := range inpPropIdx {
			if first == true {
				first = false
				inpPropIdxStr += fmt.Sprintf("%d", idx)
			} else {
				inpPropIdxStr += fmt.Sprintf(", %d", idx)
			}
		}

		first = true
		for idx := range inpPropIdxPrTag {
			if first == true {
				first = false
				inpPropIdxPrTagStr += fmt.Sprintf("%d", idx)
			} else {
				inpPropIdxPrTagStr += fmt.Sprintf(", %d", idx)
			}
		}
	*/

	fmt.Printf("%-10d%-10d%-10d%-10d%-10d%-10d%-12d%-12s%-12s\n",
		resp.GetSpec().GetKeyOrHandle().GetSegmentId(),
		resp.GetStatus().GetL2SegmentHandle(),
		resp.GetSpec().GetVrfKeyHandle().GetVrfId(),
		epdStatus.GetHwL2SegId(),
		epdStatus.GetL2SegLookupId(),
		epdStatus.GetL2SegVlanIdCpu(),
		epdStatus.GetInpPropCpuIdx(),
		inpPropIdxStr,
		inpPropIdxPrTagStr)
}
