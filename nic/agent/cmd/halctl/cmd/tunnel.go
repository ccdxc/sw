//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	tunnelID       uint64
	tunnelDetailID uint64
)

var tunnelShowCmd = &cobra.Command{
	Use:   "tunnel",
	Short: "tunnel",
	Long:  "shows tunnel",
	Run:   tunnelShowCmdHandler,
}

var tunnelDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "detail",
	Long:  "shows tunnel detail",
	Run:   tunnelDetailShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(tunnelShowCmd)
	tunnelShowCmd.AddCommand(tunnelDetailShowCmd)

	tunnelShowCmd.Flags().Uint64Var(&tunnelID, "id", 1, "Specify if-id")
	tunnelDetailShowCmd.Flags().Uint64Var(&tunnelDetailID, "id", 1, "Specify if-id")
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

func tunnelDetailShowCmdHandler(cmd *cobra.Command, args []string) {
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
					InterfaceId: tunnelDetailID,
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

	// Print IFs
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
		utils.TnnlEncTypeToStr(encType))
	if encType == halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_VXLAN {
		fmt.Printf("%-10s%-10s",
			utils.IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetVxlanInfo().GetLocalTep()),
			utils.IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetVxlanInfo().GetRemoteTep()))
	} else {
		fmt.Printf("%-10s%-10s",
			utils.IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetGreInfo().GetSource()),
			utils.IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetGreInfo().GetDestination()))
	}

	imnIndices := resp.GetStatus().GetTunnelInfo().GetInpMapNatIdx()
	imnStr := fmt.Sprintf("%d,%d,%d", imnIndices[0], imnIndices[1], imnIndices[2])
	imtIndices := resp.GetStatus().GetTunnelInfo().GetInpMapTnlIdx()
	imtStr := fmt.Sprintf("%d,%d,%d", imtIndices[0], imtIndices[1], imtIndices[2])
	fmt.Printf("%-10s%-10s%-10d\n", imnStr, imtStr,
		resp.GetStatus().GetTunnelInfo().GetTunnelRwIdx())
}
