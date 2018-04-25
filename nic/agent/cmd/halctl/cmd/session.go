package cmd

import (
	"context"
	"fmt"
	"strconv"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var sessionVrfID uint64
var sessionHandle uint64

var sessionShowCmd = &cobra.Command{
	Use:   "session",
	Short: "session",
	Long:  "shows session",
	Run:   sessionShowCmdHandler,
}

func init() {
	showCmd.AddCommand(sessionShowCmd)
	sessionShowCmd.Flags().Uint64Var(&sessionVrfID, "id", 1, "Specify vrf-id")
	sessionShowCmd.Flags().Uint64Var(&sessionHandle, "handle", 2, "Specify session handle")
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

	if cmd.Flags().Changed("id") {
		if cmd.Flags().Changed("handle") {
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

func sessionShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 115)
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-12s%-14s%-16s%-24s%-24s%-12s\n",
		"SessionID", "FlowType", "FlowKeyType", "L2SegId|VrfId", "SMAC|SIP[:sport]", "DMAC|DIP[:dport]", "Proto|EType")
	fmt.Println(hdrLine)
}

func sessionShowOneResp(resp *halproto.SessionGetResponse) {
	spec := resp.GetSpec()

	// Get initiator and responder flow
	initiatorFlow := spec.GetInitiatorFlow()
	responderFlow := spec.GetResponderFlow()

	if initiatorFlow != nil {
		flowStr := "initiator"
		flowShow(spec, initiatorFlow, flowStr)
	}

	if responderFlow != nil {
		flowStr := "responder"
		flowShow(spec, responderFlow, flowStr)
	}
}

func flowShow(spec *halproto.SessionSpec, flowSpec *halproto.FlowSpec, flowStr string) {
	var (
		keyType string
		id      uint64
		src     string
		dst     string
		ipproto string
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
		id = spec.GetMeta().GetVrfId()
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
		id = spec.GetMeta().GetVrfId()
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
	}

	fmt.Printf("%-12d%-12s%-14s%-16d%-24s%-24s%-10s\n",
		spec.GetSessionId(),
		flowStr, keyType, id,
		src, dst, ipproto)
}

// Uint32IPAddrToStr converts uint32 IP address to string
func Uint32IPAddrToStr(ip uint32) string {
	return fmt.Sprintf("%d.%d.%d.%d", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff)
}
