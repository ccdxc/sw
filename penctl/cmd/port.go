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
	portNum uint32
)

var portCmd = &cobra.Command{
	Use:   "port",
	Short: "update port object",
	Long:  "update port object",
}

var portPauseCmd = &cobra.Command{
	Use:   "set-pause",
	Short: "penctl update port --port <> set-pause [link-level|pfc|none]",
	Long:  "penctl update port --port <> set-pause [link-level|pfc|none]",
	Run:   portPauseCmdHandler,
}

func init() {
	updateCmd.AddCommand(portCmd)
	portCmd.PersistentFlags().Uint32Var(&portNum, "port", 1, "Specify port number")
	portCmd.AddCommand(portPauseCmd)
}

func portPauseCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) != 1 {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance")
		return
	}

	if isPauseTypeValid(args[0]) == false {
		fmt.Printf("Command arguments not provided correctly. Refer to help string for guidance")
		return
	}

	halctlStr := "/nic/bin/halctl debug port "
	if cmd.Flags().Changed("port") {
		halctlStr += ("--port " + fmt.Sprint(portNum) + " ")
	}
	halctlStr += ("set-pause " + args[0])

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
	case "pfc":
		return true
	case "none":
		return true
	default:
		return false
	}
}
