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

var natmappingShowCmd = &cobra.Command{
	Use:   "nat-mapping",
	Short: "nat-mapping",
	Long:  "shows nat-mapping",
	Run:   natmappingShowCmdHandler,
}

func init() {
	showCmd.AddCommand(natmappingShowCmd)
}

func natmappingShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNatClient(c.ClientConn)

	// Get all Nat Pools
	req := &halproto.NatMappingGetRequest{}
	natmappingGetReqMsg := &halproto.NatMappingGetRequestMsg{
		Request: []*halproto.NatMappingGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NatMappingGet(context.Background(), natmappingGetReqMsg)
	if err != nil {
		log.Errorf("Getting NAT Mapping failed. %v", err)
	}

	// Print Header
	natmappingShowHeader()

	// Print Routes
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		natmappingShowOneResp(resp)
	}
}

func natmappingShowHeader() {
	hdrLine := strings.Repeat("-", 80)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-24s%-15s%-10s%-10s\n",
		"vrfId", "SvcIP:Port", "MappedIP", "NatPoolId", "BiDir")
	fmt.Println(hdrLine)
}

func natmappingShowOneResp(resp *halproto.NatMappingGetResponse) {
	spec := resp.GetSpec()
	status := resp.GetStatus()
	svc := spec.GetKeyOrHandle().GetSvc()

	ipPortStr := utils.IPAddrToStr(svc.GetIpAddr())
	ipPortStr += fmt.Sprintf(":%d", svc.GetPort())

	mappedIP := utils.IPAddrToStr(status.GetMappedIp())

	fmt.Printf("%-10d%-24s%-15s%-10d%-10t\n",
		svc.GetVrfKh().GetVrfId(),
		ipPortStr, mappedIP,
		spec.GetNatPool().GetPoolKey().GetPoolId(),
		spec.GetBidir())
}
