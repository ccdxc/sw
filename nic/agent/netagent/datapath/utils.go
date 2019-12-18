package datapath

import (
	"crypto/rand"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"net"
	"os/exec"
	"strings"
	"sync"

	"strconv"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	naplesHwUUIDFile = "/tmp/fru.json"
)

// objectKey returns object key from meta
func objectKey(ometa *api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", ometa.Tenant, ometa.Name)
}

// uint32ToIPv4 converts an 32 bit integer to net.IP
func uint32ToIPv4(intIP uint32) net.IP {
	ip := make(net.IP, 4)
	binary.BigEndian.PutUint32(ip, intIP)
	return ip
}

// ipv4ToUint32 converts net.IP to a 32 bit integer
func ipv4Touint32(ip net.IP) uint32 {
	if len(ip) == 16 {
		return binary.BigEndian.Uint32(ip[12:16])
	}
	return binary.BigEndian.Uint32(ip)
}

// buildHALRuleMatches converts agent match object to hal match object.
// RuleMatches for L4 Ports are built either using App/ALG Data or match selectors
func (hd *Datapath) buildHALRuleMatches(src, dst *netproto.MatchSelector, ruleIDAppLUT *sync.Map, ruleID *int) ([]*halproto.RuleMatch, error) {
	var srcIPRanges, dstIPRanges []*halproto.IPAddressObj
	var srcProtoPorts, dstProtoPorts []string
	var appProtoPorts []string
	var app *netproto.App

	var ruleMatches []*halproto.RuleMatch
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
			for _, a := range app.Spec.ProtoPorts {
				protoPort := fmt.Sprintf("%s/%s", a.Protocol, a.Port)
				appProtoPorts = append(appProtoPorts, protoPort)
			}
		}
	}

	if src != nil {
		srcIPRanges, err = hd.convertIPs(src.Addresses)
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
		dstIPRanges, err = hd.convertIPs(dst.Addresses)
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
		var ruleMatch halproto.RuleMatch

		ruleMatch.SrcAddress = srcIPRanges
		ruleMatch.DstAddress = dstIPRanges
		appMatch := halproto.RuleMatch_AppMatch{
			App: &halproto.RuleMatch_AppMatch_IcmpInfo{
				IcmpInfo: &halproto.RuleMatch_ICMPAppInfo{
					IcmpCode: app.Spec.ALG.ICMP.Code,
					IcmpType: app.Spec.ALG.ICMP.Type,
				},
			},
		}
		ruleMatch.AppMatch = &appMatch
		halProtocol, err := hd.convertAppProtocol("icmp")
		if err != nil {
			log.Errorf("Failed to convert rule. Err: %v", err)
			return nil, fmt.Errorf("failed to convert rule. Err: %v", err)
		}
		ruleMatch.Protocol = halProtocol
		ruleMatches = append(ruleMatches, &ruleMatch)
		return ruleMatches, nil

	case len(srcProtoPorts) == 0 && len(dstProtoPorts) != 0:
		for _, protoPort := range dstProtoPorts {
			var ruleMatch halproto.RuleMatch
			var appMatch halproto.RuleMatch_AppMatch
			ruleMatch.SrcAddress = srcIPRanges
			ruleMatch.DstAddress = dstIPRanges

			protocol, matchInfo, err := hd.parseProtocolPort(protoPort)
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
				portRange, err := hd.convertPort(matchInfo)
				if err != nil {
					log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
					return nil, err
				}
				appMatch = halproto.RuleMatch_AppMatch{
					App: &halproto.RuleMatch_AppMatch_PortInfo{
						PortInfo: &halproto.RuleMatch_L4PortAppInfo{
							DstPortRange: []*halproto.L4PortRange{
								portRange,
							},
						},
					},
				}
			}

			ruleMatch.AppMatch = &appMatch
			halProtocol, err := hd.convertAppProtocol(protocol)
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
			var ruleMatch halproto.RuleMatch
			var appMatch halproto.RuleMatch_AppMatch
			ruleMatch.SrcAddress = srcIPRanges
			ruleMatch.DstAddress = dstIPRanges
			protocol, srcMatchInfo, err := hd.parseProtocolPort(protoPort)
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
				portRange, err := hd.convertPort(srcMatchInfo)
				if err != nil {
					log.Errorf("Could not parse protocol/port information from %v. Err: %v", protoPort, err)
					return nil, err
				}
				appMatch = halproto.RuleMatch_AppMatch{
					App: &halproto.RuleMatch_AppMatch_PortInfo{
						PortInfo: &halproto.RuleMatch_L4PortAppInfo{
							SrcPortRange: []*halproto.L4PortRange{
								portRange,
							},
						},
					},
				}
			}

			ruleMatch.AppMatch = &appMatch
			halProtocol, err := hd.convertAppProtocol(protocol)
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
				var ruleMatch halproto.RuleMatch
				var appMatch halproto.RuleMatch_AppMatch
				ruleMatch.SrcAddress = srcIPRanges
				ruleMatch.DstAddress = dstIPRanges
				_, srcMatchInfo, err := hd.parseProtocolPort(sProtoPort)
				if err != nil {
					log.Errorf("Could not parse source protocol/port information from %v. Err: %v", sProtoPort, err)
					return nil, err
				}

				protocol, dstMatchInfo, err := hd.parseProtocolPort(dProtoPort)
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
					srcPortRange, err := hd.convertPort(srcMatchInfo)
					if err != nil {
						log.Errorf("Could not parse source protocol/port information from %v. Err: %v", sProtoPort, err)
						return nil, err
					}
					dstPortRange, err := hd.convertPort(dstMatchInfo)
					if err != nil {
						log.Errorf("Could not parse destination protocol/port information from %v. Err: %v", dProtoPort, err)
						return nil, err
					}
					appMatch = halproto.RuleMatch_AppMatch{
						App: &halproto.RuleMatch_AppMatch_PortInfo{
							PortInfo: &halproto.RuleMatch_L4PortAppInfo{
								SrcPortRange: []*halproto.L4PortRange{
									srcPortRange,
								},
								DstPortRange: []*halproto.L4PortRange{
									dstPortRange,
								},
							},
						},
					}
				}

				ruleMatch.AppMatch = &appMatch
				halProtocol, err := hd.convertAppProtocol(protocol)
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
		var ruleMatch halproto.RuleMatch
		ruleMatch.SrcAddress = srcIPRanges
		ruleMatch.DstAddress = dstIPRanges
		ruleMatches = append(ruleMatches, &ruleMatch)
		return ruleMatches, nil

	default:
		return ruleMatches, nil
	}
}

func (hd *Datapath) convertAppProtocol(protocol string) (halproto.IPProtocol, error) {
	switch strings.ToLower(protocol) {
	case "tcp", "6":
		return halproto.IPProtocol_IPPROTO_TCP, nil
	case "udp", "17":
		return halproto.IPProtocol_IPPROTO_UDP, nil
	case "icmp", "1":
		return halproto.IPProtocol_IPPROTO_ICMP, nil
	case "any":
		return halproto.IPProtocol_IPPROTO_NONE, nil
	default:
		return halproto.IPProtocol_IPPROTO_NONE, fmt.Errorf("unsupported protocol %v", protocol)
	}
}

// ToDo Remove Mock code prior to FCS. This is needed only for UT
func (hd *Datapath) generateMockHwState() (*halproto.LifGetResponseMsg, *halproto.PortInfoGetResponseMsg, error) {
	mockLifs := &halproto.LifGetResponseMsg{
		Response: []*halproto.LifGetResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.LifSpec{
					KeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: 1,
						},
					},
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.LifSpec{
					KeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: 2,
						},
					},
				},
			},
		},
	}

	mockPorts := &halproto.PortInfoGetResponseMsg{
		Response: []*halproto.PortInfoGetResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.PortInfoSpec{
					NumLanes: 4,
					KeyOrHandle: &halproto.PortKeyHandle{
						KeyOrHandle: &halproto.PortKeyHandle_PortId{
							PortId: 1,
						},
					},
					BreakoutModes: []halproto.PortBreakoutMode{
						halproto.PortBreakoutMode_PORT_BREAKOUT_MODE_4x25G,
						halproto.PortBreakoutMode_PORT_BREAKOUT_MODE_2x50G,
						halproto.PortBreakoutMode_PORT_BREAKOUT_MODE_4x10G,
					},
					PortType: halproto.PortType_PORT_TYPE_ETH,
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.PortInfoSpec{
					NumLanes: 4,
					KeyOrHandle: &halproto.PortKeyHandle{
						KeyOrHandle: &halproto.PortKeyHandle_PortId{
							PortId: 5,
						},
					},
					BreakoutModes: []halproto.PortBreakoutMode{
						halproto.PortBreakoutMode_PORT_BREAKOUT_MODE_4x25G,
						halproto.PortBreakoutMode_PORT_BREAKOUT_MODE_2x50G,
						halproto.PortBreakoutMode_PORT_BREAKOUT_MODE_4x10G,
					},
					PortType: halproto.PortType_PORT_TYPE_ETH,
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.PortInfoSpec{
					NumLanes: 1,
					KeyOrHandle: &halproto.PortKeyHandle{
						KeyOrHandle: &halproto.PortKeyHandle_PortId{
							PortId: 9,
						},
					},
					BreakoutModes: []halproto.PortBreakoutMode{
						halproto.PortBreakoutMode_PORT_BREAKOUT_MODE_NONE,
					},
					PortType: halproto.PortType_PORT_TYPE_MGMT,
				},
			},
		},
	}

	return mockLifs, mockPorts, nil
}

// convertIPs converts ip addresses in Octet, IPMask and Hyphen separated IP Range to HAL IP Address Objects
func (hd *Datapath) convertIPs(addresses []string) ([]*halproto.IPAddressObj, error) {
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
			halAddr, err := hd.convertIPRange(ipRange[0], ipRange[1])
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

func (hd *Datapath) convertPort(port string) (*halproto.L4PortRange, error) {
	// handle empty port case
	if port == "" {
		return &halproto.L4PortRange{}, nil
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
		halPort := &halproto.L4PortRange{
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

// convertIPRange converts a start IP and end IP to hal IPAddressObj
func (hd *Datapath) convertIPRange(startIP, endIP string) (*halproto.IPAddressObj, error) {
	begin := net.ParseIP(strings.TrimSpace(startIP))
	if len(begin) == 0 {
		log.Errorf("could not parse start IP {%v}", startIP)
		return nil, ErrIPParse
	}
	end := net.ParseIP(strings.TrimSpace(endIP))
	if len(end) == 0 {
		log.Errorf("could not parse end IP {%v}", endIP)
		return nil, ErrIPParse
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

func (hd *Datapath) convertIfAdminStatus(status string) (halproto.IfStatus, error) {
	switch status {
	case "UP":
		return halproto.IfStatus_IF_STATUS_UP, nil
	case "DOWN":
		return halproto.IfStatus_IF_STATUS_DOWN, nil
	default:
		log.Errorf("Invalid admin status type")
		return halproto.IfStatus_IF_STATUS_DOWN, fmt.Errorf("invalid admin status type. %v", status)
	}

}

func (hd *Datapath) convertPortSpeed(speed string) (halPortSpeed halproto.PortSpeed, autoNeg bool) {
	switch speed {
	case "SPEED_1G":
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_1G
	case "SPEED_10G":
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_10G
	case "SPEED_25G":
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_25G
	case "SPEED_40G":
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_40G
	case "SPEED_50G":
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_50G
	case "SPEED_100G":
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_100G
	case "SPEED_AUTONEG":
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_100G
		autoNeg = true
	default:
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_NONE
	}
	return
}

func (hd *Datapath) convertPortTypeFec(portType string) (halPortType halproto.PortType, halFecType halproto.PortFecType) {
	switch portType {
	case "TYPE_ETHERNET":
		halPortType = halproto.PortType_PORT_TYPE_ETH
		halFecType = halproto.PortFecType_PORT_FEC_TYPE_RS
	case "TYPE_MANAGEMENT":
		halPortType = halproto.PortType_PORT_TYPE_MGMT
		halFecType = halproto.PortFecType_PORT_FEC_TYPE_NONE
	default:
		halPortType = halproto.PortType_PORT_TYPE_NONE
	}
	return
}

func (hd *Datapath) parseProtocolPort(protoPort string) (protocol, port string, err error) {
	if components := strings.Split(strings.TrimSpace(protoPort), "/"); len(components) == 2 {
		protocol = components[0]
		port = components[1]
	} else {
		err = fmt.Errorf("failed to parse protocol/port information from the App. %v. Err: %v", protocol, err)
		return
	}
	return
}

func (hd *Datapath) getDefaultUUID() (string, error) {

	hwUUID, err := hd.getHwUUID()
	if err == nil {
		return hwUUID, nil
	}

	b := make([]byte, 8)
	_, err = rand.Read(b)
	if err != nil {
		return "", err
	}

	// This will ensure that we have unicast MAC
	b[0] = (b[0] | 2) & 0xfe
	fakeMAC := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1], b[2], b[3], b[4], b[5])

	return fakeMAC, nil

}

func (hd *Datapath) getHwUUID() (uuid string, err error) {

	out, err := exec.Command("/bin/bash", "-c", "cat "+naplesHwUUIDFile).Output()
	if err != nil {
		return "", err
	}

	var deviceJSON map[string]interface{}
	if err := json.Unmarshal([]byte(out), &deviceJSON); err != nil {
		return "", errors.Errorf("Error reading %s file", naplesHwUUIDFile)
	}

	if val, ok := deviceJSON["mac-address"]; ok {
		return val.(string), nil
	}

	return "", errors.Errorf("Mac address not present in  %s file", naplesHwUUIDFile)
}
