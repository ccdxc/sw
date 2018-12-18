//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var (
	portNum        uint32
	portPause      string
	portFecType    string
	portAutoNeg    string
	portMtu        uint32
	portAdminState string
)

var portShowCmd = &cobra.Command{
	Use:   "port",
	Short: "show port object",
	Long:  "show port object",
	Run:   portShowCmdHandler,
}

var portCmd = &cobra.Command{
	Use:   "port",
	Short: "update port object",
	Long:  "update port object",
	Run:   portUpdateCmdHandler,
}

func init() {
	updateCmd.AddCommand(portCmd)
	portCmd.Flags().Uint32Var(&portNum, "port", 1, "Specify port number")
	portCmd.Flags().StringVar(&portPause, "pause", "none", "Specify pause - link, pfc, none")
	portCmd.Flags().StringVar(&portFecType, "fec-type", "none", "Specify fec-type - rs, fc, none")
	portCmd.Flags().StringVar(&portAutoNeg, "auto-neg", "disable", "Enable or disable auto-neg using enable | disable")
	portCmd.Flags().StringVar(&portAdminState, "admin-state", "up", "Set port admin state - none, up, down")
	portCmd.Flags().Uint32Var(&portMtu, "mtu", 0, "Specify port MTU")

	showCmd.AddCommand(portShowCmd)
	portShowCmd.Flags().Uint32Var(&portNum, "port", 1, "Specify port number")
}

func portShowCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "/nic/bin/halctl show port "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum))
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.NaplesCmdExecute{
		Executable: execCmd[0],
		Opts:       strings.Join(execCmd[1:], " "),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}

	return
}

func portUpdateCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("pause") == false && cmd.Flags().Changed("fec-type") == false &&
		cmd.Flags().Changed("auto-neg") == false && cmd.Flags().Changed("mtu") == false &&
		cmd.Flags().Changed("admin-state") == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
		return
	}

	halctlStr := "/nic/bin/halctl debug port "
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

	if cmd.Flags().Changed("mtu") == true {
		halctlStr += ("--mtu " + fmt.Sprint(portMtu) + " ")
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.NaplesCmdExecute{
		Executable: execCmd[0],
		Opts:       strings.Join(execCmd[1:], " "),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}

	return
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
	case "none":
		return true
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
