package impl

import (
	"github.com/spf13/cobra"
)

// CLIParams is parameters for CLI
type CLIParams struct {
	GRPCPort string
}

var cliParams *CLIParams

// RegisterNodes registers all the CLI nodes
func RegisterNodes(params *CLIParams, base *cobra.Command) {
	cliParams = params
	base.AddCommand(bgpShowCmd)
	bgpShowCmd.PersistentFlags().Bool("json", false, "output in json")
	bgpShowCmd.PersistentFlags().Bool("detail", false, "detailed output")
	bgpShowCmd.AddCommand(peerShowCmd)
	bgpShowCmd.AddCommand(peerAfShowCmd)
	bgpShowCmd.AddCommand(nlriPrefixShowCmd)
}
