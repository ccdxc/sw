//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var (
	portNum        string
	portPause      string
	txPause        string
	rxPause        string
	portFecType    string
	portAutoNeg    string
	portMtu        uint32
	portAdminState string
	portSpeed      string
	portNumLanes   uint32
)

var portShowCmd = &cobra.Command{
	Use:   "port",
	Short: "show port object",
	Long:  "show port object",
	Args:  cobra.NoArgs,
	RunE:  portShowCmdHandler,
}

var portStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show port object status",
	Long:  "show port object status",
	Args:  cobra.NoArgs,
	RunE:  portStatusShowCmdHandler,
}

var portUpdateCmd = &cobra.Command{
	Use:    "port",
	Short:  "update port object",
	Long:   "update port object",
	Args:   cobra.NoArgs,
	Hidden: true,
	RunE:   portUpdateCmdHandler,
}

var portStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show port statistics",
	Long:  "show port statistics",
	Args:  cobra.NoArgs,
	RunE:  portStatsShowCmdHandler,
}

func init() {
	updateCmd.AddCommand(portUpdateCmd)

	portUpdateCmd.Flags().StringVar(&portNum, "port", "eth1/1", "Specify port number")
	portUpdateCmd.Flags().StringVar(&portPause, "pause", "none", "Specify pause - link, pfc, none")
	portUpdateCmd.Flags().StringVar(&txPause, "tx-pause", "enable", "Enable or disable TX pause using enable | disable")
	portUpdateCmd.Flags().StringVar(&rxPause, "rx-pause", "enable", "Enable or disable RX pause using enable | disable")
	portUpdateCmd.Flags().StringVar(&portFecType, "fec-type", "none", "Specify fec-type - rs, fc, none")
	portUpdateCmd.Flags().StringVar(&portAutoNeg, "auto-neg", "enable", "Enable or disable auto-neg using enable | disable")
	portUpdateCmd.Flags().StringVar(&portAdminState, "admin-state", "up", "Set port admin state - up, down")
	portUpdateCmd.Flags().StringVar(&portSpeed, "speed", "", "Set port speed - none, 1g, 10g, 25g, 40g, 50g, 100g")
	portUpdateCmd.Flags().Uint32Var(&portMtu, "mtu", 0, "Specify port MTU")
	portUpdateCmd.Flags().Uint32Var(&portNumLanes, "num-lanes", 4, "Specify number of lanes")

	showCmd.AddCommand(portShowCmd)
	portShowCmd.AddCommand(portStatusShowCmd)
	portShowCmd.Flags().StringVar(&portNum, "port", "eth1/1", "Specify port number")
	portShowCmd.AddCommand(portStatsShowCmd)
}

func portStatsShowCmdHandler(cmd *cobra.Command, args []string) error {
	halctlStr := "halctl show port statistics"
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum))
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowport",
		Opts:       strings.Join(execCmd[3:], " "),
	}

	return naplesExecCmd(v)
}

func portShowCmdHandler(cmd *cobra.Command, args []string) error {
	halctlStr := "halctl show port "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum))
		execCmd := strings.Fields(halctlStr)
		v := &nmd.DistributedServiceCardCmdExecute{
			Executable: "halctlshowport",
			Opts:       strings.Join(execCmd[3:], " "),
		}
		return naplesExecCmd(v)
	}

	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowport",
		Opts:       strings.Join([]string{""}, ""),
	}

	return naplesExecCmd(v)
}

func portStatusShowCmdHandler(cmd *cobra.Command, args []string) error {
	halctlStr := "halctl show port status "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum))
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowport",
		Opts:       strings.Join(execCmd[3:], " "),
	}

	return naplesExecCmd(v)
}

func portUpdateCmdHandler(cmd *cobra.Command, args []string) error {
	if cmd.Flags().Changed("pause") == false && cmd.Flags().Changed("tx-pause") == false &&
		cmd.Flags().Changed("rx-pause") == false && cmd.Flags().Changed("fec-type") == false &&
		cmd.Flags().Changed("auto-neg") == false && cmd.Flags().Changed("mtu") == false &&
		cmd.Flags().Changed("admin-state") == false && cmd.Flags().Changed("speed") == false &&
		cmd.Flags().Changed("num-lanes") == false {
		return errors.New("Command arguments not provided correctly. Refer to help string for guidance")
	}

	halctlStr := "halctl debug port "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum) + " ")
	}

	if cmd.Flags().Changed("pause") == true {
		if isPauseTypeValid(portPause) == false {
			return errors.New("Command arguments not provided correctly. Refer to help string for guidance")
		}
		halctlStr += ("--pause " + portPause + " ")
	}

	if cmd.Flags().Changed("tx-pause") == true {
		halctlStr += ("--tx-pause " + txPause + " ")
	}

	if cmd.Flags().Changed("rx-pause") == true {
		halctlStr += ("--rx-pause " + rxPause + " ")
	}

	if cmd.Flags().Changed("fec-type") == true {
		if isFecTypeValid(portFecType) == false {
			return errors.New("Command arguments not provided correctly. Refer to help string for guidance")
		}
		halctlStr += ("--fec-type " + portFecType + " ")
	}

	if cmd.Flags().Changed("auto-neg") == true {
		if strings.Compare(portAutoNeg, "disable") == 0 {
			halctlStr += ("--auto-neg " + portAutoNeg + " ")
		} else if strings.Compare(portAutoNeg, "enable") == 0 {
			halctlStr += ("--auto-neg " + portAutoNeg + " ")
		} else {
			return errors.New("Command arguments not provided correctly. Refer to help string for guidance")
		}
	}

	if cmd.Flags().Changed("admin-state") == true {
		if isAdminStateValid(portAdminState) == false {
			return errors.New("Command arguments not provided correctly. Refer to help string for guidance")
		}
		halctlStr += ("--admin-state " + portAdminState + " ")
	}

	if cmd.Flags().Changed("speed") == true {
		if isSpeedValid(strings.ToUpper(portSpeed)) == false {
			return errors.New("Command arguments not provided correctly. Refer to help string for guidance")
		}
		halctlStr += ("--speed " + strings.ToUpper(portSpeed) + " ")
	}

	if cmd.Flags().Changed("mtu") == true {
		halctlStr += ("--mtu " + fmt.Sprint(portMtu) + " ")
	}

	if cmd.Flags().Changed("num-lanes") == true {
		halctlStr += ("--num-lanes " + fmt.Sprint(portNumLanes) + " ")
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctldebug",
		Opts:       strings.Join(execCmd[2:], " "),
	}

	return naplesExecCmd(v)
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
