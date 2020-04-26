//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"sort"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	collectorID uint64
	mirrorID    uint64
	ruleID      uint64
)

var collectorShowCmd = &cobra.Command{
	Use:   "collector",
	Short: "show collector information",
	Long:  "show collector information",
	Run:   collectorShowCmdHandler,
}

var mirrorShowCmd = &cobra.Command{
	Use:   "mirror",
	Short: "show mirror information",
	Long:  "show mirror information",
	Run:   mirrorShowCmdHandler,
}

var flowMonitorShowCmd = &cobra.Command{
	Use:   "flow-monitor",
	Short: "show flow-monitor information",
	Long:  "show flow-monitor information",
	Run:   flowMonitorShowCmdHandler,
}

func init() {
	showCmd.AddCommand(collectorShowCmd)
	showCmd.AddCommand(mirrorShowCmd)
	showCmd.AddCommand(flowMonitorShowCmd)
	collectorShowCmd.Flags().Uint64Var(&collectorID, "collector-id", 1, "Specify collector id")
	mirrorShowCmd.Flags().Uint64Var(&mirrorID, "mirror-session-id", 1, "Specify mirror session id")
	mirrorShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	flowMonitorShowCmd.Flags().Uint64Var(&ruleID, "flow-monitor-rule-id", 1, "Specify flow-monitor rule id")
}

func flowMonitorShowCmdHandler(cmd *cobra.Command, args []string) {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewTelemetryClient(c)

	defer c.Close()

	var req *halproto.FlowMonitorRuleGetRequest
	if cmd != nil && cmd.Flags().Changed("flow-monitor-rule-id") {
		req = &halproto.FlowMonitorRuleGetRequest{
			KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
				KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
					FlowmonitorruleId: ruleID,
				},
			},
		}
	} else {
		req = &halproto.FlowMonitorRuleGetRequest{}
	}

	ruleReqMsg := &halproto.FlowMonitorRuleGetRequestMsg{
		Request: []*halproto.FlowMonitorRuleGetRequest{req},
	}

	respMsg, err := client.FlowMonitorRuleGet(context.Background(),
		ruleReqMsg)
	if err != nil {
		fmt.Printf("Flow Monitor Rule Get failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		fmt.Println("Flow Monitor Rule id:		", resp.GetSpec().GetKeyOrHandle().GetFlowmonitorruleId())
		fmt.Println("Vrf:				", resp.GetSpec().GetVrfKeyHandle().GetVrfId())
		fmt.Println("Rule Match:")
		fmt.Printf("   Src IP Address:  ")
		if len(resp.GetSpec().GetMatch().SrcAddress) == 0 {
			fmt.Printf("ANY\n")
		}
		for _, src := range resp.GetSpec().GetMatch().SrcAddress {
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
		if len(resp.GetSpec().GetMatch().DstAddress) == 0 {
			fmt.Printf("ANY\n")
		}
		for _, dst := range resp.GetSpec().GetMatch().DstAddress {
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
		proto := resp.GetSpec().GetMatch().GetProtocol()
		fmt.Println("   Protocol: ", proto)
		switch resp.GetSpec().GetMatch().AppMatch.App.(type) {
		case *halproto.RuleMatch_AppMatch_PortInfo:
			fmt.Printf("   Source Ports: ")
			if len(resp.GetSpec().GetMatch().AppMatch.GetPortInfo().SrcPortRange) > 0 {
				for _, portrange := range resp.GetSpec().GetMatch().AppMatch.GetPortInfo().SrcPortRange {
					fmt.Printf(" %d-%d,", portrange.GetPortLow(), portrange.GetPortHigh())
				}
			} else {
				fmt.Printf("ANY")
			}
			fmt.Printf("\n   Destination Ports: ")
			for _, portrange := range resp.GetSpec().GetMatch().AppMatch.GetPortInfo().DstPortRange {
				fmt.Printf("%d-%d,", portrange.GetPortLow(), portrange.GetPortHigh())
			}
		case *halproto.RuleMatch_AppMatch_IcmpInfo:
			fmt.Printf("\n   ICMP Type: %d Code: %d\n", resp.GetSpec().GetMatch().AppMatch.GetIcmpInfo().GetIcmpType(),
				resp.GetSpec().GetMatch().AppMatch.GetIcmpInfo().GetIcmpCode())
		case *halproto.RuleMatch_AppMatch_EspInfo:
			fmt.Printf("\n   ESP SPI: %d\n", resp.GetSpec().GetMatch().AppMatch.GetEspInfo().GetSpi())
		case nil:
			// field not set
		default:
			break
		}
		if resp.GetSpec().GetAction() != nil {
			fmt.Printf("Monitor Action:	")
			for _, action := range resp.GetSpec().GetAction().Action {
				fmt.Printf("%s, ", action.String())
			}
			fmt.Printf("\nMirror Session Ids:	")
			for _, mirrorkey := range resp.GetSpec().GetAction().MsKeyHandle {
				fmt.Printf("%d, ", mirrorkey.GetMirrorsessionId())
			}
			fmt.Printf("\nCollector Ids:	")
			for _, collector := range resp.GetSpec().CollectorKeyHandle {
				fmt.Printf("%d, ", collector.GetCollectorId())
			}
		} else {
			fmt.Printf("No Action")
		}
		fmt.Println()
	}

}

func mirrorShowCmdHandler(cmd *cobra.Command, args []string) {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewTelemetryClient(c)

	defer c.Close()

	var req *halproto.MirrorSessionGetRequest
	if cmd != nil && cmd.Flags().Changed("mirror-session-id") {
		req = &halproto.MirrorSessionGetRequest{
			KeyOrHandle: &halproto.MirrorSessionKeyHandle{
				KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
					MirrorsessionId: mirrorID,
				},
			},
		}
	} else {
		req = &halproto.MirrorSessionGetRequest{}
	}

	mirrorReqMsg := &halproto.MirrorSessionGetRequestMsg{
		Request: []*halproto.MirrorSessionGetRequest{req},
	}

	respMsg, err := client.MirrorSessionGet(context.Background(),
		mirrorReqMsg)
	if err != nil {
		fmt.Printf("Mirror Session Get failed. %v\n", err)
		return
	}

	if len(respMsg.Response) == 0 {
		if cmd != nil && cmd.Flags().Changed("mirror-session-id") {
			fmt.Printf("Mirror Session ID %d not found.\n", mirrorID)
		}
		return
	}

	if cmd != nil && cmd.Flags().Changed("yaml") {
		mirrorDetailShow(respMsg)
	} else {
		mirrorShowHeader()
		m := make(map[uint64]*halproto.MirrorSessionGetResponse)
		for _, resp := range respMsg.Response {
			if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
				continue
			}
			m[resp.GetSpec().GetKeyOrHandle().GetMirrorsessionId()] = resp
		}
		var keys []uint64
		for k := range m {
			keys = append(keys, k)
		}
		sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
		for _, k := range keys {
			mirrorShowOneResp(m[k])
		}
	}
}

func mirrorDetailShow(respMsg *halproto.MirrorSessionGetResponseMsg) {
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b) + "\n")
		fmt.Println("---")
	}
}

func mirrorShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("SID:   SW session id             HWID:   HW Table entry id\n")
	fmt.Printf("Snap:  Snap length               LTEP:   Local Tunnel TEP\n")
	fmt.Printf("RTEP:  Remote Tunnel TEP         ETYPE:  ERSPAN Type\n")
	fmt.Printf("Flags: Vlan-Strip-Enable(VS)     SpanId: ERSpan session id\n")
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-5s%-5s%-7s%-5s%-17s%-17s%-7s%-15s\n",
		"SID", "HWID", "SpanID", "Snap", "LTEP", "RTEP", "ETYPE", "Flags")
	fmt.Println(hdrLine)
}

func mirrorShowOneResp(resp *halproto.MirrorSessionGetResponse) {
	spec := resp.GetSpec()
	fmt.Printf("%-5d%-5d%-7d%-5d%-17s%-17s%-7s%-15s\n",
		spec.GetKeyOrHandle().GetMirrorsessionId(),
		resp.GetStatus().GetHandle(),
		spec.GetErspanSpec().GetSpanId(),
		spec.GetSnaplen(),
		utils.IPAddrToStr(spec.GetErspanSpec().GetSrcIp()),
		utils.IPAddrToStr(spec.GetErspanSpec().GetDestIp()),
		strings.Replace(spec.GetErspanSpec().GetType().String(), "ERSPAN_", "", -1),
		mirrorFlagsToStr(resp))
}

func mirrorFlagsToStr(resp *halproto.MirrorSessionGetResponse) string {
	var str string

	if resp.GetSpec().GetErspanSpec().GetVlanStripEn() {
		str += "VS,"
	}

	sz := len(str)
	if sz > 0 && str[sz-1] == ',' {
		str = str[:sz-1]
	} else {
		str += "-"
	}

	return str
}

func collectorShowCmdHandler(cmd *cobra.Command, args []string) {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewTelemetryClient(c)

	defer c.Close()

	var req *halproto.CollectorGetRequest
	if cmd != nil && cmd.Flags().Changed("collector-id") {
		req = &halproto.CollectorGetRequest{
			KeyOrHandle: &halproto.CollectorKeyHandle{
				KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
					CollectorId: collectorID,
				},
			},
		}
	} else {
		req = &halproto.CollectorGetRequest{}
	}

	collectorReqMsg := &halproto.CollectorGetRequestMsg{
		Request: []*halproto.CollectorGetRequest{req},
	}

	respMsg, err := client.CollectorGet(context.Background(),
		collectorReqMsg)
	if err != nil {
		fmt.Printf("Collector StatsGet failed. %v\n", err)
		return
	}

	var ipproto string
	var expformat string
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		spec := resp.GetSpec()
		fmt.Println("\nCollector id: 			", spec.GetKeyOrHandle().GetCollectorId())
		fmt.Println("Collector Vrf:			", spec.GetVrfKeyHandle().GetVrfId())
		fmt.Println("Collector Encap Type:		", spec.GetEncap().GetEncapType())
		fmt.Println("Collector Encap Value:             ", spec.GetEncap().GetEncapValue())
		fmt.Println("Collector Destination IP:		", utils.IPAddrToStr(spec.GetDestIp()))
		fmt.Println("Source IP in the Packet:		", utils.IPAddrToStr(spec.GetSrcIp()))
		if halproto.IPProtocol(spec.GetProtocol()) == halproto.IPProtocol_IPPROTO_TCP {
			ipproto = "TCP"
		} else if halproto.IPProtocol(spec.GetProtocol()) == halproto.IPProtocol_IPPROTO_UDP {
			ipproto = "UDP"
		} else if halproto.IPProtocol(spec.GetProtocol()) == halproto.IPProtocol_IPPROTO_ICMP {
			ipproto = "ICMP"
		} else {
			ipproto = "OTHER"
		}

		fmt.Println("L4 Protocol to collector		", ipproto)
		fmt.Println("Collector Destination L4 Port	", spec.GetDestPort())
		if halproto.ExportFormat(spec.GetFormat()) == halproto.ExportFormat_IPFIX {
			expformat = "IPFIX"
		} else if halproto.ExportFormat(spec.GetFormat()) == halproto.ExportFormat_NETFLOWV9 {
			expformat = "NETFLOWV9"
		} else {
			expformat = "UNKNOWN"
		}
		fmt.Println("Collector Export Format		", expformat)
		fmt.Println("Collector Template ID		", spec.GetTemplateId())
		fmt.Println("Collector Export Interval		", spec.GetExportInterval())
		fmt.Println("Collector Stats:")
		fmt.Println("Number of Packets Exported:	", resp.GetStats().GetNumExportedPackets())
		fmt.Println("Number of Bytes Exported:		", resp.GetStats().GetNumExportedBytes())
		fmt.Println("Number of IPv4 Records Exported:	", resp.GetStats().GetNumExportedRecordsIpv4())
		fmt.Println("Number of IPv6 Records Exported:	", resp.GetStats().GetNumExportedRecordsIpv6())
		fmt.Println("Number of Non-IP Records Exported:	", resp.GetStats().GetNumExportedRecordsNonip())
	}
}
