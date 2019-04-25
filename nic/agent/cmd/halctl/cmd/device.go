//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var deviceShowCmd = &cobra.Command{
	Use:   "device",
	Short: "show device information",
	Long:  "show device object information",
}

var fwdModeShowCmd = &cobra.Command{
	Use:   "forwarding-mode",
	Short: "show device forwarding-mode",
	Long:  "show device forwarding-mode information",
	Run:   deviceShowCmdHandler,
}

func init() {
	showCmd.AddCommand(deviceShowCmd)
	deviceShowCmd.AddCommand(fwdModeShowCmd)
}

func deviceShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNicClient(c)

	req := &halproto.DeviceGetRequest{}
	reqMsg := &halproto.DeviceGetRequestMsg{
		Request: req,
	}

	// HAL call
	respMsg, err := client.DeviceGet(context.Background(), reqMsg)
	if err != nil {
		fmt.Printf("Getting device failed. %v\n", err)
		return
	}

	// Print device
	resp := respMsg.GetResponse()
	if resp.ApiStatus == halproto.ApiStatus_API_STATUS_OK {
		mode := strings.Replace(resp.GetDevice().GetDeviceMode().String(), "DEVICE_MODE_", "", -1)
		mode = strings.Replace(mode, "_", "-", -1)
		fmt.Printf("Device is in %s mode\n", mode)
	} else {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
	}
}
