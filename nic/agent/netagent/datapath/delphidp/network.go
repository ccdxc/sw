// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

// CreateNetwork creates a network
func (dp *DelphiDatapath) CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, vrf *netproto.Vrf) error {
	var nwKeyOrHandle []*halproto.NetworkKeyHandle
	var wireEncap halproto.EncapInfo
	var bcastPolicy halproto.BroadcastFwdPolicy
	// construct vrf key that gets passed on to hal
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}
	wireEncap.EncapType = halproto.EncapType_ENCAP_TYPE_DOT1Q
	wireEncap.EncapValue = nw.Spec.VlanID
	bcastPolicy = halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD

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
func (dp *DelphiDatapath) UpdateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, vrf *netproto.Vrf) error {
	var ifKeyHandles []*halproto.InterfaceKeyHandle

	for _, uplink := range uplinks {
		ifKeyHandle := halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uplink.Status.InterfaceID,
			},
		}

		ifKeyHandles = append(ifKeyHandles, &ifKeyHandle)
	}

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
		IfKeyHandle: ifKeyHandles,
	}

	err := dp.delphiClient.SetObject(seg)
	if err != nil {
		return err
	}

	return nil
}

// DeleteNetwork deletes a network from datapath
func (dp *DelphiDatapath) DeleteNetwork(nw *netproto.Network, uplinks []*netproto.Interface, vrf *netproto.Vrf) error {
	// build vrf key
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
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

	return nil
}
