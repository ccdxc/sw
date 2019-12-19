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
	l2segID   uint64
	l2segBr   bool
	l2segOif  bool
	pdL2segID uint64
	pdL2segBr bool
)

var l2segShowCmd = &cobra.Command{
	Use:   "l2seg",
	Short: "show L2 segment objects",
	Long:  "show L2 segment object information",
	Run:   l2segShowSpecCmdHandler,
}

var l2segSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "show L2 segment's spec information",
	Long:  "show L2 segment's spec information",
	Run:   l2segShowSpecCmdHandler,
}

var l2segStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show L2 segment's status information",
	Long:  "show L2 segment's status information",
	Run:   l2segShowStatusCmdHandler,
}

func init() {
	showCmd.AddCommand(l2segShowCmd)
	l2segShowCmd.AddCommand(l2segSpecShowCmd)
	l2segShowCmd.AddCommand(l2segStatusShowCmd)

	l2segShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	l2segShowCmd.Flags().Uint64Var(&l2segID, "id", 1, "Specify l2seg id")
	l2segShowCmd.Flags().BoolVar(&l2segBr, "brief", false, "Display briefly")
	l2segSpecShowCmd.Flags().Uint64Var(&l2segID, "id", 1, "Specify l2seg id")
	l2segSpecShowCmd.Flags().BoolVar(&l2segBr, "brief", false, "Display briefly")
	l2segStatusShowCmd.Flags().Uint64Var(&pdL2segID, "id", 1, "Specify l2seg id")
	l2segStatusShowCmd.Flags().BoolVar(&pdL2segBr, "brief", false, "Display briefly")
	l2segStatusShowCmd.Flags().BoolVar(&l2segOif, "oif-lists", false, "Display oif lists")
}

func l2segShowSpecCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		l2segDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewL2SegmentClient(c)

	defer c.Close()

	if len(args) > 0 {
		if strings.Compare(args[0], "spec") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

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
		fmt.Printf("Getting L2Seg failed. %v\n", err)
		return
	}

	// Print Header
	l2segShowHeader(cmd, args)

	// Print l2segs
	m := make(map[uint64]*halproto.L2SegmentGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		m[resp.GetSpec().GetKeyOrHandle().GetSegmentId()] = resp
	}
	var keys []uint64
	for k := range m {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	for _, k := range keys {
		l2segShowOneResp(m[k])
	}
}

func l2segShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewL2SegmentClient(c)

	defer c.Close()

	if len(args) > 0 {
		if strings.Compare(args[0], "status") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

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
		fmt.Printf("Getting L2Seg failed. %v\n", err)
		return
	}

	if cmd.Flags().Changed("oif-lists") {
		// // Print Header
		// multicastShowOifListHeader(cmd, args)

		// // Get map of all ifs to if names
		// ifIDToStr := ifGetAllStr()

		// // Print Entries
		// for _, resp := range respMsg.Response {
		// 	if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		// 		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		// 		continue
		// 	}
		// 	multicastShowOifList(resp.GetStatus().GetBcastLst(), ifIDToStr)
		// 	multicastShowOifList(resp.GetStatus().GetMcastLst(), ifIDToStr)
		// 	multicastShowOifList(resp.GetStatus().GetPromLst(), ifIDToStr)
		// }
	} else {
		// Print Header
		l2segPdShowHeader(cmd, args)

		ifStr := ifGetAllIdxStr()

		// Print l2segs
		m := make(map[uint64]*halproto.L2SegmentGetResponse)
		for _, resp := range respMsg.Response {
			if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
				continue
			}
			m[resp.GetSpec().GetKeyOrHandle().GetSegmentId()] = resp
		}
		var keys []uint64
		for k := range m {
			keys = append(keys, k)
		}
		sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
		for _, k := range keys {
			l2segPdShowOneResp(m[k], ifStr)
		}
	}
}

func handlel2segDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewL2SegmentClient(c)

	defer c.Close()

	var req *halproto.L2SegmentGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
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
		fmt.Printf("Getting L2Seg failed. %v\n", err)
		return
	}

	// Print L2Segments
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

func l2segDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handlel2segDetailShowCmd(cmd, nil)
}

func l2segShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("Id:            L2seg's ID                            Mode:        Classsic(CL)/Host-Pin(HP)\n")
	fmt.Printf("vrfId:         L2segs's VRF ID                       WireEncap:   Wire encap type/value\n")
	fmt.Printf("B-M-P-SB-SM:   Bcast, Mcast, Prom , Shared BC, Shared MC Repl indices\n")
	fmt.Printf("#EPs:          Num. of EPs in L2seg                  IFs:         Member Interfaces\n")
	fmt.Printf("PinUplnkId:    Pinned Uplink Interface ID            Attach:      Attached L2segs\n")
	hdrLine := strings.Repeat("-", 80)
	fmt.Println(hdrLine)
	fmt.Printf("%-8s%-5s%-6s%-10s%-40s%-5s%-10s%-10s%-20s\n",
		"Id", "Mode", "vrfId", "WireEncap", "OIFLs", "#EPs", "IFs", "PinUp", "Attach.")
	fmt.Println(hdrLine)
}

func l2segShowOneResp(resp *halproto.L2SegmentGetResponse) {
	ifList := resp.GetSpec().GetIfKeyHandle()
	ifStr := ""
	weStr := ""
	// teStr := ""
	encapType := ""

	if len(ifList) > 0 {
		for i := 0; i < len(ifList); i++ {
			ifStr += fmt.Sprintf("%d ", ifList[i].GetIfHandle())
		}
	} else {
		ifStr += "None"
	}

	encapType = encapTypeToStr(resp.GetSpec().GetWireEncap().GetEncapType())
	if encapType == "Invalid" {
		weStr = "None"
	} else {
		weStr = fmt.Sprintf("%s/%d",
			encapType,
			resp.GetSpec().GetWireEncap().GetEncapValue())
	}

	// encapType = encapTypeToStr(resp.GetSpec().GetTunnelEncap().GetEncapType())
	// if encapType == "Invalid" {
	// 	teStr = "None"
	// } else {
	// 	teStr = fmt.Sprintf("%s/%d",
	// 		encapType,
	// 		resp.GetSpec().GetTunnelEncap().GetEncapValue())
	// }

	// replIndices = fmt.Sprintf("%d-%d-%d",
	// 	resp.GetStatus().GetBcastLst().GetId(),
	// 	resp.GetStatus().GetMcastLst().GetId(),
	// 	resp.GetStatus().GetPromLst().GetId())

	// if resp.GetStatus().GetSharedBcastLst() != nil {
	// 	replIndices += fmt.Sprintf("-%d",
	// 		resp.GetStatus().GetSharedBcastLst().GetId())
	// } else {
	// 	replIndices += "-N"
	// }
	// if resp.GetStatus().GetSharedMcastLst() != nil {
	// 	replIndices += fmt.Sprintf("-%d",
	// 		resp.GetStatus().GetSharedMcastLst().GetId())
	// } else {
	// 	replIndices += "-N"
	// }

	ifIdxStr := ifGetAllIdxStr()
	oiflStr := ""
	if resp.GetStatus().GetBaseOifl() != nil {
		oiflStr += fmt.Sprintf("%d",
			resp.GetStatus().GetBaseOifl().GetId())
	}
	if resp.GetStatus().GetBaseOiflCust() != nil {
		custOifls := resp.GetStatus().GetBaseOiflCust()
		custOiflsSh := resp.GetStatus().GetBaseOiflCustShared()
		oiflIdx := 0
		for oiflIdx < len(custOifls) {
			if custOiflsSh != nil {
				oiflStr += fmt.Sprintf("%s:%d,sh:%d", ifIdxStr[uint32(oiflIdx)],
					custOifls[oiflIdx].GetId(),
					custOiflsSh[oiflIdx].GetId())
			} else {
				oiflStr += fmt.Sprintf("%s:%d", ifIdxStr[uint32(oiflIdx)],
					custOifls[oiflIdx].GetId())
			}
			oiflStr += ";"
			oiflIdx++
		}
		if len(oiflStr) > 0 {
			oiflStr = oiflStr[:len(oiflStr)-1]
		} else {
			oiflStr += "-"
		}
	}

	ifIDStr := "-"
	if resp.GetSpec().GetPinnedUplinkIfKeyHandle().GetInterfaceId() != 0 {
		ifIDStr = fmt.Sprintf("uplink-%d", resp.GetSpec().GetPinnedUplinkIfKeyHandle().GetInterfaceId())
	}

	vrfTypeStr := "Mgmt"
	vrfType, desUplink := vrfGetType(resp.GetSpec().GetVrfKeyHandle().GetVrfId())
	if vrfType == halproto.VrfType_VRF_TYPE_CUSTOMER {
		vrfTypeStr = "Cust"
	}

	attachStr := ""
	attachL2segs := resp.GetStatus().GetAttachedL2Segs()
	attachIdx := 0
	for attachIdx < len(attachL2segs) {
		if attachL2segs[attachIdx].GetSegmentId() != 0 {
			if vrfType == halproto.VrfType_VRF_TYPE_CUSTOMER {
				attachStr += fmt.Sprintf("%s:%d", ifIdxStr[uint32(attachIdx)],
					attachL2segs[attachIdx].GetSegmentId())
			} else {
				attachStr += fmt.Sprintf("Uplink-%d:%d", desUplink,
					attachL2segs[attachIdx].GetSegmentId())
			}
			attachStr += ","
		}
		attachIdx++
	}
	if len(attachStr) > 0 {
		attachStr = attachStr[:len(attachStr)-1]
	} else {
		attachStr += "-"
	}

	fmt.Printf("%-8d%-5s%-6d%-10s%-40s%-5d%-10s%-11s%-20s\n",
		resp.GetSpec().GetKeyOrHandle().GetSegmentId(),
		vrfTypeStr,
		resp.GetSpec().GetVrfKeyHandle().GetVrfId(),
		weStr,
		oiflStr,
		resp.GetStats().GetNumEndpoints(),
		ifStr,
		ifIDStr,
		attachStr)
}

func l2segPdShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("Id:         L2seg's ID                       HwId:       L2seg's Hwid used in flow lookup\n")
	fmt.Printf("LookupId:   L2seg's Lookup Id                CPUVlan:    Pkt's Vlan from CPU on this L2seg\n")
	fmt.Printf("InpPropCPU: Input Prop. table idx from CPU   BcastIdx:   Bcast replication list\n")
	fmt.Printf("InpProp.1q: Inp. Prop table indices for IFs  InpPropPr:  Inp. Prop table indices for IFs\n")
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-12s%-20s%-20s\n",
		"Id", "HwId", "LookupId", "CPUVlan", "InpPropCPU", "InpProp.1q", "InpPropPr")
	fmt.Println(hdrLine)
}

func l2segPdShowOneResp(resp *halproto.L2SegmentGetResponse, ifIDxToStr map[uint32]string) {
	if resp.GetStatus().GetEpdInfo() != nil {
		l2segEPdShowOneResp(resp, ifIDxToStr)
	} else {
		fmt.Printf("No PD")
	}
}

func l2segEPdShowOneResp(resp *halproto.L2SegmentGetResponse, ifIDxToStr map[uint32]string) {
	// status := resp.GetStatus()
	epdStatus := resp.GetStatus().GetEpdInfo()

	inpPropIdx := epdStatus.GetInpPropIdx()
	inpPropIdxPrTag := epdStatus.GetInpPropIdxPrTag()

	inpPropStr := ""
	first := true
	firstLine := true
	count := 0
	inpPropArrayIdx := 0
	inpPropPrArrayIdx := 0
	nonePrinted := false
	nonePrPrinted := false
	countPerLine := 1

	for inpPropArrayIdx < len(inpPropIdx) ||
		inpPropPrArrayIdx < len(inpPropIdxPrTag) {
		inpPropStr = ""
		count = 0
		first = true
		for inpPropArrayIdx < len(inpPropIdx) {
			ifStr := ifIDxToStr[uint32(inpPropArrayIdx)]
			if inpPropIdx[inpPropArrayIdx] > 0 {
				if (inpPropIdx[inpPropArrayIdx]&(1<<28))>>28 == 1 {
					inpPropIdx[inpPropArrayIdx] = inpPropIdx[inpPropArrayIdx] ^ (1 << 28)
					if first == true {
						first = false
						inpPropStr += fmt.Sprintf("%s::OT:%d",
							ifStr,
							inpPropIdx[inpPropArrayIdx])
					} else {
						inpPropStr += fmt.Sprintf(", %s::OT:%d",
							ifStr,
							inpPropIdx[inpPropArrayIdx])
					}
				} else {
					if first == true {
						first = false
						inpPropStr += fmt.Sprintf("%s::H:%d",
							ifStr,
							inpPropIdx[inpPropArrayIdx])
					} else {
						inpPropStr += fmt.Sprintf(", %s::H:%d",
							ifStr,
							inpPropIdx[inpPropArrayIdx])
					}
				}
				count++
			}
			inpPropArrayIdx++
			if count == countPerLine || inpPropArrayIdx == len(inpPropIdx) {
				inpPropStr = fmt.Sprintf("%-20s", inpPropStr)
				break
			}
		}

		if count == 0 {
			if nonePrinted == false {
				nonePrinted = true
				inpPropStr = fmt.Sprintf("%-20s", "None")
			} else {
				inpPropStr = fmt.Sprintf("%-20s", "")
			}
		}

		countPri := 0
		first = true
		for inpPropPrArrayIdx < len(inpPropIdxPrTag) {
			ifStr := ifIDxToStr[uint32(inpPropPrArrayIdx)]
			if inpPropIdxPrTag[inpPropPrArrayIdx] > 0 {
				if (inpPropIdxPrTag[inpPropPrArrayIdx]&(1<<28))>>28 == 1 {
					inpPropIdxPrTag[inpPropPrArrayIdx] = inpPropIdxPrTag[inpPropPrArrayIdx] ^ (1 << 28)
					if first == true {
						first = false
						inpPropStr += fmt.Sprintf("%s::OT:%d",
							ifStr,
							inpPropIdxPrTag[inpPropPrArrayIdx])
					} else {
						inpPropStr += fmt.Sprintf(", %s::OT:%d",
							ifStr,
							inpPropIdxPrTag[inpPropPrArrayIdx])
					}
				} else {
					if first == true {
						first = false
						inpPropStr += fmt.Sprintf("%s::H:%d",
							ifStr,
							inpPropIdxPrTag[inpPropPrArrayIdx])
					} else {
						inpPropStr += fmt.Sprintf(", %s::H:%d",
							ifStr,
							inpPropIdxPrTag[inpPropPrArrayIdx])
					}
				}
				countPri++
			}
			inpPropPrArrayIdx++
			if countPri == countPerLine {
				break
			}
		}

		if countPri == 0 {
			if nonePrPrinted == false {
				nonePrPrinted = true
				inpPropStr = fmt.Sprintf("%-20s%-20s", inpPropStr, "None")
			} else {
				inpPropStr = fmt.Sprintf("%-20s%-20s", inpPropStr, "")
			}
		}

		if firstLine == true {
			firstLine = false
			inpPropCPUStr := "-"
			if epdStatus.GetInpPropCpuIdx() > 0 {
				if (epdStatus.GetInpPropCpuIdx()&(1<<28))>>28 == 1 {
					idx := epdStatus.GetInpPropCpuIdx() ^ (1 << 28)
					inpPropCPUStr = fmt.Sprintf("OT:%d", idx)
				} else {
					idx := epdStatus.GetInpPropCpuIdx()
					inpPropCPUStr = fmt.Sprintf("H:%d", idx)
				}
			}
			fmt.Printf("%-10d%-10d%-10d%-10d%-12s%-40s\n",
				resp.GetSpec().GetKeyOrHandle().GetSegmentId(),
				epdStatus.GetHwL2SegId(),
				epdStatus.GetL2SegLookupId(),
				epdStatus.GetL2SegVlanIdCpu(),
				inpPropCPUStr,
				inpPropStr)
			if count == 0 && countPri == 0 {
				break
			}
		} else {
			if count == 0 && countPri == 0 {
				break
			}
			fmt.Printf("%-52s%-40s\n", "", inpPropStr)
		}
	}
}

func l2segIDGetWireEncap(id uint64) uint32 {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewL2SegmentClient(c)

	defer c.Close()

	var req *halproto.L2SegmentGetRequest
	req = &halproto.L2SegmentGetRequest{
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: id,
			},
		},
	}

	l2segGetReqMsg := &halproto.L2SegmentGetRequestMsg{
		Request: []*halproto.L2SegmentGetRequest{req},
	}

	// HAL call
	respMsg, err := client.L2SegmentGet(context.Background(), l2segGetReqMsg)
	if err != nil {
		fmt.Printf("Getting L2Seg failed. %v\n", err)
		return 0
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		return resp.GetSpec().GetWireEncap().GetEncapValue()
	}
	return 0
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
