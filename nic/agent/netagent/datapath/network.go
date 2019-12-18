package datapath

import (
	"context"
	"fmt"

	"github.com/pensando/sw/nic/agent/netagent/datapath/constants"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateNetwork creates network in datapath if spec has IPv4Subnet
// creates the l2segment and adds the l2segment on all the uplinks in datapath.
func (hd *Datapath) CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var ifKeyHandles []*halproto.InterfaceKeyHandle
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

	// build InterfaceKey Handle with all the uplinks
	for _, uplink := range uplinks {
		ifKeyHandle := halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uplink.Status.InterfaceID,
			},
		}

		ifKeyHandles = append(ifKeyHandles, &ifKeyHandle)
	}

	if nw.Spec.VlanID == constants.UntaggedCollVLAN {
		ifKeyHandles = []*halproto.InterfaceKeyHandle{}
	}
	// build l2 segment data
	seg := halproto.L2SegmentSpec{
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
		IfKeyHandle:      ifKeyHandles,
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// create the tenant. Enforce HAL Status == OK for HAL datapath
	if hd.Kind == "hal" {
		resp, err := hd.Hal.L2SegClient.L2SegmentCreate(context.Background(), &segReq)
		if err != nil {
			log.Errorf("Error creating L2 Segment. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.L2SegClient.L2SegmentCreate(context.Background(), &segReq)
		if err != nil {
			log.Errorf("Error creating L2 Segment. Err: %v", err)
			return err
		}
	}

	return nil
}

// UpdateNetwork updates a network in datapath
func (hd *Datapath) UpdateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var ifKeyHandles []*halproto.InterfaceKeyHandle
	var nwKeyOrHandle []*halproto.NetworkKeyHandle
	var wireEncap halproto.EncapInfo
	var bcastPolicy halproto.BroadcastFwdPolicy
	// construct vrf key that gets passed on to hal
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// build the appropriate wire encap
	wireEncap.EncapType = halproto.EncapType_ENCAP_TYPE_DOT1Q
	wireEncap.EncapValue = nw.Spec.VlanID
	bcastPolicy = halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD

	// build InterfaceKey Handle with all the uplinks
	for _, uplink := range uplinks {
		ifKeyHandle := halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uplink.Status.InterfaceID,
			},
		}

		ifKeyHandles = append(ifKeyHandles, &ifKeyHandle)
	}

	if nw.Spec.VlanID == constants.UntaggedCollVLAN {
		ifKeyHandles = []*halproto.InterfaceKeyHandle{}
	}
	// build l2 segment data
	seg := halproto.L2SegmentSpec{
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
		IfKeyHandle:      ifKeyHandles,
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// create the tenant. Enforce HAL Status == OK for HAL datapath
	if hd.Kind == "hal" {
		resp, err := hd.Hal.L2SegClient.L2SegmentUpdate(context.Background(), &segReq)
		if err != nil {
			log.Errorf("Error updating L2 Segment. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.L2SegClient.L2SegmentUpdate(context.Background(), &segReq)
		if err != nil {
			log.Errorf("Error updating L2 Segment. Err: %v", err)
			return err
		}
	}

	return nil
}

// DeleteNetwork deletes a network from datapath.
// It will remove the l2seg from all the uplinks, delete the l2seg and if the spec has IPv4Subnet it will delete the
// network in the datapath.
func (hd *Datapath) DeleteNetwork(nw *netproto.Network, uplinks []*netproto.Interface, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	// build vrf key
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}
	// build the segment message
	seg := halproto.L2SegmentDeleteRequest{
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		VrfKeyHandle: vrfKey,
	}

	segDelReqMsg := halproto.L2SegmentDeleteRequestMsg{
		Request: []*halproto.L2SegmentDeleteRequest{&seg},
	}

	if hd.Kind == "hal" {
		// delete the l2 seg
		resp, err := hd.Hal.L2SegClient.L2SegmentDelete(context.Background(), &segDelReqMsg)
		if err != nil {
			log.Errorf("Error deleting l2 segment. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}

	} else {
		_, err := hd.Hal.L2SegClient.L2SegmentDelete(context.Background(), &segDelReqMsg)
		if err != nil {
			log.Errorf("Error deleting l2segment. Err: %v", err)
			return err
		}
	}

	return nil
}
