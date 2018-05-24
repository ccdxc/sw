package datapath

import (
	"encoding/binary"
	"fmt"
	"net"
	"strings"

	"strconv"

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
func (hd *Datapath) convertMatchCriteria(src, dst *netproto.MatchSelector) (*halproto.RuleMatch, error) {
	var srcIPRange []*halproto.IPAddressObj
	var dstIPRange []*halproto.IPAddressObj
	var ruleMatch halproto.RuleMatch
	var err error

	// Match source attributes
	if src != nil {
		// ToDo implement IP, Prefix Match address converters.
		srcIPRange, err = hd.convertIPRange(src.Address)
		if err != nil {
			log.Errorf("Could not convert match criteria from Src: {%v}. Err: %v", src, err)
			return nil, err
		}
		ruleMatch.SrcAddress = srcIPRange
	}

	// Match dest attributes
	if dst != nil {
		dstIPRange, err = hd.convertIPRange(dst.Address)
		if err != nil {
			log.Errorf("Could not convert match criteria from Dst: {%v}. Err: %v", dst, err)
			return nil, err
		}
		ruleMatch.DstAddress = dstIPRange
	}

	ruleMatch.AppMatch, err = hd.convertAppMatchCriteria(src, dst)
	if err != nil {
		log.Errorf("Could not covert port match criteria. Err: %v", err)
		return nil, err
	}

	return &ruleMatch, nil
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
						InterfaceId: 3,
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
						InterfaceId: 4,
					},
				},
				Type: halproto.IfType_IF_TYPE_UPLINK,
			},
		},
	}
	uplinks.Response = append(uplinks.Response, mockUplinks...)

	return &lifs, &uplinks, nil
}

func (hd *Datapath) convertAppMatchCriteria(src, dst *netproto.MatchSelector) ([]*halproto.RuleMatch_AppMatchInfo, error) {
	var srcPortRange, dstPortRange []*halproto.L4PortRange
	var err error
	// walk source app match selectors
	if src != nil {
		switch src.App {
		case "L4PORT":
			srcPortRange, err = hd.convertPort(src.AppConfig)
			if err != nil {
				log.Errorf("Could not convert source port match criteria from Src: {%v}. Err: %v", src.AppConfig, err)
				return nil, err
			}
		case "ESP": // ToDo implement ESP based match converters
			return nil, nil
		default:
			return nil, nil
		}
	}

	if dst != nil {
		switch dst.App {
		case "L4PORT":
			dstPortRange, err = hd.convertPort(dst.AppConfig)
			if err != nil {
				log.Errorf("Could not convert match criteria from Dst: {%v}. Err: %v", dst, err)
				return nil, err
			}
		case "ESP": // ToDo implement ESP based match converters
		default:
			return nil, nil
		}
	}

	appMatch := []*halproto.RuleMatch_AppMatchInfo{
		{
			App: &halproto.RuleMatch_AppMatchInfo_PortInfo{
				PortInfo: &halproto.RuleMatch_L4PortAppInfo{
					SrcPortRange: srcPortRange,
					DstPortRange: dstPortRange,
				},
			},
		},
	}
	return appMatch, nil

}

func (hd *Datapath) convertPort(ports string) ([]*halproto.L4PortRange, error) {

	if len(ports) == 0 {
		return nil, ErrPortParse
	}

	portSlice := strings.Split(ports, "-")
	switch len(portSlice) {
	// single port
	case 1:
		port, err := strconv.Atoi(ports)
		if err != nil || port < 0 || port > 65535 {
			log.Errorf("invalid port format. %v", ports)
			return nil, fmt.Errorf("invalid port format. %v", ports)
		}
		portRange := []*halproto.L4PortRange{
			{
				PortLow:  uint32(port),
				PortHigh: uint32(port),
			},
		}
		return portRange, nil

	// port range
	case 2:
		startPort, err := strconv.Atoi(portSlice[0])
		if err != nil || startPort < 0 || startPort > 65535 {
			log.Errorf("invalid port format. %v", ports)
			return nil, fmt.Errorf("invalid port format. %v", ports)
		}
		endPort, err := strconv.Atoi(portSlice[1])
		if err != nil || endPort < 0 || endPort > 65535 {
			log.Errorf("invalid port format. %v", ports)
			return nil, fmt.Errorf("invalid port format. %v", ports)
		}

		portRange := []*halproto.L4PortRange{
			{
				PortLow:  uint32(startPort),
				PortHigh: uint32(endPort),
			},
		}
		return portRange, nil

	default:
		log.Errorf("invalid port format. %v. It should either be hyphen separated or a single port", ports)
		return nil, fmt.Errorf("invalid port format. %v. It should either be hyphen separated or a single port", ports)
	}
}

// convertIPRange converts a hyphen separated IPRange to hal IPAddressObj
func (hd *Datapath) convertIPRange(sel string) ([]*halproto.IPAddressObj, error) {

	if len(sel) == 0 {
		return nil, ErrIPParse
	}

	ipRange := strings.Split(sel, "-")
	if len(ipRange) != 2 {
		log.Errorf("could not parse IP Range from selector. {%v}", sel)
		return nil, ErrIPParse
	}

	startIP := net.ParseIP(strings.TrimSpace(ipRange[0]))
	if len(startIP) == 0 {
		log.Errorf("could not parse IP from {%v}", ipRange[0])
		return nil, ErrIPParse
	}
	endIP := net.ParseIP(strings.TrimSpace(ipRange[1]))
	if len(endIP) == 0 {
		log.Errorf("could not parse IP from {%v}", endIP)
		return nil, ErrIPParse
	}

	lowIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(startIP),
		},
	}

	highIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(endIP),
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

	addressObj := []*halproto.IPAddressObj{
		{
			Formats: &halproto.IPAddressObj_Address{
				Address: &halproto.Address{
					Address: addrRange,
				},
			},
		},
	}

	return addressObj, nil
}
