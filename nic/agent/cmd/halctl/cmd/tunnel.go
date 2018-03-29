package cmd

import (
	"context"
	"encoding/binary"
	"fmt"
	"net"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	tunnelID uint64
)

var tunnelShowCmd = &cobra.Command{
	Use:   "tunnel",
	Short: "tunnel",
	Long:  "shows tunnel",
	Run:   tunnelShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(tunnelShowCmd)

	tunnelShowCmd.Flags().Uint64Var(&tunnelID, "id", 1, "Specify if-id")
}

func tunnelShowCmdHandler(cmd *cobra.Command, args []string) {

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
					InterfaceId: tunnelID,
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
	tunnelShowHeader(cmd, args)

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		tunnelShowOneResp(resp)
	}
	c.Close()
}

func tunnelShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 90)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "IfType", "EncType", "LTep", "RTep", "IMNative", "IMTunnel", "RWIdx")
	fmt.Println(hdrLine)
}

func tunnelShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_TUNNEL {
		return
	}
	encType := resp.GetSpec().GetIfTunnelInfo().GetEncapType()
	fmt.Printf("%-10d%-10d%-10s%-10s",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		resp.GetStatus().GetIfHandle(),
		ifTypeToStr(ifType),
		tnnlEncTypeToStr(encType))
	if encType == halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_VXLAN {
		fmt.Printf("%-10s%-10s",
			IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetVxlanInfo().GetLocalTep()),
			IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetVxlanInfo().GetRemoteTep()))
	} else {
		fmt.Printf("%-10s%-10s",
			IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetGreInfo().GetSource()),
			IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetGreInfo().GetDestination()))
	}

	imnIndices := resp.GetStatus().GetTunnelInfo().GetInpMapNatIdx()
	imnStr := fmt.Sprintf("%d,%d,%d", imnIndices[0], imnIndices[1], imnIndices[2])
	imtIndices := resp.GetStatus().GetTunnelInfo().GetInpMapTnlIdx()
	imtStr := fmt.Sprintf("%d,%d,%d", imtIndices[0], imtIndices[1], imtIndices[2])
	fmt.Printf("%-10s%-10s%-10d\n", imnStr, imtStr,
		resp.GetStatus().GetTunnelInfo().GetTunnelRwIdx())
}

// IPAddrToStr converts HAL proto IP address to string
func IPAddrToStr(ipAddr *halproto.IPAddress) string {
	if ipAddr.GetIpAf() == halproto.IPAddressFamily_IP_AF_INET {
		v4Addr := ipAddr.GetV4Addr()
		ip := make(net.IP, 4)
		binary.BigEndian.PutUint32(ip, v4Addr)
		return ip.String()
		/*
			v4Addr := ipAddr.GetV4Addr()
			v4Str := fmt.Sprintf("%d.%d.%d.%d", ((v4Addr >> 24) & 0xff), ((v4Addr >> 16) & 0xff),
				((v4Addr >> 8) & 0xff), (v4Addr & 0xff))
			return v4Str
		*/
	}
	v6Addr := ipAddr.GetV6Addr()
	ip := make(net.IP, 16)
	copy(ip, v6Addr)
	return ip.String()
	/*
		v6Addr := ipAddr.GetV6Addr()
		v6Str := fmt.Sprintf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
			v6Addr[0], v6Addr[1], v6Addr[2], v6Addr[3],
			v6Addr[4], v6Addr[5], v6Addr[6], v6Addr[7],
			v6Addr[8], v6Addr[9], v6Addr[10], v6Addr[11],
			v6Addr[12], v6Addr[13], v6Addr[14], v6Addr[15])
		return v6Str
	*/

}

func tnnlEncTypeToStr(encType halproto.IfTunnelEncapType) string {
	switch encType {
	case halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_VXLAN:
		return "Vxlan"
	case halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_GRE:
		return "Gre"
	default:
		return "Invalid"
	}
}
