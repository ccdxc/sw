//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	snakeTestVlan uint32
)

var snakeTestShowCmd = &cobra.Command{
	Use:   "snake-test",
	Short: "shows snake test",
	Long:  "shows snake test",
	Run:   snakeTestShowCmdHandler,
}

var snakeTestCmd = &cobra.Command{
	Use:   "snake-test",
	Short: "snake test",
	Long:  "snake test",
}

var snakeTestLoopCmd = &cobra.Command{
	Use:   "loop",
	Short: "Enable snake test loop",
	Long:  "Enable snake test loop",
	Run:   snakeTestLoopCmdHandler,
}

var snakeTestUp2UpCmd = &cobra.Command{
	Use:   "up2up",
	Short: "Enable snake test up2up",
	Long:  "Enable snake test up2up",
	Run:   snakeTestUp2UpCmdHandler,
}

var snakeTestArmCmd = &cobra.Command{
	Use:   "arm",
	Short: "Enable snake test arm",
	Long:  "Enable snake test arm",
	Run:   snakeTestArmCmdHandler,
}

var snakeTestLoopDisableCmd = &cobra.Command{
	Use:   "disable",
	Short: "Disable snake test",
	Long:  "Disable snake test",
	Run:   snakeTestLoopDisableCmdHandler,
}

var snakeTestArmDisableCmd = &cobra.Command{
	Use:   "disable",
	Short: "Disable snake test",
	Long:  "Disable snake test",
	Run:   snakeTestArmDisableCmdHandler,
}

var snakeTestUp2UpDisableCmd = &cobra.Command{
	Use:   "disable",
	Short: "Disable snake test",
	Long:  "Disable snake test",
	Run:   snakeTestUp2UpDisableCmdHandler,
}

func init() {
	// Show cmd
	showCmd.AddCommand(snakeTestShowCmd)

	// Debug cmds
	debugCmd.AddCommand(snakeTestCmd)
	snakeTestCmd.AddCommand(snakeTestLoopCmd)
	snakeTestCmd.AddCommand(snakeTestArmCmd)
	snakeTestCmd.AddCommand(snakeTestUp2UpCmd)
	snakeTestLoopCmd.AddCommand(snakeTestLoopDisableCmd)
	snakeTestArmCmd.AddCommand(snakeTestArmDisableCmd)
	snakeTestUp2UpCmd.AddCommand(snakeTestUp2UpDisableCmd)

	snakeTestLoopCmd.Flags().Uint32Var(&snakeTestVlan, "vlan", 0, "Specify vlan")
	snakeTestArmCmd.Flags().Uint32Var(&snakeTestVlan, "vlan", 0, "Specify vlan")
	snakeTestUp2UpCmd.Flags().Uint32Var(&snakeTestVlan, "vlan", 0, "Specify vlan")
}

func snakeTestLoopCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c)

	req := &halproto.SnakeTestRequest{
		Type: halproto.SnakeTestType_SNAKE_TEST_TYPE_LOOP,
		Vlan: snakeTestVlan,
	}
	snakeTestReqMsg := &halproto.SnakeTestRequestMsg{
		Request: []*halproto.SnakeTestRequest{req},
	}

	// HAL call
	respMsg, err := client.SnakeTestCreate(context.Background(), snakeTestReqMsg)
	if err != nil {
		fmt.Printf("Snake Test install failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
	}

	fmt.Println("Success: Snake Test for Loop Installed.")
}

func snakeTestUp2UpCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c)

	req := &halproto.SnakeTestRequest{
		Type: halproto.SnakeTestType_SNAKE_TEST_TYPE_UP2UP,
		Vlan: snakeTestVlan,
	}
	snakeTestReqMsg := &halproto.SnakeTestRequestMsg{
		Request: []*halproto.SnakeTestRequest{req},
	}

	// HAL call
	respMsg, err := client.SnakeTestCreate(context.Background(), snakeTestReqMsg)
	if err != nil {
		fmt.Printf("Snake Test install failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
	}

	fmt.Println("Success: Snake Test for Loop Installed.")
}

func snakeTestUp2UpDisableCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c)

	req := &halproto.SnakeTestDeleteRequest{
		Type: halproto.SnakeTestType_SNAKE_TEST_TYPE_UP2UP,
	}
	snakeTestDelReqMsg := &halproto.SnakeTestDeleteRequestMsg{
		Request: []*halproto.SnakeTestDeleteRequest{req},
	}

	// HAL call
	respMsg, err := client.SnakeTestDelete(context.Background(), snakeTestDelReqMsg)
	if err != nil {
		fmt.Printf("Snake Test uninstall failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
	}

	fmt.Println("Success: Snake Test for Loop Uninstalled.")
}

func snakeTestLoopDisableCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c)

	req := &halproto.SnakeTestDeleteRequest{
		Type: halproto.SnakeTestType_SNAKE_TEST_TYPE_LOOP,
	}
	snakeTestDelReqMsg := &halproto.SnakeTestDeleteRequestMsg{
		Request: []*halproto.SnakeTestDeleteRequest{req},
	}

	// HAL call
	respMsg, err := client.SnakeTestDelete(context.Background(), snakeTestDelReqMsg)
	if err != nil {
		fmt.Printf("Snake Test uninstall failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
	}

	fmt.Println("Success: Snake Test for Loop Uninstalled.")
}

func snakeTestArmCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c)

	req := &halproto.SnakeTestRequest{
		Type: halproto.SnakeTestType_SNAKE_TEST_TYPE_ARM_TO_ARM,
		Vlan: snakeTestVlan,
	}
	snakeTestReqMsg := &halproto.SnakeTestRequestMsg{
		Request: []*halproto.SnakeTestRequest{req},
	}

	// HAL call
	respMsg, err := client.SnakeTestCreate(context.Background(), snakeTestReqMsg)
	if err != nil {
		fmt.Printf("Snake Test install failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
	}

	fmt.Println("Success: Snake Test for Arm->Arm Installed.")
}

func snakeTestArmDisableCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c)

	req := &halproto.SnakeTestDeleteRequest{
		Type: halproto.SnakeTestType_SNAKE_TEST_TYPE_ARM_TO_ARM,
	}
	snakeTestDelReqMsg := &halproto.SnakeTestDeleteRequestMsg{
		Request: []*halproto.SnakeTestDeleteRequest{req},
	}

	// HAL call
	respMsg, err := client.SnakeTestDelete(context.Background(), snakeTestDelReqMsg)
	if err != nil {
		fmt.Printf("Snake Test uninstall failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
	}

	fmt.Println("Success: Snake Test for Arm->Arm Uninstalled.")
}

func snakeTestShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewDebugClient(c)

	var empty *halproto.Empty
	respMsg, err := client.SnakeTestGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting if failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b) + "\n")
		fmt.Println("---")
	}
}
