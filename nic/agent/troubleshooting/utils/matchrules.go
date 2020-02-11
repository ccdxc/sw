package utils

import (
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/utils/log"
)

// ErrInvalidFlowMonitorRule error code is returned when flow monitor rule is invalid
var ErrInvalidFlowMonitorRule = errors.New("Flow monitor rule is incorrect")

// ValidateMatchRules validates matchrules
// If all rules in the spec pass sanity check, then return true.
// sanity check include correctness of IPaddr string, mac addr string,
// application selectors
func ValidateMatchRules(objMeta api.ObjectMeta, matchRules []netproto.MatchRule, findEndpoint func(api.ObjectMeta) (*netproto.Endpoint, error)) error {
	for _, rule := range matchRules {
		srcSelectors := rule.Src
		destSelectors := rule.Dst
		//appSelectors := rule.AppProtoSel
		if srcSelectors != nil {
			//if len(srcSelectors.Endpoints) > 0 {
			//	for _, ep := range srcSelectors.Endpoints {
			//		epMeta := api.ObjectMeta{
			//			Tenant:    objMeta.Tenant,
			//			Namespace: objMeta.Namespace,
			//			Name:      ep,
			//		}
			//		if _, err := findEndpoint(epMeta); err != nil {
			//			return fmt.Errorf("Src Endpoint %v not found, error: %s", ep, err)
			//		}
			//	}
			//} else
			if len(srcSelectors.Addresses) > 0 {
				for _, ipAddr := range srcSelectors.Addresses {
					if ipAddr == "any" {
						continue
					}
					_, isRange, isSubnet := getIPAddrDetails(ipAddr)
					if !isRange {
						if !isSubnet {
							if net.ParseIP(ipAddr) == nil {
								return fmt.Errorf("invalid ip address %v", ipAddr)
							}
						} else {
							_, _, err := net.ParseCIDR(ipAddr)
							if err != nil {
								return fmt.Errorf("invalid ip address %v", ipAddr)
							}
						}
					} else {
						// TODO: Handle IPaddr range
					}
				}
			}
			//else if len(srcSelectors.MACAddresses) > 0 {
			//	for _, macAddr := range srcSelectors.MACAddresses {
			//		_, err := net.ParseMAC(macAddr)
			//		if err != nil {
			//			return fmt.Errorf("invalid mac address %v", macAddr)
			//		}
			//	}
			//}
		}
		if destSelectors != nil {
			//if len(destSelectors.Endpoints) > 0 {
			//	for _, ep := range destSelectors.Endpoints {
			//		epMeta := api.ObjectMeta{
			//			Tenant:    objMeta.Tenant,
			//			Namespace: objMeta.Namespace,
			//			Name:      ep,
			//		}
			//		if _, err := findEndpoint(epMeta); err != nil {
			//			return fmt.Errorf("Src Endpoint %v not found", ep)
			//		}
			//
			//	}
			//} else
			if len(destSelectors.Addresses) > 0 {
				for _, ipAddr := range destSelectors.Addresses {
					if ipAddr == "any" {
						continue
					}
					_, isRange, isSubnet := getIPAddrDetails(ipAddr)
					if !isRange {
						if !isSubnet {
							if net.ParseIP(ipAddr) == nil {
								return fmt.Errorf("invalid ip address %v", ipAddr)
							}
						} else {
							_, _, err := net.ParseCIDR(ipAddr)
							if err != nil {
								return fmt.Errorf("invalid ip address %v", ipAddr)
							}
						}
					} else {
						// TODO: Handle IPaddr range
					}
				}
			}
			//else if len(destSelectors.MACAddresses) > 0 {
			//	for _, macAddr := range destSelectors.MACAddresses {
			//		_, err := net.ParseMAC(macAddr)
			//		if err != nil {
			//			return fmt.Errorf("invalid mac address %v", macAddr)
			//		}
			//	}
			//}
		}

		//if appSelectors != nil {
		//	if len(appSelectors.Ports) > 0 {
		//		// Ports specified by controller will be in the form
		//		// "tcp/5000"
		//		// When Protocol is invalid or when port# is not specified
		//		// fail sanity check.
		//		for _, protoPort := range appSelectors.Ports {
		//			if !strings.Contains(protoPort, "any") && GetProtocol(protoPort) == -1 {
		//				return fmt.Errorf("invalid protocol in %v", protoPort)
		//			}
		//			if !strings.Contains(protoPort, "any") && GetPort(protoPort) == -1 {
		//				return fmt.Errorf("invalid port in %v", protoPort)
		//			}
		//		}
		//	} else if len(appSelectors.Apps) > 0 {
		//		//TODO: Handle Application selection later. "Ex: Redis"
		//	}
		//}
	}
	// TODO need to check if both srcIP and destIP are of same type (either both v4 or both v6)
	return nil
}

//returns ipaddr type, is Range (only last byte is allowed to have range), is ip prefix
func getIPAddrDetails(ipAddr string) (bool, bool, bool) {
	return isIpv4(ipAddr), isRangeAddr(ipAddr), isSubnetAddr(ipAddr)
}

// BuildIPAddrDetails converts ip address to ip/subnet
func BuildIPAddrDetails(ipaddr string) *types.IPAddrDetails {
	var ip net.IP

	if ipaddr == "any" {
		ipaddr = "0.0.0.0/0"
	}
	isIpv4, isRange, isSubnet := getIPAddrDetails(ipaddr)
	ipAddr := &types.IPAddrDetails{}
	if !isRange {
		if !isSubnet {
			if isIpv4 {
				ip = net.ParseIP(ipaddr).To4()
			} else {
				ip = net.ParseIP(ipaddr)
			}
			ipAddr = &types.IPAddrDetails{
				IP:       ip,
				IsIpv4:   isIpv4,
				IsSubnet: isSubnet,
				//prefixLen: 0
			}
		} else {
			ip, _, _ = net.ParseCIDR(ipaddr)
			if isIpv4 {
				ip = ip.To4()
			}
			ipAddr = &types.IPAddrDetails{
				IP:        ip,
				IsIpv4:    isIpv4,
				IsSubnet:  isSubnet,
				PrefixLen: uint32(getPrefixLen(ipaddr)),
			}
		}
	} else {
		// TODO Handle address range.
	}
	return ipAddr
}

// GetProtocol extracts IPProtocol value corresponding to "TCP" from string "TCP/123"
func GetProtocol(protocol string) int32 {
	switch strings.ToLower(protocol) {
	case "tcp":
		return int32(halproto.IPProtocol_IPPROTO_TCP)
	case "udp":
		return int32(halproto.IPProtocol_IPPROTO_UDP)
	case "icmp":
		return int32(halproto.IPProtocol_IPPROTO_ICMP)
	default:
		return int32(halproto.IPProtocol_IPPROTO_NONE)
	}
	//strs := strings.Split(portString, "/")
	//if len(strs) > 1 {
	//	protoStr := "IPPROTO_" + strings.ToUpper(strs[0])
	//	return halproto.IPProtocol_value[protoStr]
	//}
	//return -1
}

// GetPortList extracts port from string "TCP/123,124,135"
func GetPortList(portRange string) ([]int32, error) {
	portList := []int32{}

	// no ports ?
	if len(portRange) == 0 {
		return []int32{0}, nil
	}

	for _, ports := range strings.Split(portRange, ",") {
		values := strings.Split(ports, "-")
		start, err := strconv.Atoi(values[0])
		if err != nil {
			return nil, err
		}

		if len(values) == 1 {
			portList = append(portList, int32(start))

		} else {
			end, err := strconv.Atoi(values[1])
			if err != nil {
				return nil, err
			}

			for i := start; i <= end; i++ {
				portList = append(portList, int32(i))
			}
		}
	}

	return portList, nil
}

func isIpv4(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '.':
			return true
		case ':':
			return false
		}
	}
	return false
}
func isRangeAddr(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '-':
			return true
		}
	}
	return false
}
func isSubnetAddr(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '/':
			return true
		}
	}
	return false
}
func getPrefixLen(ip string) int32 {
	strs := strings.Split(ip, "/")
	if len(strs) > 1 {
		v, err := strconv.Atoi(strs[1])
		if err == nil {
			return int32(v)
		}
	}
	return -1
}

// ExpandCompositeMatchRule Process trouble shooting/flow export MatchRule (Which can be composite)
// and prepare list of first class match items.
//      List of source IPAddr or source MacAddrs
//      List of Destination IPAddr or Destination  MacAddrs
//      List of Application Selectors
// The caller of the function is expected to create cross product of
// these 3 lists and use each tuple (src, dest, app) as an atomic rule
// that can be sent to HAL
func ExpandCompositeMatchRule(objMeta api.ObjectMeta, rule *netproto.MatchRule, findEndpoint func(api.ObjectMeta) (*netproto.Endpoint, error)) ([]*types.IPAddrDetails, []*types.IPAddrDetails, []uint64, []uint64, []*types.AppPortDetails, []string, []string) {
	srcSelectors := rule.Src
	destSelectors := rule.Dst
	//appSelectors := rule.AppProtoSel
	var srcIPs []*types.IPAddrDetails
	var srcIPStrings []string
	var srcMACs []uint64
	var destIPs []*types.IPAddrDetails
	var destIPStrings []string
	var destMACs []uint64
	var appPorts []*types.AppPortDetails
	if srcSelectors != nil {
		//if len(srcSelectors.Endpoints) > 0 {
		//	for _, ep := range srcSelectors.Endpoints {
		//		epMeta := api.ObjectMeta{
		//			Tenant:    objMeta.Tenant,
		//			Namespace: objMeta.Namespace,
		//			Name:      ep,
		//		}
		//		epObj, err := findEndpoint(epMeta)
		//		if err == nil {
		//			for _, address := range epObj.Spec.IPv4Addresses {
		//				if address != "" {
		//					srcIPs = append(srcIPs, BuildIPAddrDetails(address))
		//					srcIPStrings = append(srcIPStrings, address)
		//				}
		//			}
		//
		//			for _, address := range epObj.Spec.IPv6Addresses {
		//				if address != "" {
		//					srcIPs = append(srcIPs, BuildIPAddrDetails(address))
		//					srcIPStrings = append(srcIPStrings, address)
		//				}
		//			}
		//		}
		//	}
		//} else
		if len(srcSelectors.Addresses) > 0 {
			for _, ipaddr := range srcSelectors.Addresses {
				srcIPs = append(srcIPs, BuildIPAddrDetails(ipaddr))
				srcIPStrings = append(srcIPStrings, ipaddr)
			}
		} else {
			srcIPs = append(srcIPs, BuildIPAddrDetails("0.0.0.0/0"))
			srcIPStrings = append(srcIPStrings, "0.0.0.0/0")
		}
		//else if len(srcSelectors.MACAddresses) > 0 {
		//	for _, macAddr := range srcSelectors.MACAddresses {
		//		hwMac, _ := net.ParseMAC(macAddr)
		//		// move mac address to 8 byte array
		//		srcMACs = append(srcMACs, binary.BigEndian.Uint64(append([]byte{0, 0}, hwMac...)))
		//	}
		//}
	} else {
		srcIPs = append(srcIPs, BuildIPAddrDetails("0.0.0.0/0"))
		srcIPStrings = append(srcIPStrings, "0.0.0.0/0")
	}
	if destSelectors != nil {
		//if len(destSelectors.Endpoints) > 0 {
		//	for _, ep := range destSelectors.Endpoints {
		//		epMeta := api.ObjectMeta{
		//			Tenant:    objMeta.Tenant,
		//			Namespace: objMeta.Namespace,
		//			Name:      ep,
		//		}
		//		epObj, err := findEndpoint(epMeta)
		//		if err == nil {
		//			for _, address := range epObj.Spec.IPv4Addresses {
		//				if address != "" {
		//					destIPs = append(destIPs, BuildIPAddrDetails(address))
		//					destIPStrings = append(destIPStrings, address)
		//				}
		//			}
		//
		//			for _, address := range epObj.Spec.IPv6Addresses {
		//				if address != "" {
		//					destIPs = append(destIPs, BuildIPAddrDetails(address))
		//					destIPStrings = append(destIPStrings, address)
		//				}
		//			}
		//		}
		//	}
		//} else
		if len(destSelectors.Addresses) > 0 {
			for _, ipaddr := range destSelectors.Addresses {
				destIPs = append(destIPs, BuildIPAddrDetails(ipaddr))
				destIPStrings = append(destIPStrings, ipaddr)
			}
		} else {
			destIPs = append(destIPs, BuildIPAddrDetails("0.0.0.0/0"))
			destIPStrings = append(destIPStrings, "0.0.0.0/0")
		}
		//else if len(destSelectors.MACAddresses) > 0 {
		//	for _, macAddr := range destSelectors.MACAddresses {
		//		hwMac, _ := net.ParseMAC(macAddr)
		//		// move mac address to 8 byte array
		//		destMACs = append(destMACs, binary.BigEndian.Uint64(append([]byte{0, 0}, hwMac...)))
		//	}
		//}
	} else {
		destIPs = append(destIPs, BuildIPAddrDetails("0.0.0.0/0"))
		destIPStrings = append(destIPStrings, "0.0.0.0/0")
	}

	if destSelectors != nil {
		if len(destSelectors.ProtoPorts) > 0 {
			// Ports specified by controller will be in the form
			// "tcp/5000"
			for _, protoPort := range destSelectors.ProtoPorts {
				var protocol int32
				if !strings.Contains(protoPort.Protocol, "any") {
					protocol = GetProtocol(protoPort.Protocol)
				}

				if !strings.Contains(protoPort.Port, "any") {
					pn, err := GetPortList(protoPort.Port)
					if err != nil {
						log.Errorf("failed to parse port %v, %v", protoPort.Port, err)
						continue
					}

					for _, p := range pn {
						appPorts = append(appPorts, &types.AppPortDetails{
							Ipproto: protocol,
							L4port:  p,
						})
					}
				} else {
					appPorts = append(appPorts, &types.AppPortDetails{
						Ipproto: protocol,
					})
				}
				log.Infof("added port %+v", appPorts)
			}
		} else {
			appPort := &types.AppPortDetails{}
			appPorts = append(appPorts, appPort)
		}
		//else if len(appSelectors.Apps) > 0 {
		//	//TODO: Handle Application selection later. "Ex: Redis"
		//}
	} else {
		appPort := &types.AppPortDetails{}
		appPorts = append(appPorts, appPort)
	}
	return srcIPs, destIPs, srcMACs, destMACs, appPorts, srcIPStrings, destIPStrings
}

// CreateIPAddrCrossProductRuleList create rule combination
func CreateIPAddrCrossProductRuleList(srcIPs, destIPs []*types.IPAddrDetails, appPorts []*types.AppPortDetails, srcIPStrings []string, destIPStrings []string) ([]*types.FlowMonitorIPRuleDetails, error) {
	if len(srcIPs) == 0 && len(destIPs) == 0 {
		return nil, ErrInvalidFlowMonitorRule
	}
	var flowMonitorRules []*types.FlowMonitorIPRuleDetails

	log.Infof("SrcIPs: %+v", srcIPs)
	log.Infof("DstIPs: %+v", destIPs)
	log.Infof("AppPorts: %+v", appPorts)

	for i := 0; i < len(srcIPs); i++ {
		for j := 0; j < len(destIPs); j++ {
			for k := 0; k < len(appPorts); k++ {
				iprule := &types.FlowMonitorIPRuleDetails{
					SrcIPObj:     srcIPs[i],
					SrcIPString:  srcIPStrings[i],
					DestIPObj:    destIPs[j],
					DestIPString: destIPStrings[j],
					AppPortObj:   appPorts[k],
				}
				flowMonitorRules = append(flowMonitorRules, iprule)
			}
		}
	}
	return flowMonitorRules, nil
}

// CreateMACAddrCrossProductRuleList create rule combination
func CreateMACAddrCrossProductRuleList(srcMACs, destMACs []uint64, appPorts []*types.AppPortDetails) ([]*types.FlowMonitorMACRuleDetails, error) {

	if len(srcMACs) == 0 && len(destMACs) == 0 {
		return nil, ErrInvalidFlowMonitorRule
	}
	var flowMonitorRules []*types.FlowMonitorMACRuleDetails

	for i := 0; i < len(srcMACs); i++ {
		for j := 0; j < len(destMACs); j++ {
			for k := 0; k < len(appPorts); k++ {
				iprule := &types.FlowMonitorMACRuleDetails{
					SrcMAC:     srcMACs[i],
					DestMAC:    destMACs[j],
					AppPortObj: appPorts[k],
				}
				flowMonitorRules = append(flowMonitorRules, iprule)
			}
		}
	}
	return flowMonitorRules, nil
}

// BuildIPAddrObjProtoObj builds hal object
func BuildIPAddrObjProtoObj(ipaddr *types.IPAddrDetails) *halproto.IPAddressObj {

	addrObj := &halproto.IPAddressObj{}

	if ipaddr.IsIpv4 && !ipaddr.IsSubnet {
		//v4, no-cidr
		addrObj.Formats = &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: &halproto.Address_Range{
					Range: &halproto.AddressRange{
						Range: &halproto.AddressRange_Ipv4Range{
							Ipv4Range: &halproto.IPRange{
								LowIpaddr: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET,
									V4OrV6: &halproto.IPAddress_V4Addr{
										V4Addr: binary.BigEndian.Uint32(ipaddr.IP),
									},
								},
								HighIpaddr: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET,
									V4OrV6: &halproto.IPAddress_V4Addr{
										V4Addr: binary.BigEndian.Uint32(ipaddr.IP),
									},
								},
							},
						},
					},
				},
			},
		}
	} else if ipaddr.IsIpv4 && ipaddr.IsSubnet {
		//v4, cidr
		addrObj.Formats = &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: &halproto.Address_Prefix{
					Prefix: &halproto.IPSubnet{
						Subnet: &halproto.IPSubnet_Ipv4Subnet{
							Ipv4Subnet: &halproto.IPPrefix{
								Address: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET,
									V4OrV6: &halproto.IPAddress_V4Addr{
										V4Addr: binary.BigEndian.Uint32(ipaddr.IP),
									},
								},
								PrefixLen: ipaddr.PrefixLen,
							},
						},
					},
				},
			},
		}
	} else if !ipaddr.IsIpv4 && !ipaddr.IsSubnet {
		//v6, no-cidr
		addrObj.Formats = &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: &halproto.Address_Range{
					Range: &halproto.AddressRange{
						Range: &halproto.AddressRange_Ipv6Range{
							Ipv6Range: &halproto.IPRange{
								LowIpaddr: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET6,
									V4OrV6: &halproto.IPAddress_V6Addr{
										V6Addr: ipaddr.IP,
									},
								},
								HighIpaddr: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET6,
									V4OrV6: &halproto.IPAddress_V6Addr{
										V6Addr: ipaddr.IP,
									},
								},
							},
						},
					},
				},
			},
		}
	} else if !ipaddr.IsIpv4 && ipaddr.IsSubnet {
		//v6, cidr
		addrObj.Formats = &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: &halproto.Address_Prefix{
					Prefix: &halproto.IPSubnet{
						Subnet: &halproto.IPSubnet_Ipv6Subnet{
							Ipv6Subnet: &halproto.IPPrefix{
								Address: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET6,
									V4OrV6: &halproto.IPAddress_V6Addr{
										V6Addr: ipaddr.IP,
									},
								},
								PrefixLen: ipaddr.PrefixLen,
							},
						},
					},
				},
			},
		}
	} else {
		return nil
	}

	return addrObj
}

// BuildAppMatchInfoObj build hal object
func BuildAppMatchInfoObj(appPort *types.AppPortDetails) *halproto.RuleMatch_AppMatch {

	appMatch := &halproto.RuleMatch_AppMatch{}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_TCP) || appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_UDP) {
		l4port := &halproto.L4PortRange{
			PortLow:  uint32(appPort.L4port),
			PortHigh: uint32(appPort.L4port),
		}
		appMatch.App = &halproto.RuleMatch_AppMatch_PortInfo{
			PortInfo: &halproto.RuleMatch_L4PortAppInfo{
				//Controller provided port# and proto is applied on only dest
				// srcPort is not populated.
				DstPortRange: []*halproto.L4PortRange{l4port},
				//SrcPortRange:
			},
		}
	}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_ICMP) {
		appMatch.App = &halproto.RuleMatch_AppMatch_IcmpInfo{
			IcmpInfo: &halproto.RuleMatch_ICMPAppInfo{
				IcmpType: 0, //TODO : Get this value from ctrler
				IcmpCode: 0, //TODO : Get this value from ctrler
			},
		}
	}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_ICMPV6) {
		appMatch.App = &halproto.RuleMatch_AppMatch_Icmpv6Info{
			Icmpv6Info: &halproto.RuleMatch_ICMPv6AppInfo{
				Icmpv6Type: 0, //TODO : Get this value from ctrler
				Icmpv6Code: 0, //TODO : Get this value from ctrler
			},
		}
	}

	return appMatch
}
