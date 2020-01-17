// +build iris

package iris

import (
	"fmt"
	"net"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ############# Hal conversion Routines  ##############

// TODO Remove this once we optimize rule conversion

func convertHALFirewallRules(nsp netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDToAppMapping *sync.Map) []*halapi.SecurityRule {
	var fwRules []*halapi.SecurityRule
	for idx, r := range nsp.Spec.Rules {
		ruleMatches, err := buildHALRuleMatches(r.Src, r.Dst, ruleIDToAppMapping, &idx)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
		}
		for _, match := range ruleMatches {
			ruleAction, err := convertRuleAction(idx, ruleIDToAppMapping, r.Action)
			if err != nil {
				log.Errorf("Failed to convert rule action. Err: %v", err)
			}
			rule := &halapi.SecurityRule{
				RuleId: r.ID,
				Match:  match,
				Action: ruleAction,
			}
			fwRules = append(fwRules, rule)
		}

	}
	return fwRules
}

func buildHALRuleMatches(src, dst *netproto.MatchSelector, ruleIDAppLUT *sync.Map, ruleID *int) ([]*halapi.RuleMatch, error) {
	var srcIPRanges, dstIPRanges []*halapi.IPAddressObj
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
		srcIPRanges, err = convertIPs(src.Addresses)
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
		dstIPRanges, err = convertIPs(dst.Addresses)
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
		var ruleMatch halapi.RuleMatch

		ruleMatch.SrcAddress = srcIPRanges
		ruleMatch.DstAddress = dstIPRanges
		appMatch := halapi.RuleMatch_AppMatch{
			App: &halapi.RuleMatch_AppMatch_IcmpInfo{
				IcmpInfo: &halapi.RuleMatch_ICMPAppInfo{
					IcmpCode: app.Spec.ALG.ICMP.Code,
					IcmpType: app.Spec.ALG.ICMP.Type,
				},
			},
		}
		ruleMatch.AppMatch = &appMatch
		halProtocol, err := convertAppProtocol("icmp")
		if err != nil {
			log.Errorf("Failed to convert rule. Err: %v", err)
			return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
		}
		ruleMatch.Protocol = halProtocol
		ruleMatches = append(ruleMatches, &ruleMatch)
		return ruleMatches, nil

	case len(srcProtoPorts) == 0 && len(dstProtoPorts) != 0:
		for _, protoPort := range dstProtoPorts {
			var ruleMatch halapi.RuleMatch
			var appMatch halapi.RuleMatch_AppMatch
			ruleMatch.SrcAddress = srcIPRanges
			ruleMatch.DstAddress = dstIPRanges

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
			// Handle TCP/UDP proto/ports here.
			if protocol != "icmp" {
				portRange, err := convertHalPort(matchInfo)
				if err != nil {
					log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
					return nil, err
				}
				appMatch = halapi.RuleMatch_AppMatch{
					App: &halapi.RuleMatch_AppMatch_PortInfo{
						PortInfo: &halapi.RuleMatch_L4PortAppInfo{
							DstPortRange: []*halapi.L4PortRange{
								portRange,
							},
						},
					},
				}
			}

			ruleMatch.AppMatch = &appMatch
			halProtocol, err := convertAppProtocol(protocol)
			if err != nil {
				log.Errorf("Failed to convert rule. Err: %v", err)
				return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
			}
			ruleMatch.Protocol = halProtocol

			ruleMatches = append(ruleMatches, &ruleMatch)

		}
		return ruleMatches, nil
	case len(dstProtoPorts) == 0 && len(srcProtoPorts) != 0:
		for _, protoPort := range srcProtoPorts {
			var ruleMatch halapi.RuleMatch
			var appMatch halapi.RuleMatch_AppMatch
			ruleMatch.SrcAddress = srcIPRanges
			ruleMatch.DstAddress = dstIPRanges
			protocol, srcMatchInfo, err := parseProtocolPort(protoPort)
			if err != nil {
				log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
				return nil, err
			}

			// Ensure that port is not specified with icmp protocol

			if protocol == "icmp" && len(srcMatchInfo) != 0 {
				log.Errorf("specifying port with icmp protocol is invalid")
				return nil, fmt.Errorf("specifying port with icmp protocol is invalid")
			}

			// Handle TCP/UDP proto/ports here.
			if protocol != "icmp" {
				portRange, err := convertHalPort(srcMatchInfo)
				if err != nil {
					log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
					return nil, err
				}
				appMatch = halapi.RuleMatch_AppMatch{
					App: &halapi.RuleMatch_AppMatch_PortInfo{
						PortInfo: &halapi.RuleMatch_L4PortAppInfo{
							SrcPortRange: []*halapi.L4PortRange{
								portRange,
							},
						},
					},
				}
			}

			ruleMatch.AppMatch = &appMatch
			halProtocol, err := convertAppProtocol(protocol)
			if err != nil {
				log.Errorf("Failed to convert rule. Err: %v", err)
				return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
			}
			ruleMatch.Protocol = halProtocol
			ruleMatches = append(ruleMatches, &ruleMatch)

		}
		return ruleMatches, nil
	case len(srcProtoPorts) > 0 && len(dstProtoPorts) > 0:
		for _, sProtoPort := range srcProtoPorts {
			for _, dProtoPort := range dstProtoPorts {
				var ruleMatch halapi.RuleMatch
				var appMatch halapi.RuleMatch_AppMatch
				ruleMatch.SrcAddress = srcIPRanges
				ruleMatch.DstAddress = dstIPRanges
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
					appMatch = halapi.RuleMatch_AppMatch{
						App: &halapi.RuleMatch_AppMatch_PortInfo{
							PortInfo: &halapi.RuleMatch_L4PortAppInfo{
								SrcPortRange: []*halapi.L4PortRange{
									srcPortRange,
								},
								DstPortRange: []*halapi.L4PortRange{
									dstPortRange,
								},
							},
						},
					}
				}

				ruleMatch.AppMatch = &appMatch
				halProtocol, err := convertAppProtocol(protocol)
				if err != nil {
					log.Errorf("Failed to convert rule. Err: %v", err)
					return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
				}
				ruleMatch.Protocol = halProtocol
				ruleMatches = append(ruleMatches, &ruleMatch)
			}
		}
		return ruleMatches, nil
		// Empty src and dst proto ports. No flattening needed.
	case len(srcProtoPorts) == 0 && len(dstProtoPorts) == 0:
		var ruleMatch halapi.RuleMatch
		ruleMatch.SrcAddress = srcIPRanges
		ruleMatch.DstAddress = dstIPRanges
		ruleMatches = append(ruleMatches, &ruleMatch)
		return ruleMatches, nil

	default:
		return ruleMatches, nil
	}
}

func convertRuleAction(ruleIdx int, ruleIDAppLUT *sync.Map, action string) (*halapi.SecurityRuleAction, error) {
	var ruleAction halapi.SecurityRuleAction

	obj, ok := ruleIDAppLUT.Load(ruleIdx)
	// Rule has a corresponding ALG information
	if ok {
		app, ok := obj.(*netproto.App)
		if !ok {
			log.Errorf("failed to cast App object. %v", obj)
			return nil, fmt.Errorf("failed to cast App object. %v", obj)
		}
		alg, err := convertHalAlg(app)
		if err != nil {
			log.Errorf("failed to convert alg data. Err: %v, App: %v", err, app)
		}
		ruleAction.AppData = alg
	}
	switch action {
	case "PERMIT":
		ruleAction.SecAction = halapi.SecurityAction_SECURITY_RULE_ACTION_ALLOW
	case "DENY":
		ruleAction.SecAction = halapi.SecurityAction_SECURITY_RULE_ACTION_DENY
	case "REJECT":
		ruleAction.SecAction = halapi.SecurityAction_SECURITY_RULE_ACTION_REJECT
	case "LOG":
		ruleAction.LogAction = halapi.LogAction_LOG_ON_SESSION_END
	default:
		ruleAction.SecAction = halapi.SecurityAction_SECURITY_RULE_ACTION_NONE
		ruleAction.LogAction = halapi.LogAction_LOG_NONE
		log.Errorf("invalid policy action %v specified.", action)
	}
	return &ruleAction, nil
}

func convertIPs(addresses []string) ([]*halapi.IPAddressObj, error) {
	var halAddresses []*halapi.IPAddressObj
	for _, a := range addresses {
		if ip := net.ParseIP(strings.TrimSpace(a)); len(ip) > 0 {
			// try parsing as an octet

			halAddr := &halapi.IPAddressObj{
				Formats: &halapi.IPAddressObj_Address{
					Address: &halapi.Address{
						Address: &halapi.Address_Prefix{
							Prefix: &halapi.IPSubnet{
								Subnet: &halapi.IPSubnet_Ipv4Subnet{
									Ipv4Subnet: &halapi.IPPrefix{
										Address: &halapi.IPAddress{
											IpAf: halapi.IPAddressFamily_IP_AF_INET,
											V4OrV6: &halapi.IPAddress_V4Addr{
												V4Addr: utils.Ipv4Touint32(ip),
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

			halAddr := &halapi.IPAddressObj{
				Formats: &halapi.IPAddressObj_Address{
					Address: &halapi.Address{
						Address: &halapi.Address_Prefix{
							Prefix: &halapi.IPSubnet{
								Subnet: &halapi.IPSubnet_Ipv4Subnet{
									Ipv4Subnet: &halapi.IPPrefix{
										Address: &halapi.IPAddress{
											IpAf: halapi.IPAddressFamily_IP_AF_INET,
											V4OrV6: &halapi.IPAddress_V4Addr{
												V4Addr: utils.Ipv4Touint32(ip),
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
			halAddr := &halapi.IPAddressObj{
				Formats: &halapi.IPAddressObj_Address{
					Address: &halapi.Address{
						Address: &halapi.Address_Prefix{
							Prefix: &halapi.IPSubnet{
								Subnet: &halapi.IPSubnet_Ipv4Subnet{
									Ipv4Subnet: &halapi.IPPrefix{
										Address: &halapi.IPAddress{
											IpAf:   halapi.IPAddressFamily_IP_AF_INET,
											V4OrV6: &halapi.IPAddress_V4Addr{},
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

func convertAppProtocol(protocol string) (int32, error) {
	switch strings.ToLower(protocol) {
	case "tcp", "6":
		return int32(halapi.IPProtocol_IPPROTO_TCP), nil
	case "udp", "17":
		return int32(halapi.IPProtocol_IPPROTO_UDP), nil
	case "icmp", "1":
		return int32(halapi.IPProtocol_IPPROTO_ICMP), nil
	case "any":
		return int32(halapi.IPProtocol_IPPROTO_NONE), nil
	default:
		return int32(halapi.IPProtocol_IPPROTO_NONE), fmt.Errorf("unsupported protocol %v", protocol)
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

func convertHalPort(port string) (*halapi.L4PortRange, error) {
	// handle empty port case
	if port == "" {
		return &halapi.L4PortRange{}, nil
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
		halPort := &halapi.L4PortRange{
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

		halPort := &halapi.L4PortRange{
			PortLow:  uint32(startPort),
			PortHigh: uint32(endPort),
		}
		return halPort, nil
	default:
		log.Errorf("invalid port format. %v. It should either be hyphen separated or a single port", port)
		return nil, fmt.Errorf("invalid port format. %v. It should either be hyphen separated or a single port", port)
	}
}

func convertHalAlg(app *netproto.App) (*halapi.AppData, error) {
	var appData halapi.AppData
	algSpec := app.Spec.ALG
	// Convert App Idle Timeout
	if len(app.Spec.AppIdleTimeout) > 0 {
		dur, err := time.ParseDuration(app.Spec.AppIdleTimeout)
		if err != nil {
			return nil, fmt.Errorf("invalid time duration %s", app.Spec.AppIdleTimeout)
		}
		appData.IdleTimeout = uint32(dur.Seconds())
	} else {
		appData.IdleTimeout = types.DefaultTimeout
	}

	// Convert ALG Specific Information.

	// DNS
	if algSpec.DNS != nil {
		appData.Alg = halapi.ALGName_APP_SVC_DNS
		var queryRespTimeout uint32
		var maxMsgLength uint32
		if len(algSpec.DNS.QueryResponseTimeout) > 0 {
			dur, err := time.ParseDuration(algSpec.DNS.QueryResponseTimeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.DNS.QueryResponseTimeout)
			}
			queryRespTimeout = uint32(dur.Seconds())
		} else {
			queryRespTimeout = types.DefaultTimeout
		}

		if algSpec.DNS.MaxMessageLength > 0 {
			maxMsgLength = algSpec.DNS.MaxMessageLength
		} else {
			maxMsgLength = types.DefaultDNSMaxMessageLength
		}

		appData.AppOptions = &halapi.AppData_DnsOptionInfo{
			DnsOptionInfo: &halapi.AppData_DNSOptions{
				DropMultiQuestionPackets:   algSpec.DNS.DropMultiQuestionPackets,
				DropLargeDomainNamePackets: algSpec.DNS.DropLargeDomainPackets,
				MaxMsgLength:               maxMsgLength,
				QueryResponseTimeout:       queryRespTimeout,
				DropLongLabelPackets:       algSpec.DNS.DropLongLabelPackets,
				DropMultizonePackets:       algSpec.DNS.DropMultiZonePackets,
			},
		}
		return &appData, nil
	}

	// FTP
	if algSpec.FTP != nil {
		appData.Alg = halapi.ALGName_APP_SVC_FTP

		appData.AppOptions = &halapi.AppData_FtpOptionInfo{
			FtpOptionInfo: &halapi.AppData_FTPOptions{
				AllowMismatchIpAddress: algSpec.FTP.AllowMismatchIPAddresses,
			},
		}
		return &appData, nil
	}

	// MSRPC
	if algSpec.MSRPC != nil {
		appData.Alg = halapi.ALGName_APP_SVC_MSFT_RPC
		appData.AppOptions = &halapi.AppData_MsrpcOptionInfo{
			MsrpcOptionInfo: &halapi.AppData_MSRPCOptions{
				Data: convertRPCData(algSpec.MSRPC),
			},
		}

		return &appData, nil
	}

	// SunRPC
	if algSpec.SUNRPC != nil {
		appData.Alg = halapi.ALGName_APP_SVC_SUN_RPC

		appData.AppOptions = &halapi.AppData_SunRpcOptionInfo{
			SunRpcOptionInfo: &halapi.AppData_SunRPCOptions{
				Data: convertRPCData(algSpec.SUNRPC),
			},
		}
		return &appData, nil
	}

	// RTSP
	if algSpec.RTSP != nil {
		appData.Alg = halapi.ALGName_APP_SVC_RTSP
		return &appData, nil
	}

	// SIP
	if algSpec.SIP != nil {
		appData.Alg = halapi.ALGName_APP_SVC_SIP

		var mediaInactivityTimeout, maxCallDuration, cTimeout, t1Timeout, t4Timeout uint32

		if len(algSpec.SIP.MediaInactivityTimeout) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.MediaInactivityTimeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.MediaInactivityTimeout)
			}
			mediaInactivityTimeout = uint32(dur.Seconds())
		} else {
			mediaInactivityTimeout = types.DefaultTimeout
		}

		if len(algSpec.SIP.MaxCallDuration) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.MaxCallDuration)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.MaxCallDuration)
			}
			maxCallDuration = uint32(dur.Seconds())
		} else {
			maxCallDuration = types.DefaultTimeout
		}

		if len(algSpec.SIP.T4Timeout) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.T4Timeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.T4Timeout)
			}
			t4Timeout = uint32(dur.Seconds())
		} else {
			t4Timeout = types.DefaultTimeout
		}

		if len(algSpec.SIP.T1Timeout) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.T1Timeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.T1Timeout)
			}
			t1Timeout = uint32(dur.Seconds())
		} else {
			t1Timeout = types.DefaultTimeout
		}

		if len(algSpec.SIP.CTimeout) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.CTimeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.CTimeout)
			}
			cTimeout = uint32(dur.Seconds())
		} else {
			cTimeout = types.DefaultTimeout
		}

		appData.AppOptions = &halapi.AppData_SipOptions{
			SipOptions: &halapi.AppData_SIPOptions{
				DscpCodePoint:          algSpec.SIP.DSCPCodePoint,
				MediaInactivityTimeout: mediaInactivityTimeout,
				MaxCallDuration:        maxCallDuration,
				T4TimerValue:           t4Timeout,
				T1TimerValue:           t1Timeout,
				Ctimeout:               cTimeout,
			},
		}
		return &appData, nil
	}

	// TFTP
	if algSpec.TFTP != nil {
		appData.Alg = halapi.ALGName_APP_SVC_TFTP
		return &appData, nil
	}

	return nil, nil
}

// convertIPRange converts a start IP and end IP to hal IPAddressObj
func convertIPRange(startIP, endIP string) (*halapi.IPAddressObj, error) {
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
		IpAf: halapi.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halapi.IPAddress_V4Addr{
			V4Addr: utils.Ipv4Touint32(begin),
		},
	}

	highIP := halapi.IPAddress{
		IpAf: halapi.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halapi.IPAddress_V4Addr{
			V4Addr: utils.Ipv4Touint32(end),
		},
	}

	addrRange := &halapi.Address_Range{
		Range: &halapi.AddressRange{
			Range: &halapi.AddressRange_Ipv4Range{
				Ipv4Range: &halapi.IPRange{
					LowIpaddr:  &lowIP,
					HighIpaddr: &highIP,
				},
			},
		},
	}
	halAddresses := &halapi.IPAddressObj{
		Formats: &halapi.IPAddressObj_Address{
			Address: &halapi.Address{
				Address: addrRange,
			},
		},
	}
	return halAddresses, nil
}
