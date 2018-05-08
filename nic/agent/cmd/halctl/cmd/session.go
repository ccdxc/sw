//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strconv"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	sessionVrfID        uint64
	sessionHandle       uint64
	sessionDetailVrfID  uint64
	sessionDetailHandle uint64
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

func init() {
	showCmd.AddCommand(sessionShowCmd)
	sessionShowCmd.AddCommand(sessionDetailShowCmd)

	sessionShowCmd.Flags().Uint64Var(&sessionVrfID, "id", 1, "Specify vrf-id")
	sessionShowCmd.Flags().Uint64Var(&sessionHandle, "handle", 2, "Specify session handle")
	sessionDetailShowCmd.Flags().Uint64Var(&sessionDetailVrfID, "id", 1, "Specify vrf-id")
	sessionDetailShowCmd.Flags().Uint64Var(&sessionDetailHandle, "handle", 2, "Specify session handle")
}

func sessionShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewSessionClient(c.ClientConn)

	var sessionGetReqMsg *halproto.SessionGetRequestMsg

	if cmd.Flags().Changed("id") && cmd.Flags().Changed("handle") {
		var req *halproto.SessionGetRequest
		req = &halproto.SessionGetRequest{
			Meta: &halproto.ObjectMeta{
				VrfId: sessionVrfID,
			},
			SessionHandle: sessionHandle,
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
		log.Errorf("Getting Session failed. %v", err)
	}

	// Print Header
	sessionShowHeader(cmd, args)

	// Print Sessions
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		sessionShowOneResp(resp)
	}
}

func sessionDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewSessionClient(c.ClientConn)

	var sessionGetReqMsg *halproto.SessionGetRequestMsg

	if cmd.Flags().Changed("id") {
		if cmd.Flags().Changed("handle") {
			var req *halproto.SessionGetRequest
			req = &halproto.SessionGetRequest{
				Meta: &halproto.ObjectMeta{
					VrfId: sessionDetailVrfID,
				},
				SessionHandle: sessionDetailHandle,
			}
			sessionGetReqMsg = &halproto.SessionGetRequestMsg{
				Request: []*halproto.SessionGetRequest{req},
			}
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
		log.Errorf("Getting Session failed. %v", err)
	}

	// Print Sessions
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
}

func sessionShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 132)
	fmt.Println(hdrLine)
	fmt.Printf("%-14s%-12s%-14s%-12s%-10s%-10s%-24s%-24s%-12s\n",
		"SessionHandle", "FlowType", "FlowKeyType", "L2SegId", "SrcVrfID", "DstVrfID", "SMAC|SIP[:sport]", "DMAC|DIP[:dport]", "Proto|EType")
	fmt.Println(hdrLine)
}

func sessionShowOneResp(resp *halproto.SessionGetResponse) {
	spec := resp.GetSpec()
	status := resp.GetStatus()

	// Get initiator and responder flow
	initiatorFlow := spec.GetInitiatorFlow()
	responderFlow := spec.GetResponderFlow()

	if initiatorFlow != nil {
		flowStr := "initiator"
		flowShow(spec, status, initiatorFlow, flowStr)
	}

	if responderFlow != nil {
		flowStr := "responder"
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

	switch flowKey.GetFlowKey().(type) {
	case *halproto.FlowKey_L2Key:
		keyType = "MAC"
		l2Key := flowKey.GetL2Key()
		id = uint64(l2Key.GetL2SegmentId())
		src = utils.MactoStr(l2Key.GetSmac())
		dst = utils.MactoStr(l2Key.GetDmac())
		ipproto = strconv.Itoa(int(l2Key.GetEtherType()))
	case *halproto.FlowKey_V4Key:
		keyType = "IPv4"
		v4Key := flowKey.GetV4Key()
		sessionID = spec.GetMeta().GetVrfId()
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
			src += ":"
			src += strconv.Itoa(int(tcp.GetSport()))
			dst += ":"
			dst += strconv.Itoa(int(tcp.GetDport()))
			if halproto.IPProtocol(v4Key.GetIpProto()) == halproto.IPProtocol_IPPROTO_TCP {
				ipproto = "TCP"
			} else {
				ipproto = "UDP"
			}
		case *halproto.FlowKeyV4_Icmp:
			icmp := v4Key.GetIcmp()
			src += ":"
			src += strconv.Itoa(int(icmp.GetType()))
			src += "/"
			src += strconv.Itoa(int(icmp.GetCode()))
			dst += ":"
			dst += strconv.Itoa(int(icmp.GetId()))
			ipproto = "ICMP"
		case *halproto.FlowKeyV4_Esp:
			esp := v4Key.GetEsp()
			src += ":"
			src += strconv.Itoa(int(esp.GetSpi()))
			ipproto = "ESP"
		}
	case *halproto.FlowKey_V6Key:
		keyType = "IPv6"
		v6Key := flowKey.GetV6Key()
		sessionID = spec.GetMeta().GetVrfId()
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
			src += ":"
			src += strconv.Itoa(int(tcp.GetSport()))
			dst += ":"
			dst += strconv.Itoa(int(tcp.GetDport()))
			if halproto.IPProtocol(v6Key.GetIpProto()) == halproto.IPProtocol_IPPROTO_TCP {
				ipproto = "TCP"
			} else {
				ipproto = "UDP"
			}
		case *halproto.FlowKeyV6_Icmp:
			icmp := v6Key.GetIcmp()
			src += ":"
			src += strconv.Itoa(int(icmp.GetType()))
			src += "/"
			src += strconv.Itoa(int(icmp.GetCode()))
			dst += ":"
			dst += strconv.Itoa(int(icmp.GetId()))
			ipproto = "ICMP"
		case *halproto.FlowKeyV6_Esp:
			esp := v6Key.GetEsp()
			src += ":"
			src += strconv.Itoa(int(esp.GetSpi()))
			ipproto = "ESP"
		}
	default:
		keyType = "UNK"
		src = "UNK"
		dst = "UNK"
		id = 0
		srcID = 0
		dstID = 0
	}

	fmt.Printf("%-14d%-12s%-14s%-12d%-10d%-10d%-24s%-24s%-12s\n",
		status.GetSessionHandle(),
		flowStr, keyType, id,
		srcID, dstID,
		src, dst, ipproto)

	flowInfo := flowSpec.GetFlowData().GetFlowInfo()

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

// Uint32IPAddrToStr converts uint32 IP address to string
func Uint32IPAddrToStr(ip uint32) string {
	return fmt.Sprintf("%d.%d.%d.%d", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff)
}
