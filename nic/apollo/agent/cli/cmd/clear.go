//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/metaswitch/rtrctl/impl"
)

// clearCmd represents the clear command
var clearCmd = &cobra.Command{
	Use:   "clear",
	Short: "Clear commands",
	Long:  "Clear commands",
}

func init() {
	rootCmd.AddCommand(clearCmd)
	impl.RegisterClearNodes(&impl.CLIParams{GRPCPort: types.HalGRPCDefaultPort}, clearCmd)
}
