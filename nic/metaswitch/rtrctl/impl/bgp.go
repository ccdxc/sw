//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package impl

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	types "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/nic/metaswitch/rtrctl/utils"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
)

var bgpClearCmd = &cobra.Command{
	Use:   "bgp",
	Short: "clear BGP related information",
	Long:  "clear BGP related information",
	Args:  bgpClearCmdHandlerValidator,
	RunE:  bgpClearCmdHandler,
}

var bgpShowCmd = &cobra.Command{
	Use:   "bgp",
	Short: "show BGP related information",
	Long:  "show BGP related information",
	Args:  cobra.NoArgs,
	RunE:  bgpShowCmdHandler,
}

var peerShowCmd = &cobra.Command{
	Use:   "peers",
	Short: "show Peer information",
	Long:  "show Peer object information",
	Args:  cobra.NoArgs,
	RunE:  bgpPeersShowCmdHandler,
}

var peerAfShowCmd = &cobra.Command{
	Use:   "peers-af",
	Short: "show Peer AF information",
	Long:  "show Peer AF object information",
	Args:  cobra.NoArgs,
	RunE:  bgpPeersAfShowCmdHandler,
}

var bgpIPShowCmd = &cobra.Command{
	Use:   "ip",
	Short: "show ip afi information",
	Long:  "show ip afi information",
	Args:  cobra.NoArgs,
}

var bgpIPUnicastShowCmd = &cobra.Command{
	Use:   "unicast",
	Short: "show unicast safi information",
	Long:  "show unicast safi information",
	Args:  cobra.NoArgs,
}

var bgpIPUnicastNlriShowCmd = &cobra.Command{
	Use:   "nlri",
	Short: "show ip unicast nlri information",
	Long:  "show ip unicast nlri information",
	Args:  cobra.NoArgs,
	RunE:  bgpIPUnicastNlriShowCmdHandler,
}

var bgpL2vpnShowCmd = &cobra.Command{
	Use:   "l2vpn",
	Short: "show l2vpn afi information",
	Long:  "show l2vpn afi information",
	Args:  cobra.NoArgs,
}

var bgpL2vpnEvpnShowCmd = &cobra.Command{
	Use:   "evpn",
	Short: "show evpn safi information",
	Long:  "show evpn safi information",
	Args:  cobra.NoArgs,
}

var bgpL2vpnEvpnNlriShowCmd = &cobra.Command{
	Use:   "nlri",
	Short: "show bgp l2vpn evpn nlri information",
	Long:  "show bgp l2vpn evpn nlri information",
	Args:  cobra.NoArgs,
	RunE:  bgpL2vpnEvpnNlriShowCmdHandler,
}

func init() {

}

const (
	bgpGlobalStr = `-----------------------------------
BGP Global Configuration
-----------------------------------
Local ASN   : %d
Router ID   : %v
Cluster ID  : %v
-----------------------------------
BGP Global Status
-----------------------------------
Oper Status            : %v
NumAdjRibOutRoutes     : %v
PeakNumAdjRibOutRoutes : %v
RemDelayTime           : %v
TableVersion           : %v
-----------------------------------
`
)

func bgpShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewBGPSvcClient(c)

	req := &types.BGPGetRequest{}
	respMsg, err := client.BGPGet(context.Background(), req)
	if err != nil {
		return fmt.Errorf("Getting global config failed (%s)", err)
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}

	v := utils.NewBGPGetResp(respMsg.Response.Spec, respMsg.Response.Status)
	if cmd.Flag("json").Value.String() == "true" {
		b, _ := json.MarshalIndent(v, "", "  ")
		fmt.Println(string(b))
		return nil
	}
	fmt.Printf(bgpGlobalStr, v.LocalASN, v.RouterId, v.ClusterId, v.Status, v.NumAdjRibOutRoutes, v.PeakNumAdjRibOutRoutes, v.RemDelayTime, v.TableVer)
	return nil
}

const (
	bgpPeerFmt = `%-40s %-10s %-16v %-5v %-10v %-16v`
	bgpPeerHdr = "UUID,AdminState,Remote Address,ASN,Auth,Status"

	bgpPeerDetStr = `BGP Peer details
------------------------------------
UUID                            : %s
Admin State                     : %v
Config Local Address            : %v
Selected Local Address          : %v
Remote Address                  : %v
Remote ASN                      : %d
Authentication                  : %v
Flags                           : [ RR Client: %v / Send Community: %v / Ext Community: %v ]
ConfiguredTimers                : [ Holdtime: %v seconds / Keepalive: %v seconds ]
NegotiatedTimers                : [ Holdtime: %v seconds / Keepalive: %v seconds ]
TTL                             : %d
Status                          : %v
Previous State                  : %v
Last Err Recvd                  : [ %v ]
Last Err Sent                   : [ %v ]
Capabilities Sent               : [ %v ]
Capabilities Received           : [ %v ]
Capabilities Negotiated         : [ %v ]
Selected Local Addr Type        : [ %s ]
In Notifications                : [ %v ]
Out Notifications               : [ %v ]
In Updates                      : [ %v ]
Out Updates                     : [ %v ]
In Keepalives                   : [ %v ]
Out Keepalives                  : [ %v ]
In Refreshes                    : [ %v ]
Out Refreshes                   : [ %v ]
In Total Messages               : [ %v ]
Out Total Messages              : [ %v ]
Fsm Established Transitions     : [ %v ]
Connect Retry Count             : [ %v ]
Peer GroupID                    : [ %v ]
Stale Path Time                 : [ %v sec]
ORF Entry Count                 : [ %v ]
Rcvd Msg Elapsed Time           : [ %v sec]
Route Refresh Sent              : [ %v ]
Route Refresh Rcvd              : [ %v ]
In Prefixes                     : [ %v ]
Out Prefixes                    : [ %v ]
Out Prefixes Advertised         : [ %v ]
Connect Retry Interval          : [ %v sec]
Out Update Elapsed Time         : [ %v sec]
Out Prefixes Denied             : [ %v ]
Out Prefixes Implicit Withdrawn : [ %v ]
Out Prefixes Explicit Withdrawn : [ %v ]
In Prefixes Implicit Withdrawn  : [ %v ]
In Prefixes Explicit Withdrawn  : [ %v ]
Received HoldTime               : [ %v sec]
Fsm Established Time            : [ %v sec]
In Updates Elapsed Time         : [ %v sec]
In Opens                        : [ %v ]
Out Opens                       : [ %v ]
------------------------------------
`
)

func bgpPeersShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewBGPSvcClient(c)

	req := &types.BGPPeerGetRequest{}
	respMsg, err := client.BGPPeerGet(context.Background(), req)
	if err != nil {
		return fmt.Errorf("Getting Peers failed (%s)", err)
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}

	doJSON := cmd.Flag("json").Value.String() == "true"
	doDetail := cmd.Flag("detail").Value.String() == "true"

	if !doJSON && !doDetail {
		utils.PrintHeader(bgpPeerFmt, bgpPeerHdr)
	}
	peers := []*utils.ShadowBGPPeer{}
	for _, p := range respMsg.Response {
		peer := utils.NewBGPPeer(p)
		if doJSON {
			peers = append(peers, peer)
		} else {
			if doDetail {
				fmt.Printf(bgpPeerDetStr, peer.Spec.Id, peer.Spec.State, peer.Spec.LocalAddr,
					peer.Status.LocalAddr, peer.Spec.PeerAddr, peer.Spec.RemoteASN, peer.Spec.Password,
					peer.Spec.RRClient, peer.Spec.SendComm, peer.Spec.SendExtComm, peer.Spec.HoldTime,
					peer.Spec.KeepAlive, peer.Status.HoldTime, peer.Status.KeepAlive, peer.Spec.TTL,
					peer.Status.Status, peer.Status.PrevStatus,
					peer.Status.LastErrorRcvd, peer.Status.LastErrorSent,
					peer.Status.CapsSent, peer.Status.CapsRcvd,
					peer.Status.CapsNeg, peer.Status.SelLocalAddrType,
					peer.Status.InNotifications,
					peer.Status.OutNotifications, peer.Status.InUpdates,
					peer.Status.OutUpdates, peer.Status.InKeepalives,
					peer.Status.OutKeepalives, peer.Status.InRefreshes,
					peer.Status.OutRefreshes, peer.Status.InTotalMessages,
					peer.Status.OutTotalMessages, peer.Status.FsmEstTransitions,
					peer.Status.ConnectRetryCount, peer.Status.Peergr,
					peer.Status.StalePathTime, peer.Status.OrfEntryCount,
					peer.Status.RcvdMsgElpsTime, peer.Status.RouteRefrSent,
					peer.Status.RouteRefrRcvd, peer.Status.InPrfxes,
					peer.Status.OutPrfxes, peer.Status.OutPrfxesAdvertised,
					peer.Status.ConnectRetryInt, peer.Status.OutUpdateElpsTime,
					peer.Status.OutPrfxesDenied, peer.Status.OutPrfxesImpWdr,
					peer.Status.OutPrfxesExpWdr, peer.Status.InPrfxesImpWdr,
					peer.Status.InPrfxesExpWdr, peer.Status.ReceivedHoldTime,
					peer.Status.FsmEstablishedTime, peer.Status.InUpdatesElpsTime,
					peer.Status.InOpens, peer.Status.OutOpens)
			} else {
				fmt.Printf(bgpPeerFmt, peer.Spec.Id, peer.Spec.State, peer.Spec.PeerAddr, peer.Spec.RemoteASN, peer.Spec.Password, peer.Status.Status)
				fmt.Printf("\n")
			}
		}
	}
	if doJSON {
		b, _ := json.MarshalIndent(peers, "", "  ")
		fmt.Println(string(b))
	}
	return nil
}

const (
	bgpPeerAfHdr    = "UUID,Peer Address,AFI,SAFI"
	bgpPeerAFFmt    = "%-40s %-16v %-16v %-16v"
	bgpPeerAFDetStr = `BGP Peer Address Family
------------------------------------
UUID                 : %s
AFI/SAFI             : [ %v/%v ]
Remote Address       : %v
Remote Port          : %v
Local Address        : %v
Local Port           : %v
Local Addr Scope Id  : %v
Route Refresh        : %v
Negotiated Path Cap  : %v
Reflector Client     : %v
Update Group         : %v
------------------------------------
`
)

func bgpPeersAfShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewBGPSvcClient(c)

	req := &types.BGPPeerAfGetRequest{}
	respMsg, err := client.BGPPeerAfGet(context.Background(), req)
	if err != nil {
		return fmt.Errorf("Getting PeerAfs failed (%s)", err)
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}

	doJSON := cmd.Flag("json").Value.String() == "true"
	doDetail := cmd.Flag("detail").Value.String() == "true"

	if !doJSON && !doDetail {
		utils.PrintHeader(bgpPeerAFFmt, bgpPeerAfHdr)
	}

	var afs []*utils.ShadowBGPPeerAF
	if len(respMsg.Response) != 0 {
		for _, r := range respMsg.Response {
			afp := utils.NewBGPPeerAf(r)
			if doJSON {
				afs = append(afs, afp)
			} else {
				if doDetail {
					fmt.Printf(bgpPeerAFDetStr, afp.Spec.Id, afp.Spec.Afi, afp.Spec.Safi, afp.Spec.PeerAddr, afp.Spec.PeerPort, afp.Spec.LocalAddr, afp.Spec.LocalPort, afp.Status.LocalAddrScopeId, afp.Status.RtRefresh, afp.Status.AddPathCapNeg, afp.Status.ReflectorClient, afp.Status.UpdGrp)
				} else {
					fmt.Printf(bgpPeerAFFmt, afp.Spec.Id, afp.Spec.PeerAddr, afp.Spec.Afi, afp.Spec.Safi)
					fmt.Println("")
				}
			}
		}
	} else {
		fmt.Println("Got empty response")
	}
	if doJSON {
		b, _ := json.MarshalIndent(afs, "", "  ")
		fmt.Println(string(b))
	}
	return nil
}

const (
	bgpNLRI = `-----------------------------------
        %s 
	NextHop            : %v 
	AS Path            : %v
	RouteSource        : %s 
	Originator ID      : %v 
	FlapStatsFlapcnt   : %v
	FlapStatsSupprsd   : %v 
	IsActive           : %v 
	Stale              : %v
	FlapStartTime      : %v
	ReasonNotBest      : %v 
	EcmpRoute          : %v
	PeerAddr           : %v
	ExtComm            : %v
-----------------------------------
`
)

func bgpIPUnicastNlriShowCmdHandler(cmd *cobra.Command, args []string) error {
	return (bgpNlriShowCmdHandler(cmd, "ipv4-unicast", args))
}

func bgpL2vpnEvpnNlriShowCmdHandler(cmd *cobra.Command, args []string) error {
	return (bgpNlriShowCmdHandler(cmd, "l2vpn-evpn", args))
}

func bgpNlriShowCmdHandler(cmd *cobra.Command, _afisafi string, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewBGPSvcClient(c)

	req := &types.BGPNLRIPrefixGetRequest{}
	respMsg, err := client.BGPNLRIPrefixGet(context.Background(), req)
	if err != nil {
		return errors.New("Getting NLRIPrefix failed")
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}
	doJSON := cmd.Flag("json").Value.String() == "true"

	var nlris []*utils.ShadowBGPNLRIPrefixStatus
	var afi string
	var safi string
	var network string
	for _, p := range respMsg.Response {
		nlri := utils.NewBGPNLRIPrefixStatus(p.Status)
		nlris = append(nlris, nlri)
		if !doJSON {

			//Print AFI/SAFI info
			afi = strings.TrimPrefix(nlri.Afi.String(), "BGP_AFI_")
			safi = strings.TrimPrefix(nlri.Safi.String(), "BGP_SAFI_")

			//Lets check if its right afi/safi
			afisafi := strings.ToLower(afi) + "-" + strings.ToLower(safi)
			if strings.Compare(_afisafi, afisafi) != 0 {
				continue
			}

			var plen string
			plen = fmt.Sprint(nlri.PrefixLen)
			if network != nlri.Prefix.String()+"/"+plen {
				network = nlri.Prefix.String() + "/" + plen
				fmt.Printf("%s\n", network)
			}
			var best string
			if nlri.BestRoute {
				best = ">"
			} else {
				best = " "
			}
			fmt.Printf(bgpNLRI, best, nlri.NextHopAddr, nlri.ASPathStr,
				nlri.RouteSource, nlri.PathOrigId, nlri.FlapStatsFlapcnt,
				nlri.FlapStatsSupprsd, nlri.IsActive, nlri.Stale, nlri.FlapStartTime,
				nlri.ReasonNotBest, nlri.EcmpRoute, nlri.PeerAddr, nlri.ExtComm)
			if nlri.Prefix != nil && nlri.Prefix.AttrString() != "" {
				fmt.Println(nlri.Prefix.AttrString())
			}
		}
	}

	if doJSON {
		b, _ := json.MarshalIndent(nlris, "", "  ")
		fmt.Println(string(b))
		return nil
	}

	return nil
}

var bgpPrefixShowCmd = &cobra.Command{
	Use:   "prefix",
	Short: "show bgp prefix information",
	Long:  "show bgp prefix information",
	Args:  cobra.NoArgs,
}

var bgpPfxCountersShowCmd = &cobra.Command{
	Use:   "counters",
	Short: "show bgp counters information",
	Long:  "show bgp counters information",
	Args:  cobra.NoArgs,
	RunE:  bgpPfxCountersShowCmdHandler,
}

const (
	bgpPfxCntrs = `-----------------------------------
  EntIndex                : %v
  PeerIndex               : %v
  Afi                     : %v
  Safi                    : %v
  InPrfxes                : %v
  InPrfxesAccepted        : %v
  InPrfxesRejected        : %v
  OutPrfxes               : %v
  OutPrfxesAdvertised     : %v
  UserData                : %v
  InPrfxesFlapped         : %v
  InPrfxesFlapSuppressed  : %v
  InPrfxesFlapHistory     : %v
  InPrfxesActive          : %v
  InPrfxesDeniedByPol     : %v
  NumLocRibRoutes         : %v
  NumLocRibBestRoutes     : %v
  InPrfxesDeniedMartian   : %v
  InPrfxesDeniedAsLoop    : %v
  InPrfxesDeniedNextHop   : %v
  InPrfxesDeniedAsLength  : %v
  InPrfxesDeniedCommunity : %v
  InPrfxesDeniedLocalOrig : %v
  InTotalPrfxes           : %v
  OutTotalPrfxes          : %v
  PeerState               : %v
  OutPrfxesDenied         : %v
  OutPrfxesImpWdr         : %v
  OutPrfxesExpWdr         : %v
  InPrfxesImpWdr          : %v
  InPrfxesExpWdr          : %v
  CurPrfxesDeniedByPol    : %v
-----------------------------------
`
)

func bgpPfxCountersShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewBGPSvcClient(c)

	req := &types.BGPPrfxCntrsGetRequest{}
	respMsg, err := client.BGPPrfxCntrsGet(context.Background(), req)
	if err != nil {
		return errors.New("Getting prefix counters failed")
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}
	doJSON := cmd.Flag("json").Value.String() == "true"

	var pfxCntrs []*utils.ShadowBGPPrfxCntrsStatus
	for _, p := range respMsg.Response {
		pfxCntr := utils.NewBGPPrfxCntrsStatus(p.Status)
		pfxCntrs = append(pfxCntrs, pfxCntr)
		if !doJSON {
			fmt.Printf(bgpPfxCntrs,
				pfxCntr.EntIndex,
				pfxCntr.PeerIndex,
				pfxCntr.Afi,
				pfxCntr.Safi,
				pfxCntr.InPrfxes,
				pfxCntr.InPrfxesAccepted,
				pfxCntr.InPrfxesRejected,
				pfxCntr.OutPrfxes,
				pfxCntr.OutPrfxesAdvertised,
				pfxCntr.UserData,
				pfxCntr.InPrfxesFlapped,
				pfxCntr.InPrfxesFlapSuppressed,
				pfxCntr.InPrfxesFlapHistory,
				pfxCntr.InPrfxesActive,
				pfxCntr.InPrfxesDeniedByPol,
				pfxCntr.NumLocRibRoutes,
				pfxCntr.NumLocRibBestRoutes,
				pfxCntr.InPrfxesDeniedMartian,
				pfxCntr.InPrfxesDeniedAsLoop,
				pfxCntr.InPrfxesDeniedNextHop,
				pfxCntr.InPrfxesDeniedAsLength,
				pfxCntr.InPrfxesDeniedCommunity,
				pfxCntr.InPrfxesDeniedLocalOrig,
				pfxCntr.InTotalPrfxes,
				pfxCntr.OutTotalPrfxes,
				pfxCntr.PeerState,
				pfxCntr.OutPrfxesDenied,
				pfxCntr.OutPrfxesImpWdr,
				pfxCntr.OutPrfxesExpWdr,
				pfxCntr.InPrfxesImpWdr,
				pfxCntr.InPrfxesExpWdr,
				pfxCntr.CurPrfxesDeniedByPol)
		}
	}

	if doJSON {
		b, _ := json.MarshalIndent(pfxCntrs, "", "  ")
		fmt.Println(string(b))
		return nil
	}

	return nil
}

var bgpRouteMapShowCmd = &cobra.Command{
	Use:   "route-map",
	Short: "show bgp route-map information",
	Long:  "show bgp route-map information",
	Args:  cobra.NoArgs,
	RunE:  bgpRouteMapShowCmdHandler,
}

const (
	bgpRouteMap = `-----------------------------------
  EntIndex           : %v
  Index              : %v
  Number             : %v
  Hitcnt             : %v
  OrfAssoc           : %v
-----------------------------------
`
)

func bgpRouteMapShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewBGPSvcClient(c)

	req := &types.BGPRouteMapGetRequest{}
	respMsg, err := client.BGPRouteMapGet(context.Background(), req)
	if err != nil {
		return errors.New("Getting RouteMap failed")
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}
	doJSON := cmd.Flag("json").Value.String() == "true"

	var rtmaps []*utils.ShadowBGPRouteMapStatus
	for _, p := range respMsg.Response {
		rtmap := utils.NewBGPRouteMapStatus(p.Status)
		rtmaps = append(rtmaps, rtmap)
		if !doJSON {
			fmt.Printf(bgpRouteMap, rtmap.EntIndex, rtmap.Index, rtmap.Number, rtmap.Hitcnt, rtmap.OrfAssoc)
		}
	}

	if doJSON {
		b, _ := json.MarshalIndent(rtmaps, "", "  ")
		fmt.Println(string(b))
		return nil
	}

	return nil
}

var (
	option string
	laddr  string
	paddr  string
	afi    string
	safi   string
)

func bgpClearCmdHandlerValidator(cmd *cobra.Command, args []string) (err error) {
	if vldtor.IPAddr(laddr) != nil {
		err = fmt.Errorf("Invalid local address %v", laddr)
		return
	}
	if vldtor.IPAddr(paddr) != nil {
		err = fmt.Errorf("Invalid peer address %v", paddr)
		return
	}
	if !(option == "hard" || option == "refresh_in" || option == "refresh_out" || option == "refresh_both") {
		err = fmt.Errorf("Invalid option value passed %v. Option can be hard or refresh_in or refresh_out or refresh_both", option)
		return
	}
	if len(afi) != 0 && !(afi == "ipv4" || afi == "l2vpn") {
		err = fmt.Errorf("AFI can only be either ipv4 or l2vpn")
		return
	}
	if len(safi) != 0 && !(safi == "unicast" || safi == "evpn") {
		err = fmt.Errorf("SAFI can only be either unicast or evpn")
		return
	}
	if (len(afi) != 0 && len(safi) == 0) || (len(afi) == 0 && len(safi) != 0) {
		err = fmt.Errorf("AFI and SAFI have to be provided together")
		return
	}
	return nil
}

func bgpClearCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewBGPSvcClient(c)

	lip := utils.IPAddrStrToPdsIPAddr(laddr)
	pip := utils.IPAddrStrToPdsIPAddr(paddr)

	var opt types.BGPClearRouteOptions
	var af types.BGPAfi
	var saf types.BGPSafi

	switch option {
	case "hard":
		opt = types.BGPClearRouteOptions_BGP_CLEAR_ROUTE_HARD
	case "refresh_in":
		opt = types.BGPClearRouteOptions_BGP_CLEAR_ROUTE_REFRESH_IN
	case "refresh_out":
		opt = types.BGPClearRouteOptions_BGP_CLEAR_ROUTE_REFRESH_OUT
	case "refresh_both":
		opt = types.BGPClearRouteOptions_BGP_CLEAR_ROUTE_REFRESH_BOTH
	}

	af = types.BGPAfi_BGP_AFI_NONE
	switch afi {
	case "ipv4":
		af = types.BGPAfi_BGP_AFI_IPV4
	case "l2vpn":
		af = types.BGPAfi_BGP_AFI_L2VPN
	}

	saf = types.BGPSafi_BGP_SAFI_NONE
	switch safi {
	case "unicast":
		saf = types.BGPSafi_BGP_SAFI_UNICAST
	case "evpn":
		saf = types.BGPSafi_BGP_SAFI_EVPN
	}
	if af == types.BGPAfi_BGP_AFI_NONE {
		req := &types.BGPClearRouteRequest{
			Option: opt,
			PeerOrPeeraf: &types.BGPClearRouteRequest_Peer{
				Peer: &types.BGPPeerKeyHandle{
					IdOrKey: &types.BGPPeerKeyHandle_Key{
						Key: &types.BGPPeerKey{
							LocalAddr: lip,
							PeerAddr:  pip,
						},
					},
				},
			},
		}
		respMsg, err := client.BGPClearRoute(context.Background(), req)
		if err != nil {
			return errors.New("BGP ClearRoute failed")
		}

		if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
			fmt.Println(respMsg.ApiStatus)
			return errors.New("Operation failed with error")
		}
	} else {
		req := &types.BGPClearRouteRequest{
			Option: opt,
			PeerOrPeeraf: &types.BGPClearRouteRequest_PeerAf{
				PeerAf: &types.BGPPeerAfKeyHandle{
					IdOrKey: &types.BGPPeerAfKeyHandle_Key{
						Key: &types.BGPPeerAfKey{
							LocalAddr: lip,
							PeerAddr:  pip,
							Afi:       af,
							Safi:      saf,
						},
					},
				},
			},
		}
		respMsg, err := client.BGPClearRoute(context.Background(), req)
		if err != nil {
			return errors.New("BGP ClearRoute failed")
		}

		if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
			return errors.New("Operation failed with error")
		}
	}
	return nil
}
