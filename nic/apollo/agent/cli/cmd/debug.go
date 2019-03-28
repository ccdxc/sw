//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"
)

// debugCmd represents the show command
var debugCmd = &cobra.Command{
	Use:   "debug",
	Short: "Debug commands",
	Long:  "Debug commands",
}

func init() {
	rootCmd.AddCommand(debugCmd)
}
