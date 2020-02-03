//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	tunnelID string
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
	tunnelShowCmd.Flags().StringVarP(&tunnelID, "id", "i", "", "Specify Tunnel ID")
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
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific Tunnel
		req = &pds.TunnelGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(tunnelID).Bytes()},
		}
	} else {
		// Get all Tunnels
		req = &pds.TunnelGetRequest{
			Id: [][]byte{},
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
	if cmd != nil && cmd.Flags().Changed("yaml") {
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
	hdrLine := strings.Repeat("-", 128)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-40s%-16s%-16s%-16s\n",
		"ID", "VpcID", "Encap", "LocalIP", "RemoteIP")
	fmt.Println(hdrLine)
}

func printTunnel(tunnel *pds.Tunnel) {
	spec := tunnel.GetSpec()
	encapStr := utils.EncapToString(spec.GetEncap())
	fmt.Printf("%-40s%-40s%-16s%-16s%-16s\n",
		uuid.FromBytesOrNil(spec.GetId()).String(),
		uuid.FromBytesOrNil(spec.GetVPCId()).String(),
		encapStr, utils.IPAddrToStr(spec.GetLocalIP()),
		utils.IPAddrToStr(spec.GetRemoteIP()))
}
