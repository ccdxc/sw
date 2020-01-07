//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"io/ioutil"
	"path/filepath"
	"strings"
	"time"

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

func setSystemTimeCmdHandler(cmd *cobra.Command, args []string) error {
	timezone, err := ioutil.ReadFile("/etc/timezone")
	if err == nil {
		v := &nmd.DistributedServiceCardCmdExecute{
			Executable: "pensettimezone",
			Opts:       strings.Join([]string{string(timezone)}, ""),
		}
		if err := naplesExecCmd(v); err != nil {
			return err
		}
	}
	symlink, err := filepath.EvalSymlinks("/etc/localtime")
	if err == nil {
		v := &nmd.DistributedServiceCardCmdExecute{
			Executable: "lnlocaltime",
			Opts:       strings.Join([]string{symlink}, " "),
		}
		if err := naplesExecCmd(v); err != nil {
			return err
		}
	}
	dateString := time.Now().Format("Jan 2 15:04:05 2006")
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "setdate",
		Opts:       strings.Join([]string{dateString}, ""),
	}
	if err := naplesExecCmd(v); err != nil {
		return err
	}

	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "sethwclock",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func showSystemTimeCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "getdate",
		Opts:       "",
	}
	return naplesExecCmd(v)
}
