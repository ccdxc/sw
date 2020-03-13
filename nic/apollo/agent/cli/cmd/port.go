//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strconv"
	"strings"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

const (
	ifTypeNone        = 0
	ifTypeEth         = 1
	ifTypeEthPC       = 2
	ifTypeTunnel      = 3
	ifTypeUplink      = 5
	ifTypeUplinkPC    = 6
	ifTypeL3          = 7
	ifTypeLif         = 8
	ifTypeLoopback    = 9
	ifTypeShift       = 28
	ifSlotShift       = 24
	ifParentPortShift = 16
	ifTypeMask        = 0xF
	ifSlotMask        = 0xF
	ifParentPortMask  = 0xFF
	ifChildPortMask   = 0xFF
	ifNameDelimiter   = "/"
	invalidIfIndex    = 0xFFFFFFFF
)

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
	portStatsShowCmd.Flags().StringVarP(&portID, "port", "p", "", "Specify port uuid")
	portStatsShowCmd.Flags().Bool("yaml", true, "Output in yaml")
	portShowCmd.AddCommand(portStatusShowCmd)
	portStatusShowCmd.Flags().StringVarP(&portID, "port", "p", "", "Specify port uuid")
	portStatusShowCmd.Flags().Bool("yaml", true, "Output in yaml")

	debugCmd.AddCommand(portUpdateCmd)
	portUpdateCmd.Flags().StringVarP(&portID, "port", "p", "", "Specify port uuid")
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
	portType := pds.PortType_PORT_TYPE_NONE

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

	getReq := &pds.PortGetRequest{
		Id: [][]byte{uuid.FromStringOrNil(portID).Bytes()},
	}
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
		portType = resp.GetSpec().GetType()
	}

	var req *pds.PortUpdateRequest

	req = &pds.PortUpdateRequest{
		Spec: &pds.PortSpec{
			Id:              uuid.FromStringOrNil(portID).Bytes(),
			AdminState:      adminState,
			Type:            portType,
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
	yamlOutput := (cmd != nil) && cmd.Flags().Changed("yaml")

	var req *pds.PortGetRequest
	if cmd != nil && cmd.Flags().Changed("port") {
		req = &pds.PortGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(portID).Bytes()},
		}
	} else {
		// Get all Ports
		req = &pds.PortGetRequest{
			Id: [][]byte{},
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

	if yamlOutput {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printPortStatusHeader()
		// Print Ports
		for _, resp := range respMsg.Response {
			printPortStatus(resp)
		}
	}
}

func printPortStatusHeader() {
	hdrLine := strings.Repeat("-", 196)
	fmt.Println("MAC-Info: MAC ID/MAC Channel/Num lanes")
	fmt.Println("FEC-Type: FC - FireCode, RS - ReedSolomon")
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-10s%-12s%-7s%-10s%-6s%-10s%-6s%-7s%-7s%-10s%-12s%-15s%-12s%-20s%-10s\n",
		"Id", "Name", "IfIndex", "Speed", "MAC-Info", "FEC", "AutoNeg",
		"MTU", "Pause", "Pause", "Debounce", "State",
		"Transceiver", "NumLinkDown", "LinkSM", "Loopback")
	fmt.Printf("%-40s%-10s%-12s%-7s%-10s%-6s%-10s%-6s%-7s%-7s%-10s%-12s%-15s%-12s%-20s%-10s\n",
		"", "", "", "", "", "", "Cfg/Oper",
		"", "Type", "Tx/Rx", "(msecs)", "Admin/Oper",
		"", "", "", "")
	fmt.Println(hdrLine)
}

func printPortStatus(resp *pds.Port) {
	spec := resp.GetSpec()
	status := resp.GetStatus()
	linkStatus := status.GetLinkStatus()
	stats := resp.GetStats()

	if spec == nil {
		fmt.Printf("Error! Port spec cannot be nil\n")
		return
	}

	if status == nil {
		fmt.Printf("Error! Port status cannot be nil\n")
		return
	}

	if linkStatus == nil {
		fmt.Printf("Error! Port link status cannot be nil\n")
		return
	}

	if stats == nil {
		fmt.Printf("Error! Port stats cannot be nil\n")
		return
	}

	adminState := spec.GetAdminState()
	operStatus := linkStatus.GetOperState()
	xcvrStatus := status.GetXcvrStatus()
	speed := spec.GetSpeed()
	fecType := spec.GetFECType()
	mtu := spec.GetMtu()
	debounce := spec.GetDeBounceTimeout()

	adminStateStr := strings.Replace(adminState.String(), "PORT_ADMIN_STATE_", "", -1)
	operStatusStr := strings.Replace(operStatus.String(), "PORT_OPER_STATUS_", "", -1)
	speedStr := strings.Replace(speed.String(), "PORT_SPEED_", "", -1)
	fecStr := strings.Replace(fecType.String(), "PORT_FEC_TYPE_", "", -1)
	loopbackModeStr := strings.Replace(spec.GetLoopbackMode().String(), "PORT_LOOPBACK_MODE_", "", -1)
	pauseStr := strings.Replace(spec.GetPauseType().String(), "PORT_PAUSE_TYPE_", "", -1)
	macStr := fmt.Sprintf("%d/%d/%d", status.GetMacId(), status.GetMacCh(), linkStatus.GetNumLanes())
	fsmStr := strings.Replace(status.GetFSMState().String(), "PORT_LINK_FSM_", "", -1)

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

	outStr := fmt.Sprintf("%-40s%-10s0x%-10x%-7s%-10s%-6s",
		uuid.FromBytesOrNil(spec.GetId()).String(),
		ifIndexToPortIdStr(status.GetIfIndex()), status.GetIfIndex(),
		speedStr, macStr, fecStr)
	outStr += fmt.Sprintf("%2s/%-7s%-6d%-7s%2s/%-4s",
		utils.BoolToString(spec.GetAutoNegEn()), utils.BoolToString(linkStatus.GetAutoNegEn()),
		mtu, pauseStr, utils.BoolToString(spec.GetTxPauseEn()), utils.BoolToString(spec.GetRxPauseEn()))
	outStr += fmt.Sprintf("%-10d%4s/%-7s%-15s%-12d%-20s%-10s",
		debounce, adminStateStr, operStatusStr, xcvrStr, stats.GetNumLinkDown(),
		fsmStr, loopbackModeStr)
	fmt.Printf(outStr + "\n")
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
	yamlOutput := (cmd != nil) && cmd.Flags().Changed("yaml")

	var req *pds.PortGetRequest
	if cmd != nil && cmd.Flags().Changed("port") {
		req = &pds.PortGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(portID).Bytes()},
		}
	} else {
		// Get all Ports
		req = &pds.PortGetRequest{
			Id: [][]byte{},
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

	if yamlOutput {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printPortStatsHeader()
		// Print Ports
		for _, resp := range respMsg.Response {
			printPortStats(resp)
		}
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
		return invalidIfIndex
	}
	ifIndex = 1 << ifTypeShift // 1 is Eth port
	ifIndex |= slotIndex << ifSlotShift
	ifIndex |= portIndex << ifParentPortShift
	ifIndex |= 1 // Default child port

	return ifIndex
}

func ifIndexToSlot(ifIndex uint32) uint32 {
	return (ifIndex >> ifSlotShift) & ifSlotMask
}

func ifIndexToParentPort(ifIndex uint32) uint32 {
	return (ifIndex >> ifParentPortShift) & ifParentPortMask
}

func ifIndexToChildPort(ifIndex uint32) uint32 {
	return ifIndex & ifChildPortMask
}

func ifIndexToIfType(ifindex uint32) string {
	ifType := (ifindex >> ifTypeShift) & ifTypeMask
	switch ifType {
	case ifTypeNone:
		return "None"
	case ifTypeEth:
		return "Eth"
	case ifTypeEthPC:
		return "EthPC"
	case ifTypeTunnel:
		return "Tunnel"
	case ifTypeLoopback:
		return "lo"
	case ifTypeUplink:
		return "Uplink"
	case ifTypeUplinkPC:
		return "UplinkPC"
	case ifTypeL3:
		return "L3"
	case ifTypeLif:
		return "Lif"
	}
	return "None"
}

func ifIndexToID(ifIndex uint32) uint32 {
	return ifIndex &^ (ifTypeMask << ifTypeShift)
}

func ifIndexToPortIdStr(ifIndex uint32) string {
	if ifIndex == 0 {
		return "-"
	}
	ifType := (ifIndex >> ifTypeShift) & ifTypeMask
	ifTypeStr := ifIndexToIfType(ifIndex)
	switch ifType {
	case ifTypeEth:
		slotStr := strconv.FormatUint(uint64(ifIndexToSlot(ifIndex)), 10)
		parentPortStr := strconv.FormatUint(uint64(ifIndexToParentPort(ifIndex)), 10)
		return ifTypeStr + slotStr + ifNameDelimiter + parentPortStr
	case ifTypeEthPC, ifTypeTunnel, ifTypeL3, ifTypeLif, ifTypeLoopback:
		return ifTypeStr + strconv.FormatUint(uint64(ifIndexToID(ifIndex)), 10)
	}
	return "-"
}

func printPortStats(resp *pds.Port) {
	first := true
	macStats := resp.GetStats().GetMacStats()
	mgmtMacStats := resp.GetStats().GetMgmtMacStats()

	fmt.Printf("%-8s", ifIndexToPortIdStr(resp.GetStatus().GetIfIndex()))
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
