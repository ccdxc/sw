//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/metaswitch/rtrctl/impl"
)

// debugCmd represents the show command
var debugCmd = &cobra.Command{
	Use:   "debug",
	Short: "Debug commands",
	Long:  "Debug commands",
}

func init() {
	rootCmd.AddCommand(debugCmd)
	impl.RegisterDebugNodes(&impl.CLIParams{GRPCPort: types.PDSGRPCDefaultPort}, debugCmd)
}
