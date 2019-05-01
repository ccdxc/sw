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
	Short: "Show system clock time from Naples",
	Long:  "\n------------------------------------\n Show system clock time from Naples \n------------------------------------\n",
	RunE:  showSystemTimeCmdHandler,
}

var setSystemTimeCmd = &cobra.Command{
	Use:   cmdName,
	Short: "Set system clock time on Naples",
	Long:  "\n---------------------------------\n Set system clock time on Naples \n---------------------------------\n",
	RunE:  setSystemTimeCmdHandler,
}

func init() {
	showCmd.AddCommand(showSystemTimeCmd)

	updateCmd.AddCommand(setSystemTimeCmd)
}

func setSystemTimeCmdHandler(cmd *cobra.Command, args []string) error {
	timezone, err := ioutil.ReadFile("/etc/timezone")
	if err == nil {
		v := &nmd.NaplesCmdExecute{
			Executable: "pensettimezone",
			Opts:       strings.Join([]string{string(timezone)}, ""),
		}
		if err := naplesExecCmd(v); err != nil {
			return err
		}
	}
	symlink, err := filepath.EvalSymlinks("/etc/localtime")
	if err == nil {
		v := &nmd.NaplesCmdExecute{
			Executable: "/bin/ln",
			Opts:       strings.Join([]string{"-sf", symlink, "/etc/localtime"}, " "),
		}
		if err := naplesExecCmd(v); err != nil {
			return err
		}
	}
	dateString := time.Now().Format("Jan 2 15:04:05 2006")
	opts := strings.Join([]string{"--set ", dateString}, "")
	v := &nmd.NaplesCmdExecute{
		Executable: "/bin/date",
		Opts:       opts,
	}
	if err := naplesExecCmd(v); err != nil {
		return err
	}

	v = &nmd.NaplesCmdExecute{
		Executable: "/sbin/hwclock",
		Opts:       strings.Join([]string{"-wu"}, ""),
	}
	return naplesExecCmd(v)
}

func showSystemTimeCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "/bin/date",
		Opts:       "",
	}
	return naplesExecCmd(v)
}
