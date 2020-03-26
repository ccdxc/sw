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
)

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

	v := utils.NewBGPSpec(respMsg.Response.Spec)
	if cmd.Flag("json").Value.String() == "true" {
		b, _ := json.MarshalIndent(v, "", "  ")
		fmt.Println(string(b))
		return nil
	}
	fmt.Printf(bgpGlobalStr, v.LocalASN, v.RouterId, v.ClusterId)
	return nil
}

const (
	bgpPeerFmt = `%-40s %-10s %-16v %-5v %-10v %-16v`
	bgpPeerHdr = "UUID,AdminState,Remote Address,ASN,Auth,Status"

	bgpPeerDetStr = `BGP Peer details
------------------------------------
UUID                   : %s
Admin State            : %v
Config Local Address   : %v
Selected Local Address : %v
Remote Address         : %v
Remote ASN             : %d
Authentication         : %v
Flags                  : [ RR Client: %v / Send Community: %v / Ext Community: %v ]
ConfiguredTimers       : [ Holdtime: %v seconds / Keepalive: %v seconds ]
NegotiatedTimers       : [ Holdtime: %v seconds / Keepalive: %v seconds ]
TTL                    : %d
Status                 : %v
Previous State         : %v
Last Err Recvd         : [ %v ]
Last Err Sent          : [ %v ]
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
					peer.Status.LastErrorRcvd, peer.Status.LastErrorSent)
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
UUID            : %s
AFI/SAFI        : [ %v/%v ]
Remote Address  : %v
Flags           : [ Next-Hop-Self: %v / Default-originate: %v ]
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

	var afs []*utils.ShadowBGPPeerAFSpec
	if len(respMsg.Response) != 0 {
		for _, r := range respMsg.Response {
			afp := utils.NewBGPPeerAfSpec(r.Spec)
			if doJSON {
				afs = append(afs, afp)
			} else {
				if doDetail {
					fmt.Printf(bgpPeerAFDetStr, afp.Id, afp.Afi, afp.Safi, afp.PeerAddr, afp.NexthopSelf, afp.DefaultOrig)
				} else {
					fmt.Printf(bgpPeerAFFmt, afp.Id, afp.PeerAddr, afp.Afi, afp.Safi)
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
	bgpNLRI = `   %s NextHop %v AS Path [ %v ]
      RouteSource %s Originator ID %v
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
				nlri.RouteSource, nlri.PathOrigId)
			attrString := nlri.Prefix.AttrString()
			if attrString != "" {
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
