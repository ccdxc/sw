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
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printDHCPRelaySummary(len(respMsg.Response))
	} else {
		printDHCPRelayHeader()
		for _, resp := range respMsg.Response {
			printDHCPRelay(resp)
		}
		printDHCPRelaySummary(len(respMsg.Response))
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
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printDHCPProxySummary(len(respMsg.Response))
	} else {
		printDHCPProxyHeader()
		for _, resp := range respMsg.Response {
			printDHCPProxy(resp)
		}
		printDHCPProxySummary(len(respMsg.Response))
	}
}

func printDHCPProxySummary(count int) {
	fmt.Printf("\nNo. of DHCP proxy policies : %d\n\n", count)
}

func printDHCPProxyHeader() {
	hdrline := strings.Repeat("-", 331)
	fmt.Println(hdrline)
	fmt.Printf("%-40s%-45s%-45s%-45s%-130s%-16s%-10s\n",
		"ID", "Gateway IP", "DNS Server IP", "NTP Server IP",
		"Domain Name", "Lease", "MTU")
	fmt.Printf("%-40s%-45s%-45s%-45s%-130s%-16s%-10s\n",
		"", "", "", "", "", "Timeout(sec.)", "MTU")
	fmt.Println(hdrline)
}

func printDHCPProxy(dhcp *pds.DHCPPolicy) {

	if dhcp == nil {
		fmt.Printf("-\n")
		return
	}

	spec := dhcp.GetSpec()
	if spec == nil {
		fmt.Printf("-\n")
		return
	}

	outStr := fmt.Sprintf("%-40s", "ID", uuid.FromBytesOrNil(spec.GetId()).String())

	switch spec.GetRelayOrProxy().(type) {
	case *pds.DHCPPolicySpec_ProxySpec:
		proxySpec := spec.GetProxySpec()
		outStr += fmt.Sprintf("%-45s", utils.IPAddrToStr(proxySpec.GetGatewayIP()))
		outStr += fmt.Sprintf("%-45s", utils.IPAddrToStr(proxySpec.GetDNSServerIP()))
		outStr += fmt.Sprintf("%-45s", utils.IPAddrToStr(proxySpec.GetNTPServerIP()))
		outStr += fmt.Sprintf("%-130s", proxySpec.GetDomainName())
		outStr += fmt.Sprintf("%-20d", proxySpec.GetLeaseTimeout())
		outStr += fmt.Sprintf("%-10d", (proxySpec.GetMTU()))
	default:
		return
	}
	fmt.Println(outStr)
}

func printDHCPRelaySummary(count int) {
	fmt.Printf("\nNo. of DHCP relay policies : %d\n\n", count)
}

func printDHCPRelayHeader() {
	hdrline := strings.Repeat("-", 125)
	fmt.Println(hdrline)
	fmt.Printf("%-40s%-40s%-45s\n", "ID", "VPC ID", "Agent IP")
	fmt.Println(hdrline)
}

func printDHCPRelay(dhcp *pds.DHCPPolicy) {

	if dhcp == nil {
		fmt.Printf("-\n")
		return
	}

	spec := dhcp.GetSpec()
	if spec == nil {
		fmt.Printf("-\n")
		return
	}

	outStr := fmt.Sprintf("%-40s", "ID", uuid.FromBytesOrNil(spec.GetId()).String())

	switch spec.GetRelayOrProxy().(type) {
	case *pds.DHCPPolicySpec_RelaySpec:
		relaySpec := spec.GetRelaySpec()
		outStr += fmt.Sprintf("%-40s", uuid.FromBytesOrNil(relaySpec.GetVPCId()).String())
		outStr += fmt.Sprintf("%-45s", utils.IPAddrToStr(relaySpec.GetAgentIP()))
	default:
		return
	}
	fmt.Println(outStr)
}
