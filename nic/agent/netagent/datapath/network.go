package datapath

import (
	"context"
	"encoding/binary"
	"fmt"
	"net"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateNetwork creates a l2 segment in datapath if vlan id is specified and a network if IPSubnet is specified.
// ToDo Investigate if HAL needs network updates and deletes.
func (hd *Datapath) CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error {
	var nwKey halproto.NetworkKeyHandle
	var macAddr uint64
	var wireEncap halproto.EncapInfo
	// construct vrf key that gets passed on to hal
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

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

	if len(nw.Spec.IPv4Subnet) != 0 {
		ip, network, err := net.ParseCIDR(nw.Spec.IPv4Subnet)
		if err != nil {
			return fmt.Errorf("error parsing the subnet mask from {%v}. Err: %v", nw.Spec.IPv4Subnet, err)
		}
		prefixLen, _ := network.Mask.Size()

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

		halNw := halproto.NetworkSpec{
			KeyOrHandle: &nwKey,
			GatewayIp:   halGwIP,
			Rmac:        macAddr,
		}

		halNwReq := halproto.NetworkRequestMsg{
			Request: []*halproto.NetworkSpec{&halNw},
		}
		// create the tenant. Enforce HAL Status == OK for HAL datapath
		if hd.Kind == "hal" {
			resp, err := hd.Hal.Netclient.NetworkCreate(context.Background(), &halNwReq)
			if err != nil {
				log.Errorf("Error creating network. Err: %v", err)
				return err
			}
			if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)
				return ErrHALNotOK
			}
		} else {
			_, err := hd.Hal.Netclient.NetworkCreate(context.Background(), &halNwReq)
			if err != nil {
				log.Errorf("Error creating tenant. Err: %v", err)
				return err
			}
		}
	}
	// build the appropriate wire encap
	if nw.Spec.VxlanVNI != 0 {
		wireEncap.EncapType = halproto.EncapType_ENCAP_TYPE_VXLAN
		wireEncap.EncapValue = nw.Spec.VxlanVNI
	} else {
		wireEncap.EncapType = halproto.EncapType_ENCAP_TYPE_DOT1Q
		wireEncap.EncapValue = nw.Spec.VlanID
	}

	// build l2 segment data
	seg := halproto.L2SegmentSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		McastFwdPolicy: halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD,
		BcastFwdPolicy: halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
		WireEncap:      &wireEncap,
		VrfKeyHandle:   vrfKey,
		NetworkKeyHandle: []*halproto.NetworkKeyHandle{
			&nwKey,
		},
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// create the tenant. Enforce HAL Status == OK for HAL datapath
	if hd.Kind == "hal" {
		ifL2SegReqMsg := halproto.InterfaceL2SegmentRequestMsg{
			Request: make([]*halproto.InterfaceL2SegmentSpec, 0),
		}

		resp, err := hd.Hal.L2SegClient.L2SegmentCreate(context.Background(), &segReq)
		if err != nil {
			log.Errorf("Error creating L2 Segment. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)
			return ErrHALNotOK
		}
		for _, uplink := range uplinks {
			ifL2SegReq := halproto.InterfaceL2SegmentSpec{
				L2SegmentKeyOrHandle: seg.KeyOrHandle,
				IfKeyHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: uplink.Status.InterfaceID,
					},
				},
			}
			ifL2SegReqMsg.Request = append(ifL2SegReqMsg.Request, &ifL2SegReq)
		}
		// Perform batched Add only if uplinks exist
		if len(uplinks) != 0 {
			l2SegAddResp, err := hd.Hal.Ifclient.AddL2SegmentOnUplink(context.Background(), &ifL2SegReqMsg)
			if err != nil {
				log.Errorf("Error adding l2 segments on uplinks. Err: %v", err)
				return err
			}
			for _, r := range l2SegAddResp.Response {
				if r.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
					log.Errorf("HAL returned non OK status. Err: %v", err)
					return ErrHALNotOK
				}
			}
		} else {
			log.Errorf("Could not find uplinks.")
		}

	} else {
		ifL2SegReqMsg := halproto.InterfaceL2SegmentRequestMsg{
			Request: make([]*halproto.InterfaceL2SegmentSpec, 0),
		}
		_, err := hd.Hal.L2SegClient.L2SegmentCreate(context.Background(), &segReq)
		if err != nil {
			log.Errorf("Error creating tenant. Err: %v", err)
			return err
		}
		for _, uplink := range uplinks {
			ifL2SegReq := halproto.InterfaceL2SegmentSpec{
				L2SegmentKeyOrHandle: seg.KeyOrHandle,
				IfKeyHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: uplink.Status.InterfaceID,
					},
				},
			}
			ifL2SegReqMsg.Request = append(ifL2SegReqMsg.Request, &ifL2SegReq)
		}

		_, err = hd.Hal.Ifclient.AddL2SegmentOnUplink(context.Background(), &ifL2SegReqMsg)
		if err != nil {
			log.Errorf("Error adding l2 segments on uplinks. Err: %v", err)
			return err
		}
	}

	return nil
}

// UpdateNetwork updates a network in datapath
func (hd *Datapath) UpdateNetwork(nw *netproto.Network, ns *netproto.Namespace) error {
	// build l2 segment data
	seg := halproto.L2SegmentSpec{
		Meta: &halproto.ObjectMeta{},
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
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// update the l2 segment
	_, err := hd.Hal.L2SegClient.L2SegmentUpdate(context.Background(), &segReq)
	if err != nil {
		log.Errorf("Error updating network. Err: %v", err)
		return err
	}

	return nil
}

// DeleteNetwork deletes a network from datapath
func (hd *Datapath) DeleteNetwork(nw *netproto.Network, ns *netproto.Namespace) error {
	// build the segment message
	seg := halproto.L2SegmentDeleteRequest{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
	}

	segDelReqMsg := halproto.L2SegmentDeleteRequestMsg{
		Request: []*halproto.L2SegmentDeleteRequest{&seg},
	}

	// delete the l2 segment
	_, err := hd.Hal.L2SegClient.L2SegmentDelete(context.Background(), &segDelReqMsg)
	if err != nil {
		log.Errorf("Error deleting network. Err: %v", err)
		return err
	}

	return nil
}
