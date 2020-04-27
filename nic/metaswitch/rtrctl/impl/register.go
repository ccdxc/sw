package impl

import (
	"github.com/spf13/cobra"
)

// CLIParams is parameters for CLI
type CLIParams struct {
	GRPCPort string
}

var cliParams *CLIParams

// RegisterClearNodes registers all the CLI nodes
func RegisterClearNodes(params *CLIParams, base *cobra.Command) {
	cliParams = params

	//bgp commands
	base.AddCommand(bgpClearCmd)
	bgpClearCmd.PersistentFlags().StringVarP(&laddr, "local-addr", "l", "", "Local Address IP")
	bgpClearCmd.PersistentFlags().StringVarP(&paddr, "peer-addr", "p", "", "Peer Address IP")
	bgpClearCmd.PersistentFlags().StringVarP(&option, "option", "o", "", "Option can be either hard or refresh_in or refresh_out or refresh_both")
	bgpClearCmd.PersistentFlags().StringVarP(&afi, "afi", "a", "", "AFI can be ipv4 or l2vpn")
	bgpClearCmd.PersistentFlags().StringVarP(&safi, "safi", "s", "", "SAFI can be unicast or evpn")
	bgpClearCmd.MarkFlagRequired("local-addr")
	bgpClearCmd.MarkFlagRequired("peer-addr")
	bgpClearCmd.MarkFlagRequired("option")
}

// RegisterShowNodes registers all the CLI nodes
func RegisterShowNodes(params *CLIParams, base *cobra.Command) {
	cliParams = params

	//bgp commands
	base.AddCommand(bgpShowCmd)
	bgpShowCmd.PersistentFlags().Bool("json", false, "output in json")
	bgpShowCmd.PersistentFlags().Bool("detail", false, "detailed output")
	bgpShowCmd.AddCommand(peerShowCmd)
	bgpShowCmd.AddCommand(peerAfShowCmd)
	bgpShowCmd.AddCommand(bgpRouteMapShowCmd)

	bgpShowCmd.AddCommand(bgpPrefixShowCmd)
	bgpPrefixShowCmd.AddCommand(bgpPfxCountersShowCmd)

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

	evpnShowCmd.AddCommand(evpnBdShowCmd)
	evpnBdShowCmd.AddCommand(evpnBdMacIPShowCmd)
	evpnBdShowCmd.AddCommand(evpnBdStatusShowCmd)
	evpnBdShowCmd.AddCommand(evpnBdIntfShowCmd)

	//routing commands
	base.AddCommand(routingShowCmd)
	routingShowCmd.AddCommand(interfaceShowCmd)
	interfaceShowCmd.AddCommand(intfStatusShowCmd)
	interfaceShowCmd.PersistentFlags().Bool("json", false, "output in json")
	routingShowCmd.AddCommand(staticTableShowCmd)
	staticTableShowCmd.PersistentFlags().Bool("json", false, "output in json")
	routingShowCmd.AddCommand(osTableShowCmd)
	osTableShowCmd.AddCommand(osRouteTableShowCmd)
	osRouteTableShowCmd.PersistentFlags().Bool("json", false, "output in json")
	interfaceShowCmd.AddCommand(intfAddrShowCmd)
	intfAddrShowCmd.PersistentFlags().Bool("json", false, "output in json")
	routingShowCmd.AddCommand(routeTableShowCmd)
	routeTableShowCmd.PersistentFlags().Bool("json", false, "output in json")
	routingShowCmd.AddCommand(vrfStatusShowCmd)
	vrfStatusShowCmd.PersistentFlags().Bool("json", false, "output in json")
	routingShowCmd.AddCommand(redistTableShowCmd)
	redistTableShowCmd.PersistentFlags().Bool("json", false, "output in json")
}
