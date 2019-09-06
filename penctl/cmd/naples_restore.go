//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var factoryDefaultCmd = &cobra.Command{
	Use:   "factory-default",
	Short: "Perform \"erase-config\" plus remove all Naples internal databases and diagnostic failure logs (reboot required)",
	Long:  "\n------------------------------------------------------------------------------------------------------------------\n Perform \"erase-config\" plus remove all Naples internal databases and diagnostic failure logs (reboot required) \n------------------------------------------------------------------------------------------------------------------\n",
	RunE:  factoryDefaultCmdHandler,
}

var eraseConfigCmd = &cobra.Command{
	Use:   "erase-config",
	Short: "Erase all local Naples configuration and revert to \"host-managed\" mode. (reboot required)",
	Long:  "\n---------------------------------------------------------------------------------------------\n Erase all local Naples configuration and revert to \"host-managed\" mode. (reboot required) \n---------------------------------------------------------------------------------------------\n",
	RunE:  eraseConfigCmdHandler,
}

func init() {
	sysCmd.AddCommand(factoryDefaultCmd)
	sysCmd.AddCommand(eraseConfigCmd)
}

func eraseConfigCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "eraseConfig",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmdNoPrint(v)
}

func factoryDefaultCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "factoryDefault",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmdNoPrint(v)
}
