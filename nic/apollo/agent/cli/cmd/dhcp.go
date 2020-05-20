//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines show commands relating to dhcp object
///
//----------------------------------------------------------------------------

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
	// ID holds DHCP Policy ID
	DHCP_ID string
)

var dhcpShowCmd = &cobra.Command{
	Use:   "dhcp",
	Short: "show DHCP information",
	Long:  "show DHCP object information",
}

var dhcpProxyShowCmd = &cobra.Command{
	Use:   "proxy",
	Short: "show DHCP proxy information",
	Long:  "show DHCP proxy information",
	Run:   dhcpProxyShowCmdHandler,
}

var dhcpRelayShowCmd = &cobra.Command{
	Use:   "relay",
	Short: "show DHCP relay information",
	Long:  "show DHCP relay information",
	Run:   dhcpRelayShowCmdHandler,
}

func init() {
	showCmd.AddCommand(dhcpShowCmd)
	dhcpShowCmd.AddCommand(dhcpProxyShowCmd)
	dhcpProxyShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	dhcpProxyShowCmd.Flags().Bool("summary", false, "Display number of objects")
	dhcpProxyShowCmd.Flags().StringVarP(&DHCP_ID, "id", "i", "", "Specify DHCP Policy ID")

	dhcpShowCmd.AddCommand(dhcpRelayShowCmd)
	dhcpRelayShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	dhcpRelayShowCmd.Flags().Bool("summary", false, "Display number of objects")
	dhcpRelayShowCmd.Flags().StringVarP(&DHCP_ID, "id", "i", "", "Specify DHCP Policy ID")
}

func dhcpRelayShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewDHCPSvcClient(c)

	var req *pds.DHCPPolicyGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific DHCP policy
		req = &pds.DHCPPolicyGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(DHCP_ID).Bytes()},
		}
	} else {
		// Get all DHCP policies
		req = &pds.DHCPPolicyGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.DHCPPolicyGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting DHCP policy failed, err %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print DHCP Policy
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			if resp.GetSpec().GetRelaySpec() == nil {
				continue
			}
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printDHCPRelaySummary(len(respMsg.Response))
	} else {
		printDHCPRelayHeader()
		count := 0
		for _, resp := range respMsg.Response {
			if printDHCPRelay(resp) {
				count++
			}
		}
		printDHCPRelaySummary(count)
	}
}
func dhcpProxyShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewDHCPSvcClient(c)

	var req *pds.DHCPPolicyGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific DHCP policy
		req = &pds.DHCPPolicyGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(DHCP_ID).Bytes()},
		}
	} else {
		// Get all DHCP policies
		req = &pds.DHCPPolicyGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.DHCPPolicyGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting DHCP policy failed, err %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print DHCP Policy
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			if resp.GetSpec().GetProxySpec() == nil {
				continue
			}
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printDHCPProxySummary(len(respMsg.Response))
	} else {
		printDHCPProxyHeader()
		count := 0
		for _, resp := range respMsg.Response {
			if printDHCPProxy(resp) {
				count++
			}
		}
		printDHCPProxySummary(count)
	}
}

func printDHCPProxySummary(count int) {
	fmt.Printf("\nNo. of DHCP proxy policies : %d\n\n", count)
}

func printDHCPProxyHeader() {
	hdrline := strings.Repeat("-", 250)
	fmt.Println(hdrline)
	fmt.Printf("%-40s%-20s%-20s%-20s%-10s%-10s%-130s\n",
		"ID", "Gateway IP", "DNS Server IP", "NTP Server IP",
		"Lease", "MTU", "Domain Name")
	fmt.Printf("%-40s%-20s%-20s%-20s%-10s%-10s%-130s\n",
		"", "", "", "", "(secs)", "", "")
	fmt.Println(hdrline)
}

func printDHCPProxy(dhcp *pds.DHCPPolicy) bool {

	if dhcp == nil {
		fmt.Printf("-\n")
		return false
	}

	spec := dhcp.GetSpec()
	if spec == nil {
		fmt.Printf("-\n")
		return false
	}

	outStr := fmt.Sprintf("%-40s", utils.IdToStr(spec.GetId()))

	switch spec.GetRelayOrProxy().(type) {
	case *pds.DHCPPolicySpec_ProxySpec:
		proxySpec := spec.GetProxySpec()
		outStr += fmt.Sprintf("%-20s", utils.IPAddrToStr(proxySpec.GetGatewayIP()))
		outStr += fmt.Sprintf("%-20s", utils.IPAddrToStr(proxySpec.GetDNSServerIP()))
		outStr += fmt.Sprintf("%-20s", utils.IPAddrToStr(proxySpec.GetNTPServerIP()))
		outStr += fmt.Sprintf("%-10d", proxySpec.GetLeaseTimeout())
		outStr += fmt.Sprintf("%-10d", (proxySpec.GetMTU()))
		outStr += fmt.Sprintf("%-130s", proxySpec.GetDomainName())
	default:
		return false
	}
	fmt.Println(outStr)
	return true
}

func printDHCPRelaySummary(count int) {
	fmt.Printf("\nNo. of DHCP relay policies : %d\n\n", count)
}

func printDHCPRelayHeader() {
	hdrline := strings.Repeat("-", 120)
	fmt.Println(hdrline)
	fmt.Printf("%-40s%-40s%-20s%-20s\n", "ID", "VPC ID", "Server IP", "Agent IP")
	fmt.Println(hdrline)
}

func printDHCPRelay(dhcp *pds.DHCPPolicy) bool {

	if dhcp == nil {
		return false
	}

	spec := dhcp.GetSpec()
	if spec == nil {
		return false
	}

	relaySpec := spec.GetRelaySpec()
	if relaySpec == nil {
		return false
	}

	fmt.Printf("%-40s%-40s%-20s%-20s\n",
		utils.IdToStr(spec.GetId()),
		utils.IdToStr(relaySpec.GetVPCId()),
		utils.IPAddrToStr(relaySpec.GetServerIP()),
		utils.IPAddrToStr(relaySpec.GetAgentIP()))
	return true
}
