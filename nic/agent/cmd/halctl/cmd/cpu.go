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
	cpuID       uint64
	cpuDetailID uint64
)

var cpuShowCmd = &cobra.Command{
	Use:   "cpu",
	Short: "cpu",
	Long:  "shows cpu",
	Run:   cpuShowCmdHandler,
}

var cpuDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "detail",
	Long:  "shows cpu detail",
	Run:   cpuDetailShowCmdHandler,
}

func init() {
	ifShowCmd.AddCommand(cpuShowCmd)
	cpuShowCmd.AddCommand(cpuDetailShowCmd)

	cpuShowCmd.Flags().Uint64Var(&cpuID, "id", 1, "Specify if-id")
	cpuDetailShowCmd.Flags().Uint64Var(&cpuDetailID, "id", 1, "Specify if-id")
}

func cpuShowCmdHandler(cmd *cobra.Command, args []string) {
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
		log.Errorf("Getting if failed. %v", err)
	}

	// Print Header
	cpuShowHeader(cmd, args)

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		cpuShowOneResp(resp)
	}
	c.Close()
}

func cpuDetailShowCmdHandler(cmd *cobra.Command, args []string) {
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

func cpuShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("Id:    Interface ID         Handle:  IF's handle\n")
	fmt.Printf("Ifype: Interface type       LportId: IF's Lport\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 40)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "IfType", "LportId")
	fmt.Println(hdrLine)
}

func cpuShowOneResp(resp *halproto.InterfaceGetResponse) {
	ifType := resp.GetSpec().GetType()
	if ifType != halproto.IfType_IF_TYPE_CPU {
		return
	}
	fmt.Printf("%-10d%-10d%-10s%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		resp.GetStatus().GetIfHandle(),
		ifTypeToStr(ifType),
		resp.GetStatus().GetCpuInfo().GetCpuLportId())
}
