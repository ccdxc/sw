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
	nhShowCmd.Flags().Bool("summary", false, "Display number of objects")
	nhShowCmd.Flags().StringVar(&nhType, "type", "overlay", "Specify nexthop type (overlay, underlay or ip)")
	nhShowCmd.Flags().StringVarP(&nhID, "id", "i", "", "Specify nexthop ID")

	showCmd.AddCommand(nhGroupShowCmd)
	nhGroupShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	nhGroupShowCmd.Flags().Bool("summary", false, "Display number of objects")
	nhGroupShowCmd.Flags().StringVar(&nhType, "type", "overlay-ecmp", "Specify nexthop-group type (overlay-ecmp or underlay-ecmp)")
	nhGroupShowCmd.Flags().StringVarP(&nhID, "id", "i", "", "Specify nexthop group ID")
}

func nhGroupShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
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
			Gettype: &pds.NhGroupGetRequest_Id{
				Id: uuid.FromStringOrNil(nhID).Bytes(),
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("type") {
		if checkNhGroupTypeValid(nhType) != true {
			fmt.Printf("Invalid nexthop group type\n")
			return
		}
		// Get specific NhGroup
		req = &pds.NhGroupGetRequest{
			Gettype: &pds.NhGroupGetRequest_Type{
				Type: nhGroupTypeToPdsNhGroupType(nhType),
			},
		}
	} else {
		fmt.Printf("--type or --id argument needed\n")
		return
	}

	// PDS call
	respMsg, err := client.NhGroupGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting nexthop group failed, err %v\n", err)
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
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printNhGroupSummary(len(respMsg.Response))
	} else {
		printNhGroupHeader(nhType)
		for _, resp := range respMsg.Response {
			printNhGroup(resp)
		}
		printNhGroupSummary(len(respMsg.Response))
	}
}

func checkNhGroupTypeValid(nh string) bool {
	switch nh {
	case "underlay-ecmp":
		return true
	case "overlay-ecmp":
		return true
	default:
		return false
	}
}

func nhGroupTypeToPdsNhGroupType(nh string) pds.NhGroupType {
	switch nh {
	case "underlay-ecmp":
		return pds.NhGroupType_NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP
	case "overlay-ecmp":
		return pds.NhGroupType_NEXTHOP_GROUP_TYPE_OVERLAY_ECMP
	default:
		return pds.NhGroupType_NEXTHOP_GROUP_TYPE_NONE
	}
}

func printNhGroupUnderlayHeader() {
	hdrLine := strings.Repeat("-", 118)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-10s%-16s%-10s%-12s%-12s%-18s\n",
		"Id", "HwID", "Type", "#Members",
		"MemberPort", "MemberVLAN", "MemberMAC")
	fmt.Println(hdrLine)
}

func printNhGroupOverlayHeader() {
	hdrLine := strings.Repeat("-", 104)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-14s%-10s%-40s\n",
		"Id", "Type", "#Members", "TunnelIP")
	fmt.Println(hdrLine)
}

func printNhGroupHeader(nh string) {
	switch nh {
	case "underlay-ecmp":
		printNhGroupUnderlayHeader()
	case "overlay-ecmp":
		printNhGroupOverlayHeader()
	}
}

func printNhGroupSummary(count int) {
	fmt.Printf("\nNo. of nexthop groups : %d\n\n", count)
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

	for i := 0; i < numMembers; i++ {
		switch typeStr {
		case "UNDERLAY-ECMP":
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
			break
		case "OVERLAY-ECMP":
			if first {
				fmt.Printf("%-40s%-14s%-10d%-40s\n",
					uuid.FromBytesOrNil(spec.GetId()).String(),
					typeStr, numMembers,
					utils.IPAddrToStr(memberStatus[i].GetOverlayNhInfo().GetTunnelIP()))
				first = false
			} else {
				fmt.Printf("%-64s%-40s\n",
					"",
					utils.IPAddrToStr(memberStatus[i].GetOverlayNhInfo().GetTunnelIP()))
			}
			break
		default:
			break
		}
	}
}

func nhShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
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
		fmt.Printf("Getting nexthop failed, err %v\n", err)
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
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printNexthopSummary(len(respMsg.Response))
	} else {
		first := true
		for _, resp := range respMsg.Response {
			if first == true {
				printNexthopHeader(resp)
				first = false
			}
			printNexthop(resp)
		}
		printNexthopSummary(len(respMsg.Response))
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

func printNexthopSummary(count int) {
	fmt.Printf("\nNo. of nexthops : %d\n\n", count)
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
	case *pds.NexthopSpec_OverlayNhInfo:
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
	case *pds.NexthopSpec_OverlayNhInfo:
		{
			nhInfo := spec.GetOverlayNhInfo()
			fmt.Printf("%-40s%-40s\n",
				uuid.FromBytesOrNil(nhInfo.GetTunnelId()).String(),
				utils.IPAddrToStr(status.GetOverlayNhInfo().GetTunnelIP()))
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
