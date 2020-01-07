//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
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

var portShowCmd = &cobra.Command{
	Use:   "port",
	Short: "show port object",
	Long:  "show port object",
	Args:  cobra.NoArgs,
	Run:   portShowCmdHandler,
}

var portStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show port object status",
	Long:  "show port object status",
	Args:  cobra.NoArgs,
	Run:   portStatusShowCmdHandler,
}

var portCmd = &cobra.Command{
	Use:   "port",
	Short: "update port object",
	Long:  "update port object",
	Args:  cobra.NoArgs,
	Run:   portUpdateCmdHandler,
}

var portStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show port statistics",
	Long:  "show port statistics",
	Args:  cobra.NoArgs,
	Run:   portStatsShowCmdHandler,
}

func init() {
	updateCmd.AddCommand(portCmd)
	portCmd.Flags().Uint32Var(&portNum, "port", 0, "Specify port number")
	portCmd.Flags().StringVar(&portPause, "pause", "none", "Specify pause - link, pfc, none")
	portCmd.Flags().StringVar(&portFecType, "fec-type", "none", "Specify fec-type - rs, fc, none")
	portCmd.Flags().StringVar(&portAutoNeg, "auto-neg", "enable", "Enable or disable auto-neg using enable | disable")
	portCmd.Flags().StringVar(&portAdminState, "admin-state", "up", "Set port admin state - up, down")
	portCmd.Flags().StringVar(&portSpeed, "speed", "", "Set port speed - none, 1g, 10g, 25g, 40g, 50g, 100g")
	portCmd.Flags().Uint32Var(&portMtu, "mtu", 0, "Specify port MTU")
	portCmd.MarkFlagRequired("port")

	showCmd.AddCommand(portShowCmd)
	portShowCmd.AddCommand(portStatusShowCmd)
	portShowCmd.PersistentFlags().Uint32Var(&portNum, "port", 1, "Specify port number")

	portShowCmd.AddCommand(portStatsShowCmd)
}

func portStatsShowCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "halctl show port statistics"
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum))
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowport",
		Opts:       strings.Join(execCmd[3:], " "),
	}

	naplesExecCmd(v)

	return
}

func portShowCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "halctl show port "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum))
		execCmd := strings.Fields(halctlStr)
		v := &nmd.DistributedServiceCardCmdExecute{
			Executable: "halctlshowport",
			Opts:       strings.Join(execCmd[3:], " "),
		}
		naplesExecCmd(v)
		return
	}

	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowport",
		Opts:       strings.Join([]string{""}, ""),
	}

	naplesExecCmd(v)

	return
}

func portStatusShowCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "halctl show port status "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum))
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowport",
		Opts:       strings.Join(execCmd[3:], " "),
	}

	naplesExecCmd(v)

	return
}

func portUpdateCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("pause") == false && cmd.Flags().Changed("fec-type") == false &&
		cmd.Flags().Changed("auto-neg") == false && cmd.Flags().Changed("mtu") == false &&
		cmd.Flags().Changed("admin-state") == false && cmd.Flags().Changed("speed") == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
		return
	}

	halctlStr := "halctl debug port "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum) + " ")
	}

	if cmd.Flags().Changed("pause") == true {
		if isPauseTypeValid(portPause) == false {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
		halctlStr += ("--pause " + portPause + " ")
	}

	if cmd.Flags().Changed("fec-type") == true {
		if isFecTypeValid(portFecType) == false {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
		halctlStr += ("--fec-type " + portFecType + " ")
	}

	if cmd.Flags().Changed("auto-neg") == true {
		if strings.Compare(portAutoNeg, "disable") == 0 {
			halctlStr += ("--auto-neg " + portAutoNeg + " ")
		} else if strings.Compare(portAutoNeg, "enable") == 0 {
			halctlStr += ("--auto-neg " + portAutoNeg + " ")
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
		halctlStr += ("--admin-state " + portAdminState + " ")
	}

	if cmd.Flags().Changed("speed") == true {
		if isSpeedValid(strings.ToUpper(portSpeed)) == false {
			fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
			return
		}
		halctlStr += ("--speed " + strings.ToUpper(portSpeed) + " ")
	}

	if cmd.Flags().Changed("mtu") == true {
		halctlStr += ("--mtu " + fmt.Sprint(portMtu) + " ")
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctldebug",
		Opts:       strings.Join(execCmd[2:], " "),
	}

	naplesExecCmd(v)

	return
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
