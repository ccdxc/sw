//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"os"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/utils/log"
)

var cfgFile string

// rootCmd represents the base command when called without any subcommands
var rootCmd = &cobra.Command{
	Use:   "pdsctl",
	Short: "PDS CLI",
	Long:  "\n----------------------\n Pensando Dataplane Services CLI \n----------------------\n",
}

// Execute adds all child commands to the root command and sets flags appropriately.
// This is called by main.main(). It only needs to happen once to the rootCmd.
func Execute() {
	if err := rootCmd.Execute(); err != nil {
		os.Exit(1)
	}
}

func init() {
	// Fill logger config params
	logConfig := &log.Config{
		Module:      "PDS CLI",
		Format:      log.JSONFmt,
		Filter:      log.AllowErrorFilter,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}
	log.SetConfig(logConfig)

	// Cobra also supports local flags, which will only run
	// when this action is called directly.
	rootCmd.Flags().BoolP("toggle", "t", false, "Help message for toggle")

	rootCmd.GenBashCompletionFile("out.sh")
}

// NewPdsctlCommand exports the rootCmd for bash-completion
func NewPdsctlCommand() *cobra.Command {
	return rootCmd
}
