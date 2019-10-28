// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"encoding/binary"
	"fmt"
	"net"
	"regexp"
	"strconv"

	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

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

// CreateLocalEndpoint creates a local endpoint in datapath
func (dp *DelphiDatapath) CreateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup, lifID, enicID uint64, vrf *netproto.Vrf) (*types.IntfInfo, error) {
	var halIPAddresses []*halproto.IPAddress
	var lifHandle *halproto.LifKeyHandle

	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	for _, address := range ep.Spec.IPv4Addresses {
		ipaddr, _, err := net.ParseCIDR(address)
		if err != nil {
			return nil, fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
		}
		// convert v4 address
		v4Addr := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ipaddr),
			},
		}

		halIPAddresses = append(halIPAddresses, v4Addr)
	}

	vrfKey := halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle
	for _, sg := range sgs {
		sgKey := halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		}
		sgHandles = append(sgHandles, &sgKey)
	}

	l2Key := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nt.Status.NetworkID,
		},
	}

	ifKey := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
			InterfaceId: enicID,
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKey,
		UsegVlan:           ep.Spec.UsegVlan,
		IpAddress:          halIPAddresses,
		SgKeyHandle:        sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	if lifID != 0 {
		lifHandle = &halproto.LifKeyHandle{
			KeyOrHandle: &halproto.LifKeyHandle_LifId{
				LifId: lifID,
			},
		}
	}

	// build enic message
	enicSpec := &halproto.InterfaceSpec{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: enicID,
			},
		},
		Type: halproto.IfType_IF_TYPE_ENIC,
		// associate the lif id
		IfInfo: &halproto.InterfaceSpec_IfEnicInfo{
			IfEnicInfo: &halproto.IfEnicInfo{
				EnicType:       halproto.IfEnicType_IF_ENIC_TYPE_USEG,
				LifKeyOrHandle: lifHandle,
				EnicTypeInfo: &halproto.IfEnicInfo_EnicInfo{
					EnicInfo: &halproto.EnicInfo{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
						EncapVlanId:        ep.Spec.UsegVlan,
					},
				},
			},
		},
	}

	// write it to delphi
	err := dp.delphiClient.SetObject(enicSpec)
	if err != nil {
		return nil, err
	}

	// build endpoint message
	epinfo := halproto.EndpointSpec{
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
		VrfKeyHandle:  &vrfKey,
	}

	err = dp.delphiClient.SetObject(&epinfo)
	if err != nil {
		return nil, err
	}

	return nil, nil
}

// UpdateLocalEndpoint updates a local endpoint in datapath
// FIXME: revisit this
func (dp *DelphiDatapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error {
	var halIPAddresses []*halproto.IPAddress

	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	for _, address := range ep.Spec.IPv4Addresses {
		ipaddr, _, err := net.ParseCIDR(address)
		if err != nil {
			return fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
		}
		// convert v4 address
		v4Addr := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ipaddr),
			},
		}

		halIPAddresses = append(halIPAddresses, v4Addr)
	}

	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle
	for _, sg := range sgs {
		sgKey := halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		}
		sgHandles = append(sgHandles, &sgKey)
	}

	l2Key := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nt.Status.NetworkID,
		},
	}

	epAttrs := halproto.EndpointAttributes{
		UsegVlan:    ep.Spec.UsegVlan,
		IpAddress:   halIPAddresses,
		SgKeyHandle: sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	epinfo := &halproto.EndpointSpec{
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
	}

	err := dp.delphiClient.SetObject(epinfo)
	if err != nil {
		return err
	}

	return nil
}

// DeleteLocalEndpoint deletes a local endpoint in datapath
func (dp *DelphiDatapath) DeleteLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, enicID uint64) error {
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	l2Key := &halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nw.Status.NetworkID,
		},
	}
	epKey := &halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	epinfo := &halproto.EndpointSpec{
		KeyOrHandle: epKey,
	}

	err := dp.delphiClient.DeleteObject(epinfo)
	if err != nil {
		return err
	}
	enicSpec := &halproto.InterfaceSpec{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: enicID,
			},
		},
	}

	err = dp.delphiClient.DeleteObject(enicSpec)
	if err != nil {
		return err
	}
	return nil
}

// CreateRemoteEndpoint creates a remote endpoint in datapath
func (dp *DelphiDatapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup, uplinkID uint64, vrf *netproto.Vrf) error {
	var halIPAddresses []*halproto.IPAddress

	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	for _, address := range ep.Spec.IPv4Addresses {
		ipaddr, _, err := net.ParseCIDR(address)
		if err != nil {
			return fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
		}
		// convert v4 address
		v4Addr := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ipaddr),
			},
		}

		halIPAddresses = append(halIPAddresses, v4Addr)
	}
	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle
	for _, sg := range sgs {
		sgKey := halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		}
		sgHandles = append(sgHandles, &sgKey)
	}

	l2Key := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nt.Status.NetworkID,
		},
	}

	ifKey := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
			InterfaceId: uplinkID,
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKey,
		UsegVlan:           ep.Spec.UsegVlan,
		IpAddress:          halIPAddresses,
		SgKeyHandle:        sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	// build endpoint message
	epinfo := &halproto.EndpointSpec{
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: vrf.Status.VrfID,
			},
		},
	}

	err := dp.delphiClient.SetObject(epinfo)
	if err != nil {
		return err
	}

	return nil
}

// UpdateRemoteEndpoint updates a remote endpoint in datapath
func (dp *DelphiDatapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error {
	var halIPAddresses []*halproto.IPAddress

	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	for _, address := range ep.Spec.IPv4Addresses {
		ipaddr, _, err := net.ParseCIDR(address)
		if err != nil {
			return fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
		}
		// convert v4 address
		v4Addr := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ipaddr),
			},
		}

		halIPAddresses = append(halIPAddresses, v4Addr)
	}
	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle
	for _, sg := range sgs {
		sgKey := halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		}
		sgHandles = append(sgHandles, &sgKey)
	}

	l2Key := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nt.Status.NetworkID,
		},
	}

	epAttrs := halproto.EndpointAttributes{
		UsegVlan:    ep.Spec.UsegVlan,
		IpAddress:   halIPAddresses,
		SgKeyHandle: sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	epinfo := &halproto.EndpointSpec{
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
	}

	err := dp.delphiClient.SetObject(epinfo)
	if err != nil {
		return err
	}

	return nil
}

// DeleteRemoteEndpoint deletes a remote endpoint in datapath
func (dp *DelphiDatapath) DeleteRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network) error {
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	l2Key := &halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nw.Status.NetworkID,
		},
	}
	epKey := &halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	epinfo := &halproto.EndpointSpec{
		KeyOrHandle: epKey,
	}

	err := dp.delphiClient.DeleteObject(epinfo)
	if err != nil {
		return err
	}

	return nil
}
