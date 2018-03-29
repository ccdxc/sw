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

var epShowCmd = &cobra.Command{
	Use:   "ep",
	Short: "ep",
	Long:  "shows ep",
	Run:   epShowCmdHandler,
}

func init() {
	showCmd.AddCommand(epShowCmd)
}

func epShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewEndpointClient(c.ClientConn)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		log.Errorf("Getting Endpoint failed. %v", err)
	}

	// Print Header
	epShowHeader(cmd, args)

	// Print VRFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		epShowOneResp(resp)
	}
}

func epShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 120)
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-12s%-24s%-10s%-10s%-10s%-20s\n",
		"EPHandle", "L2SegID", "Mac", "IfId", "IsLocal", "NumIPs", "IPs")
	fmt.Println(hdrLine)
}

func epShowOneResp(resp *halproto.EndpointGetResponse) {
	epAddr := resp.GetStatus().GetIpAddress()
	ipStr := IPAddrToStr(epAddr[0].GetIpAddress())
	if len(epAddr) > 0 {
		for i := 1; i < len(epAddr); i++ {
			ipStr += ", "
			ipStr += IPAddrToStr(epAddr[i].GetIpAddress())
		}
	}
	macStr := MactoStr(resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetMacAddress())
	fmt.Printf("%-12d%-12d%-24s%-10d%-10t%-10d%-20s\n",
		resp.GetStatus().GetEndpointHandle(),
		resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetL2SegmentKeyHandle().GetSegmentId(),
		macStr,
		resp.GetSpec().GetEndpointAttrs().GetInterfaceKeyHandle().GetInterfaceId(),
		resp.GetStatus().GetIsEndpointLocal(),
		len(resp.GetStatus().GetIpAddress()),
		ipStr)
}

// MactoStr converts a uint64 to a MAC string
func MactoStr(mac uint64) string {
	var bytes [6]byte

	bytes[0] = byte(mac & 0xFF)
	bytes[1] = byte((mac >> 8) & 0xFF)
	bytes[2] = byte((mac >> 16) & 0xFF)
	bytes[3] = byte((mac >> 24) & 0xFF)
	bytes[4] = byte((mac >> 32) & 0xFF)
	bytes[5] = byte((mac >> 40) & 0xFF)

	macStr := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0])

	return macStr
}
