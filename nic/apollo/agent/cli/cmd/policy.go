//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bytes"
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
	policyID string
)

var securityPolicyShowCmd = &cobra.Command{
	Use:   "security-policy",
	Short: "show security policy",
	Long:  "show security policy",
	Run:   securityPolicyShowCmdHandler,
}

func init() {
	showCmd.AddCommand(securityPolicyShowCmd)
	securityPolicyShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	securityPolicyShowCmd.Flags().StringVarP(&policyID, "id", "i", "", "Specify ID")
}

func securityPolicyShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSecurityPolicySvcClient(c)

	var req *pds.SecurityPolicyGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific Rule/Policy
		req = &pds.SecurityPolicyGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(policyID).Bytes()},
		}
	} else {
		// Get all Rules/Policies
		req = &pds.SecurityPolicyGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.SecurityPolicyGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting policy failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print the rules/policies
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		for _, resp := range respMsg.Response {
			printPolicy(resp)
		}
	}
}

func printPolicyRuleHeader() {
	hdrLine := strings.Repeat("-", 215)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-10s%-48s%-48s%-13s%-13s%-13s%-10s%-10s%-10s\n",
		"Rule ID", "Protocol", "      Source", "    Destination",
		"Source Port", "Dest Port", "ICMP", "Priority", "Stateful", "Action")
	fmt.Printf("%-40s%-10s%-48s%-48s%-13s%-13s%-13s%-10s%-10s%-10s\n",
		"", "", "Prefix | Range | Tag", "Prefix | Range | Tag",
		"Low-High", "Low-High", "Type/Code", "", "", "")
	fmt.Println(hdrLine)
}

func printPolicy(resp *pds.SecurityPolicy) {
	spec := resp.GetSpec()

	fmt.Printf("%-18s : %-40s\n", "Policy ID",
		uuid.FromBytesOrNil(spec.GetId()).String())
	fmt.Printf("%-18s : %-10s\n", "Address Family",
		utils.AddrFamilyToStr(spec.GetAddrFamily()))
	fmt.Printf("%-18s : %-20s\n", "Default FW Action",
		strings.Replace(spec.GetDefaultFWAction().String(), "SECURITY_RULE_ACTION_", "", -1))
	printPolicyRuleHeader()

	for _, rule := range spec.Rules {
		outStr := ""
		ipRangeStr := ""
		ipv6SrcRangeStr := "" // To be printed in second line
		ipv6DstRangeStr := "" // To be printed in second line

		emptyByte := make([]byte, len(rule.GetId()))
		if !bytes.Contains(rule.GetId(), emptyByte) {
			outStr += fmt.Sprintf("%-40s", uuid.FromBytesOrNil(rule.GetId()).String())
		} else {
			outStr += fmt.Sprintf("%-40s", "-")
		}

		// L3 info:
		// Print Protocol
		// Print Source (Prefix or Range or Tag)
		// Print Destination (Prefix or Range or Tag)

		l3Match := rule.GetMatch().GetL3Match()
		outStr += fmt.Sprintf("%-10d", l3Match.GetProtoNum())

		switch l3Match.GetSrcmatch().(type) {
		case *pds.RuleL3Match_SrcPrefix:
			outStr += fmt.Sprintf("%-48s", utils.IPPrefixToStr(l3Match.GetSrcPrefix()))
		case *pds.RuleL3Match_SrcRange:

			switch l3Match.GetSrcRange().GetRange().(type) {
			case *pds.AddressRange_IPv4Range:
				ipRange := l3Match.GetSrcRange().GetIPv4Range()
				ipRangeStr = fmt.Sprintf("%s - %s",
					utils.IPAddrToStr(ipRange.GetLow()),
					utils.IPAddrToStr(ipRange.GetHigh()))
			case *pds.AddressRange_IPv6Range:
				ipRange := l3Match.GetSrcRange().GetIPv6Range()
				ipRangeStr = fmt.Sprintf("%s -",
					utils.IPAddrToStr(ipRange.GetLow()))
				ipv6SrcRangeStr = fmt.Sprintf(" %s",
					utils.IPAddrToStr(ipRange.GetHigh()))
			default:
				ipRangeStr = "-"
			}
			outStr += fmt.Sprintf("%-48s", ipRangeStr)

		case *pds.RuleL3Match_SrcTag:
			outStr += fmt.Sprintf("%-48d", l3Match.GetSrcTag())
		default:
			outStr += fmt.Sprintf("%-48s", "-")
		}

		switch l3Match.GetDstmatch().(type) {
		case *pds.RuleL3Match_DstPrefix:
			outStr += fmt.Sprintf("%-48s", utils.IPPrefixToStr(l3Match.GetDstPrefix()))
		case *pds.RuleL3Match_DstRange:
			switch l3Match.GetDstRange().GetRange().(type) {
			case *pds.AddressRange_IPv4Range:
				ipRange := l3Match.GetDstRange().GetIPv4Range()
				ipRangeStr = fmt.Sprintf("%s - %s",
					utils.IPAddrToStr(ipRange.GetLow()),
					utils.IPAddrToStr(ipRange.GetHigh()))
			case *pds.AddressRange_IPv6Range:
				ipRange := l3Match.GetDstRange().GetIPv6Range()
				ipRangeStr = fmt.Sprintf("%s -",
					utils.IPAddrToStr(ipRange.GetLow()))
				ipv6DstRangeStr = fmt.Sprintf(" %s",
					utils.IPAddrToStr(ipRange.GetHigh()))
			default:
				ipRangeStr = "-"
			}
			outStr += fmt.Sprintf("%-48s", ipRangeStr)
		case *pds.RuleL3Match_DstTag:
			outStr += fmt.Sprintf("%-48d", l3Match.GetDstTag())
		default:
			outStr += fmt.Sprintf("%-48s", "-")
		}

		// L4 info:
		// Print Source Port: Low to High
		// Print Dest Port: Low to High
		// Print ICMP info
		l4Match := rule.GetMatch().GetL4Match()

		switch l4Match.GetL4Info().(type) {
		case *pds.RuleL4Match_Ports:
			srcPortRange := l4Match.GetPorts().GetSrcPortRange()
			dstPortRange := l4Match.GetPorts().GetDstPortRange()
			srcLowHighStr := fmt.Sprintf("%d-%d",
				srcPortRange.GetPortLow(), srcPortRange.GetPortHigh())
			dstLowHighStr := fmt.Sprintf("%d-%d",
				dstPortRange.GetPortLow(), dstPortRange.GetPortHigh())
			outStr += fmt.Sprintf("%-13s%-13s%-13s",
				srcLowHighStr, dstLowHighStr, "-")
		case *pds.RuleL4Match_TypeCode:
			outStr += fmt.Sprintf("%-13s%-13s%6d/-%6d", "-", "-",
				l4Match.GetTypeCode().GetTypeNum(),
				l4Match.GetTypeCode().GetCodeNum())
		default:
			outStr += fmt.Sprintf("%-13s%-13s%-13s", "-", "-", "-")
		}

		outStr += fmt.Sprintf("%-10d", rule.GetPriority())
		outStr += fmt.Sprintf("%-10s", utils.BoolToString(rule.GetStateful()))
		outStr += fmt.Sprintf("%-10s",
			strings.ToLower(strings.Replace(rule.GetAction().String(), "SECURITY_RULE_ACTION_", "", -1)))

		// Display the overhang of IPv6 Src/Dst Range in the second line
		if ipv6SrcRangeStr != "" || ipv6DstRangeStr != "" {
			outStr += fmt.Sprintf("\n%-40s%-10s%-48s%-48s",
				"", "", ipv6SrcRangeStr, ipv6DstRangeStr)
		}

		fmt.Println(outStr)
	}
}
