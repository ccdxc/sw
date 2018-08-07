package datapath

import (
	"encoding/binary"
	"fmt"
	"net"
	"strings"

	"strconv"

	"crypto/md5"
	"math/big"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
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
	var srcIPRanges []*halproto.IPAddressObj
	var dstIPRanges []*halproto.IPAddressObj
	var srcPortRanges []*halproto.L4PortRange
	var dstPortRanges []*halproto.L4PortRange
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
			sPort, err := hd.convertPort(s.Port)
			if err != nil {
				log.Errorf("Could not convert port match criteria from: {%v} . Err: %v", s, err)
				return nil, err
			}
			srcPortRanges = append(srcPortRanges, sPort)
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
			dPort, err := hd.convertPort(d.Port)
			if err != nil {
				log.Errorf("Could not convert port match criteria from: {%v} . Err: %v", d, err)
				return nil, err
			}
			dstPortRanges = append(dstPortRanges, dPort)
		}
	}

	// flatten if needed
	switch {
	case len(srcPortRanges) == 0:
		for _, p := range dstPortRanges {
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
			ruleMatches = append(ruleMatches, &ruleMatch)
		}
		return ruleMatches, nil
	case len(dstPortRanges) == 0:
		for _, p := range srcPortRanges {
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
			ruleMatches = append(ruleMatches, &ruleMatch)
		}
		return ruleMatches, nil
	case len(srcPortRanges) > 0 && len(dstPortRanges) > 0:
		for _, sPort := range srcPortRanges {
			for _, dPort := range dstPortRanges {
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
func generateMockHwState() (*halproto.LifGetResponseMsg, *halproto.InterfaceGetResponseMsg, error) {
	var lifs halproto.LifGetResponseMsg
	var uplinks halproto.InterfaceGetResponseMsg

	mockLifs := []*halproto.LifGetResponse{
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
	}
	lifs.Response = append(lifs.Response, mockLifs...)

	mockUplinks := []*halproto.InterfaceGetResponse{
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.InterfaceSpec{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: 1,
					},
				},
				Type: halproto.IfType_IF_TYPE_UPLINK,
			},
		},
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.InterfaceSpec{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: 2,
					},
				},
				Type: halproto.IfType_IF_TYPE_UPLINK,
			},
		},
	}
	uplinks.Response = append(uplinks.Response, mockUplinks...)

	return &lifs, &uplinks, nil
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
			fmt.Println("BALERION: ", ip, network, err)
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

func (hd *Datapath) generateHash(data []byte) uint64 {
	hash := big.NewInt(0)
	h := md5.New()
	h.Write(data)
	hash.SetBytes(h.Sum(nil))
	return hash.Uint64()
}
