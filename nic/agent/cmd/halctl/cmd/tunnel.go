//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	tunnelID       uint64
	tunnelDetailID uint64
)

var tunnelShowCmd = &cobra.Command{
	Use:   "tunnel",
	Short: "show tunnel interface information",
	Long:  "show tunnel interface object information",
	Run:   tunnelShowCmdHandler,
}

var tunnelSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "show spec information about tunnel interface",
	Long:  "show spec information about tunnel interface object",
	Run:   tunnelShowCmdHandler,
}

var tunnelStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show status information about tunnel interface",
	Long:  "show status information about tunnel interface object",
	Run:   tunnelShowStatusCmdHandler,
}

var tunnelDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed information about tunnel interface",
	Long:  "show detailed information about tunnel interface object",
	Run:   tunnelDetailShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(tunnelShowCmd)
	tunnelShowCmd.AddCommand(tunnelDetailShowCmd)
	tunnelShowCmd.AddCommand(tunnelSpecShowCmd)
	tunnelShowCmd.AddCommand(tunnelStatusShowCmd)

	tunnelShowCmd.Flags().Uint64Var(&tunnelID, "id", 1, "Specify if-id")
	tunnelDetailShowCmd.Flags().Uint64Var(&tunnelDetailID, "id", 1, "Specify if-id")
}

func handleTunnelShowCmd(cmd *cobra.Command, spec bool, status bool) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	defer c.Close()

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
		fmt.Printf("Getting if failed. %v\n", err)
		return
	}

	// Print Header
	if spec == true {
		tunnelShowHeader()
	} else if status == true {
		tunnelShowStatusHeader()
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		if spec == true {
			tunnelShowOneResp(resp)
		} else if status == true {
			tunnelShowStatusOneResp(resp)
		}
	}
}

func tunnelShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleTunnelShowCmd(cmd, true, false)
}

func tunnelShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleTunnelShowCmd(cmd, false, true)
}

func tunnelDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

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
		fmt.Printf("Getting if failed. %v\n", err)
		return
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func tunnelShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:       Interface ID                Ifype:    Interface type\n")
	fmt.Printf("EncType:  Tunnel Encap type           LTep:     Local Tep IP\n")
	fmt.Printf("RTep:     Remote TEP IP               VrfId:    Vrf ID\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 60)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "IfType", "EncType", "LTep", "RTep", "VrfId")
	fmt.Println(hdrLine)
}

func tunnelShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_TUNNEL {
		return
	}
	encType := resp.GetSpec().GetIfTunnelInfo().GetEncapType()
	fmt.Printf("%-10d%-10s%-10s",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
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
	fmt.Printf("%-10d\n",
		resp.GetSpec().GetIfTunnelInfo().GetVrfKeyHandle().GetVrfId())
}

func tunnelShowStatusHeader() {
	fmt.Printf("\n")
	fmt.Printf("Handle:   IF's handle                 Status:  IF's status\n")
	fmt.Printf("IMNative: Input Map. Native Idx       IMTunnel: Input Map. Tunnel Idx\n")
	fmt.Printf("RWIdx:    Rewrite Index\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 50)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s\n",
		"Handle", "Status", "IMNative", "IMTunnel", "RWIdx")
	fmt.Println(hdrLine)
}

func tunnelShowStatusOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_TUNNEL {
		return
	}
	fmt.Printf("%-10d%-10s",
		resp.GetStatus().GetIfHandle(),
		strings.ToLower(strings.Replace(resp.GetStatus().GetIfStatus().String(), "IF_STATUS_", "", -1)))

	imnIndices := resp.GetStatus().GetTunnelInfo().GetInpMapNatIdx()
	imnStr := fmt.Sprintf("%d,%d,%d", imnIndices[0], imnIndices[1], imnIndices[2])
	imtIndices := resp.GetStatus().GetTunnelInfo().GetInpMapTnlIdx()
	imtStr := fmt.Sprintf("%d,%d,%d", imtIndices[0], imtIndices[1], imtIndices[2])
	fmt.Printf("%-10s%-10s%-10d\n", imnStr, imtStr,
		resp.GetStatus().GetTunnelInfo().GetTunnelRwIdx())
}
