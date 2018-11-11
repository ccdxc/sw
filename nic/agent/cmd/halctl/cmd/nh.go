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
	nhID uint64
)

var nhShowCmd = &cobra.Command{
	Use:   "nexthop",
	Short: "show nexthop objects",
	Long:  "show nexthop object information",
	Run:   nhShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nhShowCmd)

	nhShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	nhShowCmd.Flags().Uint64Var(&nhID, "id", 1, "Specify nexthop-id")
}

func nhShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		nhDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *halproto.NexthopGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.NexthopGetRequest{
			KeyOrHandle: &halproto.NexthopKeyHandle{
				KeyOrHandle: &halproto.NexthopKeyHandle_NexthopId{
					NexthopId: nhID,
				},
			},
		}
	} else {
		// Get all NHs
		req = &halproto.NexthopGetRequest{}
	}
	nexthopGetReqMsg := &halproto.NexthopGetRequestMsg{
		Request: []*halproto.NexthopGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NexthopGet(context.Background(), nexthopGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Nexthop failed. %v\n", err)
		return
	}

	// Print Header
	nhShowHeader()

	// Print NHs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		nhShowOneResp(resp)
	}
}

func nhDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *halproto.NexthopGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.NexthopGetRequest{
			KeyOrHandle: &halproto.NexthopKeyHandle{
				KeyOrHandle: &halproto.NexthopKeyHandle_NexthopId{
					NexthopId: nhID,
				},
			},
		}
	} else {
		// Get all NHs
		req = &halproto.NexthopGetRequest{}
	}
	nexthopGetReqMsg := &halproto.NexthopGetRequestMsg{
		Request: []*halproto.NexthopGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NexthopGet(context.Background(), nexthopGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Nexthop failed. %v\n", err)
		return
	}

	// Print NHs
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

func nhShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:       Next Hop Id         Handle: Next Hop HAL Handle\n")
	fmt.Printf("EPHandle: EP's Handle\n")
	fmt.Printf("\n")
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s\n",
		"Id", "Handle", "EPHandle")
	fmt.Println(hdrLine)
}

func nhShowOneResp(resp *halproto.NexthopGetResponse) {
	fmt.Printf("%-10d%-10d%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetNexthopId(),
		resp.GetStatus().GetNexthopHandle(),
		resp.GetSpec().GetEpKeyOrHandle().GetEndpointHandle()) // TODO: Handle IF
}
