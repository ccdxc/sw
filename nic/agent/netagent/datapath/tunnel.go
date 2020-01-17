package datapath

import (
	"context"
	"fmt"
	"net"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateTunnel creates a tunnel
func (hd *Datapath) CreateTunnel(tun *netproto.Tunnel, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()

	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	ifInfo, err := convertIfInfo(&tun.Spec, vrfKey)
	if err != nil {
		log.Errorf("invalid tunnel interface spec. Tunnel: {%v} Err: %v", tun.Spec, err)
		return err
	}

	status, err := hd.convertIfAdminStatus(tun.Spec.AdminStatus)
	if err != nil {
		return err
	}
	tunReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: tun.Status.TunnelID,
					},
				},
				Type:        halproto.IfType_IF_TYPE_TUNNEL,
				AdminStatus: status,
				IfInfo:      ifInfo,
			},
		},
	}

	// create route object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), tunReqMsg)
		if err != nil {
			log.Errorf("Error creating tunnel interface. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), tunReqMsg)
		if err != nil {
			log.Errorf("Error creating tunnel interface. Err: %v", err)
			return err
		}
	}
	return nil

}

// UpdateTunnel updates a tunnel
func (hd *Datapath) UpdateTunnel(tun *netproto.Tunnel, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	ifInfo, err := convertIfInfo(&tun.Spec, vrfKey)
	if err != nil {
		log.Errorf("invalid tunnel interface spec. Tunnel: {%v} Err: %v", tun.Spec, err)
		return err
	}

	status, err := hd.convertIfAdminStatus(tun.Spec.AdminStatus)
	if err != nil {
		return err
	}
	tunUpdateReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: tun.Status.TunnelID,
					},
				},
				Type:        halproto.IfType_IF_TYPE_TUNNEL,
				AdminStatus: status,
				IfInfo:      ifInfo,
			},
		},
	}

	// create route object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceUpdate(context.Background(), tunUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating tunnel interface. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceUpdate(context.Background(), tunUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating tunnel interface. Err: %v", err)
			return err
		}
	}
	return nil
}

// DeleteTunnel deletes a tunnel
func (hd *Datapath) DeleteTunnel(tun *netproto.Tunnel, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	tunDelReqMsg := &halproto.InterfaceDeleteRequestMsg{
		Request: []*halproto.InterfaceDeleteRequest{
			{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: tun.Status.TunnelID,
					},
				},
			},
		},
	}

	// create route object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceDelete(context.Background(), tunDelReqMsg)
		if err != nil {
			log.Errorf("Error deleting tunnel interface. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())

			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceDelete(context.Background(), tunDelReqMsg)
		if err != nil {
			log.Errorf("Error deleting tunnel interface. Err: %v", err)
			return err
		}
	}
	return nil
}

// convertIFInfo converts tunnel spec to halproto vxlan info or gre info
func convertIfInfo(tunSpec *netproto.TunnelSpec, vrfKey *halproto.VrfKeyHandle) (*halproto.InterfaceSpec_IfTunnelInfo, error) {
	localIP := net.ParseIP(tunSpec.Src)
	if len(localIP) == 0 {
		return nil, fmt.Errorf("could not parse source IP from %v", tunSpec.Src)
	}

	srcIP := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(localIP),
		},
	}

	remoteIP := net.ParseIP(tunSpec.Dst)
	if len(remoteIP) == 0 {
		return nil, fmt.Errorf("could not parse destination IP from %v", tunSpec.Dst)
	}

	dstIP := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(remoteIP),
		},
	}

	switch tunSpec.Type {
	case "VXLAN":
		ifInfo := halproto.InterfaceSpec_IfTunnelInfo{
			IfTunnelInfo: &halproto.IfTunnelInfo{
				EncapType: halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_VXLAN,
				EncapInfo: &halproto.IfTunnelInfo_VxlanInfo{
					VxlanInfo: &halproto.IfTunnelVxlanInfo{
						LocalTep:  srcIP,
						RemoteTep: dstIP,
					},
				},
				VrfKeyHandle: vrfKey,
			},
		}
		return &ifInfo, nil
	case "GRE":
		ifInfo := halproto.InterfaceSpec_IfTunnelInfo{
			IfTunnelInfo: &halproto.IfTunnelInfo{
				EncapType: halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_GRE,
				EncapInfo: &halproto.IfTunnelInfo_GreInfo{
					GreInfo: &halproto.IfTunnelGREInfo{
						Source:      srcIP,
						Destination: dstIP,
						Mtu:         9192,
						Ttl:         64,
					},
				},
				VrfKeyHandle: vrfKey,
			},
		}
		return &ifInfo, nil
	default:
		log.Errorf("invalid tunnel interface type. %v", tunSpec.Type)
		return nil, fmt.Errorf("invalid tunnel interface type. %v. Must be either VXLAN or GRE", tunSpec.Type)
	}
}
