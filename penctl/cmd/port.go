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
	portNum   uint32
	portPause string
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
	if cmd.Flags().Changed("pause") == false || isPauseTypeValid(portPause) == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance\n")
		return
	}

	halctlStr := "/nic/bin/halctl debug port "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum) + " ")
	}
	halctlStr += ("--pause " + portPause)

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
