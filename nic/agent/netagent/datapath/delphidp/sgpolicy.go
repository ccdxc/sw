// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"
	"sync"

	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

func convertAppProtocol(protocol string) halproto.IPProtocol {
	switch protocol {
	case "tcp":
		return halproto.IPProtocol_IPPROTO_TCP
	case "udp":
		return halproto.IPProtocol_IPPROTO_UDP
	case "icmp":
		return halproto.IPProtocol_IPPROTO_ICMP
	default:
		return halproto.IPProtocol_IPPROTO_NONE
	}

}

// convertIPRange converts a start IP and end IP to hal IPAddressObj
func convertIPRange(startIP, endIP string) (*halproto.IPAddressObj, error) {
	begin := net.ParseIP(strings.TrimSpace(startIP))
	if len(begin) == 0 {
		log.Errorf("could not parse start IP {%v}", startIP)
		return nil, errors.New("delphi datapath could not parse the IP")
	}
	end := net.ParseIP(strings.TrimSpace(endIP))
	if len(end) == 0 {
		log.Errorf("could not parse end IP {%v}", endIP)
		return nil, errors.New("delphi datapath could not parse the IP")
	}

	lowIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(begin),
		},
	}

	highIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(end),
		},
	}

	addrRange := &halproto.Address_Range{
		Range: &halproto.AddressRange{
			Range: &halproto.AddressRange_Ipv4Range{
				Ipv4Range: &halproto.IPRange{
					LowIpaddr:  &lowIP,
					HighIpaddr: &highIP,
				},
			},
		},
	}
	halAddresses := &halproto.IPAddressObj{
		Formats: &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: addrRange,
			},
		},
	}
	return halAddresses, nil
}

func convertIPs(addresses []string) ([]*halproto.IPAddressObj, error) {
	var halAddresses []*halproto.IPAddressObj
	for _, a := range addresses {
		if ip := net.ParseIP(strings.TrimSpace(a)); len(ip) > 0 {
			// try parsing as an octet

			halAddr := &halproto.IPAddressObj{
				Formats: &halproto.IPAddressObj_Address{
					Address: &halproto.Address{
						Address: &halproto.Address_Prefix{
							Prefix: &halproto.IPSubnet{
								Subnet: &halproto.IPSubnet_Ipv4Subnet{
									Ipv4Subnet: &halproto.IPPrefix{
										Address: &halproto.IPAddress{
											IpAf: halproto.IPAddressFamily_IP_AF_INET,
											V4OrV6: &halproto.IPAddress_V4Addr{
												V4Addr: ipv4Touint32(ip),
											},
										},
										PrefixLen: uint32(32),
									},
								},
							},
						},
					},
				},
			}
			halAddresses = append(halAddresses, halAddr)
		} else if ip, network, err := net.ParseCIDR(strings.TrimSpace(a)); err == nil {
			// try parsing as IPMask
			prefixLen, _ := network.Mask.Size()

			halAddr := &halproto.IPAddressObj{
				Formats: &halproto.IPAddressObj_Address{
					Address: &halproto.Address{
						Address: &halproto.Address_Prefix{
							Prefix: &halproto.IPSubnet{
								Subnet: &halproto.IPSubnet_Ipv4Subnet{
									Ipv4Subnet: &halproto.IPPrefix{
										Address: &halproto.IPAddress{
											IpAf: halproto.IPAddressFamily_IP_AF_INET,
											V4OrV6: &halproto.IPAddress_V4Addr{
												V4Addr: ipv4Touint32(ip),
											},
										},
										PrefixLen: uint32(prefixLen),
									},
								},
							},
						},
					},
				},
			}
			halAddresses = append(halAddresses, halAddr)

		} else if ipRange := strings.Split(strings.TrimSpace(a), "-"); len(ipRange) == 2 {
			// try parsing as hyphen separated range
			halAddr, err := convertIPRange(ipRange[0], ipRange[1])
			if err != nil {
				log.Errorf("failed to parse IP Range {%v}. Err: %v", ipRange, err)
				return nil, err
			}
			halAddresses = append(halAddresses, halAddr)
		} else if a == "any" {
			// Interpret it as 0.0.0.0/0
			halAddr := &halproto.IPAddressObj{
				Formats: &halproto.IPAddressObj_Address{
					Address: &halproto.Address{
						Address: &halproto.Address_Prefix{
							Prefix: &halproto.IPSubnet{
								Subnet: &halproto.IPSubnet_Ipv4Subnet{
									Ipv4Subnet: &halproto.IPPrefix{
										Address: &halproto.IPAddress{
											IpAf:   halproto.IPAddressFamily_IP_AF_INET,
											V4OrV6: &halproto.IPAddress_V4Addr{},
										},
									},
								},
							},
						},
					},
				},
			}
			halAddresses = append(halAddresses, halAddr)
		} else {
			// give up
			return nil, fmt.Errorf("invalid IP Address format {%v}. Should either be in an octet, CIDR or hyphen separated IP Range", a)
		}

	}
	return halAddresses, nil
}

func convertPort(port string) (*halproto.L4PortRange, error) {
	portSlice := strings.Split(port, "-")
	switch len(portSlice) {
	// single port
	case 1:
		port, err := strconv.Atoi(port)
		if err != nil || port <= 0 || port > 65535 {
			log.Errorf("invalid port format. %v", port)
			return nil, fmt.Errorf("invalid port format. %v", port)
		}
		halPort := &halproto.L4PortRange{
			PortLow:  uint32(port),
			PortHigh: uint32(port),
		}
		return halPort, nil
	// port range
	case 2:
		startPort, err := strconv.Atoi(portSlice[0])
		if err != nil || startPort <= 0 || startPort > 65535 {
			log.Errorf("invalid port format. %v", port)
			return nil, fmt.Errorf("invalid port format. %v", port)
		}
		endPort, err := strconv.Atoi(portSlice[1])
		if err != nil || endPort <= 0 || endPort > 65535 {
			log.Errorf("invalid port format. %v", port)
			return nil, fmt.Errorf("invalid port format. %v", port)
		}

		halPort := &halproto.L4PortRange{
			PortLow:  uint32(startPort),
			PortHigh: uint32(endPort),
		}
		return halPort, nil
	default:
		log.Errorf("invalid port format. %v. It should either be hyphen separated or a single port", port)
		return nil, fmt.Errorf("invalid port format. %v. It should either be hyphen separated or a single port", port)
	}
}

func convertMatchCriteria(src, dst *netproto.MatchSelector) ([]*halproto.RuleMatch, error) {
	var srcIPRanges, dstIPRanges []*halproto.IPAddressObj
	var srcPortRanges, dstPortRanges []*halproto.L4PortRange
	var srcProtocols, dstProtocols []halproto.IPProtocol
	var ruleMatches []*halproto.RuleMatch
	var err error

	// build src match attributes
	if src != nil {
		srcIPRanges, err = convertIPs(src.Addresses)
		if err != nil {
			log.Errorf("Could not convert match criteria from Src: {%v}. Err: %v", src, err)
			return nil, err
		}
		for _, s := range src.ProtoPorts {
			sPort, err := convertPort(s.Port)
			if err != nil {
				log.Errorf("Could not convert port match criteria from: {%v} . Err: %v", s, err)
				return nil, err
			}
			srcPortRanges = append(srcPortRanges, sPort)
			srcProtocols = append(srcProtocols, convertAppProtocol(s.Protocol))
		}
	}

	// build dst match attributes
	if dst != nil {
		dstIPRanges, err = convertIPs(dst.Addresses)
		if err != nil {
			log.Errorf("Could not convert match criteria from Dst: {%v}. Err: %v", dst, err)
			return nil, err
		}
		for _, d := range dst.ProtoPorts {
			dPort, err := convertPort(d.Port)
			if err != nil {
				log.Errorf("Could not convert port match criteria from: {%v} . Err: %v", d, err)
				return nil, err
			}
			dstPortRanges = append(dstPortRanges, dPort)
			dstProtocols = append(dstProtocols, convertAppProtocol(d.Protocol))
		}
	}

	// flatten if needed
	switch {
	case len(srcPortRanges) == 0 && len(dstPortRanges) == 0:
		var ruleMatch halproto.RuleMatch
		ruleMatch.SrcAddress = srcIPRanges
		ruleMatch.DstAddress = dstIPRanges
		ruleMatches = append(ruleMatches, &ruleMatch)
		return ruleMatches, nil
	case len(srcPortRanges) == 0:
		for i, p := range dstPortRanges {
			var ruleMatch halproto.RuleMatch
			ruleMatch.SrcAddress = srcIPRanges
			ruleMatch.DstAddress = dstIPRanges
			appMatch := halproto.RuleMatch_AppMatch{
				App: &halproto.RuleMatch_AppMatch_PortInfo{
					PortInfo: &halproto.RuleMatch_L4PortAppInfo{
						DstPortRange: []*halproto.L4PortRange{
							p,
						},
					},
				},
			}
			ruleMatch.AppMatch = &appMatch
			ruleMatch.Protocol = dstProtocols[i]
			ruleMatches = append(ruleMatches, &ruleMatch)
		}
		return ruleMatches, nil
	case len(dstPortRanges) == 0:
		for i, p := range srcPortRanges {
			var ruleMatch halproto.RuleMatch
			ruleMatch.SrcAddress = srcIPRanges
			ruleMatch.DstAddress = dstIPRanges
			appMatch := halproto.RuleMatch_AppMatch{
				App: &halproto.RuleMatch_AppMatch_PortInfo{
					PortInfo: &halproto.RuleMatch_L4PortAppInfo{
						SrcPortRange: []*halproto.L4PortRange{
							p,
						},
					},
				},
			}
			ruleMatch.AppMatch = &appMatch
			ruleMatch.Protocol = srcProtocols[i]
			ruleMatches = append(ruleMatches, &ruleMatch)
		}
		return ruleMatches, nil
	case len(srcPortRanges) > 0 && len(dstPortRanges) > 0:
		for _, sPort := range srcPortRanges {
			for i, dPort := range dstPortRanges {
				var ruleMatch halproto.RuleMatch
				ruleMatch.SrcAddress = srcIPRanges
				ruleMatch.DstAddress = dstIPRanges
				appMatch := halproto.RuleMatch_AppMatch{
					App: &halproto.RuleMatch_AppMatch_PortInfo{
						PortInfo: &halproto.RuleMatch_L4PortAppInfo{
							SrcPortRange: []*halproto.L4PortRange{
								sPort,
							},
							DstPortRange: []*halproto.L4PortRange{
								dPort,
							},
						},
					},
				}
				ruleMatch.AppMatch = &appMatch
				ruleMatch.Protocol = dstProtocols[i]
				ruleMatches = append(ruleMatches, &ruleMatch)
			}
		}
		return ruleMatches, nil
	default:
		return ruleMatches, nil
	}
}

func convertRuleAction(action string) *halproto.SecurityRuleAction {
	var ruleAction halproto.SecurityRuleAction
	switch action {
	case netproto.PolicyRule_PERMIT.String():
		ruleAction.SecAction = halproto.SecurityAction_SECURITY_RULE_ACTION_ALLOW
	case netproto.PolicyRule_DENY.String():
		ruleAction.SecAction = halproto.SecurityAction_SECURITY_RULE_ACTION_DENY
	case netproto.PolicyRule_REJECT.String():
		ruleAction.SecAction = halproto.SecurityAction_SECURITY_RULE_ACTION_REJECT
	case netproto.PolicyRule_LOG.String():
		ruleAction.LogAction = halproto.LogAction_LOG_ON_SESSION_END
	default:
		ruleAction.SecAction = halproto.SecurityAction_SECURITY_RULE_ACTION_NONE
		ruleAction.LogAction = halproto.LogAction_LOG_NONE
		log.Errorf("invalid policy action %v specified.", action)
	}
	return &ruleAction
}

// CreateNetworkSecurityPolicy creates a security group policy in the datapath
func (dp *DelphiDatapath) CreateNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDAppLUT *sync.Map) error {
	var fwRules []*halproto.SecurityRule
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	for _, r := range sgp.Spec.Rules {
		ruleMatches, err := convertMatchCriteria(r.Src, r.Dst)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}
		for _, match := range ruleMatches {
			rule := &halproto.SecurityRule{
				RuleId: r.ID,
				Match:  match,
				Action: convertRuleAction(r.Action),
			}
			fwRules = append(fwRules, rule)

		}

	}
	sgPolicy := &halproto.SecurityPolicySpec{
		KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
			PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
				SecurityPolicyKey: &halproto.SecurityPolicyKey{
					SecurityPolicyId: sgp.Status.NetworkSecurityPolicyID,
					VrfIdOrHandle:    vrfKey,
				},
			},
		},
		Rule: fwRules,
	}

	err := dp.delphiClient.SetObject(sgPolicy)
	if err != nil {
		return err
	}

	return nil
}

// UpdateNetworkSecurityPolicy updates a security group policy in the datapath
func (dp *DelphiDatapath) UpdateNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDAppLUT *sync.Map) error {
	var fwRules []*halproto.SecurityRule
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	for _, r := range sgp.Spec.Rules {
		ruleMatches, err := convertMatchCriteria(r.Src, r.Dst)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		for _, match := range ruleMatches {
			rule := &halproto.SecurityRule{
				RuleId: r.ID,
				Match:  match,
				Action: convertRuleAction(r.Action),
			}
			fwRules = append(fwRules, rule)

		}
	}
	sgPolicy := &halproto.SecurityPolicySpec{
		KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
			PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
				SecurityPolicyKey: &halproto.SecurityPolicyKey{
					SecurityPolicyId: sgp.Status.NetworkSecurityPolicyID,
					VrfIdOrHandle:    vrfKey,
				},
			},
		},
		Rule: fwRules,
	}

	err := dp.delphiClient.SetObject(sgPolicy)
	if err != nil {
		return err
	}

	return nil
}

// DeleteNetworkSecurityPolicy deletes sg policy
func (dp *DelphiDatapath) DeleteNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy, vrfID uint64) error {
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	sgPolicy := &halproto.SecurityPolicySpec{
		KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
			PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
				SecurityPolicyKey: &halproto.SecurityPolicyKey{
					SecurityPolicyId: sgp.Status.NetworkSecurityPolicyID,
					VrfIdOrHandle:    vrfKey,
				},
			},
		},
	}

	err := dp.delphiClient.DeleteObject(sgPolicy)
	if err != nil {
		return err
	}

	return nil
}
