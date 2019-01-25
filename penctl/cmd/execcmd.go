//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var execBinCmd = &cobra.Command{
	Use:                "execute",
	Short:              "Execute remote commands on the Naples card",
	Long:               "\n--------------------------------------------\n Execute remote commands on the Naples card \n--------------------------------------------\n",
	RunE:               execBinCmdHandler,
	DisableFlagParsing: true,
	Hidden:             true,
	Args:               execBinCmdArgsValidator,
}

func init() {
	rootCmd.AddCommand(execBinCmd)
}

func execBinCmdHandler(cmd *cobra.Command, args []string) error {
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
	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	s := strings.Replace(string(resp), `\n`, "\n", -1)
	s = strings.Replace(s, "\\", "", -1)
	if len(s) > 0 && s[0] == '"' {
		s = s[1:]
	}
	if len(s) > 0 && s[len(s)-2] == '"' {
		s = s[:len(s)-2]
	}
	fmt.Println(s)
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func execBinCmdArgsValidator(cmd *cobra.Command, args []string) error {
	if len(args) == 0 {
		return errors.New("No command to execute")
	}
	return nil
}
