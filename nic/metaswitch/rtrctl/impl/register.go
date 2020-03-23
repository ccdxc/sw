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

	//bgp commands
	base.AddCommand(bgpShowCmd)
	bgpShowCmd.PersistentFlags().Bool("json", false, "output in json")
	bgpShowCmd.PersistentFlags().Bool("detail", false, "detailed output")
	bgpShowCmd.AddCommand(peerShowCmd)
	bgpShowCmd.AddCommand(peerAfShowCmd)

	bgpShowCmd.AddCommand(bgpIPShowCmd)
	bgpIPShowCmd.AddCommand(bgpIPUnicastShowCmd)
	bgpIPUnicastShowCmd.AddCommand(bgpIPUnicastNlriShowCmd)

	bgpShowCmd.AddCommand(bgpL2vpnShowCmd)
	bgpL2vpnShowCmd.AddCommand(bgpL2vpnEvpnShowCmd)
	bgpL2vpnEvpnShowCmd.AddCommand(bgpL2vpnEvpnNlriShowCmd)

	//evpn commands
	base.AddCommand(evpnShowCmd)
	evpnShowCmd.PersistentFlags().Bool("json", false, "output in json")
	evpnShowCmd.PersistentFlags().Bool("detail", false, "detailed output")

	evpnShowCmd.AddCommand(evpnVrfShowCmd)
	evpnVrfShowCmd.AddCommand(evpnVrfStatusShowCmd)
	evpnVrfShowCmd.AddCommand(evpnVrfRtShowCmd)
	evpnVrfRtShowCmd.AddCommand(evpnVrfRtStatusShowCmd)

	evpnShowCmd.AddCommand(evpnEviShowCmd)
	evpnEviShowCmd.AddCommand(evpnEviStatusShowCmd)
	evpnEviShowCmd.AddCommand(evpnEviRtShowCmd)
	evpnEviRtShowCmd.AddCommand(evpnEviRtStatusShowCmd)
}
