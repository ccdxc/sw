//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

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
	Use:   "nh",
	Short: "show nexthop information",
	Long:  "show nexthop object information",
	Run:   nhShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nhShowCmd)
	nhShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	nhShowCmd.Flags().StringVar(&nhType, "type", "overlay", "Specify nexthop type (overlay, underlay or ip)")
	nhShowCmd.Flags().StringVarP(&nhID, "id", "i", "", "Specify nexthop ID")
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
	if cmd.Flags().Changed("id") {
		// Get specific Nexthop
		req = &pds.NexthopGetRequest{
			Gettype: &pds.NexthopGetRequest_Id{
				Id: []byte(nhID),
			},
		}
	} else if cmd.Flags().Changed("type") {
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
	if cmd.Flags().Changed("yaml") {
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
	hdrLine := strings.Repeat("-", 127)
	fmt.Println(hdrLine)
	fmt.Printf("%-36s%-10s%-i36s%-18s%-7s%-20s\n", "Id", "Type", "VPCId", "IP", "Vlan", "MAC")
	fmt.Println(hdrLine)
}

func printNexthopOverlayHeader() {
	hdrLine := strings.Repeat("-", 74)
	fmt.Println(hdrLine)
	fmt.Printf("%-36s%-36s\n", "Id", "TunnelId")
	fmt.Println(hdrLine)
}

func printNexthopUnderlayHeader() {
	hdrLine := strings.Repeat("-", 86)
	fmt.Println(hdrLine)
	fmt.Printf("%-36s%-36s%-20s\n", "Id", "L3Intf", "UnderlayMAC")
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
	switch spec.GetNhinfo().(type) {
	case *pds.NexthopSpec_IPNhInfo:
		{
			nhInfo := spec.GetIPNhInfo()
			fmt.Printf("%-36s%-10s%-36s%-18s%-7d%-20s\n",
				spec.GetId(), "IP", nhInfo.GetVPCId(),
				utils.IPAddrToStr(nhInfo.GetIP()),
				nhInfo.GetVlan(),
				utils.MactoStr(nhInfo.GetMac()))
		}
	case *pds.NexthopSpec_TunnelId:
		{
			fmt.Printf("%-36s%-36s\n",
				string(spec.GetId()), string(spec.GetTunnelId()))
		}
	case *pds.NexthopSpec_UnderlayNhInfo:
		{
			info := spec.GetUnderlayNhInfo()
			fmt.Printf("%-36s%-10d%-20s\n",
				string(spec.GetId()), info.GetL3Interface(),
				utils.MactoStr(info.GetUnderlayMAC()))
		}
	}
}
