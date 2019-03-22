//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	tunnelID uint32
)

var tunnelShowCmd = &cobra.Command{
	Use:   "tunnel",
	Short: "show Tunnel information",
	Long:  "show Tunnel object information",
	Run:   tunnelShowCmdHandler,
}

func init() {
	showCmd.AddCommand(tunnelShowCmd)
	tunnelShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	tunnelShowCmd.Flags().Uint32VarP(&tunnelID, "id", "i", 0, "Specify Tunnel ID")
}

func tunnelShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewTunnelSvcClient(c)

	var req *pds.TunnelGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific Tunnel
		req = &pds.TunnelGetRequest{
			Id: []uint32{tunnelID},
		}
	} else {
		// Get all Tunnels
		req = &pds.TunnelGetRequest{
			Id: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.TunnelGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Tunnel failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Tunnels
	if cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printTunnelHeader()
		for _, resp := range respMsg.Response {
			printTunnel(resp)
		}
	}
}

func printTunnelHeader() {
	hdrLine := strings.Repeat("-", 54)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-6s%-10s%-16s%-16s\n",
		"ID", "PcnID", "EncapType", "LocalIP", "RemoteIP")
	fmt.Println(hdrLine)
}

func printTunnel(tunnel *pds.Tunnel) {
	spec := tunnel.GetSpec()
	fmt.Printf("%-6d%-6d%-10s%-16s%-16s\n",
		spec.GetId(), spec.GetPCNId(),
		strings.Replace(strings.Replace(spec.GetEncap().String(), "TUNNEL_ENCAP_", "", -1), "_", "-", -1),
		utils.IPAddrToStr(spec.GetLocalIP()),
		utils.IPAddrToStr(spec.GetRemoteIP()))
}
