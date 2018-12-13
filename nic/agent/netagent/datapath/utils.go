package datapath

import (
	"crypto/rand"
	"encoding/binary"
	"fmt"
	"net"
	"strings"

	"strconv"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
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

// convertMatchCriteria converts agent match object to hal match object
func (hd *Datapath) convertMatchCriteria(src, dst *netproto.MatchSelector) ([]*halproto.RuleMatch, error) {
	var srcIPRanges, dstIPRanges []*halproto.IPAddressObj
	var srcPortRanges, dstPortRanges []*halproto.L4PortRange
	var srcProtocols, dstProtocols []halproto.IPProtocol
	var ruleMatches []*halproto.RuleMatch
	var err error

	// build src match attributes
	if src != nil {
		srcIPRanges, err = hd.convertIPs(src.Addresses)
		if err != nil {
			log.Errorf("Could not convert match criteria from Src: {%v}. Err: %v", src, err)
			return nil, err
		}
		for _, s := range src.AppConfigs {
			if s.Port != "" && s.Protocol != "" {
				sPort, cerr := hd.convertPort(s.Port)
				if cerr != nil {
					log.Errorf("Could not convert port match criteria from: {%v} . Err: %v", s, err)
					return nil, cerr
				}
				srcPortRanges = append(srcPortRanges, sPort)
				srcProtocols = append(srcProtocols, hd.convertAppProtocol(s.Protocol))
			}
		}
	}

	// build dst match attributes
	if dst != nil {
		dstIPRanges, err = hd.convertIPs(dst.Addresses)
		if err != nil {
			log.Errorf("Could not convert match criteria from Dst: {%v}. Err: %v", dst, err)
			return nil, err
		}
		for _, d := range dst.AppConfigs {
			if d.Port != "" && d.Protocol != "" {
				dPort, err := hd.convertPort(d.Port)
				if err != nil {
					log.Errorf("Could not convert port match criteria from: {%v} . Err: %v", d, err)
					return nil, err
				}
				dstPortRanges = append(dstPortRanges, dPort)
				dstProtocols = append(dstProtocols, hd.convertAppProtocol(d.Protocol))
			}
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

func (hd *Datapath) convertAppProtocol(protocol string) halproto.IPProtocol {
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
		halPortSpeed = halproto.PortSpeed_PORT_SPEED_NONE
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

func (hd *Datapath) generateMockUUID() (string, error) {
	b := make([]byte, 8)
	_, err := rand.Read(b)
	if err != nil {
		return "", err
	}

	// This will ensure that we have unicast MAC
	b[0] = (b[0] | 2) & 0xfe
	fakeMAC := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1], b[2], b[3], b[4], b[5])

	return fakeMAC, nil

}
