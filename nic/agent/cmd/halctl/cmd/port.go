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
	portNum        uint32
	portPause      string
	portFecType    string
	portAutoNeg    string
	portMtu        uint32
	portAdminState string
	portSpeed      string
)

var portClearStatsCmd = &cobra.Command{
	Use:   "port stats",
	Short: "clear port stats",
	Long:  "clear port stats",
	Run:   portClearStatsCmdHandler,
}

var portShowCmd = &cobra.Command{
	Use:   "port",
	Short: "show port details",
	Long:  "show port details information",
	Run:   portShowCmdHandler,
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

var portDebugCmd = &cobra.Command{
	Use:   "port",
	Short: "debug port object",
	Long:  "debug port object",
	Run:   portDebugCmdHandler,
}

func init() {
	showCmd.AddCommand(portShowCmd)
	portShowCmd.AddCommand(portStatusShowCmd)
	portShowCmd.AddCommand(portStatsShowCmd)

	portShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	portShowCmd.PersistentFlags().Uint32Var(&portNum, "port", 1, "Specify port number")

	clearCmd.AddCommand(portClearStatsCmd)
	portClearStatsCmd.Flags().Uint32Var(&portNum, "port", 1, "Speficy port number")

	debugCmd.AddCommand(portDebugCmd)
	portDebugCmd.Flags().Uint32Var(&portNum, "port", 0, "Specify port number")
	portDebugCmd.Flags().StringVar(&portPause, "pause", "none", "Specify pause - link, pfc, none")
	portDebugCmd.Flags().StringVar(&portFecType, "fec-type", "none", "Specify fec-type - rs, fc, none")
	portDebugCmd.Flags().StringVar(&portAutoNeg, "auto-neg", "disable", "Enable or disable auto-neg using enable | disable")
	portDebugCmd.Flags().StringVar(&portAdminState, "admin-state", "up", "Set port admin state - up, down")
	portDebugCmd.Flags().StringVar(&portSpeed, "speed", "", "Set port speed - none, 1g, 10g, 25g, 40g, 50g, 100g")
	portDebugCmd.Flags().Uint32Var(&portMtu, "mtu", 0, "Specify port MTU")
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

	portStatusPrintHeader()

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

		xcvrStatus := resp.GetStatus().GetXcvrStatus()
		xcvrPortNum := xcvrStatus.GetPort()
		xcvrState := xcvrStatus.GetState()
		xcvrPid := xcvrStatus.GetPid()
		xcvrStateStr := "-"
		xcvrPidStr := "-"
		xcvrStr := ""
		if xcvrPortNum > 0 {
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

		fmt.Printf("%-10d%-12s%-10s%-14s\n",
			resp.GetSpec().GetKeyOrHandle().GetPortId(),
			adminStateStr, operStatusStr,
			xcvrStr)
	}
}

func portStatusPrintHeader() {
	hdrLine := strings.Repeat("-", 46)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-12s%-10s%-14s\n",
		"PortNum", "AdminState", "OperState", "Transceiver")
	fmt.Println(hdrLine)
}

func portStatusShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handlePortStatusShowCmd(cmd, nil)
}

func portShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("yaml") {
		handlePortDetailShowCmd(cmd, nil)
		return
	}

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

	portShowHeaderPrint()

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		portShowOneResp(resp)
	}
}

func portShowHeaderPrint() {
	hdrLine := strings.Repeat("-", 152)
	fmt.Println("MAC-Info: MAC ID/MAC Channel/Num lanes          Debounce: Debounce time in ms")
	fmt.Println("FEC-Type: FC - FireCode, RS - ReedSolomon")
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-10s%-15s%-10s%-15s%-6s%-10s%-10s%-12s%-12s%-12s%-20s%-10s\n",
		"PortType/ID", "Speed", "MAC-Info", "FEC-Type", "AutoNegEnable", "MTU", "Pause", "Debounce", "AdminStatus", "OperStatus", "NumLinkDown", "LinkSM", "Loopback")
	fmt.Println(hdrLine)
}

func portShowOneResp(resp *halproto.PortGetResponse) {
	spec := resp.GetSpec()
	macStr := fmt.Sprintf("%d/%d/%d", spec.GetMacId(), spec.GetMacCh(), spec.GetNumLanes())
	speedStr := strings.Replace(spec.GetPortSpeed().String(), "PORT_SPEED_", "", -1)
	fecStr := strings.Replace(spec.GetFecType().String(), "PORT_FEC_TYPE_", "", -1)
	linkSmStr := strings.Replace(resp.GetLinkState().String(), "PORT_LINK_SM_", "", -1)
	loopbackStr := strings.Replace(spec.GetLoopbackMode().String(), "PORT_LOOPBACK_MODE_", "", -1)

	if strings.Compare(fecStr, "NONE") == 0 {
		fecStr = "None"
	}

	portStr := strings.ToLower(strings.Replace(spec.GetPortType().String(), "PORT_TYPE_", "", -1))
	portStr = fmt.Sprintf("%s/%d", portStr, spec.GetKeyOrHandle().GetPortId())
	pauseStr := strings.ToLower(strings.Replace(spec.GetPause().String(), "PORT_PAUSE_TYPE_", "", -1))
	adminStateStr := strings.Replace(resp.GetSpec().GetAdminState().String(), "PORT_ADMIN_STATE_", "", -1)
	operStatusStr := strings.Replace(resp.GetStatus().GetOperStatus().String(), "PORT_OPER_STATUS_", "", -1)

	fmt.Printf("%-12s%-10s%-15s%-10s%-15t%-6d%-10s%-10d%-12s%-12s%-12d%-20s%-10s\n",
		portStr, speedStr, macStr, fecStr, spec.GetAutoNegEnable(),
		spec.GetMtu(), pauseStr, spec.GetDebounceTime(),
		adminStateStr, operStatusStr,
		resp.GetStats().GetNumLinkDown(),
		linkSmStr, loopbackStr)
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

	fmt.Printf("\nstats for port: %d\n\n", resp.GetSpec().GetKeyOrHandle().GetPortId())

	if resp.GetSpec().GetPortType() == halproto.PortType_PORT_TYPE_MGMT {
		mgmtMacStats := resp.GetStats().GetMgmtMacStats()

		for _, s := range mgmtMacStats {
			fmt.Printf("%-31s%-5d\n",
				strings.Replace(s.GetType().String(), "_", " ", -1),
				s.GetCount())
		}
	} else {
		macStats := resp.GetStats().GetMacStats()

		for _, s := range macStats {
			fmt.Printf("%-25s%-5d\n",
				strings.Replace(s.GetType().String(), "_", " ", -1),
				s.GetCount())
		}
	}

	fmt.Println(hdrLine)
}

func portDebugCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	success := true

	if cmd.Flags().Changed("pause") == false && cmd.Flags().Changed("fec-type") == false &&
		cmd.Flags().Changed("auto-neg") == false && cmd.Flags().Changed("mtu") == false &&
		cmd.Flags().Changed("admin-state") == false && cmd.Flags().Changed("speed") == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
		return
	}

	pauseType := inputToPauseType("none")
	fecType := inputToFecType("none")
	adminState := inputToAdminState("none")
	speed := inputToSpeed("none")
	autoNeg := false
	var mtu uint32

	if cmd.Flags().Changed("pause") == true {
		if isPauseTypeValid(portPause) == false {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
		pauseType = inputToPauseType(portPause)
	}

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

	if cmd.Flags().Changed("mtu") == true {
		mtu = portMtu
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

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			success = false
			continue
		}

		if cmd.Flags().Changed("pause") == false {
			pauseType = resp.GetSpec().GetPause()
		}
		if cmd.Flags().Changed("fec-type") == false {
			fecType = resp.GetSpec().GetFecType()
		}
		if cmd.Flags().Changed("auto-neg") == false && cmd.Flags().Changed("speed") == false {
			autoNeg = resp.GetSpec().GetAutoNegEnable()
		}
		if cmd.Flags().Changed("admin-state") == false {
			adminState = resp.GetSpec().GetAdminState()
		}
		if cmd.Flags().Changed("speed") == false {
			speed = resp.GetSpec().GetPortSpeed()
		}
		if cmd.Flags().Changed("mtu") == false {
			mtu = resp.GetSpec().GetMtu()
		}

		var portSpec *halproto.PortSpec
		portSpec = &halproto.PortSpec{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: resp.GetSpec().GetKeyOrHandle().GetPortId(),
				},
			},

			PortType:      resp.GetSpec().GetPortType(),
			AdminState:    adminState,
			PortSpeed:     speed,
			NumLanes:      resp.GetSpec().GetNumLanes(),
			FecType:       fecType,
			AutoNegEnable: autoNeg,
			DebounceTime:  resp.GetSpec().GetDebounceTime(),
			Mtu:           mtu,
			MacStatsReset: false,
			Pause:         pauseType,
		}

		portUpdateReqMsg := &halproto.PortRequestMsg{
			Request: []*halproto.PortSpec{portSpec},
		}

		// HAL call
		updateRespMsg, err := client.PortUpdate(context.Background(), portUpdateReqMsg)
		if err != nil {
			fmt.Printf("Update Port failed. %v\n", err)
			success = false
			continue
		}

		for _, updateResp := range updateRespMsg.Response {
			if updateResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("HAL Returned non OK status. %v\n", updateResp.ApiStatus)
				success = false
				continue
			}
		}
	}

	if success == true {
		fmt.Printf("Update port succeeded\n")
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

func inputToSpeed(str string) halproto.PortSpeed {
	switch str {
	case "none":
		return halproto.PortSpeed_PORT_SPEED_NONE
	case "1G":
		return halproto.PortSpeed_PORT_SPEED_1G
	case "10G":
		return halproto.PortSpeed_PORT_SPEED_10G
	case "25G":
		return halproto.PortSpeed_PORT_SPEED_25G
	case "40G":
		return halproto.PortSpeed_PORT_SPEED_40G
	case "50G":
		return halproto.PortSpeed_PORT_SPEED_50G
	case "100G":
		return halproto.PortSpeed_PORT_SPEED_100G
	default:
		return halproto.PortSpeed_PORT_SPEED_NONE
	}
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

func inputToAdminState(str string) halproto.PortAdminState {
	switch str {
	case "up":
		return halproto.PortAdminState_PORT_ADMIN_STATE_UP
	case "down":
		return halproto.PortAdminState_PORT_ADMIN_STATE_DOWN
	default:
		return halproto.PortAdminState_PORT_ADMIN_STATE_NONE
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

func inputToFecType(str string) halproto.PortFecType {
	switch str {
	case "none":
		return halproto.PortFecType_PORT_FEC_TYPE_NONE
	case "rs":
		return halproto.PortFecType_PORT_FEC_TYPE_RS
	case "fc":
		return halproto.PortFecType_PORT_FEC_TYPE_FC
	default:
		return halproto.PortFecType_PORT_FEC_TYPE_NONE
	}
}

func isPauseTypeValid(str string) bool {
	switch str {
	case "link-level":
		return true
	case "link":
		return true
	case "pfc":
		return true
	case "none":
		return true
	default:
		return false
	}
}

func inputToPauseType(str string) halproto.PortPauseType {
	switch str {
	case "link-level":
		return halproto.PortPauseType_PORT_PAUSE_TYPE_LINK
	case "link":
		return halproto.PortPauseType_PORT_PAUSE_TYPE_LINK
	case "pfc":
		return halproto.PortPauseType_PORT_PAUSE_TYPE_PFC
	case "none":
		return halproto.PortPauseType_PORT_PAUSE_TYPE_NONE
	default:
		return halproto.PortPauseType_PORT_PAUSE_TYPE_NONE
	}
}

func handlePortClearStatsCmd(cmd *cobra.Command, ofile *os.File) {
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

		var portSpec *halproto.PortSpec

		portSpec = &halproto.PortSpec{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: resp.GetSpec().GetKeyOrHandle().GetPortId(),
				},
			},

			PortType:      resp.GetSpec().GetPortType(),
			AdminState:    resp.GetSpec().GetAdminState(),
			PortSpeed:     resp.GetSpec().GetPortSpeed(),
			NumLanes:      resp.GetSpec().GetNumLanes(),
			FecType:       resp.GetSpec().GetFecType(),
			AutoNegEnable: resp.GetSpec().GetAutoNegEnable(),
			DebounceTime:  resp.GetSpec().GetDebounceTime(),
			Mtu:           resp.GetSpec().GetMtu(),
			MacStatsReset: true,
		}

		portUpdateReqMsg := &halproto.PortRequestMsg{
			Request: []*halproto.PortSpec{portSpec},
		}

		// HAL call
		updateRespMsg, err := client.PortUpdate(context.Background(), portUpdateReqMsg)
		if err != nil {
			fmt.Printf("Update Port failed. %v\n", err)
			continue
		}

		for _, updateResp := range updateRespMsg.Response {
			if updateResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("HAL Returned non OK status. %v\n", updateResp.ApiStatus)
				continue
			}
		}
	}
}

func portClearStatsCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 1 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handlePortClearStatsCmd(cmd, nil)
}
