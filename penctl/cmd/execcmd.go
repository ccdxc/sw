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
	Use:                "execute",
	Short:              "Execute Binary",
	Long:               "\n-----------------------------------\n Execute Binary on Naples \n-----------------------------------\n",
	Run:                execBinCmdHandler,
	DisableFlagParsing: true,
}

func init() {
	rootCmd.AddCommand(execBinCmd)
}

func execBinCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: args[0],
		Opts:       strings.Join(args[1:], " "),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}
	s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
	fmt.Printf("%s", s)
}
