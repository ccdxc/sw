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
	ruleID   string
)

var securityPolicyShowCmd = &cobra.Command{
	Use:   "security-policy",
	Short: "show security policy",
	Long:  "show security policy",
	Run:   securityPolicyShowCmdHandler,
}

var securityProfileShowCmd = &cobra.Command{
	Use:   "security-profile",
	Short: "show security profile",
	Long:  "show security profile",
	Run:   securityProfileShowCmdHandler,
}

var securityRuleShowCmd = &cobra.Command{
	Use:   "security-rule",
	Short: "show security rule",
	Long:  "show security rule",
	Run:   securityRuleShowCmdHandler,
}

func init() {
	showCmd.AddCommand(securityPolicyShowCmd)
	securityPolicyShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	securityPolicyShowCmd.Flags().Bool("summary", false, "Display number of objects")
	securityPolicyShowCmd.Flags().StringVarP(&policyID, "id", "i", "", "Specify ID")

	showCmd.AddCommand(securityProfileShowCmd)
	securityProfileShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	securityProfileShowCmd.Flags().Bool("summary", false, "Display number of objects")

	showCmd.AddCommand(securityRuleShowCmd)
	securityRuleShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	securityRuleShowCmd.Flags().StringVarP(&policyID, "policy-id", "p", "", "Specify policy ID")
	securityRuleShowCmd.Flags().StringVarP(&ruleID, "rule-id", "r", "", "Specify rule ID")
}

func securityPolicyShowCmdHandler(cmd *cobra.Command, args []string) {
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
		fmt.Printf("Getting policy failed, err %v\n", err)
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
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printPolicySummary(len(respMsg.Response))
	} else {
		for _, resp := range respMsg.Response {
			printPolicy(resp)
		}
		printPolicySummary(len(respMsg.Response))
	}
}

func printPolicySummary(count int) {
	fmt.Printf("\nNo. of security policies : %d\n\n", count)
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

		l3Match := rule.GetAttrs().GetMatch().GetL3Match()
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
		l4Match := rule.GetAttrs().GetMatch().GetL4Match()

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

		outStr += fmt.Sprintf("%-10d", rule.GetAttrs().GetPriority())
		outStr += fmt.Sprintf("%-10s", utils.BoolToString(rule.GetAttrs().GetStateful()))
		outStr += fmt.Sprintf("%-10s",
			strings.ToLower(strings.Replace(rule.GetAttrs().GetAction().String(), "SECURITY_RULE_ACTION_", "", -1)))

		// Display the overhang of IPv6 Src/Dst Range in the second line
		if ipv6SrcRangeStr != "" || ipv6DstRangeStr != "" {
			outStr += fmt.Sprintf("\n%-40s%-10s%-48s%-48s",
				"", "", ipv6SrcRangeStr, ipv6DstRangeStr)
		}

		fmt.Println(outStr)
	}
}

func securityProfileShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSecurityPolicySvcClient(c)

	var req *pds.SecurityProfileGetRequest
	// Get profiles - its a singleton
	req = &pds.SecurityProfileGetRequest{
		Id: [][]byte{},
	}

	// PDS call
	respMsg, err := client.SecurityProfileGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting policy failed, err %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print the profiles
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printProfileSummary(len(respMsg.Response))
	} else {
		for _, resp := range respMsg.Response {
			printProfile(resp)
		}
		printProfileSummary(len(respMsg.Response))
	}
}

func printProfileSummary(count int) {
	fmt.Printf("\nNo. of security profiles : %d\n\n", count)
}

func printProfile(resp *pds.SecurityProfile) {
	spec := resp.GetSpec()
	if spec == nil {
		return
	}

	fmt.Printf("%-26s : %s\n", "ID",
		uuid.FromBytesOrNil(spec.GetId()).String())

	fmt.Printf("%-26s : %t\n", "Connection Track Enable",
		spec.GetConnTrackEn())

	fmt.Printf("%-26s : %s\n", "Default FW Action",
		strings.Replace(spec.GetDefaultFWAction().String(),
			"SECURITY_RULE_ACTION_", "", -1))

	fmt.Printf("%-26s : %d\n", "TCP Idle Timeout",
		spec.GetTCPIdleTimeout())
	fmt.Printf("%-26s : %d\n", "UDP Idle Timeout",
		spec.GetUDPIdleTimeout())
	fmt.Printf("%-26s : %d\n", "ICMP Idle Timeout",
		spec.GetICMPIdleTimeout())
	fmt.Printf("%-26s : %d\n", "Other Idle Timeout",
		spec.GetOtherIdleTimeout())

	fmt.Printf("%-26s : %d\n", "TCP Conn. Setup Timeout",
		spec.GetTCPCnxnSetupTimeout())
	fmt.Printf("%-26s : %d\n", "TCP Half Close Timeout",
		spec.GetTCPHalfCloseTimeout())
	fmt.Printf("%-26s : %d\n", "TCP Close Timeout",
		spec.GetTCPCloseTimeout())

	fmt.Printf("%-26s : %d\n", "TCP Drop Timeout",
		spec.GetTCPDropTimeout())
	fmt.Printf("%-26s : %d\n", "UDP Drop Timeout",
		spec.GetUDPDropTimeout())
	fmt.Printf("%-26s : %d\n", "ICMP Drop Timeout",
		spec.GetICMPDropTimeout())
	fmt.Printf("%-26s : %d\n", "Other Drop Timeout",
		spec.GetOtherDropTimeout())
}

func securityRuleShowCmdHandler(cmd *cobra.Command, args []string) {
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

	if cmd != nil && (!cmd.Flags().Changed("policy-id") || !cmd.Flags().Changed("rule-id")) {
		fmt.Printf("Command arguments not provided, policy-id and rule-id are required for this CLI\n")
		return
	}

	client := pds.NewSecurityPolicySvcClient(c)

	// Get security-rule - returns a singleton object
	var req *pds.SecurityRuleGetRequest
	req = &pds.SecurityRuleGetRequest{
		Id: []*pds.SecurityPolicyRuleId{
			&pds.SecurityPolicyRuleId{
				Id:               uuid.FromStringOrNil(ruleID).Bytes(),
				SecurityPolicyId: uuid.FromStringOrNil(policyID).Bytes(),
			},
		},
	}

	// PDS call
	respMsg, err := client.SecurityRuleGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting security rule failed, err %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print the rules
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		for _, resp := range respMsg.Response {
			printRule(resp)
		}
	}
}

func printRule(resp *pds.SecurityRule) {
	spec := resp.GetSpec()
	if spec == nil {
		return
	}

	attr := spec.GetAttrs()
	if attr == nil {
		return
	}

	fmt.Printf("%-30s : %s\n", "Rule ID", uuid.FromBytesOrNil(spec.GetId()).String())
	fmt.Printf("%-30s : %s\n", "Policy ID", uuid.FromBytesOrNil(spec.GetSecurityPolicyId()).String())
	fmt.Printf("%-30s : %t\n", "Stateful", attr.GetStateful())
	fmt.Printf("%-30s : %d\n\n", "Priority", attr.GetPriority())

	// L3 info:
	// Print Protocol (Protocol Number or Wildcard)
	// Print Source (Prefix or Range or Tag)
	// Print Destination (Prefix or Range or Tag)
	fmt.Printf("L3 Match:\n")
	l3Match := attr.GetMatch().GetL3Match()
	switch l3Match.GetProtomatch().(type) {
	case *pds.RuleL3Match_ProtoNum:
		fmt.Printf("%-30s : %d\n", "Protocol Number", l3Match.GetProtoNum())
	case *pds.RuleL3Match_ProtoWildcard:
		fmt.Printf("%-30s : %s\n", "Protocol Match",
			strings.Replace(l3Match.GetProtoWildcard().String(), "MATCH_", "", -1))
	default:
		fmt.Printf("%-30s : %s\n", "Protocol Number", "-")
	}

	switch l3Match.GetSrcmatch().(type) {
	case *pds.RuleL3Match_SrcPrefix:
		fmt.Printf("%-30s : %s\n", "Source Match Type", "Prefix")
		fmt.Printf("%-30s : %s\n", "Source Prefix", utils.IPPrefixToStr(l3Match.GetSrcPrefix()))
	case *pds.RuleL3Match_SrcRange:
		switch l3Match.GetSrcRange().GetRange().(type) {
		case *pds.AddressRange_IPv4Range:
			fmt.Printf("%-30s : %s\n", "Source Match Type", "Range")
			ipRange := l3Match.GetSrcRange().GetIPv4Range()
			fmt.Printf("%-30s : %d\n", "Source Range Low",
				utils.IPAddrToStr(ipRange.GetLow()))
			fmt.Printf("%-30s : %d\n", "Source Range High",
				utils.IPAddrToStr(ipRange.GetHigh()))
		case *pds.AddressRange_IPv6Range:
			fmt.Printf("%-30s : %s\n", "Source Match Type", "Range")
			ipRange := l3Match.GetSrcRange().GetIPv6Range()
			fmt.Printf("%-30s : %d\n", "Source Range Low",
				utils.IPAddrToStr(ipRange.GetLow()))
			fmt.Printf("%-30s : %d\n", "Source Range High",
				utils.IPAddrToStr(ipRange.GetHigh()))
		default:
			fmt.Printf("%-30s : %s\n", "Source Match Type", "Range")
			fmt.Printf("%-30s : %s\n", "Source Range Low", "-")
			fmt.Printf("%-30s : %s\n", "Source Range High", "-")
		}

	case *pds.RuleL3Match_SrcTag:
		fmt.Printf("%-30s : %s\n", "Source Match Type", "Tag")
		fmt.Printf("%-30s : %d\n", "Source Tag", l3Match.GetSrcTag())
	default:
		fmt.Printf("%-30s : %s\n", "Source Match Type", "-")
	}

	switch l3Match.GetDstmatch().(type) {
	case *pds.RuleL3Match_DstPrefix:
		fmt.Printf("%-30s : %s\n", "Destination Match Type", "Prefix")
		fmt.Printf("%-30s : %s\n", "Destination Prefix",
			utils.IPPrefixToStr(l3Match.GetDstPrefix()))
	case *pds.RuleL3Match_DstRange:
		switch l3Match.GetDstRange().GetRange().(type) {
		case *pds.AddressRange_IPv4Range:
			fmt.Printf("%-30s : %s\n", "Destination Match Type", "Range")
			ipRange := l3Match.GetDstRange().GetIPv4Range()
			fmt.Printf("%-30s : %d\n", "Destination Range Low",
				utils.IPAddrToStr(ipRange.GetLow()))
			fmt.Printf("%-30s : %d\n", "Destination Range High",
				utils.IPAddrToStr(ipRange.GetHigh()))
		case *pds.AddressRange_IPv6Range:
			fmt.Printf("%-30s : %s\n", "Destination Match Type", "Range")
			ipRange := l3Match.GetDstRange().GetIPv6Range()
			fmt.Printf("%-30s : %d\n", "Destination Range Low",
				utils.IPAddrToStr(ipRange.GetLow()))
			fmt.Printf("%-30s : %d\n", "Destination Range High",
				utils.IPAddrToStr(ipRange.GetHigh()))
		default:
			fmt.Printf("%-30s : %s\n", "Destination Match Type", "Range")
			fmt.Printf("%-30s : %s\n", "Destination Range Low", "-")
			fmt.Printf("%-30s : %s\n", "Destination Range High", "-")
		}
	case *pds.RuleL3Match_DstTag:
		fmt.Printf("%-30s : %s\n", "Destination Match Type", "Tag")
		fmt.Printf("%-30s : %d\n", "Destination Tag", l3Match.GetDstTag())
	default:
		fmt.Printf("%-30s : %s\n", "Destination Match Type", "-")
	}

	// L4 info:
	// Print Source Port: Low to High
	// Print Dest Port: Low to High
	// Print ICMP info
	fmt.Printf("\nL4 Match:\n")
	l4Match := attr.GetMatch().GetL4Match()
	switch l4Match.GetL4Info().(type) {
	case *pds.RuleL4Match_Ports:
		fmt.Printf("%-30s : %s\n", "Match Type", "Port")
		srcPortRange := l4Match.GetPorts().GetSrcPortRange()
		dstPortRange := l4Match.GetPorts().GetDstPortRange()
		fmt.Printf("%-30s : %d\n", "Source Port Low",
			srcPortRange.GetPortLow())
		fmt.Printf("%-30s : %d\n", "Source Port High",
			srcPortRange.GetPortHigh())
		fmt.Printf("%-30s : %d\n", "Destination Port Low",
			dstPortRange.GetPortLow())
		fmt.Printf("%-30s : %d\n", "Destination Port High",
			dstPortRange.GetPortHigh())
	case *pds.RuleL4Match_TypeCode:
		fmt.Printf("%-30s : %s\n", "Match Type", "ICMP")
		switch l4Match.GetTypeCode().GetTypematch().(type) {
		case *pds.ICMPMatch_TypeNum:
			fmt.Printf("%-30s : %d\n", "Type number",
				l4Match.GetTypeCode().GetTypeNum())
		case *pds.ICMPMatch_TypeWildcard:
			fmt.Printf("%-30s : %s\n", "Type",
				strings.Replace(l4Match.GetTypeCode().GetTypeWildcard().String(), "MATCH_", "", -1))
		default:
			fmt.Printf("%-30s : %s\n", "Type Number", "-")
		}

		switch l4Match.GetTypeCode().GetCodematch().(type) {
		case *pds.ICMPMatch_CodeNum:
			fmt.Printf("%-30s : %d\n", "Code number",
				l4Match.GetTypeCode().GetCodeNum())
		case *pds.ICMPMatch_CodeWildcard:
			fmt.Printf("%-30s : %s\n", "Code",
				strings.Replace(l4Match.GetTypeCode().GetCodeWildcard().String(), "MATCH_", "", -1))
		default:
		}
	default:
		fmt.Printf("%-30s : %s\n", "Match Type", "-")
	}

	fmt.Printf("\n%-30s : %s\n", "Security Rule Action",
		strings.Replace(attr.GetAction().String(), "SECURITY_RULE_ACTION_", "", -1))
}
