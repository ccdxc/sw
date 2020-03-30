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
	tunnelID uint64
)

var tunnelShowCmd = &cobra.Command{
	Use:   "tunnel",
	Short: "show tunnel interface information",
	Long:  "show tunnel interface object information",
	Run:   tunnelShowCmdHandler,
}

var mplsoudpShowCmd = &cobra.Command{
	Use:   "mplsoudp",
	Short: "Show MPLSoUDP tunnel interface information",
	Long:  "Show MPLSoUDP tunnel interface information",
	Run:   mplsoudpShowCmdHandler,
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

func init() {
	ifShowCmd.AddCommand(tunnelShowCmd)
	tunnelShowCmd.AddCommand(tunnelSpecShowCmd)
	tunnelShowCmd.AddCommand(tunnelStatusShowCmd)
	tunnelShowCmd.AddCommand(mplsoudpShowCmd)

	tunnelShowCmd.PersistentFlags().Bool("yaml", false, "Output in yaml")
	tunnelShowCmd.PersistentFlags().Uint64Var(&tunnelID, "id", 1, "Specify if-id")
}

func handleTunnelShowCmd(cmd *cobra.Command, spec bool, status bool, mplsoudp bool) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c)

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
	} else if mplsoudp == true {
		mplsoudpShowHeader()
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		if spec == true {
			tunnelShowOneResp(resp)
		} else if status == true {
			tunnelShowStatusOneResp(resp)
		} else if mplsoudp == true {
			mplsoudpShowOneResp(resp)
		}
	}
}

func mplsoudpShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		tunnelDetailShowCmdHandler(cmd, args, true)
		return
	}

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleTunnelShowCmd(cmd, false, false, true)
}

func tunnelShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		tunnelDetailShowCmdHandler(cmd, args, false)
		return
	}

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleTunnelShowCmd(cmd, true, false, false)
}

func tunnelShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleTunnelShowCmd(cmd, false, true, false)
}

func tunnelDetailShowCmdHandler(cmd *cobra.Command, args []string, mplsoudp bool) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c)

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

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		if resp.GetSpec().GetType() != halproto.IfType_IF_TYPE_TUNNEL {
			continue
		}
		if mplsoudp == true {
			if resp.GetSpec().GetIfTunnelInfo().GetEncapType() != halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS {
				continue
			}
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func mplsoudpShowHeader() {
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 232)
	fmt.Println(hdrLine)
	fmt.Printf("%-22s%-10s%-16s%-32s%-14s%-16s%-12s%-20s%-10s%-10s%-18s%-18s%-18s%-16s\n",
		"Id", "EncType", "SubstrateIP", "OverlayIP", "InMplsTag", "TunnelDestIP",
		"OutMplsTag", "SourceGW", "IngressBW", "EgressBW", "GWMacDA", "PfMac", "OverlayMac", "LifName")
	fmt.Println(hdrLine)
}

func mplsoudpShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_TUNNEL {
		return
	}
	encType := resp.GetSpec().GetIfTunnelInfo().GetEncapType()
	if encType != halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS {
		return
	}
	ifID := resp.GetSpec().GetKeyOrHandle().GetInterfaceId()
	mpls := resp.GetSpec().GetIfTunnelInfo().GetPropMplsInfo()
	subIP := utils.IPAddrToStr(mpls.GetSubstrateIp())
	overlayIP := mpls.GetOverlayIp()
	index := 0
	overlayStr := ""
	for index < len(overlayIP) {
		overlayStr += utils.IPAddrToStr(overlayIP[index])
		index++
		if index < len(overlayIP) {
			overlayStr += ","
		}
	}
	mplsTag := mpls.GetMplsIf()
	index = 0
	inMpls := ""
	for index < len(mplsTag) {
		inMpls += fmt.Sprintf("%d", mplsTag[index].GetLabel())
		index++
		if index < len(mplsTag) {
			inMpls += ","
		}
	}
	outMpls := mpls.GetMplsTag().GetLabel()
	tunnelDestIP := utils.IPAddrToStr(mpls.GetTunnelDestIp())
	sourceGW := utils.IPAddrToStr(mpls.GetSourceGw().GetPrefix().GetIpv4Subnet().GetAddress()) + "/" + fmt.Sprintf("%d", mpls.GetSourceGw().GetPrefix().GetIpv4Subnet().GetPrefixLen())
	gwMac := utils.MactoStr(mpls.GetGwMacDa())
	overlayMac := utils.MactoStr(mpls.GetOverlayMac())
	pfMac := utils.MactoStr(mpls.GetPfMac())

	fmt.Printf("%-22d%-10s%-16s%-32s%-14s%-16s%-12d%-20s%-10d%-10d%-18s%-18s%-18s%-16s\n",
		ifID, utils.TnnlEncTypeToStr(encType), subIP, overlayStr, inMpls, tunnelDestIP, outMpls,
		sourceGW, mpls.GetIngressBw(), mpls.GetEgressBw(), gwMac, pfMac, overlayMac, mpls.GetLifName())
}

func tunnelShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:       Interface ID                EncType:  Tunnel Encap type\n")
	fmt.Printf("LTep:     Local Tep IP                RTep:     Remote TEP IP\n")
	fmt.Printf("VrfId:    Vrf ID\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 67)
	fmt.Println(hdrLine)
	fmt.Printf("%-15s%-10s%-15s%-15s%-10s\n",
		"Id", "EncType", "LTep", "RTep", "VrfId")
	fmt.Println(hdrLine)
}

func tunnelShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_TUNNEL {
		return
	}
	encType := resp.GetSpec().GetIfTunnelInfo().GetEncapType()
	ifID := fmt.Sprintf("tunnel-%d", resp.GetSpec().GetKeyOrHandle().GetInterfaceId())
	fmt.Printf("%-15s%-10s",
		ifID,
		utils.TnnlEncTypeToStr(encType))
	if encType == halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_VXLAN {
		fmt.Printf("%-15s%-15s",
			utils.IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetVxlanInfo().GetLocalTep()),
			utils.IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetVxlanInfo().GetRemoteTep()))
	} else if encType == halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_GRE {
		fmt.Printf("%-15s%-15s",
			utils.IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetGreInfo().GetSource()),
			utils.IPAddrToStr(resp.GetSpec().GetIfTunnelInfo().GetGreInfo().GetDestination()))
	} else if encType == halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS {
		fmt.Printf("%-15s%-15s", "-", "-")
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
