//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"strconv"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	nwsecProfID   uint32
	nwsecVrfID    uint64
	nwsecPolicyID uint64
)

var nwsecShowCmd = &cobra.Command{
	Use:   "nwsec",
	Short: "show Security information",
	Long:  "show Security object information",
}

var nwsecProfShowCmd = &cobra.Command{
	Use:   "profile",
	Short: "show Security profile information",
	Long:  "show Security profile object information",
	Run:   nwsecProfShowCmdHandler,
}

var nwsecPolicyShowCmd = &cobra.Command{
	Use:   "policy",
	Short: "show Security policy information",
	Long:  "show Security policy object information",
	Run:   nwsecPolicyShowCmdHandler,
}

var nwsecFlowGateShowCmd = &cobra.Command{
	Use:   "flow-gate",
	Short: "show Security flow-gate information",
	Long:  "show Security flow-gate object information",
	Run:   nwsecFlowGateShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nwsecShowCmd)
	nwsecShowCmd.AddCommand(nwsecProfShowCmd)
	nwsecShowCmd.AddCommand(nwsecPolicyShowCmd)
	// Commenting yaml option as on large polices halclt runs out of memory
	//nwsecPolicyShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	nwsecShowCmd.AddCommand(nwsecFlowGateShowCmd)

	nwsecProfShowCmd.Flags().Uint32Var(&nwsecProfID, "id", 1, "Specify security profile ID")
	nwsecPolicyShowCmd.Flags().Uint64Var(&nwsecVrfID, "vrf-id", 1, "Specify VRF ID")
	nwsecPolicyShowCmd.Flags().Uint64Var(&nwsecPolicyID, "policy-id", 1, "Specify security policy ID")
}

func handleNwsecProfShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c)

	var req *halproto.SecurityProfileGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		req = &halproto.SecurityProfileGetRequest{
			KeyOrHandle: &halproto.SecurityProfileKeyHandle{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
					ProfileId: nwsecProfID,
				},
			},
		}
	} else {
		// Get all Security Profiles
		req = &halproto.SecurityProfileGetRequest{}
	}
	secProfGetReqMsg := &halproto.SecurityProfileGetRequestMsg{
		Request: []*halproto.SecurityProfileGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityProfileGet(context.Background(), secProfGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Security Profile failed. %v\n", err)
		return
	}

	// Print Security Profiles
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func nwsecProfShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handleNwsecProfShowCmd(cmd, nil)
}

func nwsecPolicyShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("yaml") {
		nwsecPolicyDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c)

	var req *halproto.SecurityPolicyGetRequest
	if cmd != nil && cmd.Flags().Changed("vrf-id") && cmd.Flags().Changed("policy-id") {
		req = &halproto.SecurityPolicyGetRequest{
			KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
				PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
					SecurityPolicyKey: &halproto.SecurityPolicyKey{
						SecurityPolicyId: nwsecPolicyID,
						VrfIdOrHandle: &halproto.VrfKeyHandle{
							KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
								VrfId: nwsecVrfID,
							},
						},
					},
				},
			},
		}
	} else {
		// Get all Security Policies
		req = &halproto.SecurityPolicyGetRequest{}
	}

	secPolicyGetReqMsg := &halproto.SecurityPolicyGetRequestMsg{
		Request: []*halproto.SecurityPolicyGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityPolicyGet(context.Background(), secPolicyGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Security Policy failed. %v\n", err)
		return
	}

	// Print Security Policy
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			fmt.Println("Policy Show not ok")
			continue
		}
		nwsecPolicyShowOneResp(resp)
	}
}

func nwsecPolicyShowOneResp(resp *halproto.SecurityPolicyGetResponse) {
	spec := resp.GetSpec()
	stats := resp.GetPolStats()
	status := resp.GetStatus()

	hdrLine := strings.Repeat("-", 50)
	fmt.Println(hdrLine)
	fmt.Printf("\nSecurity Policy Id: %d%s%d\n", spec.GetKeyOrHandle().GetSecurityPolicyKey().GetSecurityPolicyId(),
		" Vrf id: ", spec.GetKeyOrHandle().GetSecurityPolicyKey().GetVrfIdOrHandle().GetVrfId())
	fmt.Println(hdrLine)
	fmt.Println("\nRules:")
	for i, rule := range spec.Rule {
		ruleStatus := status.RuleStatus[i]
		fmt.Printf("\nRule Id: %-5d\n", rule.GetRuleId())
		fmt.Printf("Rule Priority: %-5d\n", ruleStatus.GetPriority())
		fmt.Println("Rule Match:")
		fmt.Printf("   Src IP Address:  ")
		if len(rule.Match.SrcAddress) == 0 {
			fmt.Printf("ANY\n")
		}
		for _, src := range rule.Match.SrcAddress {
			switch src.Formats.(type) {
			case *halproto.IPAddressObj_Type:
				iptype := strings.ToLower(strings.Replace(src.GetType().String(), "IP_ADDRESS_", "", -1))
				fmt.Printf(iptype)
			case *halproto.IPAddressObj_Address:
				address := src.GetAddress()
				switch address.Address.(type) {
				case *halproto.Address_Range:
					addrrange := address.GetRange()
					switch addrrange.Range.(type) {
					case *halproto.AddressRange_Ipv4Range:
						low := addrrange.GetIpv4Range().GetLowIpaddr().GetV4Addr()
						high := addrrange.GetIpv4Range().GetHighIpaddr().GetV4Addr()
						fmt.Printf("%-5s - %-5s", Uint32IPAddrToStr(low), Uint32IPAddrToStr(high))
					default:
						break
					}
				case *halproto.Address_Prefix:
					prefix := address.GetPrefix()
					switch prefix.Subnet.(type) {
					case *halproto.IPSubnet_Ipv4Subnet:
						addr := prefix.GetIpv4Subnet().GetAddress().GetV4Addr()
						fmt.Printf("%-5s%s%-5d", Uint32IPAddrToStr(addr), "/",
							prefix.GetIpv4Subnet().GetPrefixLen())
					default:
						break
					}
				default:
					break
				}
			default:
				break
			}
			fmt.Println(",")
		}
		fmt.Printf("   Dst IP Address:  ")
		if len(rule.Match.DstAddress) == 0 {
			fmt.Printf("ANY\n")
		}
		for _, dst := range rule.Match.DstAddress {
			switch dst.Formats.(type) {
			case *halproto.IPAddressObj_Type:
				iptype := strings.ToLower(strings.Replace(dst.GetType().String(), "IP_ADDRESS_", "", -1))
				fmt.Printf(iptype)
			case *halproto.IPAddressObj_Address:
				address := dst.GetAddress()
				switch address.Address.(type) {
				case *halproto.Address_Range:
					addrrange := address.GetRange()
					switch addrrange.Range.(type) {
					case *halproto.AddressRange_Ipv4Range:
						low := addrrange.GetIpv4Range().GetLowIpaddr().GetV4Addr()
						high := addrrange.GetIpv4Range().GetHighIpaddr().GetV4Addr()
						fmt.Printf("%s-%s", Uint32IPAddrToStr(low), Uint32IPAddrToStr(high))
					default:
						break
					}
				case *halproto.Address_Prefix:
					prefix := address.GetPrefix()
					switch prefix.Subnet.(type) {
					case *halproto.IPSubnet_Ipv4Subnet:
						addr := prefix.GetIpv4Subnet().GetAddress().GetV4Addr()
						fmt.Printf("%s%s%d", Uint32IPAddrToStr(addr), "/",
							prefix.GetIpv4Subnet().GetPrefixLen())
					default:
						break
					}
				default:
					break
				}
			default:
				break
			}
			fmt.Println(",")
		}
		proto := rule.Match.GetProtocol()
		fmt.Println("   Protocol: ", proto)
		switch rule.Match.AppMatch.App.(type) {
		case *halproto.RuleMatch_AppMatch_PortInfo:
			fmt.Printf("   Source Ports: ")
			if len(rule.Match.AppMatch.GetPortInfo().SrcPortRange) > 0 {
				for _, portrange := range rule.Match.AppMatch.GetPortInfo().SrcPortRange {
					fmt.Printf(" %d-%d,", portrange.GetPortLow(), portrange.GetPortHigh())
				}
			} else {
				fmt.Printf("ANY")
			}
			fmt.Printf("\n   Destination Ports: ")
			for _, portrange := range rule.Match.AppMatch.GetPortInfo().DstPortRange {
				fmt.Printf("%d-%d,", portrange.GetPortLow(), portrange.GetPortHigh())
			}
		case *halproto.RuleMatch_AppMatch_IcmpInfo:
			fmt.Printf("\n   ICMP Type: %d Code: %d\n", rule.Match.AppMatch.GetIcmpInfo().GetIcmpType(),
				rule.Match.AppMatch.GetIcmpInfo().GetIcmpCode())
		case *halproto.RuleMatch_AppMatch_EspInfo:
			fmt.Printf("\n   ESP SPI: %d\n", rule.Match.AppMatch.GetEspInfo().GetSpi())
		case nil:
			// field not set
		default:
			break
		}
		fmt.Printf("\n   Apps: ")
		for _, app := range rule.Appid {
			fmt.Printf("%s", app)
		}
		fmt.Println("\nRule Actions:")
		secAction := strings.Replace(rule.Action.GetSecAction().String(), "SECURITY_RULE_ACTION_", "", -1)
		fmt.Println("   Security Action: ", secAction)
		logAction := strings.Replace(rule.Action.GetLogAction().String(), "LOG_", "", -1)
		fmt.Println("   Log Action: ", logAction)
		alg := strings.Replace(rule.Action.GetAppData().GetAlg().String(), "APP_SVC_", "", -1)
		fmt.Println("   ALG: ", alg)
		fmt.Println("   Alg Info:")
		appData := rule.Action.GetAppData()
		switch rule.Action.GetAppData().GetAlg() {
		case 2: /* APP_SVC_FTP */
			fmt.Printf("      Allow Mismatch IP: %t\n", appData.GetFtpOptionInfo().GetAllowMismatchIpAddress())
		case 3: /* APP_SVC_DNS */
			fmt.Println("      Drop Multi Question Packets: ", appData.GetDnsOptionInfo().GetDropMultiQuestionPackets())
			fmt.Println("      Drop Large Domain Name Packets: ", appData.GetDnsOptionInfo().GetDropLargeDomainNamePackets())
			fmt.Println("      Drop Long Label Packets: ", appData.GetDnsOptionInfo().GetDropLongLabelPackets())
			fmt.Println("      Drop Multi Zone Packets: ", appData.GetDnsOptionInfo().GetDropMultizonePackets())
			fmt.Println("      Maximum DNS Message Length: ", appData.GetDnsOptionInfo().GetMaxMsgLength())
			fmt.Println("      Query Response Timeout: ", appData.GetDnsOptionInfo().GetQueryResponseTimeout())
		case 5: /* APP_SVC_SUN_RPC */
			fmt.Printf("      ProgramId%sIdletimeout: ", "/")
			for _, rpcData := range appData.GetSunRpcOptionInfo().Data {
				fmt.Printf(" (%s%s%d),", rpcData.GetProgramId(), "/", rpcData.GetIdleTimeout())
			}
			fmt.Printf("\n")
		case 6: /*APP_SVC_MSFT_RPC */
			fmt.Printf("      ProgramId%sIdletimeout: ", "/")
			for _, rpcData := range appData.GetMsrpcOptionInfo().Data {
				fmt.Printf(" (%s%s%d),", rpcData.GetProgramId(), "/", rpcData.GetIdleTimeout())
			}
			fmt.Printf("\n")
		default:
			break
		}
		fmt.Println("Rule Stats: ")
		for _, stat := range stats.RuleStats {
			if stat.GetRuleId() == rule.GetRuleId() {
				fmt.Println("   Total Hits: ", stat.GetNumHits())
				fmt.Println("   Number of TCP Hits: ", stat.GetNumTcpHits())
				fmt.Println("   Number of UDP Hits: ", stat.GetNumUdpHits())
				fmt.Println("   Number of ICMP Hits: ", stat.GetNumIcmpHits())
				break
			}
		}
	}
}

func nwsecPolicyDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleNwsecPolicyDetailShowCmd(cmd, nil)
}

func handleNwsecPolicyDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c)

	var req *halproto.SecurityPolicyGetRequest
	if cmd != nil && cmd.Flags().Changed("vrf-id") && cmd.Flags().Changed("policy-id") {
		req = &halproto.SecurityPolicyGetRequest{
			KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
				PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
					SecurityPolicyKey: &halproto.SecurityPolicyKey{
						SecurityPolicyId: nwsecPolicyID,
						VrfIdOrHandle: &halproto.VrfKeyHandle{
							KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
								VrfId: nwsecVrfID,
							},
						},
					},
				},
			},
		}
	} else {
		// Get all Security Policies
		req = &halproto.SecurityPolicyGetRequest{}
	}

	secPolicyGetReqMsg := &halproto.SecurityPolicyGetRequestMsg{
		Request: []*halproto.SecurityPolicyGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityPolicyGet(context.Background(), secPolicyGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Security Policy failed. %v\n", err)
		return
	}

	// Print Security Policy
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			fmt.Println("Policy Show not ok")
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func nwsecFlowGateShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c)

	var req *halproto.SecurityFlowGateGetRequest

	req = &halproto.SecurityFlowGateGetRequest{}
	secProfGetReqMsg := &halproto.SecurityFlowGateGetRequestMsg{
		Request: []*halproto.SecurityFlowGateGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityFlowGateGet(context.Background(), secProfGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Security Flow Gates failed. %v\n", err)
		return
	}

	nwsecFlowGateShowHeader()

	// Print Security Profiles
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		nwsecFlowGateShowOneResp(resp)
	}
}

func nwsecFlowGateShowHeader() {
	hdrLine := strings.Repeat("-", 130)
	fmt.Printf("Legend:\n")
	fmt.Printf("Dir: Direction: U (Uplink), H (Host)\n")
	fmt.Printf("KeyType: Flow Key Type\n")
	fmt.Printf("VrfID: Source VRF ID/Destination VRF ID\n")
	fmt.Printf("SMAC|SIP[:sport]: Source MAC Address | Source IP Address and Port Number\n")
	fmt.Printf("DMAC|DIP[:dport]: Destination MAC Address | Destination IP Address and Port Number\n")
	fmt.Printf("Proto: L4 Protocol\n")
	fmt.Printf("ALG: ALG that opened flow gate\n")
	fmt.Printf("State: MD (Marked for deletion)\t Ref Count: Number of active sessions based on the gate\n")
	fmt.Printf("Time To Age: Inactivity time remaining for flow to age in seconds (IDF- No aging indefinite time)\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-8s%-10s%-24s%-24s%-16s%-10s%-16s%-10s%-10s\n",
		"Dir", "KeyType", "VrfID",
		"SIP[:sport]", "DIP[:dport]", "Proto", "ALG", "State",
		"RefCount", "Time To Age")
	fmt.Println(hdrLine)
}

func nwsecFlowGateShowOneResp(resp *halproto.SecurityFlowGateGetResponse) {
	flowGatekey := resp.GetFlowGateKey()
	flowStr := strings.Replace(flowGatekey.GetDirection().String(), "FLOW_DIRECTION_FROM_", "", -1)
	flowStr = flowStr[0:1]

	keyType := "IPv4"
	src := Uint32IPAddrToStr(flowGatekey.GetSrcIp().GetV4Addr())
	dst := Uint32IPAddrToStr(flowGatekey.GetDstIp().GetV4Addr())
	src += ":["
	src += strconv.Itoa(int(flowGatekey.GetSrcPort()))
	src += "]"
	dst += ":["
	dst += strconv.Itoa(int(flowGatekey.GetDstPort()))
	dst += "]"
	proto := strings.Replace(flowGatekey.GetIpProto().String(), "IPPROTO_", "", -1)
	svrf := flowGatekey.GetSrcVrfId()
	dvrf := flowGatekey.GetDstVrfId()
	vrfStr := strconv.Itoa(int(svrf)) + "/" + strconv.Itoa(int(dvrf))
	mdStr := ""
	if resp.GetDeleteMarked() == true {
		mdStr = "MD"
	}
	algStr := strings.Replace(resp.GetAlg().String(), "APP_SVC_", "", -1)
	timeToAge := resp.GetTimeToAge()
	timeToAgeStr := ""
	if timeToAge == 0xFFFFFFFF {
		timeToAgeStr = "IDF"
	} else {
		if timeToAge > 59 {
			timeToAgeStr += strconv.Itoa(int(timeToAge/60)) + "m "
		}
		timeToAgeStr += strconv.Itoa(int(timeToAge%60)) + "s"
	}
	refCountstr := strconv.Itoa(int(resp.GetRefCount()))
	fmt.Printf("%-6s%-8s%-10s%-24s%-24s%-16s%-10s%-16s%-10s%-10s\n",
		flowStr, keyType, vrfStr, src, dst, proto, algStr,
		mdStr, refCountstr, timeToAgeStr)
}
