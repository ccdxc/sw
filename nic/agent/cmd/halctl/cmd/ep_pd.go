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

var epPdShowCmd = &cobra.Command{
	Use:   "pd",
	Short: "pd",
	Long:  "shows ep pd",
	Run:   epPdShowCmdHandler,
}

func init() {
	epShowCmd.AddCommand(epPdShowCmd)
}

func epPdShowCmdHandler(cmd *cobra.Command, args []string) {

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
	epPdShowHeader(cmd, args)

	// Print EPs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		epPdShowOneResp(resp)
	}
}

func epPdShowHeader(cmd *cobra.Command, args []string) {
	hdrLine := strings.Repeat("-", 100)
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-12s%-12s%-36s\n",
		"EPHandle", "L2SegID", "MacTblIdx", "RwTblIdx")
	fmt.Println(hdrLine)
}

func epPdShowOneResp(resp *halproto.EndpointGetResponse) {
	epd := resp.GetStatus().GetEpdStatus()

	rwTblStr := ""
	first := true
	rwTblIdx := epd.GetRwTblIdx()

	for _, val := range rwTblIdx {
		if first == true {
			first = false
			rwTblStr += fmt.Sprintf("%d", val)
		} else {
			rwTblStr += fmt.Sprintf(", %d", val)
		}
	}

	regMacTblIdx := epd.GetRegMacTblIdx()
	regMacTblStr := ""
	if regMacTblIdx == 0xFFFFFFFF {
		regMacTblStr = "-"
	} else {
		regMacTblStr = fmt.Sprintf("%d", regMacTblIdx)
	}

	fmt.Printf("%-12d%-12d%-12s%-36s\n",
		resp.GetStatus().GetEndpointHandle(),
		resp.GetSpec().GetKeyOrHandle().GetEndpointKey().GetL2Key().GetL2SegmentKeyHandle().GetSegmentId(),
		regMacTblStr,
		rwTblStr)
}
