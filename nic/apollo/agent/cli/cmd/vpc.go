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
	// ID holds VPC ID
	ID string
)

var vpcShowCmd = &cobra.Command{
	Use:   "vpc",
	Short: "show VPC information",
	Long:  "show VPC object information",
	Run:   vpcShowCmdHandler,
}

func init() {
	showCmd.AddCommand(vpcShowCmd)
	vpcShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	vpcShowCmd.Flags().StringVarP(&ID, "id", "i", "", "Specify VPC ID")
}

func vpcShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewVPCSvcClient(c)

	var req *pds.VPCGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific VPC
		req = &pds.VPCGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(ID).Bytes()},
		}
	} else {
		// Get all VPCs
		req = &pds.VPCGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.VPCGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting VPC failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print VPCs
	if cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printVPCHeader()
		for _, resp := range respMsg.Response {
			printVPC(resp)
		}
	}
}

func printVPCHeader() {
	hdrLine := strings.Repeat("-", 116)
	fmt.Println(hdrLine)
	fmt.Printf("%-36s%-10s%-36s%-20s%-14s\n",
		"ID", "Type", "V4RtTblId", "VR MAC", "FabricEncap")
	fmt.Println(hdrLine)
}

func printVPC(vpc *pds.VPC) {
	spec := vpc.GetSpec()
	fmt.Printf("%-36s%-10s%-36s%-20s%-14s\n",
		uuid.FromBytesOrNil(spec.GetId()).String(),
		strings.Replace(spec.GetType().String(), "VPC_TYPE_", "", -1),
		uuid.FromBytesOrNil(spec.GetV4RouteTableId()).String(),
		utils.MactoStr(spec.GetVirtualRouterMac()),
		utils.EncapToString(spec.GetFabricEncap()))
}
