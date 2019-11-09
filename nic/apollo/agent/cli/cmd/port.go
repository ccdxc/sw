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
	portFecType    string
	portAutoNeg    string
	portSpeed      string
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
	portUpdateCmd.Flags().StringVar(&portFecType, "fec-type", "none", "Specify fec-type - rs, fc, none")
	portUpdateCmd.Flags().StringVar(&portAutoNeg, "auto-neg", "enable", "Enable or disable auto-neg using enable | disable")
	portUpdateCmd.Flags().StringVar(&portSpeed, "speed", "", "Set port speed - none, 1g, 10g, 25g, 40g, 50g, 100g")
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

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("admin-state") == false &&
		cmd.Flags().Changed("auto-neg") == false &&
		cmd.Flags().Changed("speed") == false &&
		cmd.Flags().Changed("fec-type") == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
		return
	}

	var mtu uint32
	var debounceTimeout uint32
	var numLanes uint32
	adminState := inputToAdminState("none")
	speed := inputToSpeed("none")
	fecType := inputToFecType("none")
	autoNeg := false
	pauseType := pds.PortPauseType_PORT_PAUSE_TYPE_NONE
	loopbackMode := pds.PortLoopBackMode_PORT_LOOPBACK_MODE_NONE

	if cmd.Flags().Changed("fec-type") == true {
		if isFecTypeValid(portFecType) == false {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
		fecType = inputToFecType(portFecType)
	}

	if cmd.Flags().Changed("auto-neg") == true {
		if strings.Compare(portAutoNeg, "disable") == 0 {
			autoNeg = false
		} else if strings.Compare(portAutoNeg, "enable") == 0 {
			autoNeg = true
		} else {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
	}

	if cmd.Flags().Changed("admin-state") == true {
		if isAdminStateValid(portAdminState) == false {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
		adminState = inputToAdminState(portAdminState)
	}

	if cmd.Flags().Changed("speed") == true {
		if isSpeedValid(strings.ToUpper(portSpeed)) == false {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
		speed = inputToSpeed(strings.ToUpper(portSpeed))
		autoNeg = false
	}

	client := pds.NewPortSvcClient(c)

	ifIndex := portIDStrToIfIndex(portID)
	if ifIndex == InvalidIfIndex {
		fmt.Printf("Invalid port ID\n")
		return
	}

	getReq := &pds.PortGetRequest{
		Id: []uint32{ifIndex},
	}

	// PDS call
	getRespMsg, err := client.PortGet(context.Background(), getReq)
	if err != nil {
		fmt.Printf("Getting Port failed. %v\n", err)
		return
	}

	if getRespMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", getRespMsg.ApiStatus)
		return
	}

	for _, resp := range getRespMsg.Response {
		if cmd.Flags().Changed("fec-type") == false {
			fecType = resp.GetSpec().GetFECType()
		}
		if cmd.Flags().Changed("auto-neg") == false && cmd.Flags().Changed("speed") == false {
			autoNeg = resp.GetSpec().GetAutoNegEn()
		}
		if cmd.Flags().Changed("admin-state") == false {
			adminState = resp.GetSpec().GetAdminState()
		}
		if cmd.Flags().Changed("speed") == false {
			speed = resp.GetSpec().GetSpeed()
		}
		debounceTimeout = resp.GetSpec().GetDeBounceTimeout()
		mtu = resp.GetSpec().GetMtu()
		pauseType = resp.GetSpec().GetPauseType()
		loopbackMode = resp.GetSpec().GetLoopbackMode()
		numLanes = resp.GetSpec().GetNumLanes()
	}

	var req *pds.PortUpdateRequest

	req = &pds.PortUpdateRequest{
		Spec: &pds.PortSpec{
			Id:              ifIndex,
			AdminState:      adminState,
			Speed:           speed,
			FECType:         fecType,
			AutoNegEn:       autoNeg,
			DeBounceTimeout: debounceTimeout,
			Mtu:             mtu,
			PauseType:       pauseType,
			LoopbackMode:    loopbackMode,
			NumLanes:        numLanes,
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

func isFecTypeValid(str string) bool {
	switch str {
	case "none":
		return true
	case "rs":
		return true
	case "fc":
		return true
	default:
		return false
	}
}

func inputToFecType(str string) pds.PortFecType {
	switch str {
	case "none":
		return pds.PortFecType_PORT_FEC_TYPE_NONE
	case "rs":
		return pds.PortFecType_PORT_FEC_TYPE_RS
	case "fc":
		return pds.PortFecType_PORT_FEC_TYPE_FC
	default:
		return pds.PortFecType_PORT_FEC_TYPE_NONE
	}
}

func isSpeedValid(str string) bool {
	switch str {
	case "none":
		return true
	case "1G":
		return true
	case "10G":
		return true
	case "25G":
		return true
	case "40G":
		return true
	case "50G":
		return true
	case "100G":
		return true
	default:
		return false
	}
}

func inputToSpeed(str string) pds.PortSpeed {
	switch str {
	case "none":
		return pds.PortSpeed_PORT_SPEED_NONE
	case "1G":
		return pds.PortSpeed_PORT_SPEED_1G
	case "10G":
		return pds.PortSpeed_PORT_SPEED_10G
	case "25G":
		return pds.PortSpeed_PORT_SPEED_25G
	case "40G":
		return pds.PortSpeed_PORT_SPEED_40G
	case "50G":
		return pds.PortSpeed_PORT_SPEED_50G
	case "100G":
		return pds.PortSpeed_PORT_SPEED_100G
	default:
		return pds.PortSpeed_PORT_SPEED_NONE
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

	fmt.Printf("%-10s0x%-12x%-12s%-10s%-14s\n",
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

func ifIndexToIfType(ifindex uint32) string {
	ifType := (ifindex >> IfTypeShift) & IfTypeMask
	switch ifType {
	case 0:
		return "None"
	case 1:
		return "Eth"
	case 2:
		return "EthPC"
	case 3:
		return "Tunnel"
	case 4:
		return "Mgmt"
	case 5:
		return "Uplink"
	case 6:
		return "UplinkPC"
	case 7:
		return "L3"
	case 8:
		return "Lif"
	}
	return "None"
}

func ifIndexToPortIdStr(ifIndex uint32) string {
	if ifIndex != 0 {
		slotStr := strconv.FormatUint(uint64(ifIndexToSlot(ifIndex)), 10)
		parentPortStr := strconv.FormatUint(uint64(ifIndexToParentPort(ifIndex)), 10)
		ifTypeStr := ifIndexToIfType(ifIndex)
		return ifTypeStr + slotStr + "/" + parentPortStr
	} else {
		return "-"
	}
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
