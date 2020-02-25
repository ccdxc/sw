//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"

	"github.com/satori/go.uuid"
	"github.com/spf13/cobra"

	types "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	pegasusClient "github.com/pensando/sw/nic/metaswitch/gen/agent"
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

var nlriPrefixShowCmd = &cobra.Command{
	Use:   "nlri-prefix",
	Short: "show nlri prefix information",
	Long:  "show nlri prefix object information",
	Args:  cobra.NoArgs,
	RunE:  bgpNlriPrefixShowCmdHandler,
}

func init() {
	showCmd.AddCommand(bgpShowCmd)
	bgpShowCmd.PersistentFlags().Bool("json", false, "output in json")
	bgpShowCmd.AddCommand(peerShowCmd)
	bgpShowCmd.AddCommand(peerAfShowCmd)
	bgpShowCmd.AddCommand(nlriPrefixShowCmd)
}

const bgpGlobalStr = `BGP Global Configuration
Local ASN   : %d
Router ID   : %v
Cluster ID  : %v
`
const bgpPeerStr = `BGP Peer details
------------------------------------
UUID            : %s
Admin State     : %v
Local Address   : %v
Remote Address  : %v
Remote ASN      : %d
Authentication  : %v
Flags           : [ RR Client: %v / Send Community: %v / Ext Community: %v ]
Timers          : [ Holdtime: %v seconds / Keepalive: %v seconds ]

Status          : %v
Previous State  : %v
Last Err Recvd  : %v
Last Err Sent   : %v
------------------------------------
`

const bgpNLRI = `
------------------------------------
AFI/SAFI        : [ %v/%v ]
Route Source    : %d
Path ID         : %d
AS Path         : %v
Originator      : %v
Next Hop Addr   : %v
BestPath        : %v
Prefix          : %v
------------------------------------
`

const bgpPeerAFStr = `BGP Peer Address Family
------------------------------------
UUID            : %s
AFI/SAFI        : [ %v/%v ]
LocalAddress    : %v
Remote Address  : %v
Flags           : [ Disable: %v /Next-Hop-Self: %v / Default-originate: %v ]
------------------------------------
`

func printBGPPeerAF(paf pegasusClient.BGPPeerAf) string {
	uid, err := uuid.FromBytes(paf.Spec.Id)
	uidStr := ""
	if err != nil {
		uidStr = uid.String()
	}

	return fmt.Sprintf(bgpPeerAFStr, uidStr, paf.Spec.Afi.String(), paf.Spec.Safi.String(), utils.PdsIPToString(paf.Spec.LocalAddr), utils.PdsIPToString(paf.Spec.PeerAddr), paf.Spec.Disable, paf.Spec.NexthopSelf, paf.Spec.DefaultOrig)
}

func bgpShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := pegasusClient.NewBGPSvcClient(c)

	req := &pegasusClient.BGPRequest{}
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

func bgpPeersShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := pegasusClient.NewBGPSvcClient(c)

	req := &pegasusClient.BGPPeerRequest{}
	respMsg, err := client.BGPPeerGet(context.Background(), req)
	if err != nil {
		return fmt.Errorf("Getting Peers failed (%s)", err)
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}

	doJSON := cmd.Flag("json").Value.String() == "true"
	peers := []*utils.ShadowBGPPeer{}
	for _, p := range respMsg.Response {
		peer := utils.NewBGPPeer(p)
		if doJSON {
			peers = append(peers, peer)
		} else {
			fmt.Printf(bgpPeerStr, peer.Spec.Id, peer.Spec.State.String(), peer.Spec.LocalAddr,
				peer.Spec.PeerAddr, peer.Spec.RemoteASN, peer.Spec.Password, peer.Spec.RRClient,
				peer.Spec.SendComm, peer.Spec.SendExtComm, peer.Spec.HoldTime, peer.Spec.KeepAlive,
				peer.Status.Status, peer.Status.PrevStatus, peer.Status.LastErrorRcvd,
				peer.Status.LastErrorSent)
		}
	}
	if doJSON {
		b, _ := json.MarshalIndent(peers, "", "  ")
		fmt.Println(string(b))
		return nil
	}
	return nil
}

func bgpPeersAfShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := pegasusClient.NewBGPSvcClient(c)

	req := &pegasusClient.BGPPeerAfRequest{}
	respMsg, err := client.BGPPeerAfGet(context.Background(), req)
	if err != nil {
		return fmt.Errorf("Getting PeerAfs failed (%s)", err)
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}

	if len(respMsg.Response) != 0 {
		fmt.Println()
	} else {
		fmt.Println("Got empty response")
	}

	return nil
}

func bgpNlriPrefixShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := pegasusClient.NewBGPSvcClient(c)

	req := &pegasusClient.BGPNLRIPrefixRequest{}
	respMsg, err := client.BGPNLRIPrefixGet(context.Background(), req)
	if err != nil {
		return errors.New("Getting NLRIPrefix failed")
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}
	doJSON := cmd.Flag("json").Value.String() == "true"

	nlris := []*utils.ShadowBGPNLRIPrefixStatus{}
	for _, p := range respMsg.Response {
		nlris = append(nlris, utils.NewBGPNLRIPrefixStatus(p.Status))
		if !doJSON {

		}
	}

	if doJSON {
		b, _ := json.MarshalIndent(nlris, "", "  ")
		fmt.Println(string(b))
		return nil
	}

	return nil
}
