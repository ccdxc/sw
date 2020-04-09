//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

const cmdName string = "time"

var showSystemTimeCmd = &cobra.Command{
	Use:   cmdName,
	Short: "Show system clock time from Distributed Service Card",
	Long:  "\n------------------------------------\n Show system clock time from Distributed Service Card \n------------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  showSystemTimeCmdHandler,
}

var setSystemTimeCmd = &cobra.Command{
	Use:   cmdName,
	Short: "Set system clock time on Distributed Service Card",
	Long:  "\n---------------------------------\n Set system clock time on Distributed Service Card \n---------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  setSystemTimeCmdHandler,
}

func init() {
	showCmd.AddCommand(showSystemTimeCmd)

	updateCmd.AddCommand(setSystemTimeCmd)
}

func showSystemTimeCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "getdate",
		Opts:       "",
	}
	return naplesExecCmd(v)
}
