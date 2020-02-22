// +build apulu

package apulu

import (
	"fmt"
	"net"
	"strconv"
	"strings"
	"sync"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// ############# Hal conversion Routines  ##############

func convertHALFirewallRules(nsp netproto.NetworkSecurityPolicy, ruleIDToAppMapping *sync.Map) []*halapi.SecurityRule {
	var fwRules []*halapi.SecurityRule
	for idx, r := range nsp.Spec.Rules {
		ruleMatches, err := buildHALRuleMatches(r.Src, r.Dst, ruleIDToAppMapping, &idx)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
		}
		for _, match := range ruleMatches {
			rule := &halapi.SecurityRule{
				Stateful: true,
				Match:    match,
				Action:   convertRuleAction(r.Action),
			}
			fwRules = append(fwRules, rule)
		}
	}
	return fwRules
}

func buildHALRuleMatches(src, dst *netproto.MatchSelector, ruleIDAppLUT *sync.Map, ruleID *int) ([]*halapi.RuleMatch, error) {
	var srcL3Match, dstL3Match *halapi.RuleL3Match
	var srcProtoPorts, dstProtoPorts []string
	var appProtoPorts []string
	var app *netproto.App

	var ruleMatches []*halapi.RuleMatch
	var err error

	// Check if ProtoPorts need to be parsed from the app object
	if ruleIDAppLUT != nil && ruleID != nil {
		obj, ok := ruleIDAppLUT.Load(*ruleID)
		// Rule has a corresponding ALG information
		if ok {
			app, ok = obj.(*netproto.App)
			if !ok {
				log.Errorf("failed to cast App object. %v", obj)
				return nil, fmt.Errorf("failed to cast App object. %v", obj)
			}
			for _, s := range app.Spec.ProtoPorts {
				protoPort := fmt.Sprintf("%s/%s", s.Protocol, s.Port)
				appProtoPorts = append(appProtoPorts, protoPort)
			}
		}
	}

	if src != nil {
		srcL3Match, err = convertIPs(src.Addresses, true)
		if err != nil {
			log.Errorf("Could not convert IP Addresses from Src: {%v}. Err: %v", src.Addresses, err)
			return nil, err
		}

		// Build proto/port from src app configs.
		for _, s := range src.ProtoPorts {
			// TODO Unify the proto/port definitions between App Object and Match Selectors to avoid manually building this.
			protoPort := fmt.Sprintf("%s/%s", s.Protocol, s.Port)
			srcProtoPorts = append(srcProtoPorts, protoPort)
		}
	}

	if dst != nil {
		dstL3Match, err = convertIPs(dst.Addresses, false)
		if err != nil {
			log.Errorf("Could not convert IP Addresses from Dst: {%v}. Err: %v", dst.Addresses, err)
			return nil, err
		}

		for _, d := range dst.ProtoPorts {
			// TODO Unify the proto/port definitions between App Object and Match Selectors to avoid manually building this.
			protoPort := fmt.Sprintf("%s/%s", d.Protocol, d.Port)
			dstProtoPorts = append(dstProtoPorts, protoPort)
		}
	}

	// For App parsed proto/ports there is no src or dst. Override dstProtoPorts here if there is a valid app for a given rule.
	// TODO decide if we want to support src and dst based app proto/ports

	if len(appProtoPorts) > 0 {
		dstProtoPorts = appProtoPorts
	}

	// flatten appropriately
	// flatten if needed
	switch {
	// ICMP ALG Handler
	case app != nil && app.Spec.ALG.ICMP != nil:
		halProtocol, err := convertAppProtocol("icmp")
		if err != nil {
			log.Errorf("Failed to convert rule. Err: %v", err)
			return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
		}
		ruleL3Match := &halapi.RuleL3Match{
			Protocol: halProtocol,
			Srcmatch: srcL3Match.Srcmatch,
			Dstmatch: dstL3Match.Dstmatch,
		}

		ruleL4Match := &halapi.RuleL4Match{
			L4Info: &halapi.RuleL4Match_TypeCode{
				TypeCode: &halapi.ICMPMatch{
					Type: app.Spec.ALG.ICMP.Type,
					Code: app.Spec.ALG.ICMP.Code,
				},
			},
		}

		ruleMatch := &halapi.RuleMatch{
			L3Match: ruleL3Match,
			L4Match: ruleL4Match,
		}
		ruleMatches = append(ruleMatches, ruleMatch)
		return ruleMatches, nil

	case len(srcProtoPorts) == 0 && len(dstProtoPorts) != 0:
		for _, protoPort := range dstProtoPorts {
			protocol, matchInfo, err := parseProtocolPort(protoPort)
			if err != nil {
				log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
				return nil, err
			}
			// Ensure that port is not specified with icmp protocol
			if protocol == "icmp" && len(matchInfo) != 0 {
				log.Errorf("specifying port with icmp protocol is invalid")
				return nil, fmt.Errorf("specifying port with icmp protocol is invalid")
			}

			halProtocol, err := convertAppProtocol(protocol)
			if err != nil {
				log.Errorf("Failed to convert rule. Err: %v", err)
				return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
			}
			ruleL3Match := &halapi.RuleL3Match{
				Protocol: halProtocol,
				Srcmatch: srcL3Match.Srcmatch,
				Dstmatch: dstL3Match.Dstmatch,
			}
			var ruleL4Match *halapi.RuleL4Match

			// Handle TCP/UDP proto/ports here.
			if protocol != "icmp" {
				portRange, err := convertHalPort(matchInfo)
				if err != nil {
					log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
					return nil, err
				}
				ruleL4Match = &halapi.RuleL4Match{
					L4Info: &halapi.RuleL4Match_Ports{
						Ports: &halapi.PortMatch{
							DstPortRange: portRange,
						},
					},
				}
			}

			ruleMatch := &halapi.RuleMatch{
				L3Match: ruleL3Match,
				L4Match: ruleL4Match,
			}

			ruleMatches = append(ruleMatches, ruleMatch)
		}
		return ruleMatches, nil
	case len(dstProtoPorts) == 0 && len(srcProtoPorts) != 0:
		for _, protoPort := range srcProtoPorts {
			protocol, matchInfo, err := parseProtocolPort(protoPort)
			if err != nil {
				log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
				return nil, err
			}
			// Ensure that port is not specified with icmp protocol
			if protocol == "icmp" && len(matchInfo) != 0 {
				log.Errorf("specifying port with icmp protocol is invalid")
				return nil, fmt.Errorf("specifying port with icmp protocol is invalid")
			}

			halProtocol, err := convertAppProtocol(protocol)
			if err != nil {
				log.Errorf("Failed to convert rule. Err: %v", err)
				return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
			}
			ruleL3Match := &halapi.RuleL3Match{
				Protocol: halProtocol,
				Srcmatch: srcL3Match.Srcmatch,
				Dstmatch: dstL3Match.Dstmatch,
			}
			var ruleL4Match *halapi.RuleL4Match

			// Handle TCP/UDP proto/ports here.
			if protocol != "icmp" {
				portRange, err := convertHalPort(matchInfo)
				if err != nil {
					log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
					return nil, err
				}
				ruleL4Match = &halapi.RuleL4Match{
					L4Info: &halapi.RuleL4Match_Ports{
						Ports: &halapi.PortMatch{
							SrcPortRange: portRange,
						},
					},
				}
			}

			ruleMatch := &halapi.RuleMatch{
				L3Match: ruleL3Match,
				L4Match: ruleL4Match,
			}

			ruleMatches = append(ruleMatches, ruleMatch)
		}
		return ruleMatches, nil
	case len(srcProtoPorts) > 0 && len(dstProtoPorts) > 0:
		for _, sProtoPort := range srcProtoPorts {
			for _, dProtoPort := range dstProtoPorts {
				_, srcMatchInfo, err := parseProtocolPort(sProtoPort)
				if err != nil {
					log.Errorf("Could not parse source protocol/port information from %v. Err: %v", sProtoPort, err)
					return nil, err
				}

				protocol, dstMatchInfo, err := parseProtocolPort(dProtoPort)
				if err != nil {
					log.Errorf("Could not parse destination protocol/port information from %v. Err: %v", dProtoPort, err)
					return nil, err
				}
				// Ensure that port is not specified with icmp protocol
				if protocol == "icmp" && len(dstMatchInfo) != 0 {
					log.Errorf("specifying port with icmp protocol is invalid")
					return nil, fmt.Errorf("specifying port with icmp protocol is invalid")
				}
				halProtocol, err := convertAppProtocol(protocol)
				if err != nil {
					log.Errorf("Failed to convert rule. Err: %v", err)
					return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
				}

				ruleL3Match := &halapi.RuleL3Match{
					Protocol: halProtocol,
					Srcmatch: srcL3Match.Srcmatch,
					Dstmatch: dstL3Match.Dstmatch,
				}
				var ruleL4Match *halapi.RuleL4Match

				// Handle TCP/UDP proto/ports here.
				if protocol != "icmp" {
					srcPortRange, err := convertHalPort(srcMatchInfo)
					if err != nil {
						log.Errorf("Could not parse source protocol/port information from %v. Err: %v", sProtoPort, err)
						return nil, err
					}
					dstPortRange, err := convertHalPort(dstMatchInfo)
					if err != nil {
						log.Errorf("Could not parse destination protocol/port information from %v. Err: %v", dProtoPort, err)
						return nil, err
					}
					ruleL4Match = &halapi.RuleL4Match{
						L4Info: &halapi.RuleL4Match_Ports{
							Ports: &halapi.PortMatch{
								SrcPortRange: srcPortRange,
								DstPortRange: dstPortRange,
							},
						},
					}
				}

				ruleMatch := &halapi.RuleMatch{
					L3Match: ruleL3Match,
					L4Match: ruleL4Match,
				}
				ruleMatches = append(ruleMatches, ruleMatch)
			}
		}
		return ruleMatches, nil
		// Empty src and dst proto ports. No flattening needed.
	case len(srcProtoPorts) == 0 && len(dstProtoPorts) == 0:
		ruleL3Match := &halapi.RuleL3Match{
			Srcmatch: srcL3Match.Srcmatch,
			Dstmatch: dstL3Match.Dstmatch,
		}
		ruleMatch := &halapi.RuleMatch{
			L3Match: ruleL3Match,
		}
		ruleMatches = append(ruleMatches, ruleMatch)
		return ruleMatches, nil

	default:
		return ruleMatches, nil
	}
}

func convertRuleAction(action string) halapi.SecurityRuleAction {
	switch action {
	case "PERMIT":
		return halapi.SecurityRuleAction_SECURITY_RULE_ACTION_ALLOW
	case "DENY":
		return halapi.SecurityRuleAction_SECURITY_RULE_ACTION_DENY
	default:
		log.Errorf("invalid policy action %v specified.", action)
		return halapi.SecurityRuleAction_SECURITY_RULE_ACTION_NONE
	}
}

// In Cloud pipeline we pass only one srcmatch/dstmatch to HAL, so take the
// first one.
func convertIPs(addresses []string, srcMatch bool) (*halapi.RuleL3Match, error) {
	for _, a := range addresses {
		if ip := net.ParseIP(strings.TrimSpace(a)); len(ip) > 0 {
			// try parsing as an octet
			if srcMatch {
				return &halapi.RuleL3Match{
					Srcmatch: &halapi.RuleL3Match_SrcPrefix{
						SrcPrefix: &halapi.IPPrefix{
							Addr: &halapi.IPAddress{
								Af: halapi.IPAF_IP_AF_INET,
								V4OrV6: &halapi.IPAddress_V4Addr{
									V4Addr: utils.Ipv4Touint32(ip),
								},
							},
							Len: uint32(32),
						},
					},
				}, nil
			}
			return &halapi.RuleL3Match{
				Dstmatch: &halapi.RuleL3Match_DstPrefix{
					DstPrefix: &halapi.IPPrefix{
						Addr: &halapi.IPAddress{
							Af: halapi.IPAF_IP_AF_INET,
							V4OrV6: &halapi.IPAddress_V4Addr{
								V4Addr: utils.Ipv4Touint32(ip),
							},
						},
						Len: uint32(32),
					},
				},
			}, nil
		} else if ip, network, err := net.ParseCIDR(strings.TrimSpace(a)); err == nil {
			// try parsing as IPMask
			prefixLen, _ := network.Mask.Size()
			if srcMatch {
				return &halapi.RuleL3Match{
					Srcmatch: &halapi.RuleL3Match_SrcPrefix{
						SrcPrefix: &halapi.IPPrefix{
							Addr: &halapi.IPAddress{
								Af: halapi.IPAF_IP_AF_INET,
								V4OrV6: &halapi.IPAddress_V4Addr{
									V4Addr: utils.Ipv4Touint32(ip),
								},
							},
							Len: uint32(prefixLen),
						},
					},
				}, nil
			}
			return &halapi.RuleL3Match{
				Dstmatch: &halapi.RuleL3Match_DstPrefix{
					DstPrefix: &halapi.IPPrefix{
						Addr: &halapi.IPAddress{
							Af: halapi.IPAF_IP_AF_INET,
							V4OrV6: &halapi.IPAddress_V4Addr{
								V4Addr: utils.Ipv4Touint32(ip),
							},
						},
						Len: uint32(prefixLen),
					},
				},
			}, nil
		} else if ipRange := strings.Split(strings.TrimSpace(a), "-"); len(ipRange) == 2 {
			// try parsing as hyphen separated range
			addrRange, err := convertIPRange(ipRange[0], ipRange[1])
			if err != nil {
				log.Errorf("failed to parse IP Range {%v}. Err: %v", ipRange, err)
				return nil, err
			}
			if srcMatch {
				return &halapi.RuleL3Match{
					Srcmatch: &halapi.RuleL3Match_SrcRange{
						SrcRange: addrRange,
					},
				}, nil
			}
			return &halapi.RuleL3Match{
				Dstmatch: &halapi.RuleL3Match_DstRange{
					DstRange: addrRange,
				},
			}, nil
		} else if a == "any" {
			// Interpret it as 0.0.0.0/0
			if srcMatch {
				return &halapi.RuleL3Match{
					Srcmatch: &halapi.RuleL3Match_SrcPrefix{
						SrcPrefix: &halapi.IPPrefix{
							Addr: &halapi.IPAddress{
								Af:     halapi.IPAF_IP_AF_INET,
								V4OrV6: &halapi.IPAddress_V4Addr{},
							},
						},
					},
				}, nil
			}
			return &halapi.RuleL3Match{
				Dstmatch: &halapi.RuleL3Match_DstPrefix{
					DstPrefix: &halapi.IPPrefix{
						Addr: &halapi.IPAddress{
							Af:     halapi.IPAF_IP_AF_INET,
							V4OrV6: &halapi.IPAddress_V4Addr{},
						},
					},
				},
			}, nil
		} else {
			// give up
			return nil, fmt.Errorf("invalid IP Address format {%v}. Should either be in an octet, CIDR or hyphen separated IP Range", a)
		}
	}
	return &halapi.RuleL3Match{}, nil
}

func convertAppProtocol(protocol string) (uint32, error) {
	switch strings.ToLower(protocol) {
	case "tcp", "6":
		return 6, nil
	case "udp", "17":
		return 17, nil
	case "icmp", "1":
		return 1, nil
	case "any":
		return 0, nil
	default:
		return 0, fmt.Errorf("unsupported protocol %v", protocol)
	}
}

func parseProtocolPort(protoPort string) (protocol, port string, err error) {
	if components := strings.Split(strings.TrimSpace(protoPort), "/"); len(components) == 2 {
		protocol = components[0]
		port = components[1]
	} else {
		err = fmt.Errorf("failed to parse protocol/port information from the App. %v. Err: %v", protocol, err)
		return
	}
	return
}

func convertHalPort(port string) (*halapi.PortRange, error) {
	// handle empty port case
	if port == "" {
		return &halapi.PortRange{}, nil
	}

	// parse port range
	portSlice := strings.Split(port, "-")
	switch len(portSlice) {
	// single port
	case 1:
		p, err := strconv.Atoi(port)
		if err != nil || p < 0 || p > 65535 {
			log.Errorf("invalid port format. %v", port)
			return nil, fmt.Errorf("invalid port format. %v", port)
		}
		halPort := &halapi.PortRange{
			PortLow:  uint32(p),
			PortHigh: uint32(p),
		}
		return halPort, nil
	// port range
	case 2:
		startPort, err := strconv.Atoi(portSlice[0])
		if err != nil || startPort < 0 || startPort > 65535 {
			log.Errorf("invalid port format. %v", port)
			return nil, fmt.Errorf("invalid port format. %v", port)
		}
		endPort, err := strconv.Atoi(portSlice[1])
		if err != nil || endPort < 0 || endPort > 65535 {
			log.Errorf("invalid port format. %v", port)
			return nil, fmt.Errorf("invalid port format. %v", port)
		}

		halPort := &halapi.PortRange{
			PortLow:  uint32(startPort),
			PortHigh: uint32(endPort),
		}
		return halPort, nil
	default:
		log.Errorf("invalid port format. %v. It should either be hyphen separated or a single port", port)
		return nil, fmt.Errorf("invalid port format. %v. It should either be hyphen separated or a single port", port)
	}
}

// convertIPRange converts a start IP and end IP to hal IPAddressObj
func convertIPRange(startIP, endIP string) (*halapi.AddressRange, error) {
	begin := net.ParseIP(strings.TrimSpace(startIP))
	if len(begin) == 0 {
		log.Errorf("could not parse start IP {%v}", startIP)
		return nil, fmt.Errorf("could not parse start IP {%v}", startIP)
	}
	end := net.ParseIP(strings.TrimSpace(endIP))
	if len(end) == 0 {
		log.Errorf("could not parse end IP {%v}", endIP)
		return nil, fmt.Errorf("could not parse start IP {%v}", startIP)
	}

	lowIP := halapi.IPAddress{
		Af: halapi.IPAF_IP_AF_INET,
		V4OrV6: &halapi.IPAddress_V4Addr{
			V4Addr: utils.Ipv4Touint32(begin),
		},
	}

	highIP := halapi.IPAddress{
		Af: halapi.IPAF_IP_AF_INET,
		V4OrV6: &halapi.IPAddress_V4Addr{
			V4Addr: utils.Ipv4Touint32(end),
		},
	}

	return &halapi.AddressRange{
		Range: &halapi.AddressRange_IPv4Range{
			IPv4Range: &halapi.IPRange{
				Low:  &lowIP,
				High: &highIP,
			},
		},
	}, nil
}
