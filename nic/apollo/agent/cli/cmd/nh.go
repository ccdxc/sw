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
	nhID uint32
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
	nhShowCmd.Flags().Uint32VarP(&nhID, "id", "i", 0, "Specify nexthop ID")
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
			Id: []uint32{nhID},
		}
	} else {
		// Get all Nexthops
		req = &pds.NexthopGetRequest{
			Id: []uint32{},
		}
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
		printNexthopHeader()
		for _, resp := range respMsg.Response {
			printNexthop(resp)
		}
	}
}

func printNexthopHeader() {
	hdrLine := strings.Repeat("-", 67)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-10s%-6s%-18s%-7s%-20s\n", "Id", "Type", "VPCId", "IP", "Vlan", "MAC")
	fmt.Println(hdrLine)
}

func printNexthop(nh *pds.Nexthop) {
	//spec := nh.GetSpec()
	//if spec.GetType() == pds.NexthopType_NEXTHOP_TYPE_IP {
		//nhInfo := spec.GetIPNhInfo()
		//fmt.Printf("%-6d%-10s%-6d%-18s%-7d%-20s\n",
			//spec.GetId(),
			//strings.Replace(spec.GetType().String(), "NEXTHOP_TYPE_", "", -1),
			//nhInfo.GetVPCId(),
			//utils.IPAddrToStr(nhInfo.GetIP()),
			//nhInfo.GetVlan(),
			//utils.MactoStr(nhInfo.GetMac()))
	//} else {
		//fmt.Printf("%-6d%-10s%-6d%-18s%-7d%-20s\n",
			//spec.GetId(),
			//strings.Replace(spec.GetType().String(), "NEXTHOP_TYPE_", "", -1),
			//"-", "-", "-", "-")
	//}
}
