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
	nhID uint64
)

var nhShowCmd = &cobra.Command{
	Use:   "nexthop",
	Short: "nh",
	Long:  "shows nexthop",
	Run:   nhShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nhShowCmd)

	nhShowCmd.Flags().Uint64Var(&nhID, "id", 1, "Specify nexthop-id")
}

func nhShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	var req *halproto.NexthopGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.NexthopGetRequest{
			KeyOrHandle: &halproto.NexthopKeyHandle{
				KeyOrHandle: &halproto.NexthopKeyHandle_NexthopId{
					NexthopId: nhID,
				},
			},
		}
	} else {
		// Get all NHs
		req = &halproto.NexthopGetRequest{}
	}
	nexthopGetReqMsg := &halproto.NexthopGetRequestMsg{
		Request: []*halproto.NexthopGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NexthopGet(context.Background(), nexthopGetReqMsg)
	if err != nil {
		log.Errorf("Getting Nexthop failed. %v", err)
	}

	// Print Header
	nhShowHeader()

	// Print NHs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		nhShowOneResp(resp)
	}
}

func nhShowHeader() {
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s\n",
		"Id", "Handle", "EPHandle")
	fmt.Println(hdrLine)
}

func nhShowOneResp(resp *halproto.NexthopGetResponse) {
	fmt.Printf("%-10d%-10d%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetNexthopId(),
		resp.GetStatus().GetNexthopHandle(),
		resp.GetSpec().GetEpKeyOrHandle().GetEndpointHandle()) // TODO: Handle IF
}
