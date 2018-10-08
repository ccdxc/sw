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
	cpuID       uint64
	cpuDetailID uint64
)

var cpuShowCmd = &cobra.Command{
	Use:   "cpu",
	Short: "show CPU interface information",
	Long:  "show CPU interface object information",
	Run:   cpuShowCmdHandler,
}

var cpuSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "show CPU interface spec information",
	Long:  "show CPU interface object spec information",
	Run:   cpuShowCmdHandler,
}

var cpuStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show CPU interface status information",
	Long:  "show CPU interface object status information",
	Run:   cpuShowStatusCmdHandler,
}

var cpuDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed CPU interface information",
	Long:  "show detailed CPU interface object information",
	Run:   cpuDetailShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(cpuShowCmd)
	cpuShowCmd.AddCommand(cpuSpecShowCmd)
	cpuShowCmd.AddCommand(cpuStatusShowCmd)
	cpuShowCmd.AddCommand(cpuDetailShowCmd)

	cpuShowCmd.Flags().Uint64Var(&cpuID, "id", 1, "Specify if-id")
	cpuDetailShowCmd.Flags().Uint64Var(&cpuDetailID, "id", 1, "Specify if-id")
}

func handleCPUShowCmd(cmd *cobra.Command, spec bool, status bool) {
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
					InterfaceId: cpuID,
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
		cpuShowHeader()
	} else if status == true {
		cpuShowStatusHeader()
	}

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		if spec == true {
			cpuShowOneResp(resp)
		} else if status == true {
			cpuShowStatusOneResp(resp)
		}
	}
}

func cpuShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleCPUShowCmd(cmd, false, true)
}

func cpuShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleCPUShowCmd(cmd, true, false)
}

func cpuDetailShowCmdHandler(cmd *cobra.Command, args []string) {
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
					InterfaceId: cpuDetailID,
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

func cpuShowStatusHeader() {
	fmt.Printf("\n")
	fmt.Printf("Handle:  IF's handle        Status:  IF's status\n")
	fmt.Printf("LportId: IF's Lport\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s\n",
		"Handle", "Status", "LportId")
	fmt.Println(hdrLine)
}

func cpuShowStatusOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_CPU {
		return
	}
	fmt.Printf("%-10d%-10s%-10d\n",
		resp.GetStatus().GetIfHandle(),
		strings.ToLower(strings.Replace(resp.GetStatus().GetIfStatus().String(), "IF_STATUS_", "", -1)),
		resp.GetStatus().GetCpuInfo().GetCpuLportId())
}

func cpuShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:      Interface ID         IfType: Interface type\n")
	fmt.Printf("Status:  IF's status          LifId: LIF's ID\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 40)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s\n",
		"Id", "IfType", "Status", "LifID")
	fmt.Println(hdrLine)
}

func cpuShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_CPU {
		return
	}
	fmt.Printf("%-10d%-10s%-10s%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		ifTypeToStr(ifType),
		strings.ToLower(strings.Replace(resp.GetSpec().GetAdminStatus().String(), "IF_STATUS_", "", -1)),
		resp.GetSpec().GetIfCpuInfo().GetLifKeyOrHandle().GetLifId())
}
