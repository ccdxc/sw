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

var execBinCmd = &cobra.Command{
	Use:                "exec",
	Short:              "Execute Binary",
	Long:               "\n-----------------------------------\n Execute Binary on Naples \n-----------------------------------\n",
	Run:                execBinCmdHandler,
	DisableFlagParsing: true,
}

func init() {
	rootCmd.AddCommand(execBinCmd)
}

func execBinCmdHandler(cmd *cobra.Command, args []string) {
	var v *nmd.NaplesCmdExecute
	if args[0] == "-v" {
		v = &nmd.NaplesCmdExecute{
			Executable: args[1],
			Opts:       strings.Join(args[2:], " "),
		}
	} else {
		v = &nmd.NaplesCmdExecute{
			Executable: args[0],
			Opts:       strings.Join(args[1:], " "),
		}
	}

	if args[0] == "-v" {
		verbose = true
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
	if verbose {
		fmt.Println(string(resp))
	}
}
