// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"encoding/binary"
	"fmt"
	"net"

	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
)

// CreateNetwork creates a network
func (dp *DelphiDatapath) CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error {
	var nwKey halproto.NetworkKeyHandle
	var nwKeyOrHandle []*halproto.NetworkKeyHandle
	var macAddr uint64
	var wireEncap halproto.EncapInfo
	var bcastPolicy halproto.BroadcastFwdPolicy
	// construct vrf key that gets passed on to hal
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	if len(nw.Spec.IPv4Subnet) != 0 {
		ip, network, err := net.ParseCIDR(nw.Spec.IPv4Subnet)
		if err != nil {
			return fmt.Errorf("error parsing the subnet mask from {%v}. Err: %v", nw.Spec.IPv4Subnet, err)
		}
		prefixLen, _ := network.Mask.Size()

		gwIP := net.ParseIP(nw.Spec.IPv4Gateway)
		if len(gwIP) == 0 {
			return fmt.Errorf("could not parse IP from {%v}", nw.Spec.IPv4Gateway)
		}

		halGwIP := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(gwIP),
			},
		}

		nwKey = halproto.NetworkKeyHandle{
			KeyOrHandle: &halproto.NetworkKeyHandle_NwKey{
				NwKey: &halproto.NetworkKey{
					IpPrefix: &halproto.IPPrefix{
						Address: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: ipv4Touint32(ip),
							},
						},
						PrefixLen: uint32(prefixLen),
					},
					VrfKeyHandle: vrfKey,
				},
			},
		}
		nwKeyOrHandle = append(nwKeyOrHandle, &nwKey)

		if len(nw.Spec.RouterMAC) != 0 {
			mac, err := net.ParseMAC(nw.Spec.RouterMAC)
			if err != nil {
				return fmt.Errorf("could not parse router mac from %v", nw.Spec.RouterMAC)
			}
			b := make([]byte, 8)
			// oui-48 format
			if len(mac) == 6 {
				// fill 0 lsb
				copy(b[2:], mac)
			}
			macAddr = binary.BigEndian.Uint64(b)

		}

		halNw := &halproto.NetworkSpec{
			KeyOrHandle: &nwKey,
			GatewayIp:   halGwIP,
			Rmac:        macAddr,
		}

		err = dp.delphiClient.SetObject(halNw)
		if err != nil {
			return err
		}
	}
	// build the appropriate wire encap
	if nw.Spec.VxlanVNI != 0 {
		wireEncap.EncapType = halproto.EncapType_ENCAP_TYPE_VXLAN
		wireEncap.EncapValue = nw.Spec.VxlanVNI
		bcastPolicy = halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_DROP
	} else {
		wireEncap.EncapType = halproto.EncapType_ENCAP_TYPE_DOT1Q
		wireEncap.EncapValue = nw.Spec.VlanID
		bcastPolicy = halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD
	}

	// TODO Remove uplink pinning prior to FCS. This is needed temporarily to enable bring up.
	pinnedUplinkIdx := nw.Status.NetworkID % uint64(len(uplinks))

	// build l2 segment data
	seg := &halproto.L2SegmentSpec{
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		McastFwdPolicy:   halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_NONE,
		BcastFwdPolicy:   bcastPolicy,
		WireEncap:        &wireEncap,
		VrfKeyHandle:     vrfKey,
		NetworkKeyHandle: nwKeyOrHandle,
		PinnedUplinkIfKeyHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uplinks[pinnedUplinkIdx].Status.InterfaceID,
			},
		},
	}

	err := dp.delphiClient.SetObject(seg)
	if err != nil {
		return err
	}

	return nil
}

// UpdateNetwork updates a network in datapath
func (dp *DelphiDatapath) UpdateNetwork(nw *netproto.Network, ns *netproto.Namespace) error {
	// build l2 segment data
	seg := &halproto.L2SegmentSpec{
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		McastFwdPolicy: halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD,
		BcastFwdPolicy: halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
		WireEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		TunnelEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_VXLAN,
			EncapValue: nw.Spec.VlanID,
		},
	}

	err := dp.delphiClient.SetObject(seg)
	if err != nil {
		return err
	}

	return nil
}

// DeleteNetwork deletes a network from datapath
func (dp *DelphiDatapath) DeleteNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error {
	// build vrf key
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}
	// build the segment message
	seg := &halproto.L2SegmentSpec{
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		VrfKeyHandle: vrfKey,
	}

	err := dp.delphiClient.DeleteObject(seg)
	if err != nil {
		return err
	}

	// Check if we need to perform network deletes as well
	if len(nw.Spec.IPv4Subnet) != 0 {
		ip, network, err := net.ParseCIDR(nw.Spec.IPv4Subnet)
		if err != nil {
			return fmt.Errorf("error parsing the subnet mask from {%v}. Err: %v", nw.Spec.IPv4Subnet, err)
		}
		prefixLen, _ := network.Mask.Size()

		nwKey := &halproto.NetworkKeyHandle{
			KeyOrHandle: &halproto.NetworkKeyHandle_NwKey{
				NwKey: &halproto.NetworkKey{
					IpPrefix: &halproto.IPPrefix{
						Address: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: ipv4Touint32(ip),
							},
						},
						PrefixLen: uint32(prefixLen),
					},
					VrfKeyHandle: vrfKey,
				},
			},
		}
		nwDel := &halproto.NetworkSpec{
			KeyOrHandle: nwKey,
		}

		err = dp.delphiClient.DeleteObject(nwDel)
		if err != nil {
			return err
		}
	}

	return nil
}
