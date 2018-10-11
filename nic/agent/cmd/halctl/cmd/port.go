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
	portNum uint32
)

var portShowCmd = &cobra.Command{
	Use:   "port",
	Short: "show port details",
	Long:  "show port details information",
	Run:   portDetailShowCmdHandler,
}

var portDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show port details",
	Long:  "show port details information",
	Run:   portDetailShowCmdHandler,
}

var portStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show port status",
	Long:  "show port status",
	Run:   portStatusShowCmdHandler,
}

var portStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show port statistics",
	Long:  "show port statistics",
	Run:   portStatsShowCmdHandler,
}

func init() {
	showCmd.AddCommand(portShowCmd)
	portShowCmd.AddCommand(portDetailShowCmd)
	portShowCmd.AddCommand(portStatusShowCmd)
	portShowCmd.AddCommand(portStatsShowCmd)

	portShowCmd.PersistentFlags().Uint32Var(&portNum, "port", 1, "Specify port number")
}

func handlePortDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewPortClient(c.ClientConn)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portNum,
				},
			},
		}
	} else {
		// Get all Ports
		req = &halproto.PortGetRequest{}
	}

	portGetReqMsg := &halproto.PortGetRequestMsg{
		Request: []*halproto.PortGetRequest{req},
	}

	// HAL call
	respMsg, err := client.PortGet(context.Background(), portGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Port failed. %v\n", err)
		return
	}

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

func handlePortStatusShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewPortClient(c.ClientConn)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portNum,
				},
			},
		}
	} else {
		// Get all Ports
		req = &halproto.PortGetRequest{}
	}

	portGetReqMsg := &halproto.PortGetRequestMsg{
		Request: []*halproto.PortGetRequest{req},
	}

	// HAL call
	respMsg, err := client.PortGet(context.Background(), portGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Port failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		adminState := resp.GetSpec().GetAdminState()
		operStatus := resp.GetStatus().GetOperStatus()

		adminStateStr := "DOWN"
		operStatusStr := "DOWN"

		if adminState == 2 {
			adminStateStr = "UP"
		}

		if operStatus == 1 {
			operStatusStr = "UP"
		}

		fmt.Printf("Port: %d, Admin State: %s, Link: %s\n",
			resp.GetSpec().GetKeyOrHandle().GetPortId(),
			adminStateStr,
			operStatusStr)
	}
}

func portStatusShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handlePortStatusShowCmd(cmd, nil)
}

func portDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handlePortDetailShowCmd(cmd, nil)
}

func portStatsShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := halproto.NewPortClient(c.ClientConn)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portNum,
				},
			},
		}
	} else {
		// Get all Ports
		req = &halproto.PortGetRequest{}
	}

	portGetReqMsg := &halproto.PortGetRequestMsg{
		Request: []*halproto.PortGetRequest{req},
	}

	// HAL call
	respMsg, err := client.PortGet(context.Background(), portGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Port failed. %v\n", err)
		return
	}

	// Print header
	portShowStatsHeader()

	// Print Statistics
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		portShowStatsOneResp(resp)
	}
}

func portShowStatsHeader() {
	hdrLine := strings.Repeat("-", 30)
	fmt.Println(hdrLine)
	fmt.Printf("%-25s%-5s\n",
		"Field", "Count")
	fmt.Println(hdrLine)
}

func portShowStatsOneResp(resp *halproto.PortGetResponse) {
	hdrLine := strings.Repeat("-", 30)
	macStats := resp.GetStats().GetMacStats()
	fmt.Printf("\nstats for port: %d\n\n", resp.GetSpec().GetKeyOrHandle().GetPortId())
	for _, s := range macStats {
		fmt.Printf("%-25s%-5d\n",
			strings.Replace(s.GetType().String(), "_", " ", -1),
			s.GetCount())
	}
	fmt.Println(hdrLine)
}
