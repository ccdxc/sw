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
	l2segID       uint64
	l2segBr       bool
	pdL2segID     uint64
	pdL2segBr     bool
	detailL2segID uint64
)

var l2segShowCmd = &cobra.Command{
	Use:   "l2seg",
	Short: "l2seg",
	Long:  "shows l2seg",
	Run:   l2segShowCmdHandler,
}

var l2segPdShowCmd = &cobra.Command{
	Use:   "pd",
	Short: "pd",
	Long:  "shows l2seg pd",
	Run:   l2segPdShowCmdHandler,
}

var l2segDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "detail",
	Long:  "shows l2seg detail",
	Run:   l2segDetailShowCmdHandler,
}

func init() {
	showCmd.AddCommand(l2segShowCmd)
	l2segShowCmd.AddCommand(l2segPdShowCmd)
	l2segShowCmd.AddCommand(l2segDetailShowCmd)

	l2segShowCmd.Flags().Uint64Var(&l2segID, "id", 1, "Specify l2seg id")
	l2segShowCmd.Flags().BoolVar(&l2segBr, "brief", false, "Display briefly")
	l2segPdShowCmd.Flags().Uint64Var(&pdL2segID, "id", 1, "Specify l2seg id")
	l2segPdShowCmd.Flags().BoolVar(&pdL2segBr, "brief", false, "Display briefly")
	l2segDetailShowCmd.Flags().Uint64Var(&detailL2segID, "id", 1, "Specify l2seg id")
}

func l2segShowCmdHandler(cmd *cobra.Command, args []string) {
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
	l2segShowHeader(cmd, args)

	// Print VRFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		l2segShowOneResp(resp)
	}
	c.Close()
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
					SegmentId: pdL2segID,
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

func l2segDetailShowCmdHandler(cmd *cobra.Command, args []string) {
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
					SegmentId: detailL2segID,
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

	// Print L2Segments
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

func l2segShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "vrfId", "weT", "weV", "teT", "teV", "MFP", "BFP", "numEPs")
	fmt.Println(hdrLine)
}

func l2segShowOneResp(resp *halproto.L2SegmentGetResponse) {
	fmt.Printf("%-10d%-10d%-10d%-10s%-10d%-10s%-10d%-10s%-10s%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetSegmentId(),
		resp.GetStatus().GetL2SegmentHandle(),
		resp.GetSpec().GetVrfKeyHandle().GetVrfId(),
		encapTypeToStr(resp.GetSpec().GetWireEncap().GetEncapType()),
		resp.GetSpec().GetWireEncap().GetEncapValue(),
		encapTypeToStr(resp.GetSpec().GetTunnelEncap().GetEncapType()),
		resp.GetSpec().GetTunnelEncap().GetEncapValue(),
		mcastFwdPolToStr(resp.GetSpec().GetMcastFwdPolicy()),
		bcastFwdPolToStr(resp.GetSpec().GetBcastFwdPolicy()),
		resp.GetStats().GetNumEndpoints())

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

func bcastFwdPolToStr(pol halproto.BroadcastFwdPolicy) string {
	switch pol {
	case halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_DROP:
		return "Drop"
	case halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD:
		return "Flood"
	default:
		return "Invalid"
	}
}

func mcastFwdPolToStr(pol halproto.MulticastFwdPolicy) string {
	switch pol {
	case halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD:
		return "Flood"
	case halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_REPLICATE:
		return "Repl"
	case halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_DROP:
		return "Drop"
	default:
		return "Invalid"
	}
}

func encapTypeToStr(encType halproto.EncapType) string {
	switch encType {
	case halproto.EncapType_ENCAP_TYPE_DOT1Q:
		return ".1q"
	case halproto.EncapType_ENCAP_TYPE_VXLAN:
		return "vxlan"
	case halproto.EncapType_ENCAP_TYPE_IP_IN_IP:
		return "IPinIP"
	case halproto.EncapType_ENCAP_TYPE_GRE:
		return "GRE"
	case halproto.EncapType_ENCAP_TYPE_IPSEC:
		return "IPSec"
	default:
		return "Invalid"
	}
}
