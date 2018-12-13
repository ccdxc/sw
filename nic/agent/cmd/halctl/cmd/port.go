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
}

var portPauseCmd = &cobra.Command{
	Use:   "set-pause",
	Short: "halctl debug port --port <> set-pause [link-level|pfc|none]",
	Long:  "halctl debug port --port <> set-pause [link-level|pfc|none]",
	Run:   portPauseCmdHandler,
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
	portDebugCmd.PersistentFlags().Uint32Var(&portNum, "port", 1, "Specify port number")
	portDebugCmd.AddCommand(portPauseCmd)
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
	hdrLine := strings.Repeat("-", 110)
	fmt.Println("MAC-Info: MAC ID/MAC Channel/Num lanes          Debounce: Debounce time in ms")
	fmt.Println("FEC-Type: FC - FireCode, RS - ReedSolomon")
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-15s%-10s%-15s%-6s%-10s%-10s%-12s%-12s\n",
		"Port", "Speed", "MAC-Info", "FEC-Type", "AutoNegEnable", "MTU", "Pause", "Debounce", "AdminStatus", "OperStatus")
	fmt.Println(hdrLine)
}

func portShowOneResp(resp *halproto.PortGetResponse) {
	spec := resp.GetSpec()
	macStr := fmt.Sprintf("%d/%d/%d", spec.GetMacId(), spec.GetMacCh(), spec.GetNumLanes())
	speedStr := strings.Replace(spec.GetPortSpeed().String(), "PORT_SPEED_", "", -1)
	fecStr := strings.Replace(spec.GetFecType().String(), "PORT_FEC_TYPE_", "", -1)
	if strings.Compare(fecStr, "NONE") == 0 {
		fecStr = "None"
	}
	portStr := strings.ToLower(strings.Replace(spec.GetPortType().String(), "PORT_TYPE_", "", -1))
	portStr = fmt.Sprintf("%s%d", portStr, spec.GetKeyOrHandle().GetPortId())
	pauseStr := strings.ToLower(strings.Replace(spec.GetPause().String(), "PORT_PAUSE_TYPE_", "", -1))
	adminStateStr := strings.Replace(resp.GetSpec().GetAdminState().String(), "PORT_ADMIN_STATE_", "", -1)
	operStatusStr := strings.Replace(resp.GetStatus().GetOperStatus().String(), "PORT_OPER_STATUS_", "", -1)

	fmt.Printf("%-10s%-10s%-15s%-10s%-15t%-6d%-10s%-10d%-12s%-12s\n",
		portStr, speedStr, macStr, fecStr, spec.GetAutoNegEnable(),
		spec.GetMtu(), pauseStr, spec.GetDebounceTime(),
		adminStateStr, operStatusStr)
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

func portPauseCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	success := true
	if len(args) != 1 {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance")
		return
	}

	if isPauseTypeValid(args[0]) == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance")
		return
	}
	pauseType := inputToPauseType(args[0])

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

func isPauseTypeValid(str string) bool {
	switch str {
	case "link-level":
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
