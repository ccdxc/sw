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
	subnetID string
)

var subnetShowCmd = &cobra.Command{
	Use:   "subnet",
	Short: "show Subnet information",
	Long:  "show Subnet object information",
	Run:   subnetShowCmdHandler,
}

func init() {
	showCmd.AddCommand(subnetShowCmd)
	subnetShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	subnetShowCmd.Flags().StringVarP(&subnetID, "id", "i", "", "Specify Subnet ID")
}

func subnetShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSubnetSvcClient(c)

	var req *pds.SubnetGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific Subnet
		req = &pds.SubnetGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(subnetID).Bytes()},
		}
	} else {
		// Get all Subnets
		req = &pds.SubnetGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.SubnetGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Subnet failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Subnets
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printSubnetHeader()
		for _, resp := range respMsg.Response {
			printSubnet(resp)
		}
	}
}

func printSubnetHeader() {
	hdrLine := strings.Repeat("-", 269)
	fmt.Printf("\n")
	fmt.Printf("RtTblID - Route Table IDs (IPv4/IPv6)           HostIf    - Host interface subnet is deployed on\n")
	fmt.Printf("IngSGID - Ingress Security Group ID (IPv4/IPv6) EgSGID  - Egress Security Group ID (IPv4/IPv6)\n")
	fmt.Printf("ToS     - Type of Service in outer IP header\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-40s%-10s%-20s%-16s%-20s%-40s%-40s%-40s%-3s\n",
		"ID", "VpcID", "HostIf", "IPv4Prefix", "VR IPv4", "VR MAC",
		"RtTblID", "IngSGID", "EgSGID", "ToS")
	fmt.Println(hdrLine)
}

func printSubnet(subnet *pds.Subnet) {
	spec := subnet.GetSpec()
	lifName := "-"
	if len(spec.GetHostIf()) > 0 {
		lifName = lifGetNameFromKey(spec.GetHostIf())
	}
	ingSecPol, egSecPol := "", ""
	if x := spec.GetIngV4SecurityPolicyId(); x != nil {
		ingSecPol = uuid.FromBytesOrNil(x[0]).String()
	}
	if x := spec.GetEgV4SecurityPolicyId(); x != nil {
		egSecPol = uuid.FromBytesOrNil(x[0]).String()
	}

	fmt.Printf("%-40s%-40s%-10s%-20s%-16s%-20s%-40s%-40s%-40s%-3d\n",
		uuid.FromBytesOrNil(spec.GetId()).String(),
		uuid.FromBytesOrNil(spec.GetVPCId()).String(), lifName,
		utils.IPv4PrefixToStr(spec.GetV4Prefix()),
		utils.Uint32IPAddrtoStr(spec.GetIPv4VirtualRouterIP()),
		utils.MactoStr(spec.GetVirtualRouterMac()),
		uuid.FromBytesOrNil(spec.GetV4RouteTableId()).String(),
		ingSecPol,
		egSecPol,
		spec.GetToS())
}
