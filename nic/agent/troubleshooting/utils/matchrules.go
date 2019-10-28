package utils

import (
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
)

// ErrInvalidFlowMonitorRule error code is returned when flow monitor rule is invalid
var ErrInvalidFlowMonitorRule = errors.New("Flow monitor rule is incorrect")

// ValidateMatchRules validates matchrules
// If all rules in the spec pass sanity check, then return true.
// sanity check include correctness of IPaddr string, mac addr string,
// application selectors
func ValidateMatchRules(objMeta api.ObjectMeta, matchRules []tsproto.MatchRule, findEndpoint func(api.ObjectMeta) (*netproto.Endpoint, error)) error {
	for _, rule := range matchRules {
		srcSelectors := rule.Src
		destSelectors := rule.Dst
		appSelectors := rule.AppProtoSel
		if srcSelectors != nil {
			if len(srcSelectors.Endpoints) > 0 {
				for _, ep := range srcSelectors.Endpoints {
					epMeta := api.ObjectMeta{
						Tenant:    objMeta.Tenant,
						Namespace: objMeta.Namespace,
						Name:      ep,
					}
					if _, err := findEndpoint(epMeta); err != nil {
						return fmt.Errorf("Src Endpoint %v not found, error: %s", ep, err)
					}
				}
			} else if len(srcSelectors.IPAddresses) > 0 {
				for _, ipAddr := range srcSelectors.IPAddresses {
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
			} else if len(srcSelectors.MACAddresses) > 0 {
				for _, macAddr := range srcSelectors.MACAddresses {
					_, err := net.ParseMAC(macAddr)
					if err != nil {
						return fmt.Errorf("invalid mac address %v", macAddr)
					}
				}
			}
		}
		if destSelectors != nil {
			if len(destSelectors.Endpoints) > 0 {
				for _, ep := range destSelectors.Endpoints {
					epMeta := api.ObjectMeta{
						Tenant:    objMeta.Tenant,
						Namespace: objMeta.Namespace,
						Name:      ep,
					}
					if _, err := findEndpoint(epMeta); err != nil {
						return fmt.Errorf("Src Endpoint %v not found", ep)
					}

				}
			} else if len(destSelectors.IPAddresses) > 0 {
				for _, ipAddr := range destSelectors.IPAddresses {
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
			} else if len(destSelectors.MACAddresses) > 0 {
				for _, macAddr := range destSelectors.MACAddresses {
					_, err := net.ParseMAC(macAddr)
					if err != nil {
						return fmt.Errorf("invalid mac address %v", macAddr)
					}
				}
			}
		}

		if appSelectors != nil {
			if len(appSelectors.Ports) > 0 {
				// Ports specified by controller will be in the form
				// "tcp/5000"
				// When Protocol is invalid or when port# is not specified
				// fail sanity check.
				for _, protoPort := range appSelectors.Ports {
					if !strings.Contains(protoPort, "any") && GetProtocol(protoPort) == -1 {
						return fmt.Errorf("invalid protocol in %v", protoPort)
					}
					if !strings.Contains(protoPort, "any") && GetPort(protoPort) == -1 {
						return fmt.Errorf("invalid port in %v", protoPort)
					}
				}
			} else if len(appSelectors.Apps) > 0 {
				//TODO: Handle Application selection later. "Ex: Redis"
			}
		}
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
func GetProtocol(portString string) int32 {
	strs := strings.Split(portString, "/")
	if len(strs) > 1 {
		protoStr := "IPPROTO_" + strings.ToUpper(strs[0])
		return halproto.IPProtocol_value[protoStr]
	}
	return -1
}

// GetPort extracts port from string "TCP/123"
func GetPort(portString string) int32 {
	strs := strings.Split(portString, "/")
	if len(strs) > 1 {
		v, err := strconv.Atoi(strs[1])
		if err == nil {
			return int32(v)
		}
	}
	return -1
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
func ExpandCompositeMatchRule(objMeta api.ObjectMeta, rule *tsproto.MatchRule, findEndpoint func(api.ObjectMeta) (*netproto.Endpoint, error)) ([]*types.IPAddrDetails, []*types.IPAddrDetails, []uint64, []uint64, []*types.AppPortDetails, []string, []string) {
	srcSelectors := rule.Src
	destSelectors := rule.Dst
	appSelectors := rule.AppProtoSel
	var srcIPs []*types.IPAddrDetails
	var srcIPStrings []string
	var srcMACs []uint64
	var destIPs []*types.IPAddrDetails
	var destIPStrings []string
	var destMACs []uint64
	var appPorts []*types.AppPortDetails
	if srcSelectors != nil {
		if len(srcSelectors.Endpoints) > 0 {
			for _, ep := range srcSelectors.Endpoints {
				epMeta := api.ObjectMeta{
					Tenant:    objMeta.Tenant,
					Namespace: objMeta.Namespace,
					Name:      ep,
				}
				epObj, err := findEndpoint(epMeta)
				if err == nil {
					for _, address := range epObj.Spec.IPv4Addresses {
						if address != "" {
							srcIPs = append(srcIPs, BuildIPAddrDetails(address))
							srcIPStrings = append(srcIPStrings, address)
						}
					}

					for _, address := range epObj.Spec.IPv6Addresses {
						if address != "" {
							srcIPs = append(srcIPs, BuildIPAddrDetails(address))
							srcIPStrings = append(srcIPStrings, address)
						}
					}
				}
			}
		} else if len(srcSelectors.IPAddresses) > 0 {
			for _, ipaddr := range srcSelectors.IPAddresses {
				srcIPs = append(srcIPs, BuildIPAddrDetails(ipaddr))
				srcIPStrings = append(srcIPStrings, ipaddr)
			}
		} else if len(srcSelectors.MACAddresses) > 0 {
			for _, macAddr := range srcSelectors.MACAddresses {
				hwMac, _ := net.ParseMAC(macAddr)
				// move mac address to 8 byte array
				srcMACs = append(srcMACs, binary.BigEndian.Uint64(append([]byte{0, 0}, hwMac...)))
			}
		}
	} else {
		srcIPs = append(srcIPs, BuildIPAddrDetails("0.0.0.0/0"))
		srcIPStrings = append(srcIPStrings, "0.0.0.0/0")
	}
	if destSelectors != nil {
		if len(destSelectors.Endpoints) > 0 {
			for _, ep := range destSelectors.Endpoints {
				epMeta := api.ObjectMeta{
					Tenant:    objMeta.Tenant,
					Namespace: objMeta.Namespace,
					Name:      ep,
				}
				epObj, err := findEndpoint(epMeta)
				if err == nil {
					for _, address := range epObj.Spec.IPv4Addresses {
						if address != "" {
							destIPs = append(destIPs, BuildIPAddrDetails(address))
							destIPStrings = append(destIPStrings, address)
						}
					}

					for _, address := range epObj.Spec.IPv6Addresses {
						if address != "" {
							destIPs = append(destIPs, BuildIPAddrDetails(address))
							destIPStrings = append(destIPStrings, address)
						}
					}
				}
			}
		} else if len(destSelectors.IPAddresses) > 0 {
			for _, ipaddr := range destSelectors.IPAddresses {
				destIPs = append(destIPs, BuildIPAddrDetails(ipaddr))
				destIPStrings = append(destIPStrings, ipaddr)
			}
		} else if len(destSelectors.MACAddresses) > 0 {
			for _, macAddr := range destSelectors.MACAddresses {
				hwMac, _ := net.ParseMAC(macAddr)
				// move mac address to 8 byte array
				destMACs = append(destMACs, binary.BigEndian.Uint64(append([]byte{0, 0}, hwMac...)))
			}
		}
	} else {
		destIPs = append(destIPs, BuildIPAddrDetails("0.0.0.0/0"))
		destIPStrings = append(destIPStrings, "0.0.0.0/0")
	}
	if appSelectors != nil {
		if len(appSelectors.Ports) > 0 {
			// Ports specified by controller will be in the form
			// "tcp/5000"
			for _, protoPort := range appSelectors.Ports {
				protoAny := false
				portAny := false
				protoType := int32(0)
				portNum := int32(0)
				strs := strings.Split(protoPort, "/")
				if !strings.Contains(strs[0], "any") {
					protoType = GetProtocol(protoPort)
				} else {
					protoAny = true
				}
				if len(strs) > 1 && !strings.Contains(strs[1], "any") {
					portNum = GetPort(protoPort)
				} else {
					portAny = true
				}
				appPort := &types.AppPortDetails{}
				if !protoAny {
					appPort.Ipproto = protoType
				}
				if !portAny {
					appPort.L4port = portNum
				}
				appPorts = append(appPorts, appPort)
			}
		} else if len(appSelectors.Apps) > 0 {
			//TODO: Handle Application selection later. "Ex: Redis"
		}
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
