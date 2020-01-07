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
	Short: "Perform \"erase-config\" plus remove all Distributed Service Card internal databases and diagnostic failure logs (reboot required)",
	Long:  "\n------------------------------------------------------------------------------------------------------------------\n Perform \"erase-config\" plus remove all Distributed Service Card internal databases and diagnostic failure logs (reboot required) \n------------------------------------------------------------------------------------------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  factoryDefaultCmdHandler,
}

var eraseConfigCmd = &cobra.Command{
	Use:   "erase-config",
	Short: "Erase all local Distributed Service Card configuration and revert to \"host-managed\" mode. (reboot required)",
	Long:  "\n---------------------------------------------------------------------------------------------\n Erase all local Distributed Service Card configuration and revert to \"host-managed\" mode. (reboot required) \n---------------------------------------------------------------------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  eraseConfigCmdHandler,
}

func init() {
	sysCmd.AddCommand(factoryDefaultCmd)
	sysCmd.AddCommand(eraseConfigCmd)
}

func eraseConfigCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "eraseConfig",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmdNoPrint(v)
}

func factoryDefaultCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "factoryDefault",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmdNoPrint(v)
}
