package cmd

import (
	"github.com/spf13/cobra"
)

func init() {
	rootCmd.AddCommand(verifyCmd)
	verifyCmd.AddCommand(verifyPingCmd)
}

var verifyCmd = &cobra.Command{
	Use:   "verify",
	Short: "verify actions",
}

var verifyPingCmd = &cobra.Command{
	Use:   "ping",
	Short: "verify ping between endpoints",
	Run:   verifyPingAction,
}

func verifyPingAction(cmd *cobra.Command, args []string) {
	// TBD
}

type epInfo struct {
	local bool
	vlan  int
}
