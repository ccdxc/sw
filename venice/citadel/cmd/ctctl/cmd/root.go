package cmd

import (
	"fmt"
	"os"

	"github.com/spf13/cobra"
)

var rootCmd = &cobra.Command{
	Use:   "ctctl",
	Short: "citadel cli",
	Long:  `citadel cli`,
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Printf("ctctl version 1.0 \n")
	},
}

// Execute run the command
func Execute() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Printf("%s", err)
		os.Exit(1)
	}
}

var addr string

func init() {
	rootCmd.PersistentFlags().StringVarP(&addr, "citadel URL", "u", "http://localhost:7086", "url to connect to")

}
