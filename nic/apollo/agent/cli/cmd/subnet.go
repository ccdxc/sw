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
	subnetShowCmd.Flags().Bool("summary", false, "Display number of objects")
	subnetShowCmd.Flags().StringVarP(&subnetID, "id", "i", "", "Specify Subnet ID")
}

func subnetShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
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
		fmt.Printf("Getting Subnet failed, err %v\n", err)
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
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printSubnetSummary(len(respMsg.Response))
	} else {
		for _, resp := range respMsg.Response {
			printSubnetDetail(resp)
		}
		printSubnetSummary(len(respMsg.Response))
	}
}

func printSubnetSummary(count int) {
	fmt.Printf("\nNo. of subnets : %d\n\n", count)
}

func printSubnetHeader() {
	hdrLine := strings.Repeat("-", 152)
	fmt.Printf("\n")
	fmt.Printf("RtTblID - Route Table IDs (IPv4/IPv6)           HostIf    - Host interface subnet is deployed on\n")
	fmt.Printf("IngSGID - Ingress Security Group ID (IPv4/IPv6) EgSGID  - Egress Security Group ID (IPv4/IPv6)\n")
	fmt.Printf("ToS     - Type of Service in outer IP header\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-40s%-20s%-16s%-20s%-16s\n",
		"ID", "VpcID", "IPv4Prefix", "VR IPv4", "VR MAC", "Encap")
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-40s%-10s%-40s%-40s%-3s\n",
		"", "RtTblID", "HostIf", "IngSGID", "EgSGID", "ToS")
	fmt.Println(hdrLine)
}

func printSubnetDetail(subnet *pds.Subnet) {
	spec := subnet.GetSpec()
	ingressPolicy := spec.GetIngV4SecurityPolicyId()
	egressPolicy := spec.GetEgV4SecurityPolicyId()
	dhcpPolicy := spec.GetDHCPPolicyId()
	hostIf := spec.GetHostIf()
	lifName := "-"

	if len(hostIf) != 0 {
		lifName = ""
		for i := 0; i < len(hostIf); i++ {
			lifName += lifGetNameFromKey(hostIf[i])
			if i != len(hostIf)-1 {
				lifName += ", "
			}
		}
	}
	fmt.Printf("%-30s : %s\n", "Subnet Id", utils.IdToStr(spec.GetId()))
	fmt.Printf("%-30s : %s\n", "VPC Id", utils.IdToStr(spec.GetVPCId()))
	fmt.Printf("%-30s : %s\n", "Fabric Encap", utils.EncapToString(spec.GetFabricEncap()))
	fmt.Printf("%-30s : %s\n", "IPv4 Prefix", utils.IPv4PrefixToStr(spec.GetV4Prefix()))
	fmt.Printf("%-30s : %s\n", "Virtual Router IPv4 Address", utils.Uint32IPAddrtoStr(spec.GetIPv4VirtualRouterIP()))
	fmt.Printf("%-30s : %s\n", "Virtual Router MAC Address", utils.MactoStr(spec.GetVirtualRouterMac()))
	fmt.Printf("%-30s : %s\n", "Route Table Id", utils.IdToStr(spec.GetV4RouteTableId()))
	if len(ingressPolicy) != 0 {
		keyStr := fmt.Sprintf("%-30s : ", "Ingress Security Group Id")
		for i := 0; i < len(ingressPolicy); i++ {
			fmt.Printf("%-33s%s\n", keyStr, utils.IdToStr(ingressPolicy[i]))
			keyStr = ""
		}
	} else {
		fmt.Printf("%-30s : %s\n", "Ingress Security Group Id", "-")
	}
	if len(egressPolicy) != 0 {
		keyStr := fmt.Sprintf("%-30s : ", "Egress Security Group Id")
		for i := 0; i < len(egressPolicy); i++ {
			fmt.Printf("%-33s%s\n", keyStr, utils.IdToStr(egressPolicy[i]))
			keyStr = ""
		}
	} else {
		fmt.Printf("%-30s : %s\n", "Egress Security Group ID", "-")
	}
	if len(dhcpPolicy) != 0 {
		keyStr := fmt.Sprintf("%-30s : ", "DHCP Policy Id")
		for i := 0; i < len(dhcpPolicy); i++ {
			fmt.Printf("%-33s%s\n", keyStr, utils.IdToStr(dhcpPolicy[i]))
			keyStr = ""
		}
	} else {
		fmt.Printf("%-30s : %s\n", "DHCP Policy Id", "-")
	}
	fmt.Printf("%-30s : %s\n", "Host Interface", lifName)
	fmt.Printf("%-30s : %d\n", "Type of Service", spec.GetToS())
	lineStr := strings.Repeat("-", 70)
	fmt.Println(lineStr)
}

func printSubnet(subnet *pds.Subnet) {
	spec := subnet.GetSpec()
	numHostIfs := len(spec.GetHostIf())
	numIngressPolicy := len(spec.GetIngV4SecurityPolicyId())
	numEgressPolicy := len(spec.GetEgV4SecurityPolicyId())
	ingressPolicy := spec.GetIngV4SecurityPolicyId()
	egressPolicy := spec.GetEgV4SecurityPolicyId()
	numLines := 0
	lifName, ingressStr, egressStr := "-", "-", "-"

	numLines = numHostIfs
	if numIngressPolicy > numLines {
		numLines = numIngressPolicy
	}
	if numEgressPolicy > numLines {
		numLines = numEgressPolicy
	}

	if numLines != 0 {
		for i := 0; i < numLines; i++ {
			if i < numIngressPolicy {
				ingressStr = utils.IdToStr(ingressPolicy[i])
			} else {
				ingressStr = "-"
			}
			if i < numEgressPolicy {
				egressStr = utils.IdToStr(egressPolicy[i])
			} else {
				egressStr = "-"
			}
			if i < numHostIfs {
				lifName = lifGetNameFromKey(spec.GetHostIf()[i])
			} else {
				lifName = "-"
			}
			if i == 0 {
				fmt.Printf("%-40s%-40s%-20s%-16s%-20s%-16s\n%-10s%-40s%-10s%-40s%-40s%-3d\n",
					utils.IdToStr(spec.GetId()),
					utils.IdToStr(spec.GetVPCId()),
					utils.IPv4PrefixToStr(spec.GetV4Prefix()),
					utils.Uint32IPAddrtoStr(spec.GetIPv4VirtualRouterIP()),
					utils.MactoStr(spec.GetVirtualRouterMac()),
					utils.EncapToString(spec.GetFabricEncap()),
					"", utils.IdToStr(spec.GetV4RouteTableId()),
					lifName, ingressStr, egressStr, spec.GetToS())
			} else {
				fmt.Printf("%-50s%-10s%-40s%-40s\n", "", lifName, ingressStr, egressStr)
			}
		}
	} else {
		fmt.Printf("%-40s%-40s%-20s%-16s%-20s%-16s\n%-10s%-40s%-10s%-40s%-40s%-3d\n",
			utils.IdToStr(spec.GetId()),
			utils.IdToStr(spec.GetVPCId()),
			utils.IPv4PrefixToStr(spec.GetV4Prefix()),
			utils.Uint32IPAddrtoStr(spec.GetIPv4VirtualRouterIP()),
			utils.MactoStr(spec.GetVirtualRouterMac()),
			utils.EncapToString(spec.GetFabricEncap()),
			"", utils.IdToStr(spec.GetV4RouteTableId()),
			lifName, ingressStr, egressStr, spec.GetToS())
	}
}
