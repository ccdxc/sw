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
	natpoolHandle uint64
)
var natpoolShowCmd = &cobra.Command{
	Use:   "nat-pool",
	Short: "nat-pool",
	Long:  "shows nat-pool",
	Run:   natpoolShowCmdHandler,
}

func init() {
	showCmd.AddCommand(natpoolShowCmd)

	natpoolShowCmd.Flags().Uint64Var(&natpoolHandle, "handle", 1, "Specify natpool handle")
}

func natpoolShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNatClient(c.ClientConn)

	var req *halproto.NatPoolGetRequest
	if cmd.Flags().Changed("handle") {
		req = &halproto.NatPoolGetRequest{
			KeyOrHandle: &halproto.NatPoolKeyHandle{
				KeyOrHandle: &halproto.NatPoolKeyHandle_PoolHandle{
					PoolHandle: natpoolHandle,
				},
			},
		}
	} else {
		// Get all Nat Pools
		req = &halproto.NatPoolGetRequest{}
	}
	natpoolGetReqMsg := &halproto.NatPoolGetRequestMsg{
		Request: []*halproto.NatPoolGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NatPoolGet(context.Background(), natpoolGetReqMsg)
	if err != nil {
		log.Errorf("Getting NAT Pool failed. %v", err)
	}

	// Print Header
	natpoolShowHeader()

	// Print Routes
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		natpoolShowOneResp(resp)
	}
}

func natpoolShowHeader() {
	hdrLine := strings.Repeat("-", 80)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-40s%-10s\n",
		"VrfId", "PoolId", "Handle", "IPRange", "PortRange")
	fmt.Println(hdrLine)
}

func natpoolShowOneResp(resp *halproto.NatPoolGetResponse) {
	fmt.Printf("%-10d%-10d%-10d",
		resp.GetSpec().GetKeyOrHandle().GetPoolKey().GetVrfKh().GetVrfId(),
		resp.GetSpec().GetKeyOrHandle().GetPoolKey().GetPoolId(),
		resp.GetStatus().GetPoolHandle())

	ipAddr := resp.GetSpec().GetAddress()
	ipAddrStr := ""
	if len(ipAddr) > 0 {
		for i := 0; i < len(ipAddr); i++ {
			ipAddrStr += utils.AddressToStr(ipAddr[0])
			ipAddrStr += ","
		}
	}
	ipAddrStr = ipAddrStr[:len(ipAddrStr)-1]

	fmt.Printf("%-40s%-10s\n",
		ipAddrStr,
		utils.L4PortRangeToStr(resp.GetSpec().GetPortRange()))
}
