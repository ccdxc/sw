//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"
)

var verShowCmd = &cobra.Command{
	Use:   "version",
	Short: "Show version of penctl",
	Long:  "\n------------------------\n Show Version of PenCtl \n------------------------\n",
	Run:   verShowCmdHandler,
}

func init() {
	rootCmd.AddCommand(verShowCmd)
}

func verShowCmdHandler(cmd *cobra.Command, args []string) {
	printPenctlVer()
}
