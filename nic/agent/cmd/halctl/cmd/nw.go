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

var nwShowCmd = &cobra.Command{
	Use:   "network",
	Short: "nw",
	Long:  "shows network",
	Run:   nwShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nwShowCmd)
}

func nwShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	// Get all Nw
	req := &halproto.NetworkGetRequest{}
	networkGetReqMsg := &halproto.NetworkGetRequestMsg{
		Request: []*halproto.NetworkGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NetworkGet(context.Background(), networkGetReqMsg)
	if err != nil {
		log.Errorf("Getting Network failed. %v", err)
	}

	// Print Header
	nwShowHeader()

	// Print NHs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		nwShowOneResp(resp)
	}
}

func nwShowHeader() {
	hdrLine := strings.Repeat("-", 60)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-24s%-24s\n",
		"VrfId", "NWHandle", "Router MAC", "SGHandle")
	fmt.Println(hdrLine)
}

func nwShowOneResp(resp *halproto.NetworkGetResponse) {
	spec := resp.GetSpec()
	status := resp.GetStatus()

	sgKhList := spec.GetSgKeyHandle()
	sgKhStr := ""
	first := true
	for _, sgKh := range sgKhList {
		if first == true {
			first = false
			sgKhStr += fmt.Sprintf("%d", sgKh.GetSecurityGroupHandle())
		} else {
			sgKhStr += fmt.Sprintf(", %d", sgKh.GetSecurityGroupHandle())
		}
	}

	fmt.Printf("%-10d%-10d%-24s%-24s\n",
		spec.GetKeyOrHandle().GetNwKey().GetVrfKeyHandle().GetVrfId(),
		status.GetNwHandle(),
		utils.MactoStr(spec.GetRmac()),
		sgKhStr)
}
