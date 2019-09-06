//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var enableConsoleConfigCmd = &cobra.Command{
	Use:   "enable-console",
	Short: "Enable serial console on Naples",
	Long:  "\n------------------------------\n Enable serial console on Naples \n------------------------------\n",
	RunE:  enableConsoleConfigCmdHandler,
}

var disableConsoleConfigCmd = &cobra.Command{
	Use:   "disable-console",
	Short: "Disable serial console on Naples",
	Long:  "\n------------------------------\n Disable serial console on Naples \n------------------------------\n",
	RunE:  disableConsoleConfigCmdHandler,
}

func init() {
	sysCmd.AddCommand(enableConsoleConfigCmd)
	sysCmd.AddCommand(disableConsoleConfigCmd)
}

func disableConsoleConfigCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "consoledisable",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func enableConsoleConfigCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "consoleenable",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}
