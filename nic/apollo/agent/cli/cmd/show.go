//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/metaswitch/rtrctl/impl"
)

// showCmd represents the show command
var showCmd = &cobra.Command{
	Use:   "show",
	Short: "Show commands",
	Long:  "Show commands",
}

func init() {
	rootCmd.AddCommand(showCmd)
	impl.RegisterShowNodes(&impl.CLIParams{GRPCPort: types.PDSGRPCDefaultPort}, showCmd)
}
