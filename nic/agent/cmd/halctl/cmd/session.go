//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"io"
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
	sessionVrfID   uint64
	sessionHandle  uint64
	sessionSrcIP   string
	sessionDstIP   string
	sessionSrcPort uint32
	sessionDstPort uint32
	sessionIPProto uint32
	sessionL2SegID uint32
	sessionAlg     string
)

var sessionShowCmd = &cobra.Command{
	Use:   "session",
	Short: "show session information",
	Long:  "show session object information",
	Run:   sessionShowCmdHandler,
}

var sessionClearCmd = &cobra.Command{
	Use:   "session",
	Short: "clear session information",
	Long:  "clear session object information",
	Run:   sessionClearCmdHandler,
}

func init() {
	showCmd.AddCommand(sessionShowCmd)

	sessionShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	sessionShowCmd.Flags().Uint64Var(&sessionVrfID, "vrfid", 0, "Specify vrf-id (default is 0)")
	sessionShowCmd.Flags().Uint64Var(&sessionHandle, "handle", 0, "Specify session handle")
	sessionShowCmd.Flags().StringVar(&sessionSrcIP, "srcip", "0.0.0.0", "Specify session src ip")
	sessionShowCmd.Flags().StringVar(&sessionDstIP, "dstip", "0.0.0.0", "Specify session dst ip")
	sessionShowCmd.Flags().Uint32Var(&sessionSrcPort, "srcport", 0, "Specify session src port")
	sessionShowCmd.Flags().Uint32Var(&sessionDstPort, "dstport", 0, "Specify session dst port")
	sessionShowCmd.Flags().Uint32Var(&sessionIPProto, "ipproto", 0, "Specify session IP proto")
	sessionShowCmd.Flags().Uint32Var(&sessionL2SegID, "l2segid", 0, "Specify session L2 Segment ID")
	sessionShowCmd.Flags().StringVar(&sessionAlg, "alg", "none", "Specify ALG (tftp/ftp/rtsp/msft_rpc/sun_rpc/dns/sip)")

	clearCmd.AddCommand(sessionClearCmd)
	sessionClearCmd.Flags().Uint64Var(&sessionVrfID, "vrfid", 0, "Specify vrf-id (default is 0)")
	sessionClearCmd.Flags().Uint64Var(&sessionHandle, "handle", 0, "Specify session handle")
	sessionClearCmd.Flags().StringVar(&sessionSrcIP, "srcip", "0.0.0.0", "Specify session src ip")
	sessionClearCmd.Flags().StringVar(&sessionDstIP, "dstip", "0.0.0.0", "Specify session dst ip")
	sessionClearCmd.Flags().Uint32Var(&sessionSrcPort, "srcport", 0, "Specify session src port")
	sessionClearCmd.Flags().Uint32Var(&sessionDstPort, "dstport", 0, "Specify session dst port")
	sessionClearCmd.Flags().Uint32Var(&sessionIPProto, "ipproto", 0, "Specify session IP proto")
	sessionClearCmd.Flags().Uint32Var(&sessionL2SegID, "l2segid", 0, "Specify session L2 Segment ID")
	sessionClearCmd.Flags().StringVar(&sessionAlg, "alg", "none", "Specify ALG (tftp/ftp/rtsp/msft_rpc/sun_rpc/dns/sip)")
}

func sessionShowCmdHandler(cmd *cobra.Command, args []string) {
	supportedAlgs := []string{"none", "tftp", "ftp", "sun_rpc", "msft_rpc", "rtsp", "dns", "sip"}
	if cmd != nil && cmd.Flags().Changed("yaml") {
		sessionDetailShowCmdHandler(cmd, args)
		return
	}

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

	client := halproto.NewSessionClient(c)

	var sessionGetReqMsg *halproto.SessionGetRequestMsg

	found := false
	if cmd != nil {
		if cmd.Flags().Changed("alg") {
			for _, v := range supportedAlgs {
				if v == sessionAlg {
					found = true
					break
				}
			}
			if found != true {
				fmt.Printf("Unsupported ALG please try - tftp/ftp/rtsp/msft_rpc/sun_rpc/dns/sip\n")
				return
			}
		}

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
			cmd.Flags().Changed("l2segid") || cmd.Flags().Changed("alg") {
			var req *halproto.SessionGetRequest
			if cmd.Flags().Changed("srcip") && cmd.Flags().Changed("dstip") {
				req = &halproto.SessionGetRequest{
					GetBy: &halproto.SessionGetRequest_SessionFilter{
						SessionFilter: &halproto.SessionFilter{
							SrcIp: &halproto.IPAddress{
								IpAf: halproto.IPAddressFamily_IP_AF_INET,
								V4OrV6: &halproto.IPAddress_V4Addr{
									V4Addr: utils.IPAddrStrtoUint32(sessionSrcIP),
								},
							},
							DstIp: &halproto.IPAddress{
								IpAf: halproto.IPAddressFamily_IP_AF_INET,
								V4OrV6: &halproto.IPAddress_V4Addr{
									V4Addr: utils.IPAddrStrtoUint32(sessionDstIP),
								},
							},
							SrcPort:     sessionSrcPort,
							DstPort:     sessionDstPort,
							IpProto:     halproto.IPProtocol(sessionIPProto),
							VrfId:       sessionVrfID,
							L2SegmentId: sessionL2SegID,
							Alg:         algNameToEnum(sessionAlg),
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
									V4Addr: utils.IPAddrStrtoUint32(sessionSrcIP),
								},
							},
							SrcPort:     sessionSrcPort,
							DstPort:     sessionDstPort,
							IpProto:     halproto.IPProtocol(sessionIPProto),
							VrfId:       sessionVrfID,
							L2SegmentId: sessionL2SegID,
							Alg:         algNameToEnum(sessionAlg),
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
									V4Addr: utils.IPAddrStrtoUint32(sessionDstIP),
								},
							},
							SrcPort:     sessionSrcPort,
							DstPort:     sessionDstPort,
							IpProto:     halproto.IPProtocol(sessionIPProto),
							VrfId:       sessionVrfID,
							L2SegmentId: sessionL2SegID,
							Alg:         algNameToEnum(sessionAlg),
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
							Alg:         algNameToEnum(sessionAlg),
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
	} else {
		// Get all Sessions
		sessionGetReqMsg = &halproto.SessionGetRequestMsg{
			Request: []*halproto.SessionGetRequest{},
		}
	}

	// Print Header
	sessionShowHeader(cmd, args)

	// HAL call
	stream, err := client.SessionGet(context.Background(), sessionGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Session failed. %v\n", err)
		return
	}

	for {
		respMsg, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Printf("Getting session stream failure. %v\n", err)
		}

		// Print Sessions
		for _, resp := range respMsg.Response {
			if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
				continue
			}
			sessionShowOneResp(resp)
		}
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

	client := halproto.NewSessionClient(c)

	var sessionGetReqMsg *halproto.SessionGetRequestMsg

	if cmd != nil && cmd.Flags().Changed("handle") {
		var req *halproto.SessionGetRequest
		req = &halproto.SessionGetRequest{
			GetBy: &halproto.SessionGetRequest_SessionHandle{
				SessionHandle: sessionHandle,
			},
		}
		sessionGetReqMsg = &halproto.SessionGetRequestMsg{
			Request: []*halproto.SessionGetRequest{req},
		}
	} else if cmd != nil && (cmd.Flags().Changed("vrfid") || cmd.Flags().Changed("srcip") ||
		cmd.Flags().Changed("dstip") || cmd.Flags().Changed("srcport") ||
		cmd.Flags().Changed("dstport") || cmd.Flags().Changed("ipproto") ||
		cmd.Flags().Changed("l2segid") || cmd.Flags().Changed("alg")) {
		var req *halproto.SessionGetRequest
		if cmd.Flags().Changed("srcip") && cmd.Flags().Changed("dstip") {
			req = &halproto.SessionGetRequest{
				GetBy: &halproto.SessionGetRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: utils.IPAddrStrtoUint32(sessionSrcIP),
							},
						},
						DstIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: utils.IPAddrStrtoUint32(sessionDstIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
						Alg:         algNameToEnum(sessionAlg),
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
								V4Addr: utils.IPAddrStrtoUint32(sessionSrcIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
						Alg:         algNameToEnum(sessionAlg),
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
								V4Addr: utils.IPAddrStrtoUint32(sessionDstIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
						Alg:         algNameToEnum(sessionAlg),
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
						Alg:         algNameToEnum(sessionAlg),
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
	stream, err := client.SessionGet(context.Background(), sessionGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Session failed. %v\n", err)
		return
	}
	for {
		respMsg, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Printf("Getting session stream failure. %v\n", err)
		}

		// Print Sessions
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
}

func sessionDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleSessionDetailShowCmd(cmd, nil)
}

func sessionShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 130)
	fmt.Printf("Legend:\nHandle: Session Handle\n")
	fmt.Printf("Role: Direction/Instance\n")
	fmt.Printf("      Direction: U (Uplink), H (Host)\n      Instance: P (Primary), S (Secondary)\n")
	fmt.Printf("KeyType: Flow Key Type\tL2SegID: L2 Segment ID\n")
	fmt.Printf("VrfID: Source VRF ID/Destination VRF ID\n")
	fmt.Printf("SMAC|SIP[:sport]: Source MAC Address | Source IP Address and Port Number\n")
	fmt.Printf("DMAC|DIP[:dport]: Destination MAC Address | Destination IP Address and Port Number\n")
	fmt.Printf("P|E: L4 Protocol | Ethernet Type\n")
	fmt.Printf("Flow Action: A (Allowed flow) D (drop flow)\n")
	fmt.Printf("TCP State: State for TCP flows\tAge: Age in mins and secs\n")
	fmt.Printf("Time To Age: Inactivity time remaining for flow to age in seconds (IDF- No aging indefinite time)\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-8s%-6s%-8s%-8s%-10s%-24s%-24s%-6s%-10s%-16s%-10s%-15s\n",
		"Handle", "Role", "KeyType", "L2SegID", "VrfID",
		"SMAC|SIP[:sport]", "DMAC|DIP[:dport]", "P|E", "Flow State", "TCP State",
		"Age", "Time To Age")
	fmt.Println(hdrLine)
}

func sessionShowOneResp(resp *halproto.SessionGetResponse) {
	spec := resp.GetSpec()
	status := resp.GetStatus()

	// Get initiator and responder flow
	initiatorFlow := spec.GetInitiatorFlow()
	responderFlow := spec.GetResponderFlow()
	peerInitiatorFlow := spec.GetPeerInitiatorFlow()
	peerResponderFlow := spec.GetPeerResponderFlow()

	if initiatorFlow != nil {
		flowShow(spec, status, initiatorFlow, status.GetIflowStatus())
	}

	if responderFlow != nil {
		flowShow(spec, status, responderFlow, status.GetRflowStatus())
	}

	if peerInitiatorFlow != nil {
		flowShow(spec, status, peerInitiatorFlow, status.GetPeerIflowStatus())
	}

	if peerResponderFlow != nil {
		flowShow(spec, status, peerResponderFlow, status.GetPeerRflowStatus())
	}
}

func flowShow(spec *halproto.SessionSpec, status *halproto.SessionStatus,
	flowSpec *halproto.FlowSpec, flowStatus *halproto.FlowStatus) {
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

	flowStr := strings.Replace(flowStatus.GetFlowDirection().String(), "FLOW_DIRECTION_FROM_", "", -1)
	flowStr = flowStr[0:1]
	flowStr += "/"
	flowStr += strings.Replace(flowStatus.GetFlowInstance().String(), "FLOW_INSTANCE_", "", -1)
	flowStr = flowStr[0:3]

	tcpState := "-"
	flowAction := "-"
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

	flowAction = flowActionStringCompact(flowInfo.GetFlowAction().String())
	if keyType == "L2" || keyType == "IPv6" {
		flowAction = flowActionStringCompact("FLOW_ACTION_ALLOW")
	}
	age := flowInfo.GetFlowAge()
	ageStr := ""
	if age > 59 {
		ageStr += strconv.Itoa(int(age/60)) + "m "
	}
	ageStr += strconv.Itoa(int(age%60)) + "s"

	vrfStr := strconv.Itoa(int(srcID)) + "/" + strconv.Itoa(int(dstID))

	timeToAge := flowInfo.GetTimeToAge()
	timeToAgeStr := ""
	if timeToAge > 59 {
		timeToAgeStr += strconv.Itoa(int(timeToAge/60)) + "m "
	}
	timeToAgeStr += strconv.Itoa(int(timeToAge%60)) + "s"

	if timeToAge == 0xFFFFFFFF {
		fmt.Printf("%-8d%-6s%-8s%-8d%-10s%-24s%-24s%-6s%-10s%-16s%-10s%-15s\n",
			status.GetSessionHandle(),
			flowStr, keyType, id,
			vrfStr,
			src, dst, ipproto, flowAction,
			tcpState, ageStr, "IDF")
	} else {
		fmt.Printf("%-8d%-6s%-8s%-8d%-10s%-24s%-24s%-6s%-10s%-16s%-10s%-15s\n",
			status.GetSessionHandle(),
			flowStr, keyType, id,
			vrfStr,
			src, dst, ipproto, flowAction,
			tcpState, ageStr, timeToAgeStr)
	}

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

func flowActionStringCompact(state string) string {
	var compactString string
	fmt.Sscanf(state, "FLOW_ACTION_%s", &compactString)
	if compactString == "ALLOW" {
		return "A"
	}
	return "D"
}

func algNameToEnum(algName string) halproto.ALGName {
	algStr := "APP_SVC_" + strings.ToUpper(algName)
	return halproto.ALGName(halproto.ALGName_value[algStr])
}

// Uint32IPAddrToStr converts uint32 IP address to string
func Uint32IPAddrToStr(ip uint32) string {
	return fmt.Sprintf("%d.%d.%d.%d", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff)
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

	client := halproto.NewSessionClient(c)

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
		cmd.Flags().Changed("l2segid") || cmd.Flags().Changed("alg") {
		var req *halproto.SessionDeleteRequest
		if cmd.Flags().Changed("srcip") && cmd.Flags().Changed("dstip") {
			req = &halproto.SessionDeleteRequest{
				GetBy: &halproto.SessionDeleteRequest_SessionFilter{
					SessionFilter: &halproto.SessionFilter{
						SrcIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: utils.IPAddrStrtoUint32(sessionSrcIP),
							},
						},
						DstIp: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: utils.IPAddrStrtoUint32(sessionDstIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
						Alg:         algNameToEnum(sessionAlg),
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
								V4Addr: utils.IPAddrStrtoUint32(sessionSrcIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
						Alg:         algNameToEnum(sessionAlg),
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
								V4Addr: utils.IPAddrStrtoUint32(sessionDstIP),
							},
						},
						SrcPort:     sessionSrcPort,
						DstPort:     sessionDstPort,
						IpProto:     halproto.IPProtocol(sessionIPProto),
						VrfId:       sessionVrfID,
						L2SegmentId: sessionL2SegID,
						Alg:         algNameToEnum(sessionAlg),
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
						Alg:         algNameToEnum(sessionAlg),
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
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
	}
}
