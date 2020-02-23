//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"sort"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	portNum        string
	portPause      string
	portFecType    string
	txPause        string
	rxPause        string
	portAutoNeg    string
	portMtu        uint32
	portAdminState string
	portSpeed      string
	aacsServerPort int32
	portNumLanes   uint32
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

var portShowXcvrCmd = &cobra.Command{
	Use:   "transceiver",
	Short: "show port transceiver",
	Long:  "show port transceiver",
	Run:   portXcvrShowCmdHandler,
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

var portDebugAacsStartCmd = &cobra.Command{
	Use:   "aacs-server-start",
	Short: "debug port aacs-server-start",
	Long:  "debug port aacs-server-start",
	Run:   portDebugAacsStartCmdHandler,
}

var portDebugAacsStopCmd = &cobra.Command{
	Use:   "aacs-server-stop",
	Short: "debug port aacs-server-stop",
	Long:  "debug port aacs-server-stop",
	Run:   portDebugAacsStopCmdHandler,
}

// ValidateMtu returns true if the MTU is within 64-9216
func ValidateMtu(mtu uint32) bool {
	if mtu < 64 || mtu > 9216 {
		fmt.Printf("Invalid MTU. MTU must be in the range 64-9216\n")
		return false
	}
	return true
}

// ValidateNumLanes returns true if the numLanes is between 1 to 4
func ValidateNumLanes(numLanes uint32) bool {
	if numLanes < 1 || numLanes > 4 {
		fmt.Printf("Invalid num-lanes. num-lanes must be in the range 1-4\n")
		return false
	}
	return true
}

func init() {
	showCmd.AddCommand(portShowCmd)
	portShowCmd.AddCommand(portStatusShowCmd)
	portShowCmd.AddCommand(portShowXcvrCmd)
	portShowCmd.AddCommand(portStatsShowCmd)

	portShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	portShowCmd.PersistentFlags().StringVar(&portNum, "port", "eth1/1", "Specify port number")

	clearCmd.AddCommand(portClearStatsCmd)
	portClearStatsCmd.Flags().StringVar(&portNum, "port", "eth1/1", "Speficy port number")

	debugCmd.AddCommand(portDebugCmd)
	portDebugCmd.Flags().StringVar(&portNum, "port", "eth1/1", "Specify port number")
	portDebugCmd.Flags().StringVar(&portPause, "pause", "none", "Specify pause - link, pfc, none")
	portDebugCmd.Flags().StringVar(&txPause, "tx-pause", "enable", "Enable or disable TX pause using enable | disable")
	portDebugCmd.Flags().StringVar(&rxPause, "rx-pause", "enable", "Enable or disable RX pause using enable | disable")
	portDebugCmd.Flags().StringVar(&portFecType, "fec-type", "none", "Specify fec-type - rs, fc, none")
	portDebugCmd.Flags().StringVar(&portAutoNeg, "auto-neg", "enable", "Enable or disable auto-neg using enable | disable")
	portDebugCmd.Flags().StringVar(&portAdminState, "admin-state", "up", "Set port admin state - up, down")
	portDebugCmd.Flags().StringVar(&portSpeed, "speed", "", "Set port speed - none, 1g, 10g, 25g, 40g, 50g, 100g")
	portDebugCmd.Flags().Uint32Var(&portMtu, "mtu", 0, "Specify port MTU")
	portDebugCmd.Flags().Uint32Var(&portNumLanes, "num-lanes", 4, "Specify number of lanes")

	// debug port aacs-server-start
	portDebugCmd.AddCommand(portDebugAacsStartCmd)
	portDebugAacsStartCmd.Flags().Int32Var(&aacsServerPort, "server-port", 9000, "Specify AACS server listen port")
	// debug port aacs-server-stop
	portDebugCmd.AddCommand(portDebugAacsStopCmd)
}

func handlePortDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewPortClient(c)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portIDStrToIfIndex(portNum),
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
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
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

	client := halproto.NewPortClient(c)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portIDStrToIfIndex(portNum),
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

	m := make(map[uint32]*halproto.PortGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		m[resp.GetSpec().GetKeyOrHandle().GetPortId()] = resp
	}
	var keys []uint32
	for k := range m {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	for _, k := range keys {
		resp := m[k]
		adminState := resp.GetSpec().GetAdminState()
		operStatus := resp.GetStatus().GetLinkStatus().GetOperState()

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

		fmt.Printf("%-10s%-12s%-10s%-14s\n",
			utils.IfIndexToStr(resp.GetSpec().GetKeyOrHandle().GetPortId()),
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

func portXcvrShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := halproto.NewPortClient(c)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portIDStrToIfIndex(portNum),
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
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		portXcvrShowResp(resp)
	}
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

	client := halproto.NewPortClient(c)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portIDStrToIfIndex(portNum),
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

	m := make(map[uint32]*halproto.PortGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		m[resp.GetSpec().GetKeyOrHandle().GetPortId()] = resp
	}
	var keys []uint32
	for k := range m {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	for _, k := range keys {
		portShowOneResp(m[k])
	}
}

func portShowHeaderPrint() {
	hdrLine := strings.Repeat("-", 169)
	fmt.Println("MAC-Info: MAC ID/MAC Channel/Num lanes          Debounce: Debounce time in ms")
	fmt.Println("FEC-Type: FC - FireCode, RS - ReedSolomon")
	fmt.Println(hdrLine)
	fmt.Printf("%-12s%-7s%-10s%-10s"+"%-12s%-15s"+"%-6s"+"%-6s%-8s%-8s"+"%-10s%-12s%-12s"+"%-12s%-20s%-10s\n",
		"PortType/ID", "Speed", "MAC-Info", "FEC-Type",
		"AutoNegCfg", "AutoNegEnable",
		"MTU",
		"Pause", "TxPause", "RxPause",
		"Debounce", "AdminStatus", "OperStatus",
		"NumLinkDown", "LinkSM", "Loopback")
	fmt.Println(hdrLine)
}

func portXcvrShowResp(resp *halproto.PortGetResponse) {
	xcvrStatus := resp.GetStatus().GetXcvrStatus()

	xcvrPortNum := xcvrStatus.GetPort()
	xcvrState := xcvrStatus.GetState()
	xcvrPid := xcvrStatus.GetPid()

	xcvrStateStr := "-"
	xcvrPidStr := "-"

	if xcvrPortNum <= 0 {
		return
	}

	// Strip XCVR_STATE_ from the state
	xcvrStateStr = strings.Replace(strings.Replace(xcvrState.String(), "XCVR_STATE_", "", -1), "_", "-", -1)

	// Strip XCVR_PID_ from the pid
	xcvrPidStr = strings.Replace(strings.Replace(xcvrPid.String(), "XCVR_PID_", "", -1), "_", "-", -1)

	xcvrSprom := xcvrStatus.GetXcvrSprom()

	lengthOm3 := 0
	vendorRev := ""
	lengthSmfKm := int(xcvrSprom[14])
	lengthOm2 := int(xcvrSprom[16])
	lengthOm1 := int(xcvrSprom[17])
	lengthDac := int(xcvrSprom[18])
	vendorName := string(xcvrSprom[20:35])
	vendorPn := string(xcvrSprom[40:55])
	vendorSn := string(xcvrSprom[68:83])

	if strings.Contains(xcvrPid.String(), "QSFP") {
		// convert from units of 2m to meters
		lengthOm3 = int(xcvrSprom[15]) * 2

		vendorRev = string(xcvrSprom[56:57])
	} else {
		lengthOm3 = int(xcvrSprom[19])
		vendorRev = string(xcvrSprom[56:59])

		// convert from units of 10m to meters
		lengthOm1 *= 10
		lengthOm2 *= 10
		lengthOm3 *= 10
	}

	fmt.Printf("Port: %d\n"+
		"State: %s\n"+
		"PID: %s\n"+
		"Length Single Mode Fiber: %d KM\n"+
		"Length 62.5um OM1 Fiber:  %d Meters\n"+
		"Length 50um   OM2 Fiber:  %d Meters\n"+
		"Length 50um   OM3 Fiber:  %d Meters\n"+
		"Length Copper:            %d Meters\n"+
		"vendor name: %s\n"+
		"vendor part number: %s\n"+
		"vendor revision: %s\n"+
		"vendor serial number: %s\n",
		xcvrPortNum, xcvrStateStr, xcvrPidStr,
		lengthSmfKm,
		lengthOm1,
		lengthOm2,
		lengthOm3,
		lengthDac,
		vendorName,
		vendorPn,
		vendorRev,
		vendorSn)
}

func portIDStrToIfIndex(portIDStr string) uint32 {
	var slotIndex uint32
	var portIndex uint32
	var ifIndex uint32

	portIDStr = strings.ToLower(portIDStr)
	n, err := fmt.Sscanf(portIDStr, "eth%d/%d", &slotIndex, &portIndex)
	if err != nil || n != 2 {
		return utils.InvalidIfIndex
	}
	ifIndex = 1 << utils.IfTypeShift // 1 is Eth port
	ifIndex |= slotIndex << utils.IfSlotShift
	ifIndex |= portIndex << utils.IfParentPortShift
	ifIndex |= 1 // Default child port

	return ifIndex
}

func portShowOneResp(resp *halproto.PortGetResponse) {
	spec := resp.GetSpec()
	status := resp.GetStatus()
	linkStatus := status.GetLinkStatus()
	stats := resp.GetStats()
	if spec == nil {
		fmt.Printf("Error! Port spec cannot be nil\n")
		return
	}
	if spec.GetKeyOrHandle() == nil {
		fmt.Printf("Error! Port key handle cannot be nil\n")
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
	macStr := fmt.Sprintf("%d/%d/%d", spec.GetMacId(), spec.GetMacCh(), linkStatus.GetNumLanes())
	speedStr := strings.Replace(spec.GetPortSpeed().String(), "PORT_SPEED_", "", -1)
	fecStr := strings.Replace(spec.GetFecType().String(), "PORT_FEC_TYPE_", "", -1)
	linkSmStr := strings.Replace(resp.GetLinksmState().String(), "PORT_LINK_SM_", "", -1)
	loopbackStr := strings.Replace(spec.GetLoopbackMode().String(), "PORT_LOOPBACK_MODE_", "", -1)

	if strings.Compare(fecStr, "NONE") == 0 {
		fecStr = "None"
	}

	portStr := utils.IfIndexToStr(spec.GetKeyOrHandle().GetPortId())
	pauseStr := strings.ToLower(strings.Replace(spec.GetPause().String(), "PORT_PAUSE_TYPE_", "", -1))
	adminStateStr := strings.Replace(spec.GetAdminState().String(), "PORT_ADMIN_STATE_", "", -1)
	operStatusStr := strings.Replace(linkStatus.GetOperState().String(), "PORT_OPER_STATUS_", "", -1)

	fmt.Printf("%-12s%-7s%-10s%-10s"+"%-12t%-15t"+"%-6d"+"%-6s%-8t%-8t"+"%-10d%-12s%-12s"+"%-12d%-20s%-10s\n",
		portStr, speedStr, macStr, fecStr,
		spec.GetAutoNegEnable(), linkStatus.GetAutoNegEnable(),
		spec.GetMtu(),
		pauseStr, spec.GetTxPauseEnable(), spec.GetRxPauseEnable(),
		spec.GetDebounceTime(), adminStateStr, operStatusStr,
		stats.GetNumLinkDown(), linkSmStr, loopbackStr)
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

	client := halproto.NewPortClient(c)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portIDStrToIfIndex(portNum),
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
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
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

	fmt.Printf("\nstats for port: %s\n\n", utils.IfIndexToStr(resp.GetSpec().GetKeyOrHandle().GetPortId()))

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

func portDebugAacsStartCmdHandler(cmd *cobra.Command, args []string) {
	var aacsRequestMsg *halproto.AacsRequestMsg

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if (cmd == nil) ||
		(cmd.Flags().Changed("server-port") == false) {
		fmt.Printf("Specify server port\n")
		return
	}

	aacsRequestMsg = &halproto.AacsRequestMsg{
		AacsServerPort: aacsServerPort,
	}

	client := halproto.NewPortClient(c)

	// HAL call
	_, err = client.StartAacsServer(context.Background(), aacsRequestMsg)
	if err != nil {
		fmt.Printf("Start AACS server failed. %v\n", err)
		return
	}
}

func portDebugAacsStopCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewPortClient(c)

	var empty *halproto.Empty

	// HAL call
	_, err = client.StopAacsServer(context.Background(), empty)
	if err != nil {
		fmt.Printf("Stop AACS server failed. %v\n", err)
		return
	}
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

	if cmd.Flags().Changed("pause") == false && cmd.Flags().Changed("tx-pause") == false &&
		cmd.Flags().Changed("rx-pause") == false && cmd.Flags().Changed("fec-type") == false &&
		cmd.Flags().Changed("auto-neg") == false && cmd.Flags().Changed("mtu") == false &&
		cmd.Flags().Changed("admin-state") == false && cmd.Flags().Changed("speed") == false &&
		cmd.Flags().Changed("num-lanes") == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
		return
	}

	pauseType := inputToPauseType("none")
	fecType := inputToFecType("none")
	adminState := inputToAdminState("none")
	speed := inputToSpeed("none")
	autoNeg := false
	var mtu uint32
	var numLanes uint32
	txPauseEnable := true
	rxPauseEnable := true

	if cmd.Flags().Changed("pause") == true {
		if isPauseTypeValid(portPause) == false {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
		pauseType = inputToPauseType(portPause)
	}

	if cmd.Flags().Changed("tx-pause") == true {
		if strings.Compare(txPause, "disable") == 0 {
			txPauseEnable = false
		} else if strings.Compare(txPause, "enable") == 0 {
			txPauseEnable = true
		} else {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
	}

	if cmd.Flags().Changed("rx-pause") == true {
		if strings.Compare(rxPause, "disable") == 0 {
			rxPauseEnable = false
		} else if strings.Compare(rxPause, "enable") == 0 {
			rxPauseEnable = true
		} else {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
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
		if ValidateMtu(portMtu) == false {
			return
		}
		mtu = portMtu
	}

	if cmd.Flags().Changed("num-lanes") == true {
		if ValidateNumLanes(portNumLanes) == false {
			return
		}
		numLanes = portNumLanes
	}

	client := halproto.NewPortClient(c)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portIDStrToIfIndex(portNum),
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
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			success = false
			continue
		}

		if cmd.Flags().Changed("pause") == false {
			pauseType = resp.GetSpec().GetPause()
		}
		if cmd.Flags().Changed("tx-pause") == false {
			txPauseEnable = resp.GetSpec().GetTxPauseEnable()
		}
		if cmd.Flags().Changed("rx-pause") == false {
			rxPauseEnable = resp.GetSpec().GetRxPauseEnable()
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
		if cmd.Flags().Changed("num-lanes") == false {
			numLanes = resp.GetSpec().GetNumLanes()
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
			NumLanes:      numLanes,
			FecType:       fecType,
			AutoNegEnable: autoNeg,
			DebounceTime:  resp.GetSpec().GetDebounceTime(),
			Mtu:           mtu,
			MacStatsReset: false,
			Pause:         pauseType,
			TxPauseEnable: txPauseEnable,
			RxPauseEnable: rxPauseEnable,
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
				fmt.Printf("Operation failed with %v error\n", updateResp.ApiStatus)
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

	client := halproto.NewPortClient(c)

	var req *halproto.PortGetRequest

	if cmd != nil && cmd.Flags().Changed("port") {
		// Get port info for specified port
		req = &halproto.PortGetRequest{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: portIDStrToIfIndex(portNum),
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
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
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
			Pause:         resp.GetSpec().GetPause(),
			TxPauseEnable: resp.GetSpec().GetTxPauseEnable(),
			RxPauseEnable: resp.GetSpec().GetRxPauseEnable(),
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
				fmt.Printf("Operation failed with %v error\n", updateResp.ApiStatus)
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
