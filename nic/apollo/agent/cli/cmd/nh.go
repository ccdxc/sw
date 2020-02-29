//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	// nhID holds Nexthop ID
	nhID   string
	nhType string
)

var nhShowCmd = &cobra.Command{
	Use:   "nexthop",
	Short: "show nexthop information",
	Long:  "show nexthop object information",
	Run:   nhShowCmdHandler,
}

var nhGroupShowCmd = &cobra.Command{
	Use:   "nexthop-group",
	Short: "show nexthop group information",
	Long:  "show nexthop group object information",
	Run:   nhGroupShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nhShowCmd)
	nhShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	nhShowCmd.Flags().StringVar(&nhType, "type", "overlay", "Specify nexthop type (overlay, underlay or ip)")
	nhShowCmd.Flags().StringVarP(&nhID, "id", "i", "", "Specify nexthop ID")

	showCmd.AddCommand(nhGroupShowCmd)
	nhGroupShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	nhGroupShowCmd.Flags().StringVarP(&nhID, "id", "i", "", "Specify nexthop group ID")
}

func nhGroupShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewNhSvcClient(c)

	var req *pds.NhGroupGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific NhGroup
		req = &pds.NhGroupGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(nhID).Bytes()},
		}
	} else {
		req = &pds.NhGroupGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.NhGroupGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting nexthop group failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Nexthop groups
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printNhGroupHeader()
		for _, resp := range respMsg.Response {
			printNhGroup(resp)
		}
	}
}

func printNhGroupHeader() {
	hdrLine := strings.Repeat("-", 118)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-10s%-16s%-10s%-12s%-12s%-18s\n",
		"Id", "HwID", "Type", "#Members",
		"MemberPort", "MemberVLAN", "MemberMAC")
	fmt.Println(hdrLine)
}

func printNhGroup(resp *pds.NhGroup) {
	spec := resp.GetSpec()
	status := resp.GetStatus()
	typeStr := strings.Replace(spec.GetType().String(), "NEXTHOP_GROUP_TYPE_", "", -1)
	typeStr = strings.Replace(typeStr, "_", "-", -1)
	memberSpec := spec.GetMembers()
	memberStatus := status.GetMembers()
	first := true
	numMembers := len(memberSpec)

	if typeStr != "UNDERLAY-ECMP" {
		return
	}

	for i := 0; i < numMembers; i++ {
		if first {
			fmt.Printf("%-40s%-10d%-16s%-10d%-12d%-12d%-18s\n",
				uuid.FromBytesOrNil(spec.GetId()).String(),
				status.GetHwId(), typeStr, numMembers,
				memberStatus[i].GetUnderlayNhInfo().GetPort(),
				memberStatus[i].GetUnderlayNhInfo().GetVlan(),
				utils.MactoStr(memberSpec[i].GetUnderlayNhInfo().GetUnderlayMAC()))
			first = false
		} else {
			fmt.Printf("%-76s%-12d%-12d%-18s\n",
				"",
				memberStatus[i].GetUnderlayNhInfo().GetPort(),
				memberStatus[i].GetUnderlayNhInfo().GetVlan(),
				utils.MactoStr(memberSpec[i].GetUnderlayNhInfo().GetUnderlayMAC()))
		}
	}
}

func nhShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewNhSvcClient(c)

	var req *pds.NexthopGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific Nexthop
		req = &pds.NexthopGetRequest{
			Gettype: &pds.NexthopGetRequest_Id{
				Id: uuid.FromStringOrNil(nhID).Bytes(),
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("type") {
		if checkNhTypeValid(nhType) != true {
			fmt.Printf("Invalid nexthop type\n")
			return
		}
		// Get specific Nexthop
		req = &pds.NexthopGetRequest{
			Gettype: &pds.NexthopGetRequest_Type{
				Type: nhTypeToPdsNhType(nhType),
			},
		}
	} else {
		fmt.Printf("--type or --id argument needed\n")
		return
	}

	// PDS call
	respMsg, err := client.NexthopGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting nexthop failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Nexthops
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		first := true
		for _, resp := range respMsg.Response {
			if first == true {
				printNexthopHeader(resp)
				first = false
			}
			printNexthop(resp)
		}
	}
}

func checkNhTypeValid(nh string) bool {
	switch nh {
	case "ip":
		return true
	case "underlay":
		return true
	case "overlay":
		return true
	default:
		return false
	}
}

func nhTypeToPdsNhType(nh string) pds.NexthopType {
	switch nh {
	case "ip":
		return pds.NexthopType_NEXTHOP_TYPE_IP
	case "underlay":
		return pds.NexthopType_NEXTHOP_TYPE_UNDERLAY
	case "overlay":
		return pds.NexthopType_NEXTHOP_TYPE_OVERLAY
	default:
		return pds.NexthopType_NEXTHOP_TYPE_NONE
	}
}

func printNexthopIPHeader() {
	hdrLine := strings.Repeat("-", 135)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-10s%-40s%-18s%-7s%-20s\n", "Id", "Type", "VPCId", "IP", "Vlan", "MAC")
	fmt.Println(hdrLine)
}

func printNexthopOverlayHeader() {
	hdrLine := strings.Repeat("-", 80)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-40s\n", "Id", "TunnelId")
	fmt.Println(hdrLine)
}

func printNexthopUnderlayHeader() {
	hdrLine := strings.Repeat("-", 82)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-12s%-12s%-18s\n", "Id", "Port", "Vlan", "UnderlayMAC")
	fmt.Println(hdrLine)
}

func printNexthopHeader(nh *pds.Nexthop) {
	spec := nh.GetSpec()
	switch spec.GetNhinfo().(type) {
	case *pds.NexthopSpec_IPNhInfo:
		printNexthopIPHeader()
	case *pds.NexthopSpec_TunnelId:
		printNexthopOverlayHeader()
	case *pds.NexthopSpec_UnderlayNhInfo:
		printNexthopUnderlayHeader()
	}
}

func printNexthop(nh *pds.Nexthop) {
	spec := nh.GetSpec()
	status := nh.GetStatus()
	switch spec.GetNhinfo().(type) {
	case *pds.NexthopSpec_IPNhInfo:
		{
			nhInfo := spec.GetIPNhInfo()
			fmt.Printf("%-40s%-10s%-40s%-18s%-7d%-20s\n",
				spec.GetId(), "IP", nhInfo.GetVPCId(),
				utils.IPAddrToStr(nhInfo.GetIP()),
				nhInfo.GetVlan(),
				utils.MactoStr(nhInfo.GetMac()))
		}
	case *pds.NexthopSpec_TunnelId:
		{
			fmt.Printf("%-40s%-40s\n",
				uuid.FromBytesOrNil(spec.GetId()).String(),
				uuid.FromBytesOrNil(spec.GetTunnelId()).String())
		}
	case *pds.NexthopSpec_UnderlayNhInfo:
		{
			info := spec.GetUnderlayNhInfo()
			fmt.Printf("%-40s%-12d%-12d%-18s\n",
				uuid.FromBytesOrNil(spec.GetId()).String(),
				status.GetUnderlayNhInfo().GetPort(),
				status.GetUnderlayNhInfo().GetVlan(),
				utils.MactoStr(info.GetUnderlayMAC()))
		}
	}
}
