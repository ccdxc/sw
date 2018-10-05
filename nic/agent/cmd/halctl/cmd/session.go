//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"strconv"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	sessionVrfID         uint64
	sessionHandle        uint64
	sessionSrcIP         string
	sessionDstIP         string
	sessionSrcPort       uint32
	sessionDstPort       uint32
	sessionIPProto       uint32
	sessionL2SegID       uint32
	sessionDetailVrfID   uint64
	sessionDetailHandle  uint64
	sessionDetailSrcIP   string
	sessionDetailDstIP   string
	sessionDetailSrcPort uint32
	sessionDetailDstPort uint32
	sessionDetailIPProto uint32
	sessionDetailL2SegID uint32
)

var sessionShowCmd = &cobra.Command{
	Use:   "session",
	Short: "show session information",
	Long:  "show session object information",
	Run:   sessionShowCmdHandler,
}

var sessionDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed session information",
	Long:  "show detailed information about session objects",
	Run:   sessionDetailShowCmdHandler,
}

var sessionClearCmd = &cobra.Command{
	Use:   "session",
	Short: "clear session information",
	Long:  "clear session object information",
	Run:   sessionClearCmdHandler,
}

func init() {
	showCmd.AddCommand(sessionShowCmd)
	sessionShowCmd.AddCommand(sessionDetailShowCmd)

	sessionShowCmd.Flags().Uint64Var(&sessionVrfID, "vrfid", 0, "Specify vrf-id (default is 0)")
	sessionShowCmd.Flags().Uint64Var(&sessionHandle, "handle", 0, "Specify session handle")
	sessionShowCmd.Flags().StringVar(&sessionSrcIP, "srcip", "0.0.0.0", "Specify session src ip")
	sessionShowCmd.Flags().StringVar(&sessionDstIP, "dstip", "0.0.0.0", "Specify session dst ip")
	sessionShowCmd.Flags().Uint32Var(&sessionSrcPort, "srcport", 0, "Specify session src port")
	sessionShowCmd.Flags().Uint32Var(&sessionDstPort, "dstport", 0, "Specify session dst port")
	sessionShowCmd.Flags().Uint32Var(&sessionIPProto, "ipproto", 0, "Specify session IP proto")
	sessionShowCmd.Flags().Uint32Var(&sessionL2SegID, "l2segid", 0, "Specify session L2 Segment ID")
	sessionDetailShowCmd.Flags().Uint64Var(&sessionDetailVrfID, "vrfid", 0, "Specify vrf-id (default is 0)")
	sessionDetailShowCmd.Flags().Uint64Var(&sessionDetailHandle, "handle", 0, "Specify session handle")
	sessionDetailShowCmd.Flags().StringVar(&sessionDetailSrcIP, "srcip", "0.0.0.0", "Specify session src ip")
	sessionDetailShowCmd.Flags().StringVar(&sessionDetailDstIP, "dstip", "0.0.0.0", "Specify session dst ip")
	sessionDetailShowCmd.Flags().Uint32Var(&sessionDetailSrcPort, "srcport", 0, "Specify session src port")
	sessionDetailShowCmd.Flags().Uint32Var(&sessionDetailDstPort, "dstport", 0, "Specify session dst port")
	sessionDetailShowCmd.Flags().Uint32Var(&sessionDetailIPProto, "ipproto", 0, "Specify session IP proto")
	sessionDetailShowCmd.Flags().Uint32Var(&sessionDetailL2SegID, "l2segid", 0, "Specify session L2 Segment ID")

	clearCmd.AddCommand(sessionClearCmd)
	sessionClearCmd.Flags().Uint64Var(&sessionVrfID, "vrfid", 0, "Specify vrf-id (default is 0)")
	sessionClearCmd.Flags().Uint64Var(&sessionHandle, "handle", 0, "Specify session handle")
	sessionClearCmd.Flags().StringVar(&sessionSrcIP, "srcip", "0.0.0.0", "Specify session src ip")
	sessionClearCmd.Flags().StringVar(&sessionDstIP, "dstip", "0.0.0.0", "Specify session dst ip")
	sessionClearCmd.Flags().Uint32Var(&sessionSrcPort, "srcport", 0, "Specify session src port")
	sessionClearCmd.Flags().Uint32Var(&sessionDstPort, "dstport", 0, "Specify session dst port")
	sessionClearCmd.Flags().Uint32Var(&sessionIPProto, "ipproto", 0, "Specify session IP proto")
	sessionClearCmd.Flags().Uint32Var(&sessionL2SegID, "l2segid", 0, "Specify session L2 Segment ID")
}

func sessionShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := halproto.NewSessionClient(c.ClientConn)

	var sessionGetReqMsg *halproto.SessionGetRequestMsg

	if cmd.Flags().Changed("handle") {
		var req *halproto.SessionGetRequest
		req = &halproto.SessionGetRequest{
			GetBy: &halproto.SessionGetRequest_SessionHandle{
				SessionHandle: sessionHandle,
			},
		}
		sessionGetReqMsg = &halproto.SessionGetRequestMsg{
			Request: []*halproto.SessionGetRequest{req},
		}
	} else if cmd.Flags().Changed("vrfid") || cmd.Flags().Changed("srcip") ||
		cmd.Flags().Changed("dstip") || cmd.Flags().Changed("srcport") ||
		cmd.Flags().Changed("dstport") || cmd.Flags().Changed("ipproto") ||
		cmd.Flags().Changed("l2segid") {
		var req *halproto.SessionGetRequest
		if cmd.Flags().Changed("srcip") && cmd.Flags().Changed("dstip") {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionSrcIP),
							},
						},
						DstIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionDstIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
					},
				},
			}
		} else if cmd.Flags().Changed("srcip") {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionSrcIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
					},
				},
			}
		} else if cmd.Flags().Changed("dstip") {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						DstIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionDstIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
					},
				},
			}
		} else {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
					},
				},
			}
		}
		sessionGetReqMsg = &halproto.SessionGetRequestMsg{
			Request: []*halproto.SessionGetRequest{req},
		}
	} else {
		// Get all Sessions
		sessionGetReqMsg = &halproto.SessionGetRequestMsg{
			Request: []*halproto.SessionGetRequest{},
		}
	}

	// HAL call
	respMsg, err := client.SessionGet(context.Background(), sessionGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Session failed. %v\n", err)
		return
	}

	// Print Header
	sessionShowHeader(cmd, args)

	// Print Sessions
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		sessionShowOneResp(resp)
	}
}

func handleSessionDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewSessionClient(c.ClientConn)

	var sessionGetReqMsg *halproto.SessionGetRequestMsg

	if cmd != nil && cmd.Flags().Changed("handle") {
		var req *halproto.SessionGetRequest
		req = &halproto.SessionGetRequest{
			GetBy: &halproto.SessionGetRequest_SessionHandle{
				SessionHandle: sessionDetailHandle,
			},
		}
		sessionGetReqMsg = &halproto.SessionGetRequestMsg{
			Request: []*halproto.SessionGetRequest{req},
		}
	} else if cmd != nil && (cmd.Flags().Changed("vrfid") || cmd.Flags().Changed("srcip") ||
		cmd.Flags().Changed("dstip") || cmd.Flags().Changed("srcport") ||
		cmd.Flags().Changed("dstport") || cmd.Flags().Changed("ipproto") ||
		cmd.Flags().Changed("l2segid")) {
		var req *halproto.SessionGetRequest
		if cmd.Flags().Changed("srcip") && cmd.Flags().Changed("dstip") {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionDetailSrcIP),
							},
						},
						DstIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionDetailDstIP),
							},
						},
						SrcPort:     sessionDetailSrcPort,
						DstPort:     sessionDetailDstPort,
						IpProto:     halproto.IPProtocol(sessionDetailIPProto),
						VrfId:       sessionDetailVrfID,
						L2SegmentId: sessionDetailL2SegID,
					},
				},
			}
		} else if cmd != nil && cmd.Flags().Changed("srcip") {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionDetailSrcIP),
							},
						},
						SrcPort:     sessionDetailSrcPort,
						DstPort:     sessionDetailDstPort,
						IpProto:     halproto.IPProtocol(sessionDetailIPProto),
						VrfId:       sessionDetailVrfID,
						L2SegmentId: sessionDetailL2SegID,
					},
				},
			}
		} else if cmd != nil && cmd.Flags().Changed("dstip") {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						DstIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionDetailDstIP),
							},
						},
						SrcPort:     sessionDetailSrcPort,
						DstPort:     sessionDetailDstPort,
						IpProto:     halproto.IPProtocol(sessionDetailIPProto),
						VrfId:       sessionDetailVrfID,
						L2SegmentId: sessionDetailL2SegID,
					},
				},
			}
		} else {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcPort:     sessionDetailSrcPort,
						DstPort:     sessionDetailDstPort,
						IpProto:     halproto.IPProtocol(sessionDetailIPProto),
						VrfId:       sessionDetailVrfID,
						L2SegmentId: sessionDetailL2SegID,
					},
				},
			}
		}
		sessionGetReqMsg = &halproto.SessionGetRequestMsg{
			Request: []*halproto.SessionGetRequest{req},
		}
	} else {
		// Get all Sessions
		sessionGetReqMsg = &halproto.SessionGetRequestMsg{
			Request: []*halproto.SessionGetRequest{},
		}
	}

	// HAL call
	respMsg, err := client.SessionGet(context.Background(), sessionGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Session failed. %v\n", err)
		return
	}

	// Print Sessions
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

func sessionDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleSessionDetailShowCmd(cmd, nil)
}

func sessionShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 136)
	fmt.Printf("Legend:\nHandle: Session Handle\tRole: I - Initiator, R - Responder\n")
	fmt.Printf("KeyType: Flow Key Type\tL2SegID: L2 Segment ID\n")
	fmt.Printf("SrcVrfID: Source VRF ID\tDstVrfID: Destination VRF ID\n")
	fmt.Printf("SMAC|SIP[:sport]: Source MAC Address | Source IP Address and Port Number\n")
	fmt.Printf("DMAC|DIP[:dport]: Destination MAC Address | Destination IP Address and Port Number\n")
	fmt.Printf("Proto|EType: L4 Protocol | Ethernet Type\n")
	fmt.Printf("TCP State: State for TCP flows\tAge: Age in Secs\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-8s%-6s%-10s%-12s%-10s%-10s%-24s%-24s%-12s%-16s%-6s\n",
		"Handle", "Role", "KeyType", "L2SegID", "SrcVrfID", "DstVrfID",
		"SMAC|SIP[:sport]", "DMAC|DIP[:dport]", "Proto|EType", "TCP State",
		"Age")
	fmt.Println(hdrLine)
}

func sessionShowOneResp(resp *halproto.SessionGetResponse) {
	spec := resp.GetSpec()
	status := resp.GetStatus()

	// Get initiator and responder flow
	initiatorFlow := spec.GetInitiatorFlow()
	responderFlow := spec.GetResponderFlow()

	if initiatorFlow != nil {
		flowStr := "I"
		flowShow(spec, status, initiatorFlow, flowStr)
	}

	if responderFlow != nil {
		flowStr := "R"
		flowShow(spec, status, responderFlow, flowStr)
	}
}

func flowShow(spec *halproto.SessionSpec, status *halproto.SessionStatus, flowSpec *halproto.FlowSpec, flowStr string) {
	var (
		keyType   string
		id        uint64
		sessionID uint64
		srcID     uint64
		dstID     uint64
		src       string
		dst       string
		ipproto   string
	)
	flowKey := flowSpec.GetFlowKey()
	flowInfo := flowSpec.GetFlowData().GetFlowInfo()

	tcpState := "-"
	switch flowKey.GetFlowKey().(type) {
	case *halproto.FlowKey_L2Key:
		keyType = "L2"
		l2Key := flowKey.GetL2Key()
		id = uint64(l2Key.GetL2SegmentId())
		src = utils.MactoStr(l2Key.GetSmac())
		dst = utils.MactoStr(l2Key.GetDmac())
		ipproto = strconv.Itoa(int(l2Key.GetEtherType()))
	case *halproto.FlowKey_V4Key:
		keyType = "IPv4"
		v4Key := flowKey.GetV4Key()
		sessionID = spec.GetVrfKeyHandle().GetVrfId()
		srcID = flowKey.GetSrcVrfId()
		dstID = flowKey.GetDstVrfId()
		if srcID == 0 {
			srcID = sessionID
		}
		if dstID == 0 {
			dstID = sessionID
		}
		src = Uint32IPAddrToStr(v4Key.GetSip())
		dst = Uint32IPAddrToStr(v4Key.GetDip())
		l4 := v4Key.GetL4Fields()
		switch l4.(type) {
		case *halproto.FlowKeyV4_TcpUdp:
			tcp := v4Key.GetTcpUdp()
			src += ":["
			src += strconv.Itoa(int(tcp.GetSport()))
			src += "]"
			dst += ":["
			dst += strconv.Itoa(int(tcp.GetDport()))
			dst += "]"
			if halproto.IPProtocol(v4Key.GetIpProto()) == halproto.IPProtocol_IPPROTO_TCP {
				ipproto = "TCP"
				tcpState = tcpStateStringCompact(flowInfo.GetTcpState().String())
			} else {
				ipproto = "UDP"
			}
		case *halproto.FlowKeyV4_Icmp:
			icmp := v4Key.GetIcmp()
			src += ":["
			src += strconv.Itoa(int(icmp.GetType()))
			src += "/"
			src += strconv.Itoa(int(icmp.GetCode()))
			src += "]"
			dst += ":["
			dst += strconv.Itoa(int(icmp.GetId()))
			dst += "]"
			ipproto = "ICMP"
		case *halproto.FlowKeyV4_Esp:
			esp := v4Key.GetEsp()
			src += ":["
			src += strconv.Itoa(int(esp.GetSpi()))
			src += "]"
			ipproto = "ESP"
		default:
			ipproto = v4Key.GetIpProto().String()
		}
	case *halproto.FlowKey_V6Key:
		keyType = "IPv6"
		v6Key := flowKey.GetV6Key()
		sessionID = spec.GetVrfKeyHandle().GetVrfId()
		srcID = flowKey.GetSrcVrfId()
		dstID = flowKey.GetDstVrfId()
		if srcID == 0 {
			srcID = sessionID
		}
		if dstID == 0 {
			dstID = sessionID
		}
		src = utils.IPAddrToStr(v6Key.GetSip())
		dst = utils.IPAddrToStr(v6Key.GetDip())
		l4 := v6Key.GetL4Fields()
		switch l4.(type) {
		case *halproto.FlowKeyV6_TcpUdp:
			tcp := v6Key.GetTcpUdp()
			src += ":["
			src += strconv.Itoa(int(tcp.GetSport()))
			src += "]"
			dst += ":["
			dst += strconv.Itoa(int(tcp.GetDport()))
			dst += "]"
			if halproto.IPProtocol(v6Key.GetIpProto()) == halproto.IPProtocol_IPPROTO_TCP {
				ipproto = "TCP"
				tcpState = tcpStateStringCompact(flowInfo.GetTcpState().String())
			} else {
				ipproto = "UDP"
			}
		case *halproto.FlowKeyV6_Icmp:
			icmp := v6Key.GetIcmp()
			src += ":["
			src += strconv.Itoa(int(icmp.GetType()))
			src += "/"
			src += strconv.Itoa(int(icmp.GetCode()))
			src += "]"
			dst += ":["
			dst += strconv.Itoa(int(icmp.GetId()))
			dst += "]"
			ipproto = "ICMP"
		case *halproto.FlowKeyV6_Esp:
			esp := v6Key.GetEsp()
			src += ":["
			src += strconv.Itoa(int(esp.GetSpi()))
			src += "]"
			ipproto = "ESP"
		default:
			ipproto = v6Key.GetIpProto().String()
		}
	default:
		keyType = "UNK"
		src = "UNK"
		dst = "UNK"
		id = 0
		srcID = 0
		dstID = 0
	}

	age := flowInfo.GetFlowAge()

	fmt.Printf("%-8d%-6s%-10s%-12d%-10d%-10d%-24s%-24s%-12s%-16s%-6d\n",
		status.GetSessionHandle(),
		flowStr, keyType, id,
		srcID, dstID,
		src, dst, ipproto,
		tcpState, age)

	natType := flowInfo.GetNatType()
	snat := false
	dnat := false

	switch natType {
	case halproto.NatType_NAT_TYPE_SNAT:
		snat = true
	case halproto.NatType_NAT_TYPE_DNAT:
		dnat = true
	case halproto.NatType_NAT_TYPE_TWICE_NAT:
		snat = true
		dnat = true
	}

	if snat {
		nsStr := utils.IPAddrToStr(flowInfo.GetNatSip())
		nsStr += fmt.Sprintf(":%d", flowInfo.GetNatSport())
		fmt.Printf("%-24s%-8s%-40s%-4s%-40s\n",
			"", "SNAT", src, "->", nsStr)
	}

	if dnat {
		ndStr := utils.IPAddrToStr(flowInfo.GetNatDip())
		ndStr += fmt.Sprintf(":%d", flowInfo.GetNatDport())
		fmt.Printf("%-24s%-8s%-40s%-4s%-40s\n",
			"", "DNAT", dst, "->", ndStr)
	}
}

func tcpStateStringCompact(state string) string {
	var compactString string
	fmt.Sscanf(state, "FLOW_TCP_STATE_%s", &compactString)
	return compactString
}

// Uint32IPAddrToStr converts uint32 IP address to string
func Uint32IPAddrToStr(ip uint32) string {
	return fmt.Sprintf("%d.%d.%d.%d", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff)
}

// IPAddrStrtoUint32 converts string IP address to uint32
func IPAddrStrtoUint32(ip string) uint32 {
	var addr [4]uint32
	fmt.Sscanf(ip, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3])
	return ((addr[0] << 24) + (addr[1] << 16) + (addr[2] << 8) + (addr[3]))
}

func sessionClearCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := halproto.NewSessionClient(c.ClientConn)

	var sessionDeleteReqMsg *halproto.SessionDeleteRequestMsg

	if cmd.Flags().Changed("handle") {
		var req *halproto.SessionDeleteRequest
		req = &halproto.SessionDeleteRequest{
			GetBy: &halproto.SessionDeleteRequest_SessionHandle{
				SessionHandle: sessionHandle,
			},
		}
		sessionDeleteReqMsg = &halproto.SessionDeleteRequestMsg{
			Request: []*halproto.SessionDeleteRequest{req},
		}
	} else if cmd.Flags().Changed("vrfid") || cmd.Flags().Changed("srcip") ||
		cmd.Flags().Changed("dstip") || cmd.Flags().Changed("srcport") ||
		cmd.Flags().Changed("dstport") || cmd.Flags().Changed("ipproto") ||
		cmd.Flags().Changed("l2segid") {
		var req *halproto.SessionDeleteRequest
		if cmd.Flags().Changed("srcip") && cmd.Flags().Changed("dstip") {
			req = &halproto.SessionDeleteRequest{
				GetBy: &halproto.SessionDeleteRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionSrcIP),
							},
						},
						DstIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionDstIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
					},
				},
			}
		} else if cmd.Flags().Changed("srcip") {
			req = &halproto.SessionDeleteRequest{
				GetBy: &halproto.SessionDeleteRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionSrcIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
					},
				},
			}
		} else if cmd.Flags().Changed("dstip") {
			req = &halproto.SessionDeleteRequest{
				GetBy: &halproto.SessionDeleteRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						DstIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: IPAddrStrtoUint32(sessionDstIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
					},
				},
			}
		} else {
			req = &halproto.SessionDeleteRequest{
				GetBy: &halproto.SessionDeleteRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
					},
				},
			}
		}
		sessionDeleteReqMsg = &halproto.SessionDeleteRequestMsg{
			Request: []*halproto.SessionDeleteRequest{req},
		}
	} else {
		// Delete all Sessions
		sessionDeleteReqMsg = &halproto.SessionDeleteRequestMsg{
			Request: []*halproto.SessionDeleteRequest{},
		}
	}

	// HAL call
	respMsg, err := client.SessionDelete(context.Background(), sessionDeleteReqMsg)
	if err != nil {
		fmt.Printf("Deleting Session failed. %v\n", err)
		return
	}

	// Sessions
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
	}
}
