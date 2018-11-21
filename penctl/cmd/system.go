//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var getSysMemCmd = &cobra.Command{
	Use:   "system-memory-usage",
	Short: "Show free/used memory on Naples (in MB)",
	Long:  "\n-----------------------------------------\n Show Free/Used Memory on Naples (in MB)\n-----------------------------------------\n",
	RunE:  getSysMemCmdHandler,
}

var getProcMemInfoCmd = &cobra.Command{
	Use:   "proc-meminfo",
	Short: "Check /proc/meminfo file on Naples",
	Long:  "\n------------------------------------\n Check /proc/meminfo file on Naples \n------------------------------------\n",
	RunE:  getProcMemInfoCmdHandler,
}

func init() {
	showCmd.AddCommand(getSysMemCmd)
	showCmd.AddCommand(getProcMemInfoCmd)
}

func getSysMemCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "free",
		Opts:       strings.Join([]string{"-m"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func getProcMemInfoCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "cat",
		Opts:       strings.Join([]string{"/proc/meminfo"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}
