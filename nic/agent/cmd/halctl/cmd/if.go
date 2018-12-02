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
	ifID       uint64
	ifStatusID uint64
)

var ifShowCmd = &cobra.Command{
	Use:   "interface",
	Short: "show interface information",
	Long:  "show interface object information",
	Run:   ifShowCmdHandler,
}

var ifShowSpecCmd = &cobra.Command{
	Use:   "spec",
	Short: "show interface spec information",
	Long:  "show interface object spec information",
	Run:   ifShowCmdHandler,
}

var ifShowStatusCmd = &cobra.Command{
	Use:   "status",
	Short: "show interface status information",
	Long:  "show interface object status information",
	Run:   ifShowStatusCmdHandler,
}

func init() {
	showCmd.AddCommand(ifShowCmd)
	ifShowCmd.AddCommand(ifShowSpecCmd)
	ifShowCmd.AddCommand(ifShowStatusCmd)

	ifShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	ifShowCmd.Flags().Uint64Var(&ifID, "id", 1, "Specify if-id")
	ifShowSpecCmd.Flags().Uint64Var(&ifID, "id", 1, "Specify if-id")
	ifShowStatusCmd.Flags().Uint64Var(&ifStatusID, "id", 1, "Specify if-id")
}

func ifShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		ifDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	defer c.Close()

	if len(args) > 0 {
		if strings.Compare(args[0], "spec") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

	var req *halproto.InterfaceGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific if
		req = &halproto.InterfaceGetRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: ifID,
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
	ifShowHeader()

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		ifShowOneResp(resp)
	}
}

func ifGetStrFromID(ifID []uint64) (int, []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	defer c.Close()

	var ifStr []string

	index := 0
	reqArray := make([]*halproto.InterfaceGetRequest, len(ifID))
	for index < len(ifID) {
		var req halproto.InterfaceGetRequest
		req = halproto.InterfaceGetRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: ifID[index],
				},
			},
		}
		reqArray[index] = &req
		index++
	}

	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: reqArray,
	}

	// HAL call
	respMsg, err := client.InterfaceGet(context.Background(), ifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting if failed. %v\n", err)
		return -1, nil
	}

	index = 0
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			return -1, nil
		}

		ifStr = append(ifStr, fmt.Sprintf("%s-%d",
			strings.ToLower(ifTypeToStr(resp.GetSpec().GetType())),
			ifID[index]))
	}

	return 0, ifStr
}

func ifShowStatusCmdHandler(cmd *cobra.Command, args []string) {
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
					InterfaceId: ifID,
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
	ifShowStatusHeader()

	// Print IFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		ifShowStatusOneResp(resp)
	}
}

func handleIfDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewInterfaceClient(c.ClientConn)

	defer c.Close()

	var req *halproto.InterfaceGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific if
		req = &halproto.InterfaceGetRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: ifID,
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
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func ifDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleIfDetailShowCmd(cmd, nil)
}

func ifShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:    Interface ID         Handle: IF's handle\n")
	fmt.Printf("Ifype: Interface type\n")
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s\n",
		"Id", "Handle", "IfType")
	fmt.Println(hdrLine)
}

func ifShowOneResp(resp *halproto.InterfaceGetResponse) {
	fmt.Printf("%-10d%-10d%-10s",
		resp.GetSpec().GetKeyOrHandle().GetInterfaceId(),
		resp.GetStatus().GetIfHandle(),
		ifTypeToStr(resp.GetSpec().GetType()))
	fmt.Printf("\n")
}

func ifTypeToStr(ifType halproto.IfType) string {
	switch ifType {
	case halproto.IfType_IF_TYPE_ENIC:
		return "Enic"
	case halproto.IfType_IF_TYPE_UPLINK:
		return "Uplink"
	case halproto.IfType_IF_TYPE_UPLINK_PC:
		return "UplinkPC"
	case halproto.IfType_IF_TYPE_TUNNEL:
		return "Tunnel"
	case halproto.IfType_IF_TYPE_CPU:
		return "CPU"
	case halproto.IfType_IF_TYPE_APP_REDIR:
		return "AppRedir"
	default:
		return "Invalid"
	}
}

func ifShowStatusHeader() {
	fmt.Printf("\n")
	fmt.Printf("Handle:    Interface handle         Status:    Interface status \n")
	hdrLine := strings.Repeat("-", 13)
	fmt.Println(hdrLine)
	fmt.Printf("%-7s%-6s\n",
		"Handle", "Status")
	fmt.Println(hdrLine)
}

func ifShowStatusOneResp(resp *halproto.InterfaceGetResponse) {
	fmt.Printf("%-7d%-6s",
		resp.GetStatus().GetIfHandle(),
		strings.ToLower(strings.Replace(resp.GetStatus().GetIfStatus().String(), "IF_STATUS_", "", -1)))
	fmt.Printf("\n")
}
