package cmd

import (
	"fmt"
	"os"

	"github.com/spf13/cobra"
)

// RootCmd represents the base command when called without any subcommands
var RootCmd = &cobra.Command{
	Use:   "heimdall",
	Short: "heimdall is the CLI to run e2e tests",
	Long:  `heimdall is an all-seeing and all-hearing guardian sentry of Asgard protecting the quality on NAPLES`,
	Run: func(cmd *cobra.Command, args []string) {
		cmd.Usage()
	},
}

// Execute adds all child commands to the root command and sets flags appropriately.
// This is called by main.main(). It only needs to happen once to the rootCmd.
func Execute() {
	RootCmd.AddCommand(runCmd)
	RootCmd.AddCommand(trafficCmd)
	RootCmd.AddCommand(genCmd)
	if err := RootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
