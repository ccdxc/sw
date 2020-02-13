//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"

	"github.com/spf13/cobra"

	types "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	pegasusClient "github.com/pensando/sw/nic/metaswitch/gen/agent"
	"github.com/pensando/sw/nic/metaswitch/pegctl/utils"
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
	bgpShowCmd.AddCommand(peerShowCmd)
	bgpShowCmd.AddCommand(peerAfShowCmd)
	bgpShowCmd.AddCommand(nlriPrefixShowCmd)
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

	bodyBytes, _ := json.MarshalIndent(respMsg.Response, "", "    ")
	fmt.Println(string(bodyBytes))

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

	if len(respMsg.Response) != 0 {
		bodyBytes, _ := json.MarshalIndent(respMsg.Response, "", "    ")
		fmt.Println(string(bodyBytes))
	} else {
		fmt.Println("Got empty response")
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
		bodyBytes, _ := json.MarshalIndent(respMsg.Response, "", "    ")
		fmt.Println(string(bodyBytes))
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

	if len(respMsg.Response) != 0 {
		bodyBytes, _ := json.MarshalIndent(respMsg.Response, "", "    ")
		fmt.Println(string(bodyBytes))
	} else {
		fmt.Println("Got empty response")
	}

	return nil
}
