//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/metaswitch/rtrctl/impl"
)

const (
	pegasusGRPCDefaultPort = "50057"
)

// showCmd represents the show command
var showCmd = &cobra.Command{
	Use:   "show",
	Short: "Show commands",
	Long:  "Show commands",
}

// clearCmd represents the clear command
var clearCmd = &cobra.Command{
	Use:   "clear",
	Short: "Clear commands",
	Long:  "Clear commands",
}

var debugCmd = &cobra.Command{
	Use:   "debug",
	Short: "Debug commands",
	Long:  "Debug commands",
	Args:  cobra.NoArgs,
	// Hide this as there are no external cmds under this tree currently
	Hidden: true,
}

func init() {
	rootCmd.AddCommand(showCmd)
	rootCmd.AddCommand(clearCmd)
	rootCmd.AddCommand(debugCmd)
	rootCmd.AddCommand(impl.GenDocsCmd)
	impl.RegisterShowNodes(&impl.CLIParams{GRPCPort: pegasusGRPCDefaultPort}, showCmd)
	impl.RegisterClearNodes(&impl.CLIParams{GRPCPort: pegasusGRPCDefaultPort}, clearCmd)
	impl.RegisterDebugNodes(&impl.CLIParams{GRPCPort: pegasusGRPCDefaultPort}, debugCmd)
}
