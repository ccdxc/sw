//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"strconv"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

const IfTypeShift = 28
const IfSlotShift = 24
const IfParentPortShift = 16
const IfTypeMask = 0xF
const IfSlotMask = 0xF
const IfParentPortMask = 0xFF
const IfChildPortMask = 0xFF
const InvalidIfIndex = 0xFFFFFFFF

var (
	portID         string
	portAdminState string
)

var portShowCmd = &cobra.Command{
	Use:   "port",
	Short: "show port information",
	Long:  "show port object information",
}

var portStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show port statistics",
	Long:  "show port statistics",
	Run:   portShowCmdHandler,
}

var portStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show port status",
	Long:  "show port status",
	Run:   portShowStatusCmdHandler,
}

var portUpdateCmd = &cobra.Command{
	Use:   "port",
	Short: "set port debug information",
	Long:  "set port debug information",
	Run:   portUpdateCmdHandler,
}

func init() {
	showCmd.AddCommand(portShowCmd)
	portShowCmd.AddCommand(portStatsShowCmd)
	portStatsShowCmd.Flags().StringVarP(&portID, "port", "p", "", "Specify Port ID. Ex: Eth1/2")
	portShowCmd.AddCommand(portStatusShowCmd)
	portStatusShowCmd.Flags().StringVarP(&portID, "port", "p", "", "Specify Port ID. Ex: Eth1/2")

	debugCmd.AddCommand(portUpdateCmd)
	portUpdateCmd.Flags().StringVarP(&portID, "port", "p", "", "Specify Port ID. Ex: Eth1/2")
	portUpdateCmd.Flags().StringVarP(&portAdminState, "admin-state", "a", "up", "Set port admin state - up, down")
	portUpdateCmd.MarkFlagRequired("admin-state")
	portUpdateCmd.MarkFlagRequired("port")
}

func portUpdateCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	adminState := inputToAdminState("none")

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("admin-state") == false ||
		cmd.Flags().Changed("port") == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
		return
	}

	if isAdminStateValid(portAdminState) == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
		return
	}
	adminState = inputToAdminState(portAdminState)

	client := pds.NewPortSvcClient(c)

	ifIndex := portIDStrToIfIndex(portID)
	if ifIndex == InvalidIfIndex {
		fmt.Printf("Invalid port ID\n")
		return
	}

	var req *pds.PortUpdateRequest

	req = &pds.PortUpdateRequest{
		Spec: &pds.PortSpec{
			Id:     ifIndex,
			AdminState: adminState,
		},
	}

	// PDS call
	respMsg, err := client.PortUpdate(context.Background(), req)
	if err != nil {
		fmt.Printf("Update Port failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	fmt.Printf("Update port succeeded\n")
}

func isAdminStateValid(str string) bool {
	switch str {
	case "up":
		return true
	case "down":
		return true
	default:
		return false
	}
}

func inputToAdminState(str string) pds.PortAdminState {
	switch str {
	case "up":
		return pds.PortAdminState_PORT_ADMIN_STATE_UP
	case "down":
		return pds.PortAdminState_PORT_ADMIN_STATE_DOWN
	default:
		return pds.PortAdminState_PORT_ADMIN_STATE_NONE
	}
}

func portShowStatusCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewPortSvcClient(c)

	var req *pds.PortGetRequest
	if cmd.Flags().Changed("port") {
		// Get specific Port
		ifIndex := portIDStrToIfIndex(portID)
		if ifIndex == InvalidIfIndex {
			fmt.Printf("Invalid port ID\n")
			return
		}

		req = &pds.PortGetRequest{
			Id: []uint32{ifIndex},
		}
	} else {
		// Get all Ports
		req = &pds.PortGetRequest{
			Id: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.PortGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Port failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	printPortStatusHeader()

	// Print Ports
	for _, resp := range respMsg.Response {
		printPortStatus(resp)
	}
}

func printPortStatusHeader() {
	hdrLine := strings.Repeat("-", 60)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-14s%-12s%-10s%-14s\n",
		"Port", "Interface Id", "AdminState", "OperState", "Transceiver")
	fmt.Println(hdrLine)
}

func printPortStatus(resp *pds.Port) {
	adminState := resp.GetSpec().GetAdminState()
	operStatus := resp.GetStatus().GetLinkStatus().GetOperState()
	xcvrStatus := resp.GetStatus().GetXcvrStatus()

	adminStateStr := strings.Replace(adminState.String(), "PORT_ADMIN_STATE_", "", -1)
	operStatusStr := strings.Replace(operStatus.String(), "PORT_OPER_STATUS_", "", -1)
	xcvrPortNum := xcvrStatus.GetPort()
	xcvrState := xcvrStatus.GetState()
	xcvrPid := xcvrStatus.GetPid()
	xcvrStateStr := "-"
	xcvrPidStr := "-"
	xcvrStr := ""
	if xcvrPortNum > 0 && (strings.Compare(xcvrState.String(), "65535") != 0) {
		// Strip XCVR_STATE_ from the state
		xcvrStateStr = strings.Replace(strings.Replace(xcvrState.String(), "XCVR_STATE_", "", -1), "_", "-", -1)
		// Strip XCVR_PID_ from the pid
		xcvrPidStr = strings.Replace(strings.Replace(xcvrPid.String(), "XCVR_PID_", "", -1), "_", "-", -1)
	}

	if strings.Compare(xcvrStateStr, "SPROM-READ") == 0 {
		xcvrStr = xcvrPidStr
	} else {
		xcvrStr = xcvrStateStr
	}

	fmt.Printf("%-10s0x%-14x%-12s%-10s%-14s\n",
		ifIndexToPortIdStr(resp.GetSpec().GetId()),
		resp.GetSpec().GetId(),
		adminStateStr, operStatusStr,
		xcvrStr)
}

func portShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewPortSvcClient(c)

	var req *pds.PortGetRequest
	if cmd.Flags().Changed("port") {
		// Get specific Port
		ifIndex := portIDStrToIfIndex(portID)
		if ifIndex == InvalidIfIndex {
			fmt.Printf("Invalid port ID\n")
			return
		}

		req = &pds.PortGetRequest{
			Id: []uint32{ifIndex},
		}
	} else {
		// Get all Ports
		req = &pds.PortGetRequest{
			Id: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.PortGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Port failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	printPortStatsHeader()

	// Print Ports
	for _, resp := range respMsg.Response {
		printPortStats(resp)
	}
}

func printPortStatsHeader() {
	hdrLine := strings.Repeat("-", 37)
	fmt.Println(hdrLine)
	fmt.Printf("%-8s%-25s%-5s\n", "PortId", "Field", "Count")
	fmt.Println(hdrLine)
}

func portIDStrToIfIndex(portIDStr string) uint32 {
	var slotIndex uint32
	var portIndex uint32
	var ifIndex uint32

	portIDStr = strings.ToLower(portIDStr)
	n, err := fmt.Sscanf(portIDStr, "eth%d/%d", &slotIndex, &portIndex)
	if err != nil || n != 2 {
		return InvalidIfIndex
	}
	ifIndex = 1 << IfTypeShift // 1 is Eth port
	ifIndex |= slotIndex << IfSlotShift
	ifIndex |= portIndex << IfParentPortShift
	ifIndex |= 1 // Default child port

	return ifIndex
}

func ifIndexToSlot(ifIndex uint32) uint32 {
	return (ifIndex >> IfSlotShift) & IfSlotMask
}

func ifIndexToParentPort(ifIndex uint32) uint32 {
	return (ifIndex >> IfParentPortShift) & IfParentPortMask
}

func ifIndexToChildPort(ifIndex uint32) uint32 {
	return ifIndex & IfChildPortMask
}

func ifIndexToPortIdStr(ifIndex uint32) string {
	slotStr := strconv.FormatUint(uint64(ifIndexToSlot(ifIndex)), 10)
	parentPortStr := strconv.FormatUint(uint64(ifIndexToParentPort(ifIndex)), 10)
	return "Eth" + slotStr + "/" + parentPortStr
}

func printPortStats(resp *pds.Port) {
	first := true
	macStats := resp.GetStats().GetMacStats()
	mgmtMacStats := resp.GetStats().GetMgmtMacStats()

	fmt.Printf("%-8s", ifIndexToPortIdStr(resp.GetSpec().GetId()))
	for _, s := range macStats {
		if first == false {
			fmt.Printf("%-8s", "")
		}
		fmt.Printf("%-25s%-5d\n",
			strings.Replace(s.GetType().String(), "_", " ", -1),
			s.GetCount())
		first = false
	}

	first = true
	for _, s := range mgmtMacStats {
		if first == false {
			fmt.Printf("%-8s", "")
		}
		str := strings.Replace(s.GetType().String(), "MGMT_MAC_", "", -1)
		str = strings.Replace(str, "_", " ", -1)
		fmt.Printf("%-25s%-5d\n", str, s.GetCount())
		first = false
	}
}
